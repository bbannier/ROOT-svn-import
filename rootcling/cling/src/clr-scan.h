// @(#)root/cint:$Id$
// Author: Zdenek Culik   16/04/2010

#ifndef __CLR_SCAN_H__
#define __CLR_SCAN_H__

#include "clang/AST/AST.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclGroup.h"
#include "clang/AST/DeclFriend.h"

#include "Reflex/Reflex.h"
#include "Reflex/Builder/ReflexBuilder.h"

#include "TString.h"

/* -------------------------------------------------------------------------- */

class TScanner
{
private:
   clang::ASTContext* fCtx;
   TObject * fReporter;

private:
   static int unknown_class_counter;
   static int unknown_enum_counter;

private:
   void ShowInfo(const TString msg, const TString location = "");
   void ShowWarning(const TString msg, const TString location = "");
   void ShowError(const TString msg, const TString location = "");

   TString GetLocation(clang::Decl* D);
   TString GetName(clang::Decl* D);

   void ScanInfo(clang::Decl* D);
   void ScanUnknown(clang::Decl* D, TString txt = "");
   void ScanUnexpected(clang::Decl* D, TString txt = "");
   void ScanUnsupported(clang::Decl* D, TString txt = "");
   void ScanUnimplemented(clang::Decl* D, TString txt = "");

   void ScanUnknownType(clang::QualType qual_type);
   void ScanUnsupportedType(clang::QualType qual_type);
   void ScanUnimplementedType(clang::QualType qual_type);
   void ScanUnimplementedType (clang::Type* T);

   Reflex::ClassBuilder* GetClassBuilder(clang::RecordDecl* D, TString template_params = "");

   Reflex::Type ScanPointerType(clang::PointerType* T);
   Reflex::Type ScanLValueReferenceType(clang::LValueReferenceType* T);
   Reflex::Type ScanMemberPointerType(clang::MemberPointerType* T);
   Reflex::Type ScanArrayType(clang::ArrayType* T);
   Reflex::Type ScanFunctionType(clang::FunctionType* T);
   Reflex::Type ScanEnumType(clang::EnumType* T);
   Reflex::Type ScanRecordType(clang::RecordType* T);
   Reflex::Type ScanElaboratedType(clang::ElaboratedType* T);
   Reflex::Type ScanTypedefType(clang::TypedefType* T);
   Reflex::Type ScanBuiltinType(clang::BuiltinType* T);
   Reflex::Type ScanQualifiedNameType(clang::QualifiedNameType* T);
   Reflex::Type ScanTemplateTypeParmType(clang::TemplateTypeParmType* T);
   Reflex::Type ScanSubstTemplateTypeParmType(clang::SubstTemplateTypeParmType* T);
   TString ConvTemplateName(clang::TemplateName& N);
   TString ExprToStr(clang::Expr* expr);
   TString ConvTemplateArguments(const clang::TemplateArgumentList& list);
   Reflex::Type ScanTemplateSpecializationType(clang::TemplateSpecializationType* T);
   Reflex::Type ScanQualType(clang::QualType qual_type);

   TString ConvTemplateParameterList(clang::TemplateParameterList* list);
   TString ScanTemplateParams(clang::TemplateDecl* D);

   void ScanClassTemplate(clang::ClassTemplateDecl* D);
   void ScanFunctionTemplate(clang::FunctionTemplateDecl* D);
   void ScanTemplateParm(clang::TemplateTemplateParmDecl* D);
   void ScanTemplateTypeParmDecl(clang::TemplateTypeParmDecl *D);

   TString FuncParameters(clang::FunctionDecl* D);
   Reflex::Type FuncType(clang::FunctionDecl* D);
   unsigned int FuncModifiers(clang::FunctionDecl* D);
   unsigned int VisibilityModifiers(clang::AccessSpecifier access);
   unsigned int Visibility(clang::Decl* D);
   unsigned int VarModifiers(clang::VarDecl* D);

   void ScanFieldMember(clang::FieldDecl* D, Reflex::ClassBuilder* builder);
   void ScanVarMember(clang::VarDecl* D, Reflex::ClassBuilder* builder);
   void ScanMethod(clang::CXXMethodDecl* D, Reflex::ClassBuilder* builder, TString template_params = "");
   void ScanEnumMember(clang::EnumDecl* D, Reflex::ClassBuilder* builder);
   void ScanTypedefMember(clang::TypedefDecl* D, Reflex::ClassBuilder* builder);
   void ScanFunctionTemplateMember(clang::FunctionTemplateDecl* D, Reflex::ClassBuilder* builder);
   void ScanFriendMember(clang::FriendDecl *D, Reflex::ClassBuilder* builder);
   void ScanMember(clang::Decl* D, Reflex::ClassBuilder* builder);
   void ScanClass(clang::RecordDecl* D, TString template_params = "");

   void ScanEnum(clang::EnumDecl* D);

   void ScanUsingDecl(clang::UsingDecl* D, Reflex::Scope Outer);
   void ScanUsingShadowDecl(clang::UsingShadowDecl* D, Reflex::Scope Outer);

   void ScanTranslationUnit(clang::TranslationUnitDecl* D, Reflex::Scope Outer);
   void ScanNamespace(clang::NamespaceDecl* D);
   void ScanLinkageSpec(clang::LinkageSpecDecl* D, Reflex::Scope Outer);
   void ScanTypedef(clang::TypedefDecl* D);
   void ScanVariable(clang::VarDecl* D);
   void ScanFunction(clang::FunctionDecl* D, TString template_params = "");
   void ScanDecl(clang::Decl* D, Reflex::Scope Outer);

public:
   TScanner ();
   virtual ~ TScanner ();

   void Scan (clang::ASTContext* C, clang::Decl* D);
};

/* -------------------------------------------------------------------------- */

extern const char* kClangKey; // property key used for connection with Clang objects

void ClrScan(clang::ASTContext* C, clang::Decl* D);
void ClrStore(clang::ASTContext* C, clang::Decl* D); // old function name, equal to ClrScan

/* -------------------------------------------------------------------------- */

#endif /* __CLR_SCAN_H__ */
