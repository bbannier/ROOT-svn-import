#ifndef __CLR_SCAN_H__
#define __CLR_SCAN_H__

// Author: Zdenek Culik   16/04/2010

#include "clang/AST/AST.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclGroup.h"
#include "clang/AST/DeclFriend.h"
#include "clang/AST/Type.h"

#include "Reflex/Reflex.h"
#include "Reflex/Builder/ReflexBuilder.h"

#include "TString.h"

#include <map>
#include <string>

class TScanner {
private:
   static int fgAnonymousClassCounter;
   static int fgBadClassCounter;
   static int fgAnonymousEnumCounter;

   static std::map<clang::Decl*, std::string> fgAnonymousClassMap;
   static std::map<clang::Decl*, std::string> fgAnonymousEnumMap;

public:
   static const char* fgClangDeclKey; // property key used for CLang declaration objects
   static const char* fgClangFuncKey; // property key for function (demangled) names

public:
   static clang::Decl* GetDeclProp(const Reflex::PropertyList& prop_list);
   static std::string GetFuncProp(const Reflex::PropertyList& prop_list);

private:
   clang::ASTContext* fCtx;
   TObject* fReporter;
   bool fUseStubs;

public:
   void UseStubs(bool value) { fUseStubs = value; }

private:
   // only for debugging

   static const int fgDeclLast = clang::Decl::Var;
   bool fDeclTable[fgDeclLast+1];

   static const int fgTypeLast = clang::Type::TemplateTypeParm;
   bool fTypeTable[fgTypeLast+1];

   clang::Decl* fLastDecl;

private:
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
   void UnimplementedType(clang::Type* T);

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

   void ScanClassTemplate(clang::ClassTemplateDecl* D);
   void ScanClassTemplateSpecialization(clang::ClassTemplateSpecializationDecl* D);
   void ScanClassTemplatePartialSpecialization(clang::ClassTemplatePartialSpecializationDecl* D);
   void ScanFunctionTemplate(clang::FunctionTemplateDecl* D);

   TString      FuncParameters(clang::FunctionDecl* D);
   std::string  FuncParameterList(clang::FunctionDecl* D);
   Reflex::Type FuncType(clang::FunctionDecl* D);
   unsigned int FuncModifiers(clang::FunctionDecl* D);

   unsigned int VisibilityModifiers(clang::AccessSpecifier access);
   unsigned int Visibility(clang::Decl* D);
   unsigned int VarModifiers(clang::VarDecl* D);

   void ScanFieldMember(clang::FieldDecl* D, Reflex::ClassBuilder* builder);
   void ScanVarMember(clang::VarDecl* D, Reflex::ClassBuilder* builder);
   void ScanFunctionMember(clang::FunctionDecl* D, Reflex::ClassBuilder* builder, TString template_params = "");
   void ScanEnumMember(clang::EnumDecl* D, Reflex::ClassBuilder* builder);
   void ScanTypedefMember(clang::TypedefDecl* D, Reflex::ClassBuilder* builder);
   void ScanFunctionTemplateMember(clang::FunctionTemplateDecl* D, Reflex::ClassBuilder* builder);
   void ScanFriendMember(clang::FriendDecl *D, Reflex::ClassBuilder* builder);
   void ScanMember(clang::Decl* D, Reflex::ClassBuilder* builder);
   void ScanClass(clang::RecordDecl* D, TString template_params = "");

   void ScanEnum(clang::EnumDecl* D);

   void ScanNamespaceAlias(clang::NamespaceAliasDecl *D);
   void ScanUsing(clang::UsingDecl* D, Reflex::Scope Outer);
   void ScanUsingShadow(clang::UsingShadowDecl* D, Reflex::Scope Outer);
   void ScanUsingDirective(clang::UsingDirectiveDecl *D, Reflex::Scope Outer);

   void ScanUnresolvedUsingValueMember(clang::UnresolvedUsingValueDecl* D, Reflex::ClassBuilder* builder);

   void ScanTranslationUnit(clang::TranslationUnitDecl* D, Reflex::Scope Outer);
   void ScanNamespace(clang::NamespaceDecl* D);
   void ScanLinkageSpec(clang::LinkageSpecDecl* D, Reflex::Scope Outer);
   void ScanTypedef(clang::TypedefDecl* D);
   void ScanVariable(clang::VarDecl* D);
   void ScanVariableOrField(clang::VarDecl* D);
   void ScanFunction(clang::FunctionDecl* D, TString template_params = "");
   void ScanMethod(clang::CXXMethodDecl* D, TString template_params = "");
   void ScanDecl(clang::Decl* D, Reflex::Scope Outer);

public:
   TScanner();
   virtual ~TScanner();
   void Scan(clang::ASTContext*);
};

#endif // __CLR_SCAN_H__
