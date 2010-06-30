// Author: Zdenek Culik   16/04/2010

#include "clang/AST/ASTConsumer.h"
#include "TObject.h"
#include "clr-scan.h"
#include "clr-reg.h"

#include <map>
#include <sstream>
#include <string>

//______________________________________________________________________________

// SHOW_WARNINGS - enable warnings
#define SHOW_WARNINGS

// SHOW_TEMPLATE_INFO -- enable information about templates encountered
//#define SHOW_TEMPLATE_INFO

// COMPLETE_TEMPLATES -- process templates, not only specializations (instantiations)
//#define COMPLETE_TEMPLATES

// CHECK_TYPES -- check if types are valid in Reflex
//#define CHECK_TYPES

// FILTER_WARNINGS -- do not repeat same type of warning
#define FILTER_WARNINGS

// DIRECT_OUTPUT -- output to stderr with gcc-compatible filename and line number
//#define DIRECT_OUTPUT

//______________________________________________________________________________

// property key used for connection with Clang objects
const char* TScanner::fgClangDeclKey = "ClangDecl";

// property key for demangled names
const char* TScanner::fgClangFuncKey = "ClangFunc";

int TScanner::fgAnonymousClassCounter = 0;
int TScanner::fgBadClassCounter = 0;
int TScanner::fgAnonymousEnumCounter  = 0;

std::map<clang::Decl*, std::string> TScanner::fgAnonymousClassMap;
std::map<clang::Decl*, std::string> TScanner::fgAnonymousEnumMap;

//______________________________________________________________________________
TScanner::TScanner()
{
   fCtx = 0;
   fReporter = new TObject;
   fUseStubs = false;
   for (int i = 0; i <= fgDeclLast; ++i) {
      fDeclTable [i] = false;
   }
   for (int i = 0; i <= fgTypeLast; ++i) {
      fTypeTable [i] = false;
   }
   fLastDecl = 0;
}

//______________________________________________________________________________
TScanner::~TScanner()
{
   delete fReporter;
}

/********************************* PROPERTIES **********************************/

//______________________________________________________________________________
inline void* ToDeclProp(clang::Decl* item)
{
   return item;
}

//______________________________________________________________________________
inline Reflex::Any ToFuncProp(std::string s)
{
   Reflex::Any a = s;
   return s;
}

//______________________________________________________________________________
clang::Decl* TScanner::GetDeclProp(const Reflex::PropertyList& prop_list)
{
   clang::Decl* result = 0;
   if (prop_list.HasProperty(TScanner::fgClangDeclKey)) {
      Reflex::Any value = prop_list.PropertyValue(TScanner::fgClangDeclKey);
      result = (clang::Decl*) Reflex::any_cast <void *> (value);
   }
   return result;
}

std::string TScanner::GetFuncProp(const Reflex::PropertyList& prop_list)
{
   // --
#if 0
   std::string result = "";
   if (prop_list.HasProperty(TScanner::fgClangFuncKey)) {
      Reflex::Any value = prop_list.PropertyValue(TScanner::fgClangFuncKey);
      result = value;
   }
   return result;
#endif // 0
   return prop_list.PropertyAsString(TScanner::fgClangFuncKey);
}

/*********************************** NUMBERS **********************************/

//______________________________________________________________________________
inline size_t APIntToSize(const llvm::APInt& num)
{
   return *num.getRawData();
}

//______________________________________________________________________________
inline long APIntToLong(const llvm::APInt& num)
{
   return *num.getRawData();
}

//______________________________________________________________________________
inline TString APIntToStr(const llvm::APInt& num)
{
   return num.toString(10, true);
}

//______________________________________________________________________________
inline TString IntToStr(int num)
{
   TString txt = "";
   txt += num;
   return txt;
}

//______________________________________________________________________________
inline std::string IntToStdStr(int num)
{
   std::ostringstream stream;
   stream << num;
   return stream.
          str();
}

/********************************** MESSAGES **********************************/

//______________________________________________________________________________
inline TString Message(const TString msg, const TString location)
{
   TString loc = location;
#ifdef DIRECT_OUTPUT
   int n = loc.Length();
   while ((n > 0) && (loc[n] != ':')) {
      --n;
   }
   if (n > 0) {
      loc = loc(0, n) + ":";
   }
#endif
   if (loc == "") {
      return msg;
   }
   return loc + " " + msg;
}

//______________________________________________________________________________
void TScanner::ShowInfo(const TString msg, const TString location)
{
   // --
#ifdef DIRECT_OUTPUT
   std::cout << Message(msg, location) << std::endl;
#else
   fReporter->Info("CLR", Message(msg, location));
#endif
   // --
}

//______________________________________________________________________________
void TScanner::ShowWarning(const TString msg, const TString location)
{
   // --
#ifdef SHOW_WARNINGS
#ifdef DIRECT_OUTPUT
   std::cout << Message(msg, location) << std::endl;
#else
   fReporter->Warning("CLR", Message(msg, location));
#endif
#endif
   // --
}

//______________________________________________________________________________
void TScanner::ShowError(const TString msg, const TString location)
{
   // --
#ifdef DIRECT_OUTPUT
   std::cout << Message(msg, location) << std::endl;
#else
   fReporter->Error("CLR", Message(msg, location));
#endif
   // --
}

//______________________________________________________________________________
void TScanner::ShowTemplateInfo(const TString msg, const TString location)
{
   // --
#ifdef SHOW_TEMPLATE_INFO
   TString loc = location;
   if (loc == "")
      loc = GetLocation(fLastDecl);
   ShowWarning(msg, loc);
#endif
   // --
}

//______________________________________________________________________________
void TScanner::ShowReflexWarning(const TString msg, const TString location)
{
   TString loc = location;
   if (loc == "") {
      loc = GetLocation(fLastDecl);
   }
   ShowWarning(msg, loc);
}

/********************************** UNKNOWN ***********************************/

//______________________________________________________________________________
TString TScanner::GetSrcLocation(clang::SourceLocation L)
{
   std::string location = "";
   llvm::raw_string_ostream stream(location);
   clang::SourceManager& source_manager = fCtx->getSourceManager();
   L.print(stream, source_manager);
   return stream.str();
}

//______________________________________________________________________________
TString TScanner::GetLocation(clang::Decl* D)
{
   if (D == 0) {
      return "";
   }
   std::string location = "";
   llvm::raw_string_ostream stream(location);
   clang::SourceManager& source_manager = fCtx->getSourceManager();
   D->getLocation().print(stream, source_manager);
   return stream.str();
}

//______________________________________________________________________________
TString TScanner::GetName(clang::Decl* D)
{
   TString name = "";
   // TString kind = D->getDeclKindName();
   if (clang::NamedDecl* ND = dyn_cast <clang::NamedDecl> (D)) {
      name = ND->getQualifiedNameAsString();
   }
   return name;
}

//______________________________________________________________________________
inline TString AddSpace(const TString txt)
{
   if (txt == "") {
      return "";
   }
   return txt + " ";
}

//______________________________________________________________________________
void TScanner::DeclInfo(clang::Decl* D)
{
   TString location = GetLocation(D);
   TString kind = D->getDeclKindName();
   TString name = GetName(D);
   ShowInfo("Scan: " + kind + " declaration " + name, location);
}

//______________________________________________________________________________
void TScanner::UnknownDecl(clang::Decl* D, TString txt)
{
   // unknown - this kind of declaration was not known to programmer
   TString location = GetLocation(D);
   TString kind = D->getDeclKindName();
   TString name = GetName(D);
   ShowWarning("Unknown " + AddSpace(txt) + kind + " declaration " + name, location);
}

//______________________________________________________________________________
void TScanner::UnexpectedDecl(clang::Decl* D, TString txt)
{
   // unexpected - this kind of declaration is unexpected (in concrete place)
   TString location = GetLocation(D);
   TString kind = D->getDeclKindName();
   TString name = GetName(D);
   ShowWarning("Unexpected " + kind + " declaration " + name, location);
}

//______________________________________________________________________________
void TScanner::UnsupportedDecl(clang::Decl* D, TString txt)
{
   // unsupported - this kind of declaration is probably not used (in current version of C++)
   TString location = GetLocation(D);
   TString kind = D->getDeclKindName();
   TString name = GetName(D);
   ShowWarning("Unsupported " + AddSpace(txt) + kind + " declaration " + name, location);
}

//______________________________________________________________________________
void TScanner::UnimportantDecl(clang::Decl* D, TString txt)
{
   // unimportant - this kind of declaration is not stored into reflex
}

//______________________________________________________________________________
void TScanner::UnimplementedDecl(clang::Decl* D, TString txt)
{
   // information about item, that should be implemented
   clang::Decl::Kind k = D->getKind();
   bool show = true;
#ifdef FILTER_WARNINGS
   if ((k >= 0) || (k <= fgDeclLast)) {
      if (fDeclTable [k]) {
         show = false; // already displayed
      }
      else {
         fDeclTable [k] = true;
      }
   }
#endif
   if (show) {
      TString location = GetLocation(D);
      TString kind = D->getDeclKindName();
      TString name = GetName(D);
      if (txt == "") {
         txt = "declaration";
      }
      ShowWarning("Unimplemented " + txt + ": " + kind + " " + name, location);
   }
}

//______________________________________________________________________________
void TScanner::UnknownType(clang::QualType qual_type)
{
   TString location = GetLocation(fLastDecl);
   TString kind = qual_type.getTypePtr()->getTypeClassName();
   ShowWarning("Unknown " + kind + " type " + qual_type.getAsString(), location);
}

//______________________________________________________________________________
void TScanner::UnsupportedType(clang::QualType qual_type)
{
   TString location = GetLocation(fLastDecl);
   TString kind = qual_type.getTypePtr()->getTypeClassName();
   ShowWarning("Unsupported " + kind + " type " + qual_type.getAsString(), location);
}

//______________________________________________________________________________
void TScanner::UnimportantType(clang::QualType qual_type)
{
   // unimportant - this kind of declaration is not stored into reflex
}

//______________________________________________________________________________
void TScanner::UnimplementedType(clang::QualType qual_type)
{
   clang::Type::TypeClass k = qual_type.getTypePtr()->getTypeClass();
   bool show = true;
#ifdef FILTER_WARNINGS
   if ((k >= 0) || (k <= fgTypeLast)) {
      if (fTypeTable [k]) {
         show = false; // already displayed
      }
      else {
         fTypeTable [k] = true;
      }
   }
#endif
   if (show) {
      TString location = GetLocation(fLastDecl);
      TString kind = qual_type.getTypePtr()->getTypeClassName();
      ShowWarning("Unimplemented type: " + kind + " " + qual_type.getAsString(), location);
   }
}

//______________________________________________________________________________
void TScanner::UnimplementedType(clang::Type* T)
{
   clang::Type::TypeClass k = T->getTypeClass();
   bool show = true;
#ifdef FILTER_WARNINGS
   if ((k >= 0) || (k <= fgTypeLast)) {
      if (fTypeTable [k]) {
         show = false; // already displayed
      }
      else {
         fTypeTable [k] = true;
      }
   }
#endif
   if (show) {
      TString location = GetLocation(fLastDecl);
      TString kind = T->getTypeClassName();
      ShowWarning("Unimplemented type: " + kind, location);
   }
}

/******************************* CLASS BUILDER ********************************/

//______________________________________________________________________________
std::string TScanner::GetClassName(clang::RecordDecl* D)
{
   std::string cls_name = D->getQualifiedNameAsString();
   // NO if (cls_name == "")
   // NO if (D->isAnonymousStructOrUnion())
   // NO if (cls_name == "<anonymous>") {
   if (!D->getDeclName()) {
      if (fgAnonymousClassMap.find(D) != fgAnonymousClassMap.end()) {
         // already encountered anonymous class
         cls_name = fgAnonymousClassMap [D];
      }
      else {
         fgAnonymousClassCounter ++;
         cls_name = "_ANONYMOUS_CLASS_" + IntToStdStr(fgAnonymousClassCounter) + "_";  // !?
         fgAnonymousClassMap [D] = cls_name;
         // ShowInfo ("anonymous class " + cls_name, GetLocation (D));
      }
   }
   return cls_name;
}

//______________________________________________________________________________
std::string TScanner::GetEnumName(clang::EnumDecl* D)
{
   std::string enum_name = D->getQualifiedNameAsString();
   if (!D->getDeclName()) {
      if (fgAnonymousEnumMap.find(D) != fgAnonymousEnumMap.end()) {
         // already encountered anonymous enumeration type
         enum_name = fgAnonymousEnumMap [D];
      }
      else {
         fgAnonymousEnumCounter ++;
         enum_name = "_ANONYMOUS_ENUM_" + IntToStdStr(fgAnonymousEnumCounter) + "_";  // !?
         fgAnonymousEnumMap [D] = enum_name;
         // ShowInfo ("anonymous enum " + enum_name, GetLocation (D));
      }
   }
   return enum_name;
}

//______________________________________________________________________________
Reflex::ClassBuilder* TScanner::GetClassBuilder(clang::RecordDecl* D, TString template_params)
{
   TString full_name = GetClassName(D) + template_params;
   // ShowInfo ("GetClassBuilder " + full_name);
   clang::TagDecl::TagKind kind = D->getTagKind();
   int style = 0;
   if (kind == clang::TTK_Class) {
      style = Reflex::CLASS;
   }
   else if (kind == clang::TTK_Struct) {
      style = Reflex::STRUCT;
   }
   else if (kind == clang::TTK_Union) {
      style = Reflex::UNION;
   }
   style = style | Reflex::PUBLIC; // !?
   Reflex::ClassBuilder* builder = 0;
   // check class
   Reflex::Type t = Reflex::Type::ByName(full_name.Data());
   bool valid = !t || t.IsClass();
   if (!valid) {
      ShowReflexWarning("invalid class " + full_name, GetLocation(D));
   }
   if (valid) {
      try {
         builder = new Reflex::ClassBuilder(full_name, typeid(Reflex::UnknownType), 0, style);
      }
      catch(std::exception& e) {
         ShowReflexWarning(TString(e.what()) + " ... class " + full_name, GetLocation(D));
         builder = 0;
      }
   }
   if (!builder) {
      ++fgBadClassCounter;
      full_name = "_BAD_CLASS_" + IntToStr(fgBadClassCounter) + "_";  // !?
      builder = new Reflex::ClassBuilder(full_name, typeid(Reflex::UnknownType), 0, Reflex::CLASS);    // !?
   }
   builder->AddProperty(fgClangDeclKey, ToDeclProp(D));
   return builder;
}

/*********************************** TYPES ************************************/

//______________________________________________________________________________
Reflex::Type TScanner::ExplorePointerType(clang::PointerType* T)
{
   Reflex::Type element = ExploreQualType(T->getPointeeType());
   return Reflex::PointerBuilder(element);
}

//______________________________________________________________________________
Reflex::Type TScanner::ExploreLValueReferenceType(clang::LValueReferenceType* T)
{
   Reflex::Type element = ExploreQualType(T->getPointeeType());
   return Reflex::ReferenceBuilder(element);
}

//______________________________________________________________________________
Reflex::Type TScanner::ExploreMemberPointerType(clang::MemberPointerType* T)
{
   Reflex::Type element = ExploreQualType(T->getPointeeType());
   Reflex::Scope base;
   const clang::Type* cls_type = T->getClass();
   if (cls_type) {
      const clang::RecordType* rec_type = dyn_cast <clang::RecordType> (cls_type);
      if (rec_type) {
         clang::RecordDecl* rec_decl = rec_type->getDecl();
         if (rec_decl) {
            std::string rec_name = rec_decl->getQualifiedNameAsString();
            base = Reflex::Scope::ByName(rec_name);
            // ShowInfo ("ScanMemberPointerType " + base.Name () + " " + element.Name ());
            return Reflex::PointerToMemberBuilder(element, base);
         }
      }
   }
   ShowWarning("Strange member pointer ", GetLocation(fLastDecl));
   return Reflex::Type(); // !? !!
}

//______________________________________________________________________________
Reflex::Type TScanner::ExploreArrayType(clang::ArrayType* T)
{
   Reflex::Type element = ExploreQualType(T->getElementType());
   size_t size = 0;
   if (clang::ConstantArrayType* CT = dyn_cast <clang::ConstantArrayType> (T)) {
      size = APIntToSize(CT->getSize()) ;
   }
   else {
      ShowWarning("Unknown array size");  // !?
   }
   return Reflex::ArrayBuilder(element, size);
}

//______________________________________________________________________________
Reflex::Type TScanner::ExploreIncompleteArrayType(clang::IncompleteArrayType* T)
{
   UnimplementedType(T);
   return Reflex::Type(); // !? !!
}

//______________________________________________________________________________
Reflex::Type TScanner::ExploreDependentSizedArrayType(clang::DependentSizedArrayType* T)
{
   UnimplementedType(T);
   return Reflex::Type(); // !? !!
}

//______________________________________________________________________________
Reflex::Type TScanner::ExploreFunctionType(clang::FunctionType* T)
{
   // parameters
   std::vector <Reflex::Type> vec;
   if (clang::FunctionProtoType* FT = dyn_cast <clang::FunctionProtoType> (T)) {
      for (clang::FunctionProtoType::arg_type_iterator I = FT->arg_type_begin(), E = FT->arg_type_end(); I != E; ++I) {
         Reflex::Type param_type = ExploreQualType(*I);
         vec.push_back(param_type);
      }
   }
   // result type
   Reflex::Type type = ExploreQualType(T->getResultType());
   return Reflex::FunctionTypeBuilder(type, vec);
}

//______________________________________________________________________________
Reflex::Type TScanner::ExploreEnumType(clang::EnumType* T)
{
   std::string enum_name = GetEnumName(T->getDecl());
   return Reflex::TypeBuilder(enum_name.c_str());
}

//______________________________________________________________________________
Reflex::Type TScanner::ExploreRecordType(clang::RecordType* T)
{
   std::string cls_name = GetClassName(T->getDecl());
   return Reflex::TypeBuilder(cls_name.c_str());
}

//______________________________________________________________________________
Reflex::Type TScanner::ExploreElaboratedType(clang::ElaboratedType* T)
{
   return ExploreQualType(T->getNamedType()); // !?
}

//______________________________________________________________________________
Reflex::Type TScanner::ExploreTypedefType(clang::TypedefType* T)
{
   clang::TypedefDecl* D = T->getDecl();
   std::string name = D->getQualifiedNameAsString();
   return Reflex::TypeBuilder(name.c_str());
}

//______________________________________________________________________________
Reflex::Type TScanner::ExploreBuiltinType(clang::BuiltinType* T)
{
   clang::LangOptions lang_opts; // !?
   const char* name = T->getName(lang_opts);
   // ShowInfo ("BuiltinType " + TString (name));
   return Reflex::TypeBuilder(name); // !?
}

//______________________________________________________________________________
Reflex::Type TScanner::ExploreInjectedClassNameType(clang::InjectedClassNameType* T)
{
   // ShowInfo ("InjectedClassNameType " + TString (name));
   return ExploreQualType(T->getInjectedSpecializationType()); // !?
}

//______________________________________________________________________________
Reflex::Type TScanner::ExploreDependentNameType(clang::DependentNameType* T)
{
   const clang::IdentifierInfo* N = T->getIdentifier();
   const char* name = N->getNameStart();
   // ShowInfo ("DependentNameType " + TString (name));
   return Reflex::TypeBuilder(name); // !?
}

//______________________________________________________________________________
Reflex::Type TScanner::ExploreTemplateTypeParmType(clang::TemplateTypeParmType* T)
{
   clang::IdentifierInfo* N = T->getName();
   const char* name = N->getNameStart();
   // ShowInfo ("TemplateTypeParmType " + TString (name));
   return Reflex::TypeBuilder(name);
}

//______________________________________________________________________________
Reflex::Type TScanner::ExploreSubstTemplateTypeParmType(clang::SubstTemplateTypeParmType* T)
{
   clang::IdentifierInfo* N = T->getName();
   const char* name = N->getNameStart();
   // ShowInfo ("SubstTemplateTypeParmType " + TString (name));
   return Reflex::TypeBuilder(name);
}

//______________________________________________________________________________
Reflex::Type TScanner::ExploreTemplateSpecializationType(clang::TemplateSpecializationType* T)
{
   clang::TemplateName N = T->getTemplateName();
   TString name = ConvTemplateName(N);
   ShowTemplateInfo("TemplateSpecializationType " + name, GetLocation(fLastDecl));
   return Reflex::TypeBuilder(name);
}

//______________________________________________________________________________
Reflex::Type TScanner::ExploreQualType(clang::QualType qual_type)
{
   Reflex::Type type;
   clang::Type* T = qual_type.getTypePtr();
   // see clang/include/clang/AST/TypeNodes.def
   switch (T->getTypeClass()) {
      case clang::Type::Pointer:
         type = ExplorePointerType(dyn_cast <clang::PointerType> (T));
         break;
      case clang::Type::MemberPointer:
         type = ExploreMemberPointerType(dyn_cast <clang::MemberPointerType> (T));
         break;
      case clang::Type::LValueReference:
         type = ExploreLValueReferenceType(dyn_cast <clang::LValueReferenceType> (T));
         break;
      case clang::Type::RValueReference:
         UnsupportedType(qual_type);
         break;
      case clang::Type::ConstantArray:
         type = ExploreArrayType(dyn_cast <clang::ArrayType> (T));
         break;
      case clang::Type::IncompleteArray:
         type = ExploreIncompleteArrayType(dyn_cast <clang::IncompleteArrayType> (T));
         break;
      case clang::Type::DependentSizedArray:
         type = ExploreDependentSizedArrayType(dyn_cast <clang::DependentSizedArrayType> (T));
         break;
      case clang::Type::VariableArray:
      case clang::Type::DependentSizedExtVector:
         UnimplementedType(qual_type);
         break;
      case clang::Type::Enum:
         type = ExploreEnumType(dyn_cast <clang::EnumType> (T));
         break;
      case clang::Type::Record:
         type = ExploreRecordType(dyn_cast <clang::RecordType> (T));
         break;
      case clang::Type::Elaborated:
         type = ExploreElaboratedType(dyn_cast <clang::ElaboratedType> (T));
         break;
      case clang::Type::FunctionProto:
         type = ExploreFunctionType(dyn_cast <clang::FunctionType> (T));
         break;
      case clang::Type::FunctionNoProto:
         UnimportantType(qual_type);
         break;
      case clang::Type::Typedef:
         type = ExploreTypedefType(dyn_cast <clang::TypedefType> (T));
         break;
      case clang::Type::Builtin:
         type = ExploreBuiltinType(dyn_cast <clang::BuiltinType> (T));
         break;
      case clang::Type::Complex:
         UnsupportedType(qual_type);
         break;
      case clang::Type::InjectedClassName:
         type = ExploreInjectedClassNameType(dyn_cast <clang::InjectedClassNameType> (T));
         break;
      case clang::Type::DependentName:
         type = ExploreDependentNameType(dyn_cast <clang::DependentNameType> (T));
         break;
      case clang::Type::SubstTemplateTypeParm:
         type = ExploreSubstTemplateTypeParmType(dyn_cast <clang::SubstTemplateTypeParmType> (T));
         break;
      case clang::Type::TemplateTypeParm:
         type = ExploreTemplateTypeParmType(dyn_cast <clang::TemplateTypeParmType> (T));
         break;
      case clang::Type::TemplateSpecialization:
         type = ExploreTemplateSpecializationType(dyn_cast <clang::TemplateSpecializationType> (T));
         break;
      default:
         UnknownType(qual_type);
   }
#ifdef CHECK_TYPES
   if (!type) {
      TString kind = qual_type.getTypePtr()->getTypeClassName();
      TString name = qual_type.getAsString();
      ShowWarning("Invalid type: " + kind + " " + name);
   }
#endif
   // const and volatile
   if (qual_type.isConstQualified()) {
      type = Reflex::ConstBuilder(type);
   }
   if (qual_type.isVolatileQualified()) {
      type = Reflex::VolatileBuilder(type);
   }
   return type;
}

/********************************* EXPRESSION *********************************/

//______________________________________________________________________________
TString TScanner::ExprToStr(clang::Expr* expr)
{
   clang::LangOptions lang_opts;
   clang::PrintingPolicy print_opts(lang_opts); // !?
   std::string text = "";
   llvm::raw_string_ostream stream(text);
   expr->printPretty(stream, 0, print_opts);
   return stream.str();
}

/********************************** TEMPLATE ***********************************/

//______________________________________________________________________________
TString TScanner::ConvTemplateName(clang::TemplateName& N)
{
   clang::LangOptions lang_opts;
   clang::PrintingPolicy print_opts(lang_opts);  // !?
   std::string text = "";
   llvm::raw_string_ostream stream(text);
   N.print(stream, print_opts);
   return stream.str();
}

#ifdef COMPLETE_TEMPLATES
//______________________________________________________________________________
TString TScanner::ConvTemplateParameterList(clang::TemplateParameterList* list)
{
   TString result = "";
   bool any = false;
   for (clang::TemplateParameterList::iterator I = list->begin(), E = list->end(); I != E; ++I) {
      if (any) {
         result += ",";
      }
      any = true;
      clang::NamedDecl * D = *I;
      switch (D->getKind()) {
         case clang::Decl::TemplateTemplateParm:
            UnimplementedDecl(dyn_cast <clang::TemplateTemplateParmDecl> (D), "template parameter");
            break;
         case clang::Decl::TemplateTypeParm: {
               clang::TemplateTypeParmDecl* P = dyn_cast <clang::TemplateTypeParmDecl> (D);
               if (P->wasDeclaredWithTypename())
                  result += "typename ";
               else
                  result += "class ";
               if (P->isParameterPack())
                  result += "... ";
               result += P->getNameAsString();
            }
            break;
         case clang::Decl::NonTypeTemplateParm: {
               clang::NonTypeTemplateParmDecl* P = dyn_cast <clang::NonTypeTemplateParmDecl> (D);
               result += P->getType().getAsString();
               if (clang::IdentifierInfo* N = P->getIdentifier()) {
                  result += " ";
                  std::string s = N->getName();
                  result += s;
               }
               if (P->hasDefaultArgument())
                  result += " = " + ExprToStr(P->getDefaultArgument());
            }
            break;
         default:
            UnknownDecl(*I, "template parameter");
            break;
      }
   }
   // ShowInfo ("template parameters <" + result + ">");
   return "<" + result + ">";
}

//______________________________________________________________________________
TString TScanner::ConvTemplateParams(clang::TemplateDecl* D)
{
   return ConvTemplateParameterList(D->getTemplateParameters());
}
#endif // COMPLETE_TEMPLATES

//______________________________________________________________________________
TString TScanner::ConvTemplateArguments(const clang::TemplateArgumentList& list)
{
   clang::LangOptions lang_opts;
   clang::PrintingPolicy print_opts(lang_opts);  // !?
   return clang::TemplateSpecializationType::PrintTemplateArgumentList(
      list.getFlatArgumentList(), list.flat_size(), print_opts);
}

//______________________________________________________________________________
void TScanner::ScanClassTemplate(clang::ClassTemplateDecl* D)
{
   ShowTemplateInfo("ClassTemplate " + D->getQualifiedNameAsString(), GetLocation(D));
#ifdef COMPLETE_TEMPLATES
   TString template_params = ConvTemplateParams(D);
   ScanClass(D->getTemplatedDecl(), template_params);
#endif
   typedef llvm::FoldingSet<clang::ClassTemplateSpecializationDecl> container_t;
   container_t& list = D->getSpecializations();
   for (container_t::iterator I = list.begin(), E = list.end(); I != E; ++I) {
      clang::ClassTemplateSpecializationDecl& S = *I;
      TString txt = S.getQualifiedNameAsString() + ConvTemplateArguments(S.getTemplateArgs());
      txt = txt + " ... " + ConvTemplateArguments(S.getTemplateInstantiationArgs());
      ShowTemplateInfo("specialization --> " + txt, GetLocation(&S));
      ScanClass(&S, ConvTemplateArguments(S.getTemplateArgs()));
   }
}

//______________________________________________________________________________
void TScanner::ScanClassTemplateSpecialization(clang::ClassTemplateSpecializationDecl* D)
{
   ShowTemplateInfo("ClassTemplateSpecialization " + D->getQualifiedNameAsString(), GetLocation(D));
   UnimportantDecl(D);
}

//______________________________________________________________________________
void TScanner::ScanClassTemplatePartialSpecialization(clang::ClassTemplatePartialSpecializationDecl* D)
{
   ShowTemplateInfo("ClassTemplatePartialSpecialization " + D->getQualifiedNameAsString(), GetLocation(D));
   UnimportantDecl(D);
}

//______________________________________________________________________________
void TScanner::ScanFunctionTemplate(clang::FunctionTemplateDecl* D)
{
   ShowTemplateInfo("FunctionTemplate " + D->getQualifiedNameAsString(), GetLocation(D));
#ifdef COMPLETE_TEMPLATES
   TString template_params = ConvTemplateParams(D);
   ScanFunction(D->getTemplatedDecl(), template_params);
#endif
   typedef llvm::FoldingSet<clang::FunctionTemplateSpecializationInfo> container_t;
   container_t& list = D->getSpecializations();
   for (container_t::iterator I = list.begin(), E = list.end(); I != E; ++I) {
      clang::FunctionTemplateSpecializationInfo& S = *I;
      TString txt = S.Function->getQualifiedNameAsString() + ConvTemplateArguments(*S.TemplateArguments);
      TString location = GetSrcLocation(S.getPointOfInstantiation());
      ShowTemplateInfo("function --> " + txt, location);
      ScanFunction(S.Function, ConvTemplateArguments(*S.TemplateArguments));
   }
}

/********************************** FUNCTION **********************************/

//______________________________________________________________________________
TString TScanner::FuncParameters(clang::FunctionDecl* D)
{
   TString result = "";
   for (clang::FunctionDecl::param_iterator I = D->param_begin(), E = D->param_end(); I != E; ++I) {
      clang::ParmVarDecl* P = *I;
      if (result != "") {
         result += ";";  // semicolon, not comma, important
      }
      TString type = P->getType().getAsString();
      TString name = P->getNameAsString();
      result += type + " " + name;
#ifdef CHECK_TYPES
      Reflex::Type t = Reflex::TypeBuilder(type);
      if (!t) {
         ShowWarning("Invalid parameter type: " + type + " " + name, GetLocation(D));
      }
#endif // CHECK_TYPES
      // NO if (P->hasDefaultArg ()) // check hasUnparsedDefaultArg () and hasUninstantiatedDefaultArg ()
      if (P->getInit()) {
         TString init_value = ExprToStr(P->getDefaultArg());
         result += "=" + init_value;
      }
   }
   return result;
}

//______________________________________________________________________________
std::string TScanner::FuncParameterList(clang::FunctionDecl* D)
{
   std::string result = "";
   for (clang::FunctionDecl::param_iterator I = D->param_begin(), E = D->param_end(); I != E; ++I) {
      clang::ParmVarDecl* P = *I;
      if (result != "") {
         result += ",";
      }
      TString type = P->getType().getAsString();
      result += type;
   }
   return "(" + result + ")";
}

//______________________________________________________________________________
Reflex::Type TScanner::FuncType(clang::FunctionDecl* D)
{
   std::vector<Reflex::Type> vec;
   for (clang::FunctionDecl::param_iterator I = D->param_begin(), E = D->param_end(); I != E; ++I) {
      clang::ParmVarDecl* P = *I;
      Reflex::Type type = ExploreQualType(P->getType());
      vec.push_back(type);
   }
   Reflex::Type type = ExploreQualType(D->getResultType());
   return Reflex::FunctionTypeBuilder(type, vec);
}

//______________________________________________________________________________
unsigned int TScanner::FuncModifiers(clang::FunctionDecl* D)
{
   unsigned int modifiers = 0;
   switch (D->getStorageClass()) {
      case clang::FunctionDecl::Extern:
         modifiers |= Reflex::EXTERN;
         break;
      case clang::FunctionDecl::Static:
         modifiers |= Reflex::STATIC;
         break;
         // case clang::VarDecl::PrivateExtern:
         // !?
         // break;
   }
   if (D->isPure()) {
      modifiers |= Reflex::ABSTRACT;
   }
   if (D->isInlineSpecified()) {
      modifiers |= Reflex::INLINE;
   }
   // NO if (D->isInlined())
   //       modifiers |= Reflex::INLINE;
   if (clang::CXXMethodDecl* M = dyn_cast <clang::CXXMethodDecl> (D)) {
      if (M->isVirtual()) {
         modifiers |= Reflex::VIRTUAL;
      }
      if (M->isStatic()) {
         modifiers |= Reflex::STATIC;
      }
      // I do not know, how to recognize void func () const
      // modifiers |= Reflex::CONST;
      if (clang::CXXConstructorDecl* S = dyn_cast <clang::CXXConstructorDecl> (M)) {
         modifiers |= Reflex::CONSTRUCTOR;
         if (S->isCopyConstructor()) {
            modifiers |= Reflex::COPYCONSTRUCTOR;
         }
         if (S->isExplicit()) {
            modifiers |= Reflex::EXPLICIT;
         }
         if (S->isConvertingConstructor(false)) {   // !? parameter allow explicit
            modifiers |= Reflex::CONVERTER;
         }
      }
      else if (clang::CXXDestructorDecl* S = dyn_cast <clang::CXXDestructorDecl> (M)) {
         modifiers |= Reflex::DESTRUCTOR;
      }
      else if (clang::CXXConversionDecl* S = dyn_cast <clang::CXXConversionDecl> (M)) {
         modifiers |= Reflex::CONVERTER;
         if (S->isExplicit()) {
            modifiers |= Reflex::EXPLICIT;
         }
      }
   }
   return modifiers;
}

/*********************************** CLASS ************************************/

//______________________________________________________________________________
unsigned int TScanner::VisibilityModifiers(clang::AccessSpecifier access)
{
   unsigned int modifiers = 0;
   switch (access) {
      case clang::AS_private:
         modifiers |= Reflex::PRIVATE;
         break;
      case clang::AS_protected:
         modifiers |= Reflex::PROTECTED;
         break;
      case clang::AS_public:
         modifiers |= Reflex::PUBLIC;
         break;
   }
   return modifiers;
}

//______________________________________________________________________________
unsigned int TScanner::Visibility(clang::Decl* D)
{
   return VisibilityModifiers(D->getAccess());
}

//______________________________________________________________________________
unsigned int TScanner::VarModifiers(clang::VarDecl* D)
{
   unsigned int modifiers = 0;
   switch (D->getStorageClass()) {
      case clang::VarDecl::Auto:
         modifiers |= Reflex::AUTO;
         break;
      case clang::VarDecl::Register:
         modifiers |= Reflex::REGISTER;
         break;
      case clang::VarDecl::Extern:
         modifiers |= Reflex::EXTERN;
         break;
      case clang::VarDecl::Static:
         modifiers |= Reflex::STATIC;
         break;
      case clang::VarDecl::PrivateExtern:
         modifiers |= Reflex::EXTERN; // !?
         break;
   }
   if (D->isStaticDataMember()) {
      modifiers |= Reflex::STATIC; // !?
   }
   return modifiers;
}

//______________________________________________________________________________
void TScanner::ScanFieldMember(clang::FieldDecl* D, Reflex::ClassBuilder* builder)
{
   Reflex::Type type = ExploreQualType(D->getType());
   std::string var_name = D->getNameAsString();
   int modifiers = Visibility(D);
   if (D->isMutable())
      modifiers |= Reflex::MUTABLE;
   int offset = 0; // !?
   try {
      builder->AddDataMember(type, var_name.c_str(), offset, modifiers);
      builder->AddProperty(fgClangDeclKey, ToDeclProp(D));
   }
   catch(std::exception& e) {
      ShowReflexWarning(TString(e.what()) + " ... field " + var_name, GetLocation(D));
   }
}

//______________________________________________________________________________
void TScanner::ScanVarMember(clang::VarDecl* D, Reflex::ClassBuilder* builder)
{
   Reflex::Type type = ExploreQualType(D->getType());
   std::string var_name = D->getNameAsString();
   int modifiers = Visibility(D) | VarModifiers(D);
   int offset = 0; // !?
   try {
      builder->AddDataMember(type, var_name.c_str(), offset, modifiers);
      builder->AddProperty(fgClangDeclKey, ToDeclProp(D));
   }
   catch(std::exception& e) {
      ShowReflexWarning(TString(e.what()) + " ... var member " + var_name, GetLocation(D));
   }
}

//______________________________________________________________________________
void TScanner::ScanFunctionMember(clang::FunctionDecl* D, Reflex::ClassBuilder* builder, TString template_params)
{
   Reflex::Type type = FuncType(D);
   TString name = D->getNameAsString() + template_params;
   std::string func_name = D->getQualifiedNameAsString() + FuncParameterList(D);
   TString params = FuncParameters(D);
   int modifiers = Visibility(D) | FuncModifiers(D);
   Reflex::StubFunction stub = 0;
   void* stub_ctx = 0;
   if (fUseStubs) {
      stub = gClrReg->GetFunctionStub(func_name.c_str());
   }
   builder->AddFunctionMember(type, name, stub, stub_ctx, params, modifiers);
   builder->AddProperty(fgClangDeclKey, ToDeclProp(D));
   builder->AddProperty(fgClangFuncKey, ToFuncProp(func_name));
}

//______________________________________________________________________________
void TScanner::ScanEnumMember(clang::EnumDecl* D, Reflex::ClassBuilder* builder)
{
   TString full_name = GetEnumName(D);
   unsigned int modifiers = Visibility(D);  // !?
   TString items = "";
   for (clang::EnumDecl::enumerator_iterator I = D->enumerator_begin(), E = D->enumerator_end(); I != E; ++I) {
      TString item_name = I->getNameAsString();
      if (items != "") {
         items = items + ";";
      }
      items = items + item_name ;
      if (I->getInitExpr() != 0) {
         items += "=" + APIntToStr(I->getInitVal());
      }
   }
   builder->AddEnum(full_name, items, &typeid(Reflex::UnknownType), modifiers);
}

//______________________________________________________________________________
void TScanner::ScanTypedefMember(clang::TypedefDecl* D, Reflex::ClassBuilder* builder)
{
   Reflex::Type type = ExploreQualType(D->getUnderlyingType());
   std::string name = D->getNameAsString();
   try {
      builder->AddTypedef(type, name.c_str());
      builder->AddProperty(fgClangDeclKey, ToDeclProp(D));
   }
   catch(std::exception& e) {
      ShowReflexWarning(TString(e.what()) + " ... typedef member " + name, GetLocation(D));
   }
}

//______________________________________________________________________________
void TScanner::ScanFunctionTemplateMember(clang::FunctionTemplateDecl* D, Reflex::ClassBuilder* builder)
{
   ShowTemplateInfo("FunctionTemplate (member) " + D->getQualifiedNameAsString(), GetLocation(D));
#ifdef COMPLETE_TEMPLATES
   TString template_params = ConvTemplateParams(D);
   clang::FunctionDecl* F = D->getTemplatedDecl();
   ScanFunctionMember(F, builder, template_params);
#endif
   // --
}

//______________________________________________________________________________
void TScanner::ScanFriendMember(clang::FriendDecl *D, Reflex::ClassBuilder* builder)
{
   UnimportantDecl(D);
}

//______________________________________________________________________________
void TScanner::ScanMember(clang::Decl* D, Reflex::ClassBuilder* builder)
{
   switch (D->getKind()) {
      case clang::Decl::Field:
         ScanFieldMember(dyn_cast <clang::FieldDecl> (D), builder);
         break;
      case clang::Decl::Var:
         ScanVarMember(dyn_cast <clang::VarDecl> (D), builder);
         break;
      case clang::Decl::CXXMethod:
      case clang::Decl::CXXConstructor:
      case clang::Decl::CXXDestructor:
      case clang::Decl::CXXConversion:
         ScanFunctionMember(dyn_cast <clang::CXXMethodDecl> (D), builder);
         break;
      case clang::Decl::Typedef:
         ScanTypedefMember(dyn_cast <clang::TypedefDecl> (D), builder);
         break;
      case clang::Decl::Enum:
         ScanEnumMember(dyn_cast <clang::EnumDecl> (D), builder);
         break;
      case clang::Decl::Record:
      case clang::Decl::CXXRecord:
         ScanClass(dyn_cast <clang::RecordDecl> (D));
         break;
      case clang::Decl::Friend:
         ScanFriendMember(dyn_cast <clang::FriendDecl> (D), builder);
         break;
      case clang::Decl::ClassTemplate:
         ScanClassTemplate(dyn_cast <clang::ClassTemplateDecl> (D));
         break;
      case clang::Decl::ClassTemplateSpecialization:
         ScanClassTemplateSpecialization(dyn_cast <clang::ClassTemplatePartialSpecializationDecl> (D));
         break;
      case clang::Decl::ClassTemplatePartialSpecialization:
         ScanClassTemplatePartialSpecialization(dyn_cast <clang::ClassTemplatePartialSpecializationDecl> (D));
         break;
      case clang::Decl::FunctionTemplate:
         ScanFunctionTemplateMember(dyn_cast <clang::FunctionTemplateDecl> (D), builder);
         break;
      case clang::Decl::Using:
         ScanUsing(dyn_cast <clang::UsingDecl> (D), builder->ToType());
         break;
      case clang::Decl::UsingShadow:
         ScanUsingShadow(dyn_cast <clang::UsingShadowDecl> (D), builder->ToType());
         break;
      case clang::Decl::UnresolvedUsingValue:
         ScanUnresolvedUsingValueMember(dyn_cast <clang::UnresolvedUsingValueDecl> (D), builder);
         break;
      case clang::Decl::AccessSpec:
         // nothing
         break;
      default:
         UnknownDecl(D, "member");
         break;
   }
}

//______________________________________________________________________________
void TScanner::ScanClass(clang::RecordDecl* D, TString template_params)
{
   Reflex::ClassBuilder* builder = GetClassBuilder(D, template_params);
   if (clang::CXXRecordDecl* C = dyn_cast <clang::CXXRecordDecl> (D)) {
      if (C->getDefinition() != 0)   // important
         for (clang::CXXRecordDecl::base_class_iterator I = C->bases_begin(), E = C->bases_end(); I != E; ++I) {
            // clang::CXXBaseSpecifier  b = *I;
            Reflex::Type type = ExploreQualType(I->getType());
            Reflex::OffsetFunction offset = 0; // !?
            unsigned int modifiers = VisibilityModifiers(I->getAccessSpecifier());
            if (I->isVirtual()) {
               modifiers |= Reflex::VIRTUAL;
            }
            builder->AddBase(type, offset, modifiers);
         }
   }
   for (clang::DeclContext::decl_iterator I = D->decls_begin(), E = D->decls_end(); I != E; ++I) {
      ScanMember(*I, builder);
   }
   // return builder->ToType ();
}

/************************************ ENUM ************************************/

//______________________________________________________________________________
void TScanner::ScanEnum(clang::EnumDecl* D)
{
   TString full_name = GetEnumName(D);
   unsigned int modifiers = Visibility(D);  // !?
   Reflex::EnumBuilder* builder =
      new Reflex::EnumBuilder(full_name, typeid(Reflex::UnknownType), modifiers);
   builder->AddProperty(fgClangDeclKey, ToDeclProp(D));
   for (clang::EnumDecl::enumerator_iterator I = D->enumerator_begin(), E = D->enumerator_end(); I != E; ++I) {
      std::string item_name = I->getNameAsString();
      long value = APIntToLong(I->getInitVal());   // !?
      builder->AddItem(item_name.c_str(), value);
   }
}

/*********************************** USING ************************************/

//______________________________________________________________________________
void TScanner::ScanNamespaceAlias(clang::NamespaceAliasDecl *D)
{
   UnimplementedDecl(D);
}

//______________________________________________________________________________
void TScanner::ScanUsing(clang::UsingDecl* D, Reflex::Scope Outer)
{
   UnimportantDecl(D);
}

//______________________________________________________________________________
void TScanner::ScanUsingDirective(clang::UsingDirectiveDecl *D, Reflex::Scope Outer)
{
   std::string name = D->getNominatedNamespace()->getQualifiedNameAsString();
   // ShowInfo("using namespace " + name, GetLocation (D));
   Reflex::Scope Inner = Reflex::Scope::ByName(name);
   Outer.AddUsingDirective(Inner);
}

//______________________________________________________________________________
void TScanner::ScanUsingShadow(clang::UsingShadowDecl* D, Reflex::Scope Outer)
{
   UnimportantDecl(D);
}

//______________________________________________________________________________
void TScanner::ScanUnresolvedUsingValueMember(clang::UnresolvedUsingValueDecl* D, Reflex::ClassBuilder* builder)
{
   UnimplementedDecl(D, "member");
}

/*********************************** GLOBAL ************************************/

//______________________________________________________________________________
void TScanner::ScanTranslationUnit(clang::TranslationUnitDecl* D, Reflex::Scope Outer)
{
   clang::DeclContext* DC = clang::TranslationUnitDecl::castToDeclContext(D);
   for (clang::DeclContext::decl_iterator I = DC->decls_begin(), E = DC->decls_end(); I != E; ++I) {
      ScanDecl(*I, Outer);
   }
}

//______________________________________________________________________________
void TScanner::ScanNamespace(clang::NamespaceDecl* D)
{
   std::string ns_name = D->getQualifiedNameAsString();
   Reflex::NamespaceBuilder* builder =
      new Reflex::NamespaceBuilder(ns_name.c_str());
   builder->AddProperty(fgClangDeclKey, ToDeclProp(D));
   clang::DeclContext* DC = dyn_cast <clang::DeclContext> (D);
   if (DC) {
      for (clang::DeclContext::decl_iterator I = DC->decls_begin(), E = DC->decls_end(); I != E; ++I) {
         ScanDecl(*I, builder->ToScope());
      }
   }
}

//______________________________________________________________________________
void TScanner::ScanLinkageSpec(clang::LinkageSpecDecl* D, Reflex::Scope Outer)
{
   for (clang::DeclContext::decl_iterator I = D->decls_begin(), E = D->decls_end(); I != E; ++I) {
      ScanDecl(*I, Outer);
   }
}

//______________________________________________________________________________
void TScanner::ScanTypedef(clang::TypedefDecl* D)
{
   Reflex::Type type = ExploreQualType(D->getUnderlyingType());
   std::string name = D->getQualifiedNameAsString();
   // ShowInfo ("TypedefDecl " + name, GetLocation (D));
   // Reflex::TypeBuilder(name.c_str ());
   return; // !? !!
   Reflex::Type result = Reflex::TypedefTypeBuilder(name.c_str(), type);
   result.Properties().AddProperty(fgClangDeclKey, ToDeclProp(D));
}

//______________________________________________________________________________
void TScanner::ScanVariable(clang::VarDecl* D)
{
   Reflex::Type type = ExploreQualType(D->getType());
   std::string var_name = D->getQualifiedNameAsString();
   unsigned int modifiers = Visibility(D) | VarModifiers(D);
   int offset = 0; // !?
   if (! Reflex::Scope::ByName("").IsNamespace()) {
      ShowInfo("Bug");
   }
   try {
      Reflex::VariableBuilder builder =
         Reflex::VariableBuilder(var_name.c_str(), type, offset, modifiers);
      builder.AddProperty(fgClangDeclKey, ToDeclProp(D));
   }
   catch(std::exception& e) {
      ShowReflexWarning(TString(e.what()) + " ... variable " + var_name, GetLocation(D));
   }
}

//______________________________________________________________________________
void TScanner::ScanVariableOrField(clang::VarDecl* D)
{
   clang::DeclContext* ctx = D->getDeclContext();
   if (ctx->isRecord()) {
      clang::RecordDecl* cls = dyn_cast <clang::RecordDecl> (ctx);
      Reflex::ClassBuilder* builder = GetClassBuilder(cls);
      ScanVarMember(D, builder);
   }
   else {
      ScanVariable(D);
   }
}

//______________________________________________________________________________
void TScanner::ScanFunction(clang::FunctionDecl* D, TString template_params)
{
   Reflex::Type type = FuncType(D);
   std::string qual_name = D->getQualifiedNameAsString();
   std::string func_name = qual_name + FuncParameterList(D);
   TString full_name = qual_name + template_params;
   TString params = FuncParameters(D);
   int modifiers = FuncModifiers(D);
   Reflex::StubFunction stub = 0;
   void* stub_ctx = 0;
   try {
      if (fUseStubs) {
         stub = gClrReg->GetFunctionStub(func_name.c_str());
      }
      Reflex::FunctionBuilder builder =
         Reflex::FunctionBuilder(type, full_name, stub, stub_ctx, params, modifiers);
      builder.AddProperty(fgClangDeclKey, ToDeclProp(D));
      builder.AddProperty(fgClangFuncKey, ToFuncProp(func_name));
   }
   catch(std::exception& e) {
      ShowReflexWarning(TString(e.what()) + " ... function " + full_name, GetLocation(D));
   }
}

//______________________________________________________________________________
void TScanner::ScanMethod(clang::CXXMethodDecl* D, TString template_params)
{
   // standalone C++ method --> ScanFunctionMember
   clang::RecordDecl* cls = D->getParent();
   Reflex::ClassBuilder* builder = GetClassBuilder(cls, template_params);
   ScanFunctionMember(D, builder, template_params);
}

/************************************ DECL ************************************/

//______________________________________________________________________________
void TScanner::ScanDecl(clang::Decl* D, Reflex::Scope Outer)
{
   // ScanInfo (D);
   // see clang/include/clang/AST/DeclNodes.def
   fLastDecl = D; // only for debugging
   switch (D->getKind()) {
      case clang::Decl::Var:
         ScanVariableOrField(dyn_cast <clang::VarDecl> (D));
         break;
      case clang::Decl::Function:
         ScanFunction(dyn_cast <clang::FunctionDecl> (D));
         break;
      case clang::Decl::CXXMethod:
      case clang::Decl::CXXConstructor:
      case clang::Decl::CXXDestructor:
      case clang::Decl::CXXConversion:
         ScanMethod(dyn_cast <clang::CXXMethodDecl> (D));
         break;
      case clang::Decl::Enum:
         ScanEnum(dyn_cast <clang::EnumDecl> (D));
         break;
      case clang::Decl::Record:
      case clang::Decl::CXXRecord:
         ScanClass(dyn_cast <clang::RecordDecl> (D));
         break;
      case clang::Decl::Typedef:
         ScanTypedef(dyn_cast <clang::TypedefDecl> (D));
         break;
      case clang::Decl::ClassTemplate:
         ScanClassTemplate(dyn_cast <clang::ClassTemplateDecl> (D));
         break;
      case clang::Decl::ClassTemplateSpecialization:
         ScanClassTemplateSpecialization(dyn_cast <clang::ClassTemplateSpecializationDecl> (D));
         break;
      case clang::Decl::ClassTemplatePartialSpecialization:
         ScanClassTemplatePartialSpecialization(dyn_cast <clang::ClassTemplatePartialSpecializationDecl> (D));
         break;
      case clang::Decl::FunctionTemplate:
         ScanFunctionTemplate(dyn_cast <clang::FunctionTemplateDecl> (D));
         break;
      case clang::Decl::TranslationUnit:
         ScanTranslationUnit(dyn_cast <clang::TranslationUnitDecl> (D), Outer);
         break;
      case clang::Decl::Namespace:
         ScanNamespace(dyn_cast <clang::NamespaceDecl> (D));
         break;
      case clang::Decl::NamespaceAlias:
         ScanNamespaceAlias(dyn_cast <clang::NamespaceAliasDecl> (D));
         break;
      case clang::Decl::LinkageSpec:
         ScanLinkageSpec(dyn_cast <clang::LinkageSpecDecl> (D), Outer);
         break;
      case clang::Decl::Using:
         ScanUsing(dyn_cast <clang::UsingDecl> (D), Outer);
         break;
      case clang::Decl::UsingDirective:
         ScanUsingDirective(dyn_cast <clang::UsingDirectiveDecl> (D), Outer);
         break;
      case clang::Decl::UsingShadow:
         ScanUsingShadow(dyn_cast <clang::UsingShadowDecl> (D), Outer);
         break;
      case clang::Decl::StaticAssert:
         UnsupportedDecl(dyn_cast <clang::StaticAssertDecl> (D));
         break;
      case clang::Decl::Friend:
      case clang::Decl::FriendTemplate:
         // should be used in class declaration
         UnexpectedDecl(D);
         break;
      case clang::Decl::EnumConstant:
         // should be used inside enumeration type declaration
         UnexpectedDecl(D);
         break;
      case clang::Decl::TemplateTemplateParm:
      case clang::Decl::TemplateTypeParm:
         // should be used in template declaration
         UnexpectedDecl(D);
         break;
      default:
         UnknownDecl(D);
         break;
   }
}

//______________________________________________________________________________
void TScanner::Scan(clang::ASTContext* C)
{
   TClrReg::Init();
   fCtx = C;
   ScanDecl(C->getTranslationUnitDecl(), Reflex::Scope::GlobalScope());
}

