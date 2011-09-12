// @(#)root/cont:$Id$
// Author: Fons Rademakers   11/08/95

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// This class registers for all classes their name, id and dictionary   //
// function in a hash table. Classes are automatically added by the     //
// ctor of a special init class when a global of this init class is     //
// initialized when the program starts (see the ClassImp macro).        //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "RConfig.h"
#include <stdlib.h>
#include <string>
#include <map>
#include <typeinfo>
#include <vector>
#include "Riostream.h"

#include "TClassTable.h"
#include "TClass.h"
#include "TClassEdit.h"
#include "TROOT.h"
#include "TString.h"
#include "TError.h"
#include "TRegexp.h"

#include "TObjString.h"
#include "TMap.h"

TClassTable *gClassTable;

TClassRec  **TClassTable::fgTable;
TClassRec  **TClassTable::fgSortedTable;
int          TClassTable::fgSize;
int          TClassTable::fgTally;
Bool_t       TClassTable::fgSorted;
int          TClassTable::fgCursor;
TClassTable::IdMap_t *TClassTable::fgIdMap;

ClassImp(TClassTable)

#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include "llvm/LLVMContext.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetSelect.h"

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclBase.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclFriend.h"
#include "clang/AST/DeclGroup.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/PrettyPrinter.h"
#include "clang/AST/Type.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/Version.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/CompilerInvocation.h"
#include "clang/Frontend/TextDiagnosticBuffer.h"
#include "clang/Lex/Preprocessor.h"

static const char* llvm_install_dir = "/local2/russo/llvm";
static const char* fake_argv[] = { "clang", "-x", "c++", "-fexceptions", "-D__CLING__", 0 };
static const int fake_argc = (sizeof(fake_argv) / sizeof(const char*)) - 1;
static llvm::LLVMContext* m_llvm_context = 0; // We own, our types.
static clang::CompilerInstance* m_CI = 0; // We own, our compiler instance.


static clang::CompilerInstance* createCI();
static clang::CompilerInstance* getCI();
static void visit_linkage_spec_decl(const clang::LinkageSpecDecl* D, int level);
static void visit_namespace_decl(const clang::NamespaceDecl* D, int level);
static void visit_enum_decl(const clang::EnumDecl* D, int level);
static void visit_record_decl(const clang::RecordDecl* D, int level);
static void visit_cxxrecord_decl(const clang::CXXRecordDecl* D, int level);
static void visit_class_template_specialization_decl(const clang::ClassTemplateSpecializationDecl* D, int level);
static void visit_class_template_partial_specialization_decl(const clang::ClassTemplatePartialSpecializationDecl* D, int level);
static void visit_typedef_decl(const clang::TypedefDecl* D, int level);
static void visit_var_decl(const clang::VarDecl* D, int level);
static void visit_translation_unit_decl(const clang::TranslationUnitDecl* D, int level);
static void visit_decl(const clang::Decl* D, int level);
static void visit_decl_context(const clang::DeclContext* DC, int level);
static void init_class_map();
static void shutdown_class_map();
static std::string get_fully_qualified_name(const clang::NamedDecl* D);

clang::ASTContext* tcling_Dict::GetASTContext(clang::ASTContext* ctx /*=0*/)
{
   static clang::ASTContext* Context = 0;
   if (ctx) {
      Context = ctx;
   }
   return ctx;
}

const clang::TranslationUnitDecl* tcling_Dict::GetTranslationUnitDecl(const clang::TranslationUnitDecl* tu /*=0*/)
{
   static const clang::TranslationUnitDecl* tu_decl = 0;
   if (tu) {
      tu_decl = tu;
   }
   return tu_decl;
}

std::multimap<const std::string, const clang::Decl*>* tcling_Dict::ClassNameToDecl()
{
   static std::multimap<const std::string, const clang::Decl*>* classNameToDecl =
      new std::multimap<const std::string, const clang::Decl*>;
   return classNameToDecl;
}

std::map<const clang::Decl*, int>* tcling_Dict::ClassDeclToIdx()
{
   static std::map<const clang::Decl*, int>* classDeclToIdx =
      new std::map<const clang::Decl*, int>;
   return classDeclToIdx;
}

std::vector<const clang::Decl*>* tcling_Dict::Classes()
{
   static std::vector<const clang::Decl*>* classes =
      new std::vector<const clang::Decl*>;
   return classes;
}

std::vector<const clang::Decl*>* tcling_Dict::GlobalVars()
{
   static std::vector<const clang::Decl*>* globalVars =
      new std::vector<const clang::Decl*>;
   return globalVars;
}

std::vector<const clang::Decl*>* tcling_Dict::GlobalFunctions()
{
   static std::vector<const clang::Decl*>* globalFunctions =
      new std::vector<const clang::Decl*>;
   return globalFunctions;
}

std::vector<const clang::Decl*>* tcling_Dict::Typedefs()
{
   static std::vector<const clang::Decl*>* typedefs =
      new std::vector<const clang::Decl*>;
   return typedefs;
}

static std::string get_fully_qualified_name(const clang::NamedDecl* D) {
   clang::PrintingPolicy P(getCI()->getASTContext().PrintingPolicy);
  const clang::DeclContext* Ctx = D->getDeclContext();
  typedef llvm::SmallVector<const clang::DeclContext*, 8> ContextsTy;
  ContextsTy Contexts;
  while (Ctx && llvm::isa<clang::NamedDecl>(Ctx)) {
    Contexts.push_back(Ctx);
    Ctx = Ctx->getParent();
  };
  std::string QualName;
  llvm::raw_string_ostream OS(QualName);
  for (
    ContextsTy::reverse_iterator I = Contexts.rbegin(), E = Contexts.rend();
    I != E;
    ++I
  ) {
    if (const clang::ClassTemplateSpecializationDecl* Spec =
        llvm::dyn_cast<clang::ClassTemplateSpecializationDecl>(*I)
    ) {
      const clang::TemplateArgumentList& TemplateArgs = Spec->getTemplateArgs();
      std::string TemplateArgsStr =
        clang::TemplateSpecializationType::PrintTemplateArgumentList(
          TemplateArgs.data(), TemplateArgs.size(), P);
      OS << Spec->getName() << TemplateArgsStr;
    } else if (const clang::NamespaceDecl* ND =
               llvm::dyn_cast<clang::NamespaceDecl>(*I)
    ) {
      if (ND->isAnonymousNamespace()) {
        OS << "<anonymous namespace>";
      }
      else {
        OS << ND;
      }
    } else if (const clang::RecordDecl* RD =
               llvm::dyn_cast<clang::RecordDecl>(*I)
    ) {
      if (!RD->getIdentifier()) {
        OS << "<anonymous " << RD->getKindName() << '>';
      }
      else {
        OS << RD;
      }
    }
    else {
      OS << llvm::cast<clang::NamedDecl>(*I);
    }
    OS << "::";
  }
  if (D->getDeclName()) {
    OS << D;
  }
  else {
    OS << "<anonymous>";
  }
  return OS.str();
}

static clang::CompilerInstance* createCI()
{
   //
   //  Create and setup a compiler instance.
   //
   clang::CompilerInstance* CI = new clang::CompilerInstance();
   llvm::IntrusiveRefCntPtr<clang::DiagnosticIDs> DiagID(new clang::DiagnosticIDs());
   {
      //
      //  Buffer the error messages while we process
      //  the compiler options.
      //
      clang::TextDiagnosticBuffer* DiagsBuffer = new clang::TextDiagnosticBuffer();
      // Diags takes ownership of DiagsBuffer
      clang::Diagnostic Diags(DiagID, DiagsBuffer);
      clang::CompilerInvocation::CreateFromArgs(CI->getInvocation(),
            fake_argv + 1, fake_argv + fake_argc, Diags);
      if (
         CI->getHeaderSearchOpts().UseBuiltinIncludes &&
         CI->getHeaderSearchOpts().ResourceDir.empty()
      ) {
         llvm::sys::Path P(llvm_install_dir);
         P.appendComponent("lib");
         P.appendComponent("clang");
         P.appendComponent(CLANG_VERSION_STRING);
         CI->getHeaderSearchOpts().ResourceDir = P.str();
      }
      CI->createDiagnostics(fake_argc - 1, const_cast<char**>(fake_argv + 1));
      if (!CI->hasDiagnostics()) {
         delete CI;
         CI = 0;
         return 0;
      }
      // Output the buffered error messages now.
      DiagsBuffer->FlushDiagnostics(CI->getDiagnostics());
      if (CI->getDiagnostics().getClient()->getNumErrors()) {
         delete CI;
         CI = 0;
         return 0;
      }
   }
   CI->setTarget(clang::TargetInfo::CreateTargetInfo(CI->getDiagnostics(),
                 CI->getTargetOpts()));
   if (!CI->hasTarget()) {
      delete CI;
      CI = 0;
      return 0;
   }
   CI->getTarget().setForcedLangOptions(CI->getLangOpts());
   CI->createFileManager();
   CI->createSourceManager(CI->getFileManager());
   return CI;
}

static clang::CompilerInstance* getCI()
{
   if (!m_CI) {
      return 0;
   }
   m_CI->createDiagnostics(fake_argc - 1, const_cast<char**>(fake_argv + 1));
   if (!m_CI->hasDiagnostics()) {
      delete m_CI;
      m_CI = 0;
      return 0;
   }
   return m_CI;
}

static void visit_linkage_spec_decl(const clang::LinkageSpecDecl* D, int level)
{
   if (D->getLanguage() == clang::LinkageSpecDecl::lang_c) {
   }
   else if (D->getLanguage() == clang::LinkageSpecDecl::lang_cxx) {
   }
   else {
   }
   if (D->hasBraces()) {
      visit_decl_context(llvm::dyn_cast<clang::DeclContext>(D), level + 1);
   }
   else {
      visit_decl(*D->decls_begin(), level);
   }
}

static void visit_namespace_decl(const clang::NamespaceDecl* D, int level)
{
   //printf("namespace %s {\n", D->getNameAsString().c_str());
   //printf("namespace %s\n", D->getNameAsString().c_str());
   //printf("namespace %s\n", get_fully_qualified_name(D).c_str());
   tcling_Dict::ClassNameToDecl()->insert(std::make_pair(get_fully_qualified_name(D), D));
   tcling_Dict::Classes()->push_back(D);
   tcling_Dict::ClassDeclToIdx()->insert(std::make_pair(D, tcling_Dict::Classes()->size() - 1));
   visit_decl_context(llvm::dyn_cast<clang::DeclContext>(D), level + 1);
}

static void visit_enum_decl(const clang::EnumDecl* D, int level)
{
   //clang::ASTContext& Context = getCI()->getASTContext();
   //clang::PrintingPolicy Policy(Context.PrintingPolicy);
#if 0 // c++2011
   if (D->isScoped()) {
      if (D->isScopedUsingClassTag()) {
         printf("class ");
      }
      else  {
         printf("struct ");
      }
   }
#endif // 0
   //printf("%s", D->getNameAsString().c_str());
   //printf("enum %s\n", D->getNameAsString().c_str());
   //printf("enum %s\n", get_fully_qualified_name(D).c_str());
   tcling_Dict::ClassNameToDecl()->insert(std::make_pair(get_fully_qualified_name(D), D));
   tcling_Dict::Classes()->push_back(D);
   tcling_Dict::ClassDeclToIdx()->insert(std::make_pair(D, tcling_Dict::Classes()->size() - 1));
#if 0 // c++2011
   if (D->isFixed()) {
      std::string Underlying;
      D->getIntegerType().getAsStringInternal(Underlying, Policy);
      printf(" : %s", Underlying.c_str());
   }
#endif // 0
   if (D->isDefinition()) {
      visit_decl_context(llvm::dyn_cast<clang::DeclContext>(D), level + 1);
   }
}

static void visit_record_decl(const clang::RecordDecl* D, int level)
{
   //printf("%s", D->getKindName());
   if (D->getIdentifier()) {
     //printf(" %s", D->getNameAsString().c_str());
     tcling_Dict::ClassNameToDecl()->insert(std::make_pair(get_fully_qualified_name(D), D));
     tcling_Dict::Classes()->push_back(D);
     tcling_Dict::ClassDeclToIdx()->insert(std::make_pair(D, tcling_Dict::Classes()->size() - 1));
   }
   //printf(" %s", get_fully_qualified_name(D).c_str());
   //printf("\n");
   if (D->isDefinition()) {
      visit_decl_context(llvm::dyn_cast<clang::DeclContext>(D), level + 1);
   }
}

static void visit_cxxrecord_decl(const clang::CXXRecordDecl* D, int level)
{
   //printf("%s", D->getKindName());
   if (D->getIdentifier()) {
     //printf(" %s", D->getNameAsString().c_str());
     if (D->isDefinition()) {
       std::string name = get_fully_qualified_name(D);
       //if (name == "TObject") {
         //fprintf(stderr, "inserting class: %s  Decl: 0x%016lx\n", get_fully_qualified_name(D).c_str(), (long) D);
         //std::multimap<const std::string, const clang::Decl*>::iterator iter = 
         //tcling_Dict::ClassNameToDecl()->insert(std::make_pair(get_fully_qualified_name(D), D));
         //fprintf(stderr, "inserted  class: %s  Decl: 0x%016lx\n", iter->first.c_str(), (long) iter->second);
         //tcling_Dict::Classes()->push_back(D);
         //tcling_Dict::ClassDeclToIdx()->insert(std::make_pair(D, tcling_Dict::Classes()->size() - 1));
       //}
       //else {
         tcling_Dict::ClassNameToDecl()->insert(std::make_pair(get_fully_qualified_name(D), D));
         tcling_Dict::Classes()->push_back(D);
         tcling_Dict::ClassDeclToIdx()->insert(std::make_pair(D, tcling_Dict::Classes()->size() - 1));
       //}
     }
   }
   //printf(" %s", get_fully_qualified_name(D).c_str());
   //printf("\n");
   if (D->isDefinition()) {
      if (D->getNumBases()) {
         for (clang::CXXRecordDecl::base_class_const_iterator Base = D->bases_begin(),
               BaseEnd = D->bases_end(); Base != BaseEnd; ++Base) {
            if (Base->isVirtual()) {
            }
            clang::AccessSpecifier AS = Base->getAccessSpecifierAsWritten();
            if (AS != clang::AS_none) {
            }
            //printf(" %s", Base->getType().getAsString().c_str());
#if 0 // c++2011
            if (Base->isPackExpansion()) {
               printf("...");
            }
#endif // 0
            //--
         }
      }
      visit_decl_context(llvm::dyn_cast<clang::DeclContext>(D), level + 1);
   }
}

static void visit_class_template_specialization_decl(const clang::ClassTemplateSpecializationDecl* D, int level)
{
   visit_cxxrecord_decl(D, level);
}

static void visit_class_template_partial_specialization_decl(const clang::ClassTemplatePartialSpecializationDecl* D, int level)
{
   visit_cxxrecord_decl(D, level);
}

static void visit_typedef_decl(const clang::TypedefDecl* D, int level)
{
   //printf("typedef %s ", D->getUnderlyingType().getAsString().c_str());
   //printf("%s", D->getNameAsString().c_str());
   //printf("%s", get_fully_qualified_name(D).c_str());
   //printf("\n");
   tcling_Dict::Typedefs()->push_back(D);
}

static void visit_function_decl(const clang::FunctionDecl* D, int level)
{
   if (D->isGlobal()) {
       tcling_Dict::GlobalFunctions()->push_back(D);
   }
}

static void visit_var_decl(const clang::VarDecl* D, int level)
{
   //clang::AccessSpecifier AS = D->getAccess();
   //clang::StorageClass SC = D->getStorageClass();
   //const clang::ParmVarDecl* Parm = clang::dyn_cast<clang::ParmVarDecl>(D);
   //clang::QualType QT = D->getType();
   //if (Parm) {
      //QT = Parm->getOriginalType();
   //}
   //{
      // Actually print var.
      //if (SC != clang::SC_None) {
         //printf("%s ", clang::VarDecl::getStorageClassSpecifierString(SC));
      //}
      //if (D->isThreadSpecified()) {
         //printf("__thread ");
      //}
      //printf("%s %s", QT.getAsString().c_str(), D->getNameAsString().c_str());
   //}
   if (D->hasGlobalStorage() && !D->isStaticDataMember()) {
      tcling_Dict::GlobalVars()->push_back(D);
   }
}

static void visit_translation_unit_decl(const clang::TranslationUnitDecl* D, int level)
{
  visit_decl_context(llvm::dyn_cast<clang::DeclContext>(D), level + 1);
}

static void visit_decl(const clang::Decl* D, int level)
{
   clang::ASTContext& Context = getCI()->getASTContext();
   clang::PrintingPolicy Policy(Context.PrintingPolicy);
   switch (D->getKind()) {
      case clang::Decl::AccessSpec: // Decl
      case clang::Decl::Block: // Decl
      case clang::Decl::FileScopeAsm: // Decl
      case clang::Decl::Friend: // Decl
      case clang::Decl::FriendTemplate: // Decl
      case clang::Decl::Label: // Named, Decl
      case clang::Decl::NamespaceAlias: // Named, Decl
      case clang::Decl::ClassTemplate: // RedeclarableTemplate, Template, Named, Decl
      case clang::Decl::FunctionTemplate: // RedeclarableTemplate, Template, Named, Decl
      case clang::Decl::TypeAliasTemplate: // RedeclarableTemplate, Template, Named, Decl
      case clang::Decl::TemplateTemplateParm: // Template, Named, Decl
      case clang::Decl::TemplateTypeParm: // Type, Named, Decl
      case clang::Decl::TypeAlias: // TypedefName, Type, Named, Decl
      case clang::Decl::UnresolvedUsingTypename: // Type, Named, Decl
      case clang::Decl::Using: // Named, Decl
      case clang::Decl::UsingDirective: // Named, Decl
      case clang::Decl::UsingShadow: // Named, Decl
      case clang::Decl::Field: // Declarator, Value, Named, Decl
      case clang::Decl::CXXMethod: // Function, Declarator, Value, Named, Decl
      case clang::Decl::CXXConstructor: // CXXMethod, Function, Declarator, Value, Named, Decl
      case clang::Decl::CXXConversion: // CXXMethod, Function, Declarator, Value, Named, Decl
      case clang::Decl::CXXDestructor: // CXXMethod, Function, Declarator, Value, Named, Decl
      case clang::Decl::NonTypeTemplateParm: // Declarator, Value, Named, Decl
      case clang::Decl::ImplicitParam: // Var, Declarator, Value, Named, Decl
      case clang::Decl::ParmVar: // Var, Declarator, Value, Named, Decl
      case clang::Decl::EnumConstant: // Value, Named, Decl
      case clang::Decl::IndirectField: // Value, Named, Decl
      case clang::Decl::UnresolvedUsingValue: // Value, Named, Decl
      case clang::Decl::StaticAssert: // Decl
      default: {
         }
         break;
      case clang::Decl::LinkageSpec: { // Decl
            const clang::LinkageSpecDecl* LSD = llvm::dyn_cast<clang::LinkageSpecDecl>(D);
            visit_linkage_spec_decl(LSD, level);
         }
         break;
      case clang::Decl::Namespace: { // Named, Decl
            const clang::NamespaceDecl* ND = llvm::dyn_cast<clang::NamespaceDecl>(D);
            visit_namespace_decl(ND, level);
         }
         break;
      case clang::Decl::Enum: { // Tag, Type, Named, Decl
            const clang::EnumDecl* ED = llvm::dyn_cast<clang::EnumDecl>(D);
            visit_enum_decl(ED, level);
         }
         break;
      case clang::Decl::Record: { // Tag, Type, Named, Decl
            const clang::RecordDecl* RD = llvm::dyn_cast<clang::RecordDecl>(D);
            visit_record_decl(RD, level);
         }
         break;
      case clang::Decl::CXXRecord: { // Record, Tag, Type, Named, Decl
            const clang::CXXRecordDecl* CXXRD = llvm::dyn_cast<clang::CXXRecordDecl>(D);
            visit_cxxrecord_decl(CXXRD, level);
         }
         break;
      case clang::Decl::ClassTemplateSpecialization: { // CXXRecord, Record, Tag, Type, Named, Decl
            const clang::ClassTemplateSpecializationDecl* CTSD = llvm::dyn_cast<clang::ClassTemplateSpecializationDecl>(D);
            visit_class_template_specialization_decl(CTSD, level);
         }
         break;
      case clang::Decl::ClassTemplatePartialSpecialization: { // ClassTemplateSpecialization, CXXRecord, Record, Tag, Type, Named, Decl
            const clang::ClassTemplatePartialSpecializationDecl* CTPSD = llvm::dyn_cast<clang::ClassTemplatePartialSpecializationDecl>(D);
            visit_class_template_partial_specialization_decl(CTPSD, level);
         }
         break;
      case clang::Decl::Typedef: { // TypedefName, Type, Named, Decl
            const clang::TypedefDecl* TDD = llvm::dyn_cast<clang::TypedefDecl>(D);
            visit_typedef_decl(TDD, level);
         }
         break;
      case clang::Decl::Function: { // Declarator, Value, Named, Decl
            const clang::FunctionDecl* FD = llvm::dyn_cast<clang::FunctionDecl>(D);
            visit_function_decl(FD, level);
         }
         break;
      case clang::Decl::Var: { // Declarator, Value, Named, Decl
            const clang::VarDecl* VD = llvm::dyn_cast<clang::VarDecl>(D);
            visit_var_decl(VD, level);
         }
         break;
      case clang::Decl::TranslationUnit: { // Decl
            const clang::TranslationUnitDecl* TUD = llvm::dyn_cast<clang::TranslationUnitDecl>(D);
            visit_translation_unit_decl(TUD, level);
         }
         break;
   }
}

static void visit_decl_context(const clang::DeclContext* DC, int level)
{
   clang::ASTContext& Context = getCI()->getASTContext();
   clang::PrintingPolicy Policy(Context.PrintingPolicy);
   for (
      clang::DeclContext::decl_iterator D_Iter = DC->decls_begin(),
      DEnd = DC->decls_end();
      D_Iter != DEnd;
      ++D_Iter
   ) {
      clang::Decl* D = *D_Iter;
      if (D->isImplicit()) {
         continue;
      }
      if (clang::NamedDecl* ND = llvm::dyn_cast<clang::NamedDecl>(D)) {
         if (clang::IdentifierInfo* II = ND->getIdentifier()) {
            if (
               II->isStr("__builtin_va_list") ||
               II->isStr("__va_list_tag") ||
               II->isStr("__int128_t") ||
               II->isStr("__uint128_t")
            ) {
               continue;
            }
         }
      }
      visit_decl(D, level);
   }
}

static void init_class_map()
{
   llvm::InitializeAllTargets();
   llvm::InitializeAllMCAsmInfos();
   llvm::InitializeAllMCCodeGenInfos();
   llvm::InitializeAllMCSubtargetInfos();
   llvm::InitializeAllAsmPrinters();
   llvm::InitializeAllAsmParsers();

   m_llvm_context = new llvm::LLVMContext;
   m_CI = createCI();
   clang::CompilerInstance* CI = 0;
   CI = getCI();
   if (!CI) {
      return;
   }
   CI->createPreprocessor();
   clang::Preprocessor& PP = CI->getPreprocessor();
   CI->getDiagnosticClient().BeginSourceFile(CI->getLangOpts(), &PP);
   CI->setASTContext(new clang::ASTContext(CI->getLangOpts(),
                                           PP.getSourceManager(), CI->getTarget(), PP.getIdentifierTable(),
                                           PP.getSelectorTable(), PP.getBuiltinInfo(), 0));
   CI->setASTConsumer(new clang::ASTConsumer());
   PP.getBuiltinInfo().InitializeBuiltins(PP.getIdentifierTable(),
                                          PP.getLangOptions());
   CI->createPCHExternalASTSource("/local2/russo/root_tcl/root.pch", true, false, 0);
   clang::TranslationUnitDecl* tu =
      CI->getASTContext().getTranslationUnitDecl();
   if (!tu) {
      fprintf(stderr, "init_class_map: No translation unit decl in root.pch!\n");
      return;
   }
   //tu->dump();
   //return;
   tcling_Dict::GetASTContext(&CI->getASTContext());
   tcling_Dict::GetTranslationUnitDecl(tu);
   visit_decl(tu, -1);
}

static void shutdown_class_map()
{
   delete m_CI;
   m_CI = 0;
   delete m_llvm_context;
   m_llvm_context = 0;
   llvm::llvm_shutdown();
}

//______________________________________________________________________________
namespace ROOT {

   class TMapTypeToClassRec {
#if defined R__USE_STD_MAP
     // This wrapper class allow to avoid putting #include <map> in the
     // TROOT.h header file.
   public:
#ifdef R__GLOBALSTL
      typedef map<string, TClassRec*>           IdMap_t;
#else
      typedef std::map<std::string, TClassRec*> IdMap_t;
#endif
      typedef IdMap_t::key_type                 key_type;
      typedef IdMap_t::const_iterator           const_iterator;
      typedef IdMap_t::size_type                size_type;
#ifdef R__WIN32
      // Window's std::map does NOT defined mapped_type
      typedef TClassRec*                        mapped_type;
#else
      typedef IdMap_t::mapped_type              mapped_type;
#endif

   private:
      IdMap_t fMap;

   public:
      void Add(const key_type &key, mapped_type &obj) {
         fMap[key] = obj;
      }

      mapped_type Find(const key_type &key) const {

         IdMap_t::const_iterator iter = fMap.find(key);
         mapped_type cl = 0;
         if (iter != fMap.end()) cl = iter->second;
         return cl;
      }

      void Remove(const key_type &key) { fMap.erase(key); }

      void Print() {
         Info("TMapTypeToClassRec::Print", "printing the typeinfo map in TClassTable");
         for (const_iterator iter = fMap.begin(); iter != fMap.end(); iter++) {
            printf("Key: %40s 0x%lx\n", iter->first.c_str(), iter->second);
         }
      }
#else
   private:
      TMap fMap;
   public:
#ifdef R__COMPLETE_MEM_TERMINATION
      ~TMapTypeToClassRec() {
         TIter next(&fMap);
         TObjString *key;
         while((key = (TObjString*)next())) {
            delete key;
         }         
      }
#endif

      void Add(const char *key, TClassRec *&obj) {
         TObjString *realkey = new TObjString(key);
         fMap.Add(realkey, (TObject*)obj);
      }

      TClassRec *Find(const char *key) const {
         const TPair *a = (const TPair *)fMap.FindObject(key);
         if (a) return (TClassRec*) a->Value();
         return 0;
      }

      void Remove(const char *key) {
         TObjString realkey(key);
         TObject *actual = fMap.Remove(&realkey);
         delete actual;
      }

      void Print() {
         Info("TMapTypeToClassRec::Print", "printing the typeinfo map in TClassTable");
         TIter next(&fMap);
         TObjString *key;
         while((key = (TObjString*)next())) {
            printf("Key: %s\n",key->String().Data());
            TClassRec *data = (TClassRec*)fMap.GetValue(key);
            if (data) {
               printf("  class: %s %d\n",data->fName,data->fId);
            } else {
               printf("  no class: \n");
            }
         }
      }
#endif
   };
}

//______________________________________________________________________________
TClassTable::TClassTable()
{
   // TClassTable is a singleton (i.e. only one can exist per application).

   if (gClassTable) return;

   fgSize  = 1009;  //this is thge result of (int)TMath::NextPrime(1000);
   fgTable = new TClassRec* [fgSize];
   fgIdMap = new IdMap_t;
   memset(fgTable, 0, fgSize*sizeof(TClassRec*));
   gClassTable = this;
}

//______________________________________________________________________________
TClassTable::~TClassTable()
{
   // TClassTable singleton is deleted in Terminate().

   // Try to avoid spurrious warning from memory leak checkers.
   if (gClassTable != this) return;

   for (Int_t i = 0; i < fgSize; i++) {
      TClassRec *r = fgTable[i];
      while (r) {
         delete [] r->fName;
         TClassRec *next = r->fNext;
         delete r;
         r = next;
      }
   }
   delete [] fgTable; fgTable = 0;
   delete [] fgSortedTable; fgSortedTable = 0;
   delete fgIdMap; fgIdMap = 0;
}

//______________________________________________________________________________
void TClassTable::Print(Option_t *option) const
{
   // Print the class table. Before printing the table is sorted
   // alphabetically. Only classes specified in option are listed.
   // The default is to list all classes.
   // Standard wilcarding notation supported.

   if (fgTally == 0 || !fgTable)
      return;

   SortTable();

   int n = 0, ninit = 0, nl = 0;

   int nch = strlen(option);
   TRegexp re(option, kTRUE);

   Printf("\nDefined classes");
   Printf("class                                 version  bits  initialized");
   Printf("================================================================");
   for (int i = 0; i < fgTally; i++) {
      if (!fgTable[i]) continue;
      TClassRec *r = fgSortedTable[i];
      if (!r) break;
      n++;
      TString s = r->fName;
      if (nch && strcmp(option,r->fName) && s.Index(re) == kNPOS) continue;
      nl++;
      if (TClass::GetClass(r->fName, kFALSE)) {
         ninit++;
         Printf("%-35s %6d %7d       Yes", r->fName, r->fId, r->fBits);
      } else
         Printf("%-35s %6d %7d       No",  r->fName, r->fId, r->fBits);
   }
   Printf("----------------------------------------------------------------");
   Printf("Listed Classes: %4d  Total classes: %4d   initialized: %4d",nl, n, ninit);
   Printf("================================================================\n");
}

//---- static members --------------------------------------------------------

//______________________________________________________________________________
char *TClassTable::At(int index)
{
    // Returns class at index from sorted class table. Don't use this iterator
    // while modifying the class table. The class table can be modified
    // when making calls like TClass::GetClass(), etc.
    // Returns 0 if index points beyond last class name.

   SortTable();
   if (index >= 0 && index < fgTally) {
      TClassRec *r = fgSortedTable[index];
      if (r) return r->fName;
   }
   return 0;
}

//______________________________________________________________________________
int   TClassTable::Classes() { return fgTally; }
//______________________________________________________________________________
void  TClassTable::Init() { fgCursor = 0; SortTable(); }

namespace ROOT { class TForNamespace {}; } // Dummy class to give a typeid to namespace (see also TGenericClassInfo)

//______________________________________________________________________________
void TClassTable::Add(const char *cname, Version_t id,  const type_info &info,
                      VoidFuncPtr_t dict, Int_t pragmabits)
{
   // Add a class to the class table (this is a static function).

   if (!gClassTable) {
      new TClassTable;
      init_class_map();
   }

   // Only register the name without the default STL template arguments ...
   TClassEdit::TSplitType splitname( cname, TClassEdit::kLong64 );
   std::string shortName;
   splitname.ShortType(shortName, TClassEdit::kDropStlDefault);

   // check if already in table, if so return
   TClassRec *r = FindElementImpl(shortName.c_str(), kTRUE);
   if (r->fName) {
      if ( strcmp(r->fInfo->name(),typeid(ROOT::TForNamespace).name())==0
           && strcmp(info.name(),typeid(ROOT::TForNamespace).name())==0 ) {
         // We have a namespace being reloaded.
         // This okay we just keep the old one.
         return;
      }
      if (splitname.IsSTLCont()==0) {
         // Warn only for class that are not STL containers.
         ::Warning("TClassTable::Add", "class %s already in TClassTable", cname);
      }
      return;
   }

   r->fName = StrDup(shortName.c_str());
   r->fId   = id;
   r->fBits = pragmabits;
   r->fDict = dict;
   r->fInfo = &info;

   fgIdMap->Add(info.name(),r);

   fgTally++;
   fgSorted = kFALSE;
}

//______________________________________________________________________________
void TClassTable::Remove(const char *cname)
{
   // Remove a class from the class table. This happens when a shared library
   // is unloaded (i.e. the dtor's of the global init objects are called).

   if (!gClassTable || !fgTable) return;

   int slot = 0;
   const char *p = cname;

   while (*p) slot = slot<<1 ^ *p++;
   if (slot < 0) slot = -slot;
   slot %= fgSize;

   TClassRec *r;
   TClassRec *prev = 0;
   for (r = fgTable[slot]; r; r = r->fNext) {
      if (!strcmp(r->fName, cname)) {
         if (prev)
            prev->fNext = r->fNext;
         else
            fgTable[slot] = r->fNext;
         fgIdMap->Remove(r->fInfo->name());
         delete [] r->fName;
         delete r;
         fgTally--;
         fgSorted = kFALSE;
         break;
      }
      prev = r;
   }
}

//______________________________________________________________________________
TClassRec *TClassTable::FindElementImpl(const char *cname, Bool_t insert)
{
   // Find a class by name in the class table (using hash of name). Returns
   // 0 if the class is not in the table. Unless arguments insert is true in
   // which case a new entry is created and returned.

   int slot = 0;
   const char *p = cname;

   while (*p) slot = slot<<1 ^ *p++;
   if (slot < 0) slot = -slot;
   slot %= fgSize;

   TClassRec *r;

   for (r = fgTable[slot]; r; r = r->fNext)
      if (strcmp(cname,r->fName)==0) return r;

   if (!insert) return 0;

   r = new TClassRec;
   r->fName = 0;
   r->fId   = 0;
   r->fDict = 0;
   r->fInfo = 0;
   r->fNext = fgTable[slot];
   fgTable[slot] = r;

   return r;
}

//______________________________________________________________________________
TClassRec *TClassTable::FindElement(const char *cname, Bool_t insert)
{
   // Find a class by name in the class table (using hash of name). Returns
   // 0 if the class is not in the table. Unless arguments insert is true in
   // which case a new entry is created and returned.

   if (!fgTable) return 0;

   // Only register the name without the default STL template arguments ...
   TClassEdit::TSplitType splitname( cname, TClassEdit::kLong64 );
   std::string shortName;
   splitname.ShortType(shortName, TClassEdit::kDropStlDefault);

   return FindElementImpl(shortName.c_str(), insert);
}

//______________________________________________________________________________
Version_t TClassTable::GetID(const char *cname)
{
   // Returns the ID of a class.

   TClassRec *r = FindElement(cname);
   if (r) return r->fId;
   return -1;
}

//______________________________________________________________________________
Int_t TClassTable::GetPragmaBits(const char *cname)
{
   // Returns the pragma bits as specified in the LinkDef.h file.

   TClassRec *r = FindElement(cname);
   if (r) return r->fBits;
   return 0;
}

//______________________________________________________________________________
VoidFuncPtr_t TClassTable::GetDict(const char *cname)
{
   // Given the class name returns the Dictionary() function of a class
   // (uses hash of name).

   if (gDebug > 9) {
      ::Info("GetDict", "searches for %s", cname);
      fgIdMap->Print();
   }

   TClassRec *r = FindElement(cname);
   if (r) return r->fDict;
   return 0;
}

//______________________________________________________________________________
VoidFuncPtr_t TClassTable::GetDict(const type_info& info)
{
   // Given the type_info returns the Dictionary() function of a class
   // (uses hash of type_info::name()).

   if (gDebug > 9) {
      ::Info("GetDict", "searches for %s at 0x%lx", info.name(), (Long_t)&info);
      fgIdMap->Print();
   }

   TClassRec *r = fgIdMap->Find(info.name());
   if (r) return r->fDict;
   return 0;
}


//______________________________________________________________________________
extern "C" {
   static int ClassComp(const void *a, const void *b)
   {
      // Function used for sorting classes alphabetically.

      return strcmp((*(TClassRec **)a)->fName, (*(TClassRec **)b)->fName);
   }
}

//______________________________________________________________________________
char *TClassTable::Next()
{
    // Returns next class from sorted class table. Don't use this iterator
    // while modifying the class table. The class table can be modified
    // when making calls like TClass::GetClass(), etc.

   if (fgCursor < fgTally) {
      TClassRec *r = fgSortedTable[fgCursor++];
      return r->fName;
   } else
      return 0;
}

//______________________________________________________________________________
void TClassTable::PrintTable()
{
   // Print the class table. Before printing the table is sorted
   // alphabetically.

   if (fgTally == 0 || !fgTable)
      return;

   SortTable();

   int n = 0, ninit = 0;

   Printf("\nDefined classes");
   Printf("class                                 version  bits  initialized");
   Printf("================================================================");
   for (int i = 0; i < fgTally; i++) {
      if (!fgTable[i]) continue;
      TClassRec *r = fgSortedTable[i];
      if (!r) break;
      n++;
      if (TClass::GetClass(r->fName, kFALSE)) {
         ninit++;
         Printf("%-35s %6d %7d       Yes", r->fName, r->fId, r->fBits);
      } else
         Printf("%-35s %6d %7d       No",  r->fName, r->fId, r->fBits);
   }
   Printf("----------------------------------------------------------------");
   Printf("Total classes: %4d   initialized: %4d", n, ninit);
   Printf("================================================================\n");
}

//______________________________________________________________________________
void TClassTable::SortTable()
{
   // Sort the class table by ascending class ID's.

   if (!fgSorted) {
      delete [] fgSortedTable;
      fgSortedTable = new TClassRec* [fgTally];

      int j = 0;
      for (int i = 0; i < fgSize; i++)
         for (TClassRec *r = fgTable[i]; r; r = r->fNext)
            fgSortedTable[j++] = r;

      ::qsort(fgSortedTable, fgTally, sizeof(TClassRec *), ::ClassComp);
      fgSorted = kTRUE;
   }
}

//______________________________________________________________________________
void TClassTable::Terminate()
{
   // Deletes the class table (this static class function calls the dtor).

   if (gClassTable) {
      for (int i = 0; i < fgSize; i++)
         for (TClassRec *r = fgTable[i]; r; ) {
            TClassRec *t = r;
            r = r->fNext;
            fgIdMap->Remove(r->fInfo->name());
            delete [] t->fName;
            delete t;
         }
      delete [] fgTable; fgTable = 0;
      delete [] fgSortedTable; fgSortedTable = 0;
      delete fgIdMap; fgIdMap = 0;
      fgSize = 0;
      SafeDelete(gClassTable);
   }
}

//______________________________________________________________________________
void ROOT::AddClass(const char *cname, Version_t id,
                    const type_info& info,
                    VoidFuncPtr_t dict,
                    Int_t pragmabits)
{
   // Global function called by the ctor of a class's init class
   // (see the ClassImp macro).

   TClassTable::Add(cname, id, info, dict, pragmabits);
}

//______________________________________________________________________________
void ROOT::ResetClassVersion(TClass *cl, const char *cname, Short_t newid)
{
   // Global function to update the version number.
   // This is called via the RootClassVersion macro.
   //
   // if cl!=0 and cname==-1, set the new class version if and only is
   // greater than the existing one and greater or equal to 2;
   // and also ignore the request if fVersionUsed is true.
   //
   // Note on class version number:
   //   If no class has been specified, TClass::GetVersion will return -1
   //   The Class Version 0 request the whole object to be transient
   //   The Class Version 1, unless specify via ClassDef indicates that the
   //      I/O should use the TClass checksum to distinguish the layout of the class

   if (cname && cname!=(void*)-1) {
      TClassRec *r = TClassTable::FindElement(cname,kFALSE);
      if (r) r->fId = newid;
   }
   if (cl) {
      if (cl->fVersionUsed) {
         // Problem, the reset is called after the first usage!
         if (cname!=(void*)-1)
            Error("ResetClassVersion","Version number of %s can not be changed after first usage!",
                  cl->GetName());
      } else {
         if (newid < 0) {
            Error("SetClassVersion","The class version (for %s) must be positive (value %d is ignored)",cl->GetName(),newid);
         }
         if (cname==(void*)-1) {
            if (cl->fClassVersion<newid && 2<=newid) {
               cl->SetClassVersion(newid);
            }
         } else {
            cl->SetClassVersion(newid);
         }
      }
   }
}


//______________________________________________________________________________
void ROOT::RemoveClass(const char *cname)
{
   // Global function called by the dtor of a class's init class
   // (see the ClassImp macro).

   // don't delete class information since it is needed by the I/O system
   // to write the StreamerInfo to file
   if (cname) {
      // Let's still remove this information to allow reloading later.
      // Anyway since the shared library has been unloaded, the dictionary
      // pointer is now invalid ....
      // We still keep the TClass object around because TFile needs to
      // get to the TStreamerInfo.
      if (gROOT && gROOT->GetListOfClasses()) {
         TObject *pcname;
         if ((pcname=gROOT->GetListOfClasses()->FindObject(cname))) {
            TClass *cl = dynamic_cast<TClass*>(pcname);
            if (cl) cl->SetUnloaded();
         }
      }
      TClassTable::Remove(cname);
   }
}

//______________________________________________________________________________
TNamed *ROOT::RegisterClassTemplate(const char *name, const char *file,
                                    Int_t line)
{
   // Global function to register the implementation file and line of
   // a class template (i.e. NOT a concrete class).

   static TList table;
   static Bool_t isInit = kFALSE;
   if (!isInit) {
      table.SetOwner(kTRUE);
      isInit = kTRUE;
   }

   TString classname(name);
   Ssiz_t loc = classname.Index("<");
   if (loc >= 1) classname.Remove(loc);
   if (file) {
      TNamed *obj = new TNamed((const char*)classname, file);
      obj->SetUniqueID(line);
      table.Add(obj);
      return obj;
   } else {
      return (TNamed*)table.FindObject(classname);
   }
}
