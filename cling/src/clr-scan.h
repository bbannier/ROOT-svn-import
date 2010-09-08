
// Author: Zdenek Culik   16/04/2010
// Modified by: Velislava Spasova

#ifndef __CLR_SCAN_H__
#define __CLR_SCAN_H__

#include "clang/AST/AST.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclGroup.h"
#include "clang/AST/DeclFriend.h"
#include "clang/AST/Type.h"
#include "clang/AST/RecursiveASTVisitor.h"

#include "llvm/Module.h"

#include "Reflex/Reflex.h"
#include "Reflex/Builder/ReflexBuilder.h"
#include "Reflex/Builder/ClassBuilder.h"
#include "Reflex/Builder/EnumBuilder.h"
#include "Reflex/Builder/VariableBuilder.h"
#include "Reflex/Builder/FunctionBuilder.h"

#include "TString.h"
#include <stack>

//using namespace clang;
/* -------------------------------------------------------------------------- */

void ClrStubFunction (void* result, void* obj, const std::vector<void*>& params, void* ctx);

struct TContext
{
   const llvm::Function * func;

   // Reflex::Member* member;
   std::string name; // only for debugging

   int index;
   TContext * next;

   TContext () :
      func (NULL),
      // member (NULL),
      index (0),
      next (NULL)
      { }
};

/* -------------------------------------------------------------------------- */

// Note form Velislava: We are inheriting here from the class RecursiveASTVisitor
// which traverses every node of the AST
class TScanner: public clang::RecursiveASTVisitor<TScanner>
{
private:
   clang::ASTContext* fCtx;
   TObject * fReporter;

public:
   static const char* fgClangDeclKey; // property key used for CLang declaration objects
   static const char* fgClangFuncKey; // property key for function (demangled) names

   static clang::Decl* GetDeclProp (const Reflex::PropertyList& prop_list);
   static std::string  GetFuncProp (const Reflex::PropertyList& prop_list);

   std::stack <Reflex::ClassBuilder*> fClassBuilders; //ADDED BY VELISLAVA - a stack of ClassBuider-s

private:
   static int fgAnonymousClassCounter;
   static int fgBadClassCounter;
   static int fgAnonymousEnumCounter;

   static std::map <clang::Decl*, std::string> fgAnonymousClassMap;
   static std::map <clang::Decl*, std::string> fgAnonymousEnumMap;

private:
   // only for debugging

   static const int fgDeclLast = clang::Decl::Var;
   bool fDeclTable [ fgDeclLast+1 ];

   static const int fgTypeLast = clang::Type::TemplateTypeParm;
   bool fTypeTable [ fgTypeLast+1 ];

   clang::Decl * fLastDecl;

#if 0
private:
   TContext* fFirstContext;
   TContext* fLastContext;
public:
   void DeleteContexts ();
   TContext* GetFirstContext () { return fFirstContext; }
#endif
private:
   TContext* AllocateContext ();

   //private:
public:
   void ShowInfo(const TString msg, const TString location = "");
   void ShowWarning(const TString msg, const TString location = "");
   void ShowError(const TString msg, const TString location = "");

   void ShowTemplateInfo(const TString msg, const TString location = "");
   void ShowReflexWarning(const TString msg, const TString location = "");

   TString GetSrcLocation(clang::SourceLocation L);
   TString GetLocation(clang::Decl* D);
   TString GetName(clang::Decl* D);

   void DeclInfo(clang::Decl* D);

   void UnknownDecl(clang::Decl* D, TString txt = "");
   void UnexpectedDecl(clang::Decl* D, TString txt = "");
   void UnsupportedDecl(clang::Decl* D, TString txt = "");
   void UnimportantDecl(clang::Decl* D, TString txt = "");
   void UnimplementedDecl(clang::Decl* D, TString txt = "");

   void UnknownType(clang::QualType qual_type);
   void UnsupportedType(clang::QualType qual_type);
   void UnimportantType(clang::QualType qual_type);
   void UnimplementedType(clang::QualType qual_type);
   void UnimplementedType (clang::Type* T);

   std::string GetClassName(clang::RecordDecl* D);
   std::string GetEnumName(clang::EnumDecl* D);
   Reflex::ClassBuilder* GetClassBuilder(clang::RecordDecl* D, TString template_params = "");

   Reflex::Type ExplorePointerType(clang::PointerType* T);
   Reflex::Type ExploreLValueReferenceType(clang::LValueReferenceType* T);
   Reflex::Type ExploreMemberPointerType(clang::MemberPointerType* T);
   Reflex::Type ExploreArrayType(clang::ArrayType* T);
   Reflex::Type ExploreIncompleteArrayType(clang::IncompleteArrayType* T);
   Reflex::Type ExploreDependentSizedArrayType(clang::DependentSizedArrayType* T);
   Reflex::Type ExploreFunctionType(clang::FunctionType* T);
   Reflex::Type ExploreEnumType(clang::EnumType* T);
   Reflex::Type ExploreRecordType(clang::RecordType* T);

   Reflex::Type ExploreElaboratedType(clang::ElaboratedType* T);
   Reflex::Type ExploreTypedefType(clang::TypedefType* T);
   Reflex::Type ExploreBuiltinType(clang::BuiltinType* T);
   Reflex::Type ExploreInjectedClassNameType(clang::InjectedClassNameType* T);
   Reflex::Type ExploreDependentNameType(clang::DependentNameType* T);

   Reflex::Type ExploreTemplateTypeParmType(clang::TemplateTypeParmType* T);
   Reflex::Type ExploreSubstTemplateTypeParmType(clang::SubstTemplateTypeParmType* T);
   Reflex::Type ExploreTemplateSpecializationType(clang::TemplateSpecializationType* T);

   Reflex::Type ExploreQualType(clang::QualType qual_type);

   TString ExprToStr(clang::Expr* expr);

   TString ConvTemplateName(clang::TemplateName& N);
   TString ConvTemplateParameterList(clang::TemplateParameterList* list);
   TString ConvTemplateParams(clang::TemplateDecl* D);
   TString ConvTemplateArguments(const clang::TemplateArgumentList& list);

   TString      FuncParameters(clang::FunctionDecl* D);
   std::string  FuncParameterList(clang::FunctionDecl* D);
   Reflex::Type FuncType(clang::FunctionDecl* D);
   unsigned int FuncModifiers(clang::FunctionDecl* D);

   unsigned int VisibilityModifiers(clang::AccessSpecifier access);
   unsigned int Visibility(clang::Decl* D);
   unsigned int VarModifiers(clang::VarDecl* D);

public:
   TScanner ();
   virtual ~ TScanner ();

public: // ADDED BY VELISLAVA 
   bool VisitVarDecl(clang::VarDecl* D); //Visitor for every VarDecl i.e. variable node in the AST
   bool VisitFieldDecl(clang::FieldDecl* D); //Visitor for e field inside a class
   bool VisitFunctionDecl(clang::FunctionDecl* D); //Visitor for every FunctionDecl i.e. function node in the AST
   bool VisitEnumDecl(clang::EnumDecl* D); //Visitor for every EnumDecl i.e. enumeration node in the AST
   bool VisitRecordDecl(clang::RecordDecl* D); // Visitor for every RecordDecl i.e. class node in the AST
   bool TraverseDeclContextHelper(clang::DeclContext *DC); // Here is the code magic :) - every Decl 
   // according to its type is processed by the corresponding Visitor method
   void RemoveBuilder(clang::DeclContext* DC); // Pop()-ing elements from the fClassBuilders stack
   void Scan (clang::ASTContext* C, clang::Decl* D);
   TString getClassName(clang::DeclContext* DC);
   void DumpDecl(clang::Decl* D, const char* msg);
};

/* -------------------------------------------------------------------------- */

#endif /* __CLR_SCAN_H__ */
