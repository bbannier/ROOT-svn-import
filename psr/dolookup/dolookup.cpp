// vim: ts=2 sw=2 foldmethod=indent
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclGroup.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclBase.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclFriend.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/ExprCXX.h"
#include "clang/AST/Expr.h"
#include "clang/AST/PrettyPrinter.h"
#include "clang/AST/Stmt.h"
#include "clang/AST/Type.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/IdentifierTable.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/Specifiers.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/TokenKinds.h"
#include "clang/Basic/Version.h"
#include "clang/CodeGen/ModuleBuilder.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/TextDiagnosticBuffer.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Lex/HeaderSearch.h"
#include "clang/Lex/MacroInfo.h"
#include "clang/Lex/PPCallbacks.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Parse/ParseAST.h"
#define private public
#include "clang/Parse/Parser.h"
#undef public
#include "clang/Sema/DeclSpec.h"
#include "clang/Sema/Sema.h"
#include "clang/Sema/Lookup.h"
#include "clang/Sema/Ownership.h"
#include "clang/Sema/Overload.h"
#include "clang/Sema/Scope.h"
#include "clang/Sema/ParsedTemplate.h"
#include "llvm/ADT/OwningPtr.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/Assembly/PrintModulePass.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/GlobalVariable.h"
#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
#include "llvm/PassManager.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/DynamicLibrary.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetSelect.h"

#include <cstdio>
#include <iostream>
#include <sstream>
#include <stack>
#include <string>
#include <utility>
#include <vector>

namespace clang {
clang::ImplicitConversionSequence
TryCopyInitialization(clang::Sema& S, clang::Expr* From, clang::QualType ToType,
                      bool SuppressUserConversions,
                      bool InOverloadResolution,
                      bool AllowObjCWritebackConversion,
                      bool AllowExplicit = false);
}

// Function Table
void init(const char* llvmdir);
void cleanup();
clang::CompilerInstance* createCI(const char* llvmdir);
clang::CompilerInstance* getCI();

// Local Variables
static const char* llvm_install_dir = "/local2/russo/llvm";
//static const char* fake_argv[] = { "clang", "-x", "c++", "-fexceptions", "-D__CLING__", "-isysroot", "/local2/russo/root_debug/include", 0 };
static const char* fake_argv[] = { "clang", "-x", "c++", "-fexceptions", "-D__CLING__", 0 };
static const int fake_argc = (sizeof(fake_argv) / sizeof(const char*)) - 1;

static llvm::LLVMContext* m_llvm_context = 0; // We own, our types.
static clang::CompilerInstance* m_CI = 0; // We own, our compiler instance.
//static llvm::Module* m_prev_module = 0; // We do *not* own, m_engine owns it.

//
// Functions
//

void init(const char* llvmdir)
{
  //
  //  Initialize the llvm library.
  //
  llvm::InitializeAllTargets();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmPrinters();
  llvm::InitializeAllAsmParsers();
  //
  //  Create a llvm context to use.
  //
  //m_llvm_context = &llvm::getGlobalContext();
  m_llvm_context = new llvm::LLVMContext;
  m_CI = createCI(llvmdir);
  //m_prev_module = new llvm::Module("_Clang_first", *m_llvm_context);
}

void cleanup()
{
  //delete m_prev_module;
  //m_prev_module = 0; // Don't do this, the engine does it.
  //m_CI->takeLLVMContext(); // Don't take down the context with the CI.
  delete m_CI;
  m_CI = 0;
  delete m_llvm_context;
  m_llvm_context = 0;
  // Shutdown the llvm library.
  llvm::llvm_shutdown();
}

clang::CompilerInstance*
createCI(const char* llvmdir)
{
  //
  //  Create and setup a compiler instance.
  //
  clang::CompilerInstance* CI = new clang::CompilerInstance();
  llvm::IntrusiveRefCntPtr<clang::DiagnosticIDs> DiagID(
    new clang::DiagnosticIDs());
  //bool first_time = true;
  //CI->setLLVMContext(m_llvm_context);
  {
    //
    //  Buffer the error messages while we process
    //  the compiler options.
    //
    clang::TextDiagnosticBuffer* DiagsBuffer = new clang::TextDiagnosticBuffer();
    // Diags takes ownership of DiagsBuffer
    clang::DiagnosticsEngine Diags(DiagID, DiagsBuffer);
    clang::CompilerInvocation::CreateFromArgs(CI->getInvocation(),
        fake_argv + 1, fake_argv + fake_argc, Diags);
    if (
      CI->getHeaderSearchOpts().UseBuiltinIncludes &&
      CI->getHeaderSearchOpts().ResourceDir.empty()
    ) {
      if (llvmdir) {
        llvm::sys::Path P(llvmdir);
        P.appendComponent("lib");
        P.appendComponent("clang");
        P.appendComponent(CLANG_VERSION_STRING);
        CI->getHeaderSearchOpts().ResourceDir = P.str();
      }
    }
    CI->createDiagnostics(fake_argc - 1, const_cast<char**>(fake_argv + 1));
    if (!CI->hasDiagnostics()) {
      //CI->takeLLVMContext();
      delete CI;
      CI = 0;
      return 0;
    }
    // Output the buffered error messages now.
    DiagsBuffer->FlushDiagnostics(CI->getDiagnostics());
    if (CI->getDiagnostics().getClient()->getNumErrors()) {
      //CI->takeLLVMContext();
      delete CI;
      CI = 0;
      return 0;
    }
  }
  CI->setTarget(clang::TargetInfo::CreateTargetInfo(CI->getDiagnostics(),
                CI->getTargetOpts()));
  if (!CI->hasTarget()) {
    //CI->takeLLVMContext();
    delete CI;
    CI = 0;
    return 0;
  }
  CI->getTarget().setForcedLangOptions(CI->getLangOpts());
  if (!CI->hasFileManager()) {
    CI->createFileManager();
  }
  if (!CI->hasSourceManager()) {
    CI->createSourceManager(CI->getFileManager());
  }
  //else {
  //  CI->getSourceManager().clearIDTables();
  //}
  //CI->createPreprocessor(); // Note: This single line takes almost all the time!
  return CI;
}

clang::CompilerInstance*
getCI()
{
  if (!m_CI) {
    return 0;
  }
  //m_CI->getSourceManager().clearIDTables();
  return m_CI;
}

bool FuncArgTypesMatch(std::vector<clang::QualType>& GivenArgTypes,
  const clang::FunctionProtoType* FPT)
{
  clang::FunctionProtoType::arg_type_iterator ATI = FPT->arg_type_begin();
  clang::FunctionProtoType::arg_type_iterator E = FPT->arg_type_end();
  std::vector<clang::QualType>::iterator GAI = GivenArgTypes.begin();
  for (; ATI && (ATI != E); ++ATI, ++GAI) {
    if (!getCI()->getASTContext().hasSameType(*ATI, *GAI)) {
      return false;
    }
  }
  return true;
}

bool IsOverload(std::vector<clang::QualType>& GivenArgTypes,
  clang::FunctionDecl* FD, bool UseUsingDeclRules)
{
  clang::FunctionTemplateDecl* FTD = FD->getDescribedFunctionTemplate();
  clang::QualType FQT =
    getCI()->getASTContext().getCanonicalType(FD->getType());
  if (llvm::isa<clang::FunctionNoProtoType>(FQT.getTypePtr())) {
    // A K&R-style function (no prototype), is considered to match the args.
    return false;
  }
  const clang::FunctionProtoType* FPT =
    llvm::cast<clang::FunctionProtoType>(FQT);
  if (
    (GivenArgTypes.size() != FPT->getNumArgs()) ||
    /*(GivenArgsAreEllipsis != FPT->isVariadic()) ||*/
    !FuncArgTypesMatch(GivenArgTypes, FPT)) {
    return true;
  }
}

int main(int argc, char** argv)
{
  init(llvm_install_dir);
  clang::CompilerInstance* CI = 0;
  CI = getCI();
  if (!CI) {
    return 0;
  }
  clang::Parser* P = 0;
  // Begin per-file stuff.
  //
  //  Setup to parse the argument list.
  //
  if (!CI->hasPreprocessor()) {
    CI->createPreprocessor();
  }
  CI->getDiagnosticClient().BeginSourceFile(
    CI->getLangOpts(), &CI->getPreprocessor());
  CI->createASTContext();
  CI->setASTConsumer(new clang::ASTConsumer());
  CI->createPCHExternalASTSource("it3.h.pch", true, false, false, 0);
  {
    llvm::MemoryBuffer* SB = llvm::MemoryBuffer::getMemBufferCopy(argv[4],
                             "lookup.arg.types");
    const clang::FileEntry* File = CI->getFileManager().getVirtualFile(
                                     SB->getBufferIdentifier(), SB->getBufferSize(), 0);
    CI->getSourceManager().createMainFileID(File);
    CI->getSourceManager().overrideFileContents(File, SB);
  }
  if (!CI->hasSema()) {
    CI->createSema(clang::TU_Complete, 0);
  }
  P = new clang::Parser(CI->getPreprocessor(), CI->getSema());
  CI->getPreprocessor().EnterMainSourceFile();
  P->Initialize();
  CI->getSema().Initialize();
  clang::Scope* TUScope = CI->getSema().getCurScope();
  if (clang::ExternalASTSource*
      External = CI->getSema().getASTContext().getExternalSource()) {
    External->StartTranslationUnit(&CI->getASTConsumer());
  }
  clang::TranslationUnitDecl* TU =
    CI->getASTContext().getTranslationUnitDecl();
  if (!TU) {
    fprintf(
      stderr
      , "astread: No translation unit decl in passed "
      "PCH file!\n"
    );
    return 0;
  }
  //
  //  Parse the arguments now.
  //
  std::vector<clang::QualType> GivenArgTypes;
  std::vector<clang::Expr*> GivenArgs;
  {
    clang::PrintingPolicy Policy(CI->getASTContext().getPrintingPolicy());
    Policy.SuppressTagKeyword = true;
    Policy.SuppressUnwrittenScope = true;
    Policy.SuppressInitializers = true;
    Policy.AnonymousTagLocations = false;
    std::string proto;
    {
      bool first_time = true;
      while (P->getCurToken().isNot(clang::tok::eof)) {
        clang::TypeResult Res(P->ParseTypeName());
        if (Res.isUsable()) {
          clang::QualType QT(Res.get().get());
          GivenArgTypes.push_back(QT.getCanonicalType());
          {
            clang::Expr* val = new(CI->getSema().getASTContext())
            clang::OpaqueValueExpr(clang::SourceLocation(),
                                   QT.getCanonicalType(),
                                   clang::Expr::getValueKindForType(QT));
            GivenArgs.push_back(val);
          }
          if (first_time) {
            first_time = false;
          }
          else {
            proto += ',';
          }
          proto += QT.getCanonicalType().getAsString(Policy);
          fprintf(stderr, "%s\n", proto.c_str());
        }
        if (!P->getCurToken().is(clang::tok::comma)) {
          break;
        }
        P->ConsumeToken();
      }
    }
  }
  //
  //  Setup to parse the scope.
  //
  {
    llvm::MemoryBuffer* SB = llvm::MemoryBuffer::getMemBufferCopy(argv[1],
                             "lookup.name.file");
    clang::FileID FID = CI->getSourceManager().createFileIDForMemBuffer(SB);
    CI->getPreprocessor().EnterSourceFile(FID, 0, clang::SourceLocation());
    CI->getPreprocessor().Lex(const_cast<clang::Token&>(P->getCurToken()));
  }
  //
  //  Parse the class name and lookup the decl.
  clang::Decl* ClassDecl = 0;
  {
    //
    //  Do parse of nested-name-specifier.
    //
    clang::CXXScopeSpec SS;
    {
      clang::ParsedType ObjectType;
      bool EnteringContext = false;
      bool MayBePseudoDestructor = false;
      if (P->ParseOptionalCXXScopeSpecifier(SS, ObjectType,
        /*EnteringContext*/false, &MayBePseudoDestructor)) {
        // error path
        cleanup();
        return 0;
      }
    }
    clang::DeclarationName Name(&CI->getPreprocessor().
      getIdentifierTable().get(argv[2]));
    clang::LookupResult Result(CI->getSema(), Name, clang::SourceLocation(),
      clang::Sema::LookupTagName, clang::Sema::ForRedeclaration);
    Result.setHideTags(false);
    if (!CI->getSema().LookupParsedName(Result, TUScope, &SS)) {
      // error path
      cleanup();
      return 0;
    }
    switch (Result.getResultKind()) {
      case clang::LookupResult::NotFound:
        // error path
        cleanup();
        return 0;
        break;
      case clang::LookupResult::NotFoundInCurrentInstantiation:
        // error path
        cleanup();
        return 0;
        break;
      case clang::LookupResult::Found:
        {
          ClassDecl = Result.getFoundDecl();
          clang::NamedDecl* ND = Result.getFoundDecl();
          std::string buf;
          clang::PrintingPolicy P(ND->getASTContext().getPrintingPolicy());
          ND->getNameForDiagnostic(buf, P, true);
          fprintf(stderr, "%s\n", buf.c_str());
        }
        break;
      case clang::LookupResult::FoundOverloaded:
        // error path
        cleanup();
        return 0;
        break;
      case clang::LookupResult::FoundUnresolvedValue:
        // error path
        cleanup();
        return 0;
        break;
      case clang::LookupResult::Ambiguous:
        // error path
        cleanup();
        return 0;
        switch (Result.getAmbiguityKind()) {
          case clang::LookupResult::AmbiguousBaseSubobjectTypes:
            // error path
            cleanup();
            return 0;
            break;
          case clang::LookupResult::AmbiguousBaseSubobjects:
            // error path
            cleanup();
            return 0;
            break;
          case clang::LookupResult::AmbiguousReference:
            // error path
            cleanup();
            return 0;
            break;
          case clang::LookupResult::AmbiguousTagHiding:
            // error path
            cleanup();
            return 0;
            break;
        }
        break;
    }
  }
#if 0
  //
  //  Setup to parse the function name.
  //
  {
    llvm::MemoryBuffer* SB = llvm::MemoryBuffer::getMemBufferCopy(argv[3],
                             "lookup.name.file");
    clang::FileID FID = CI->getSourceManager().createFileIDForMemBuffer(SB);
    CI->getPreprocessor().EnterSourceFile(FID, 0, clang::SourceLocation());
    CI->getPreprocessor().Lex(const_cast<clang::Token&>(P->getCurToken()));
  }
#endif // 0
  //
  //  Parse the function name and do overload resolution.
  //
  {
    //clang::CXXScopeSpec SS;
    clang::DeclContext* DC = llvm::cast<clang::DeclContext>(ClassDecl);
    if (DC->isDependentContext()) {
      // error path
      cleanup();
      return 0;
    }
    //if (CI->getSema().RequireCompleteDeclContext(*SS, DC)) {
    //  // error path
    //  cleanup();
    //  return 0;
    //}
    clang::DeclarationName FuncName(&CI->getPreprocessor().
      getIdentifierTable().get(argv[3]));
    clang::LookupResult Result(CI->getSema(), FuncName,
      clang::SourceLocation(), clang::Sema::LookupMemberName,
      clang::Sema::ForRedeclaration);
    if (!CI->getSema().LookupQualifiedName(Result, DC)) {
      // error path
      cleanup();
      return 0;
    }
    switch (Result.getResultKind()) {
      case clang::LookupResult::NotFound:
        // error path
        cleanup();
        return 0;
        break;
      case clang::LookupResult::NotFoundInCurrentInstantiation:
        // error path
        cleanup();
        return 0;
        break;
      case clang::LookupResult::Found:
        {
          clang::NamedDecl* ND = Result.getFoundDecl();
          std::string buf;
          clang::PrintingPolicy P(ND->getASTContext().getPrintingPolicy());
          ND->getNameForDiagnostic(buf, P, true);
          fprintf(stderr, "Found: %s\n", buf.c_str());
        }
        break;
      case clang::LookupResult::FoundOverloaded:
        {
          fprintf(stderr, "Found overload set!\n");
        }
        break;
      case clang::LookupResult::FoundUnresolvedValue:
        // error path
        cleanup();
        return 0;
        break;
      case clang::LookupResult::Ambiguous:
        // error path
        cleanup();
        return 0;
        switch (Result.getAmbiguityKind()) {
          case clang::LookupResult::AmbiguousBaseSubobjectTypes:
            // error path
            cleanup();
            return 0;
            break;
          case clang::LookupResult::AmbiguousBaseSubobjects:
            // error path
            cleanup();
            return 0;
            break;
          case clang::LookupResult::AmbiguousReference:
            // error path
            cleanup();
            return 0;
            break;
          case clang::LookupResult::AmbiguousTagHiding:
            // error path
            cleanup();
            return 0;
            break;
        }
        break;
    }
    clang::NamedDecl* Match = 0;
    for (clang::LookupResult::iterator I = Result.begin(), E = Result.end();
      I != E; ++I) {
      clang::NamedDecl* ND = *I;
      bool IsUsingDecl = false;
      if (llvm::isa<clang::UsingShadowDecl>(ND)) {
        IsUsingDecl = true;
        ND = llvm::cast<clang::UsingShadowDecl>(ND)->getTargetDecl();
      }
      // If found declaration was introduced by a using declaration,
      // we'll need to use slightly different rules for matching.
      // Essentially, these rules are the normal rules, except that
      // function templates hide function templates with different
      // return types or template parameter lists.
      bool UseMemberUsingDeclRules = IsUsingDecl && DC->isRecord();
      if (clang::FunctionTemplateDecl* FTD =
        llvm::dyn_cast<clang::FunctionTemplateDecl>(ND)) {
        if (
          !IsOverload(GivenArgTypes, FTD->getTemplatedDecl(),
            UseMemberUsingDeclRules)
        ) {
          if (UseMemberUsingDeclRules && IsUsingDecl) {
            continue;
          }
          Match = *I;
          break;
        }
      } else if (clang::FunctionDecl* FD =
        llvm::dyn_cast<clang::FunctionDecl>(ND)) {
        if (!IsOverload(GivenArgTypes, FD, UseMemberUsingDeclRules)) {
          if (UseMemberUsingDeclRules && IsUsingDecl) {
            continue;
          }
          Match = *I;
          break;
        }
      } else if (llvm::isa<clang::UsingDecl>(ND)) {
        // We can overload with these, which can show up when doing
        // redeclaration checks for UsingDecls.
        //assert(Result.getLookupKind() == clang::LookupUsingDeclName);
      } else if (llvm::isa<clang::TagDecl>(ND)) {
        // We can always overload with tags by hiding them.
      } else if (llvm::isa<clang::UnresolvedUsingValueDecl>(ND)) {
      // Optimistically assume that an unresolved using decl will
      // overload; if it doesn't, we'll have to diagnose during
      // template instantiation.
      } else {
        // (C++ 13p1):
        //   Only function declarations can be overloaded; object and type
        //   declarations cannot be overloaded.
        //Match = *I;
        //return clang::Ovl_NonFunction;
      }
    }
    if (!Match) {
      // error path
      cleanup();
      return 0;
    }
    {
      std::string buf;
      clang::PrintingPolicy Policy(CI->getASTContext().getPrintingPolicy());
      Match->getNameForDiagnostic(buf, Policy, true);
      fprintf(stderr, "%s\n", buf.c_str());
      Match->dump();
    }
  }
  //
  //  All done.
  //
  cleanup();
  return 0;
}

