// Author: Zdenek Culik   16/04/2010
// Modified by: Velislava Spasova

#include "clang/AST/ASTConsumer.h"
#include "TObject.h"
#include "clr-scan.h"

#include <iostream>
#include <sstream> // class ostringstream

/* -------------------------------------------------------------------------- */

#define SHOW_WARNINGS
// #define SHOW_TEMPLATE_INFO

// #define COMPLETE_TEMPLATES
// #define CHECK_TYPES

#define FILTER_WARNINGS
// #define DIRECT_OUTPUT


// SHOW_WARNINGS - enable warnings
// SHOW_TEMPLATE_INFO - enable informations about encoutered tempaltes

// COMPLETE_TEMPLATES - process templates, not only specializations (instantiations)
// CHECK_TYPES - check if types are valid in Reflex

// FILTER_WARNINGS -- do not repeat same type of warning
// DIRECT_OUTPUT -- output to std err with gcc compatible filename an line number

/* -------------------------------------------------------------------------- */
using namespace clang;
const char* TScanner::fgClangDeclKey = "ClangDecl"; // property key used for connection with Clang objects
const char* TScanner::fgClangFuncKey = "ClangFunc"; // property key for demangled names

int TScanner::fgAnonymousClassCounter = 0;
int TScanner::fgBadClassCounter = 0;
int TScanner::fgAnonymousEnumCounter  = 0;

std::map <clang::Decl*, std::string> TScanner::fgAnonymousClassMap;
std::map <clang::Decl*, std::string> TScanner::fgAnonymousEnumMap;

//______________________________________________________________________________
void ClrStubFunction (void* result, void* obj, const std::vector<void*>& params, void* ctx)
{
   TContext* context = reinterpret_cast <TContext *> (ctx);
   assert (context != NULL);
   std::cout << "Calling stub function " << context->index << ", " << context->name << std::endl;

   // TO DO: call LLVM function context->func

   // std::cout << "Called stub function " << context->index << ", " << context->name << std::endl;
}

//______________________________________________________________________________
TContext* TScanner::AllocateContext ()
{
   TContext* result = new TContext;

   #if 0
   if (fLastContext == NULL)
      fFirstContext = result;
   else
      fLastContext->next = result;

   fLastContext = result;
   #endif

   return result;
}



//______________________________________________________________________________
#if 0
void TScanner::DeleteContexts ()
{
   TContext* p = fFirstContext;
   while (p != NULL)
   {
      TContext* t = p;
      p = p->next;
      delete t;
   }

   fFirstContext = NULL;
   fLastContext = NULL;
}
#endif

//______________________________________________________________________________
TScanner::TScanner ()
{
   fCtx = NULL;
   fReporter = new TObject;

   #if 0
   fFirstContext = NULL;
   fLastContext = NULL;
   #endif

   for (int i = 0; i <= fgDeclLast; i ++)
      fDeclTable [i] = false;

   for (int i = 0; i <= fgTypeLast; i ++)
      fTypeTable [i] = false;

   fLastDecl = 0;
}

//______________________________________________________________________________
TScanner::~TScanner ()
{
   delete fReporter;
}

/********************************* PROPERTIES **********************************/

//______________________________________________________________________________
inline void* ToDeclProp(clang::Decl* item)
{
   /* conversion and type check used by AddProperty */
   return item;
}

//______________________________________________________________________________
inline Reflex::Any ToFuncProp(std::string s)
{
   /* conversion and type check used by AddProperty */
   Reflex::Any a = s;
   return s;
}

clang::Decl* TScanner::GetDeclProp (const Reflex::PropertyList& prop_list)
{
   clang::Decl* result = NULL;
   if (prop_list.HasProperty(TScanner::fgClangDeclKey)) {
      Reflex::Any value = prop_list.PropertyValue(TScanner::fgClangDeclKey);
      result = (clang::Decl*) Reflex::any_cast <void *> (value);
   }
   return result;
}

std::string TScanner::GetFuncProp (const Reflex::PropertyList& prop_list)
{
   #if 0
   std::string result = "";
   if (prop_list.HasProperty(TScanner::fgClangFuncKey)) {

      Reflex::Any value = prop_list.PropertyValue(TScanner::fgClangFuncKey);
      result = value;
   }
   return result;
   #endif

   return prop_list.PropertyAsString (TScanner::fgClangFuncKey);
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
inline std::string IntToStd(int num)
{
   std::ostringstream stream;
   stream << num;
   return stream.str();
}

/********************************** MESSAGES **********************************/

//______________________________________________________________________________
inline TString Message(const TString msg, const TString location)
{
   TString loc = location;

   #ifdef DIRECT_OUTPUT
      int n = loc.Length ();
      while (n > 0 && loc [n] != ':')
         n--;
      if (n > 0)
         loc = loc (0, n) + ":";
   #endif

    if (loc == "")
      return msg;
    else
      return loc + " " + msg;
}

//______________________________________________________________________________
void TScanner::ShowInfo(const TString msg, const TString location)
{
   const TString message = Message(msg, location);
   #ifdef DIRECT_OUTPUT
      std::cout << message << std::endl;
   #else
      fReporter->Info("TScanner:ShowInfo", "CLR %s", message.Data());
   #endif
}

//______________________________________________________________________________
void TScanner::ShowWarning(const TString msg, const TString location)
{
   #ifdef SHOW_WARNINGS
   const TString message = Message(msg, location);
      #ifdef DIRECT_OUTPUT
         std::cout << message << std::endl;
      #else
         fReporter->Warning("TScanner:ShowWarning", "CLR %s", message.Data());
      #endif
   #endif
}

//______________________________________________________________________________
void TScanner::ShowError(const TString msg, const TString location)
{
   const TString message = Message(msg, location);
   #ifdef DIRECT_OUTPUT
      std::cout << message << std::endl;
   #else
      fReporter->Error("TScanner:ShowError", "CLR %s", message.Data());
   #endif
}

//______________________________________________________________________________
void TScanner::ShowTemplateInfo(const TString msg, const TString location)
{
   #ifdef SHOW_TEMPLATE_INFO
      TString loc = location;
      if (loc == "")
         loc = GetLocation (fLastDecl);
      ShowWarning(msg, loc);
   #endif
}

//______________________________________________________________________________
void TScanner::ShowReflexWarning(const TString msg, const TString location)
{
   TString loc = location;
   if (loc == "")
      loc = GetLocation (fLastDecl);
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
   if (D == NULL)
   {
      return "";
   }
   else
   {
      std::string location = "";
      llvm::raw_string_ostream stream(location);
      clang::SourceManager& source_manager = fCtx->getSourceManager();
      D->getLocation().print(stream, source_manager);
      return stream.str();
   }
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
   if (txt == "")
      return "";
   else
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
      if (k >= 0 || k <= fgDeclLast) {
         if (fDeclTable [k])
            show = false; // already displayed
         else
            fDeclTable [k] = true;
      }
   #endif

   if (show)
   {
      TString location = GetLocation(D);
      TString kind = D->getDeclKindName();
      TString name = GetName(D);
      if (txt == "")
         txt = "declaration";
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
      if (k >= 0 || k <= fgTypeLast) {
         if (fTypeTable [k])
            show = false; // already displayed
         else
            fTypeTable [k] = true;
      }
   #endif

   if (show)
   {
      TString location = GetLocation(fLastDecl);
      TString kind = qual_type.getTypePtr()->getTypeClassName();
      ShowWarning("Unimplemented type: " + kind + " " + qual_type.getAsString(), location);
   }
}

//______________________________________________________________________________
void TScanner::UnimplementedType (clang::Type* T)
{
   clang::Type::TypeClass k = T->getTypeClass();

   bool show = true;
   #ifdef FILTER_WARNINGS
      if (k >= 0 || k <= fgTypeLast) {
         if (fTypeTable [k])
            show = false; // already displayed
         else
            fTypeTable [k] = true;
      }
   #endif

   if (show)
   {
      TString location = GetLocation(fLastDecl);
      TString kind = T->getTypeClassName ();
      ShowWarning ("Unimplemented type: " + kind, location);
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
   if (! D->getDeclName ()) {
      if (fgAnonymousClassMap.find (D) != fgAnonymousClassMap.end())
      {
         // already encountered anonymous class
         cls_name = fgAnonymousClassMap [D];
      }
      else
      {
         fgAnonymousClassCounter ++;
         cls_name = "_ANONYMOUS_CLASS_" + IntToStd(fgAnonymousClassCounter) + "_";  // !?
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

   if (! D->getDeclName ()) {
      if (fgAnonymousEnumMap.find (D) != fgAnonymousEnumMap.end())
      {
         // already encountered anonymous enumeration type
         enum_name = fgAnonymousEnumMap [D];
      }
      else
      {
         fgAnonymousEnumCounter ++;
         enum_name = "_ANONYMOUS_ENUM_" + IntToStd(fgAnonymousEnumCounter) + "_";  // !?
         fgAnonymousEnumMap [D] = enum_name;
         // ShowInfo ("anonymous enum " + enum_name, GetLocation (D));
      }
   }

   return enum_name;
}

//______________________________________________________________________________
Reflex::ClassBuilder* TScanner::GetClassBuilder(clang::RecordDecl* D, TString template_params)
{
   TString full_name = GetClassName (D) + template_params;
   // ShowInfo ("GetClassBuilder " + full_name);

   clang::TagDecl::TagKind kind = D->getTagKind();

   int style = 0;

   if (kind == clang::TTK_Class)
      style = Reflex::CLASS;
   else if (kind == clang::TTK_Struct)
      style = Reflex::STRUCT;
   else if (kind == clang::TTK_Union)
      style = Reflex::UNION;

   style = style | Reflex::PUBLIC; // !?

   Reflex::ClassBuilder* builder = NULL;

   // check class
   Reflex::Type t = Reflex::Type::ByName(full_name.Data());
   bool valid = ! t || t.IsClass ();
   if (! valid)
      ShowReflexWarning("invalid class " + full_name, GetLocation(D));

   if (valid)
      try {
         builder = new Reflex::ClassBuilder(full_name, typeid(Reflex::UnknownType), 0, style);
      } catch (std::exception& e) {
         ShowReflexWarning(TString(e.what()) + " ... class " + full_name, GetLocation(D));
         builder = NULL;
      }

   if (builder == NULL) {
      fgBadClassCounter ++;
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
   if (cls_type != NULL) {
      const clang::RecordType* rec_type = dyn_cast <clang::RecordType> (cls_type);
      if (rec_type != NULL) {
         clang::RecordDecl* rec_decl = rec_type->getDecl();

         if (rec_decl != NULL) {
            std::string rec_name = rec_decl->getQualifiedNameAsString();
            base = Reflex::Scope::ByName(rec_name);

            // ShowInfo ("ScanMemberPointerType " + base.Name () + " " + element.Name ());
            return Reflex::PointerToMemberBuilder(element, base);
         }
      }
   }

   ShowWarning("Strange member pointer ", GetLocation (fLastDecl));
   return Reflex::Type(); // !? !!
}

//______________________________________________________________________________
Reflex::Type TScanner::ExploreArrayType(clang::ArrayType* T)
{
   Reflex::Type element = ExploreQualType(T->getElementType());

   size_t size = 0;
   if (clang::ConstantArrayType* CT = dyn_cast <clang::ConstantArrayType> (T)) {
      size = APIntToSize(CT->getSize()) ;
   } else {
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
   return Reflex::TypeBuilder(enum_name.c_str ());
}

//______________________________________________________________________________
Reflex::Type TScanner::ExploreRecordType(clang::RecordType* T)
{
   std::string cls_name = GetClassName(T->getDecl());
   return Reflex::TypeBuilder(cls_name.c_str ());
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

   return Reflex::TypeBuilder(name.c_str ());
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

   ShowTemplateInfo ("TemplateSpecializationType " + name, GetLocation (fLastDecl));
   return Reflex::TypeBuilder(name);
}

//______________________________________________________________________________
Reflex::Type TScanner::ExploreQualType(clang::QualType qual_type)
{
   Reflex::Type type;
   clang::Type* T = qual_type.getTypePtr();

   // see clang/include/clang/AST/TypeNodes.def

   switch (T->getTypeClass ())
   {
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
   if (! type)
   {
       TString kind = qual_type.getTypePtr()->getTypeClassName();
       TString name = qual_type.getAsString();
       ShowWarning ("Invalid type: " + kind + " " + name );
   }
   #endif

   // const and volatile

   if (qual_type.isConstQualified())
      type = Reflex::ConstBuilder(type);

   if (qual_type.isVolatileQualified())
      type = Reflex::VolatileBuilder(type);

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

   expr->printPretty(stream, NULL, print_opts);

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
      if (any)
         result += ",";
      any = true;

      clang::NamedDecl * D = *I;

      switch (D->getKind()) {

         case clang::Decl::TemplateTemplateParm:
            UnimplementedDecl(dyn_cast <clang::TemplateTemplateParmDecl> (D), "template parameter");
            break;

         case clang::Decl::TemplateTypeParm:
            {
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

        case clang::Decl::NonTypeTemplateParm:
           {
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
   return clang::TemplateSpecializationType::PrintTemplateArgumentList
          (list.getFlatArgumentList(), list.flat_size(), print_opts);
}

/********************************** FUNCTION **********************************/

//______________________________________________________________________________
TString TScanner::FuncParameters(clang::FunctionDecl* D)
{
   TString result = "";

   for (clang::FunctionDecl::param_iterator I = D->param_begin(), E = D->param_end(); I != E; ++I) {
      clang::ParmVarDecl* P = *I;

      if (result != "")
         result += ";";  // semicolon, not comma, important

      TString type = P->getType().getAsString();
      TString name = P->getNameAsString();

      result += type + " " + name;

      #ifdef CHECK_TYPES
         Reflex::Type t = Reflex::TypeBuilder (type);
         if (! t)
            ShowWarning ("Invalid parameter type: " + type + " " + name, GetLocation (D));
      #endif

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

      if (result != "")
         result += ",";

      TString type = P->getType().getAsString();
      result += type;
   }

   return "(" + result + ")";
}

//______________________________________________________________________________
Reflex::Type TScanner::FuncType(clang::FunctionDecl* D)
{
   std::vector <Reflex::Type> vec;

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
      case clang::SC_Extern:
         modifiers |= Reflex::EXTERN;
         break;

      case clang::SC_Static:
         modifiers |= Reflex::STATIC;
         break;

      // case clang::VarDecl::PrivateExtern:
         // !?
         // break;
   default:
     break;
   }

   if (D->isPure())
      modifiers |= Reflex::ABSTRACT;

   if (D->isInlineSpecified())
      modifiers |= Reflex::INLINE;

   // NO if (D->isInlined())
   //       modifiers |= Reflex::INLINE;

   if (clang::CXXMethodDecl* M = dyn_cast <clang::CXXMethodDecl> (D)) {

      if (M->isVirtual())
         modifiers |= Reflex::VIRTUAL;

      if (M->isStatic())
         modifiers |= Reflex::STATIC;

      // I do not know, how to recognize void func () const
      // modifiers |= Reflex::CONST;

      if (clang::CXXConstructorDecl* S = dyn_cast <clang::CXXConstructorDecl> (M)) {
         modifiers |= Reflex::CONSTRUCTOR;

         if (S->isCopyConstructor())
            modifiers |= Reflex::COPYCONSTRUCTOR;

         if (S->isExplicit())
            modifiers |= Reflex::EXPLICIT;

         if (S->isConvertingConstructor(false))  // !? parameter allow explicit
            modifiers |= Reflex::CONVERTER;

      } else if (dyn_cast <clang::CXXDestructorDecl> (M)) {
         modifiers |= Reflex::DESTRUCTOR;

      } else if (clang::CXXConversionDecl* S = dyn_cast <clang::CXXConversionDecl> (M)) {
         modifiers |= Reflex::CONVERTER;
         if (S->isExplicit())
            modifiers |= Reflex::EXPLICIT;
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
   default:
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

      case clang::SC_Auto:
         modifiers |= Reflex::AUTO;
         break;

      case clang::SC_Register:
         modifiers |= Reflex::REGISTER;
         break;

      case clang::SC_Extern:
         modifiers |= Reflex::EXTERN;
         break;

      case clang::SC_Static:
         modifiers |= Reflex::STATIC;
         break;

      case clang::SC_PrivateExtern:
         modifiers |= Reflex::EXTERN; // !?
         break;
   default:
     break;
   }

   if (D->isStaticDataMember())
      modifiers |= Reflex::STATIC; // !?

   return modifiers;
}


/********************* Velislava's Method implementations **********************/

// This method visits a class node - for every class is created a new class buider
// irrespectful of weather the class is internal for another class declaration or not.
// For every clas the class builder is put on top of the fClassBuilders stack
bool TScanner::VisitRecordDecl(clang::RecordDecl* D)
{
  
   bool ret;
   
   // in case it is implicit we don't create a builder 
   if(D && D->isImplicit()){
      printf("\n\tImplicit declaration - we don't construct builder, VisitRecordDecl()");
      return true;
   }

   DumpDecl(D, "VisitRecordDecl()");

   Reflex::ClassBuilder* builder = GetClassBuilder(D); 	//template_parameters has a default value of ""
					// GetClassBuilder is declared in clr-scan.h
   if(builder!=NULL){ // Is it OK?
      fClassBuilders.push(builder);
      printf("\n\tPush()-ed a class builder; %d elements in the stack", (int) fClassBuilders.size());

      // I think this should be left as it is ?
      if (clang::CXXRecordDecl* C = dyn_cast <clang::CXXRecordDecl> (D)) {
         if (C->getDefinition() != NULL)                                                                                            
            for (clang::CXXRecordDecl::base_class_iterator I = C->bases_begin(), E = C->bases_end(); I != E; ++I) {
                                                                                                            
               Reflex::Type type = ExploreQualType(I->getType());
	       Reflex::OffsetFunction offset = NULL; // ?!
                                                                                                    
               unsigned int modifiers = VisibilityModifiers(I->getAccessSpecifier());

               if (I->isVirtual())
                  modifiers |= Reflex::VIRTUAL;

                  builder->AddBase(type, offset, modifiers);
                  printf("\n\tAddBase()-ed");
            }
      }
      ret = true;
      printf("\n\tReturning true ...");
   }
   else{
      ret = false;
      printf("\n\tReturning false ...");
   }
   
   return ret;
}

// This method visits an enumeration - if the enumaration is part of a class 
// the corresponding class builder is used; if the enumeration is standalone - 
// it uses the EnumerationBuilder from Reflex. 
bool TScanner::VisitEnumDecl(clang::EnumDecl* D)
{
   DumpDecl(D, "VisitEnumDecl()");

   bool ret = true;
   TString full_name = GetEnumName (D);
   unsigned int modifiers = Visibility(D); // ?!
   
   clang::DeclContext *ctx = D->getDeclContext();
   if (ctx->isRecord()){
	Reflex::ClassBuilder* builder = fClassBuilders.top();
	TString items;

	// should we do it that way ?!
        // Here we create a string with all the enum entries in the form of name=value couples
  	for (clang::EnumDecl::enumerator_iterator I = D->enumerator_begin(), E = D->enumerator_end(); I != E; ++I) {
      	   if (items != "")
           	items = items + ";";
           items = items + I->getNameAsString();;
           if (I->getInitExpr())
           	items += "=" + APIntToStr(I->getInitVal());
	}
	
	builder->AddEnum(full_name, items, &typeid(Reflex::UnknownType), modifiers); // typeid is actually
					// class_name::enum_name, at least according to genreflex output!
   }
   else{
	Reflex::EnumBuilder* builder =
      		new Reflex::EnumBuilder(full_name, typeid(Reflex::UnknownType), modifiers); // typeid is actually
					// ::enum_name, at least according to genreflex output!
        if(builder != NULL){ // Is it OK?

           builder->AddProperty(fgClangDeclKey, ToDeclProp(D));

           // shouldwe do it that way?!
           // Here we call the AddItem() method of the EnumBuilder for every enum entry
           for (clang::EnumDecl::enumerator_iterator I = D->enumerator_begin(), E = D->enumerator_end(); I != E; ++I) {
              std::string item_name = I->getNameAsString();
              long value = APIntToLong(I->getInitVal());      
                                                                                           
              builder->AddItem(item_name.c_str (), value);
           }
           ret = true;
           printf("\n\tReturning true ...");
        }
        else{
           ret = false;
           printf("\n\tReturning false ...");
        }
   }
   return ret;
}

// This method visits a varable - if the variable is part of a class 
// the corresponding class builder is used; if the variable is standalone - 
// it uses the VariableBuilder from Reflex
// Probably for the variables found in classes we should use the FieldDecl
// and to override the VisitFieldDecl() visitor function
bool TScanner::VisitVarDecl(clang::VarDecl* D)
{
   DumpDecl(D, "VisitVarDecl()");

   bool ret = true;

   Reflex::Type type = ExploreQualType(D->getType());
   std::string var_name;
   unsigned int modifiers = Visibility(D) | VarModifiers(D);
   int offset = 0;

   clang::DeclContext * ctx = D->getDeclContext();
   if (ctx->isRecord()){
	Reflex::ClassBuilder* builder = fClassBuilders.top();
	
	var_name = D->getNameAsString();

	/* What to do with this?
	if (! Reflex::Scope::ByName("").IsNamespace())
      	   ShowInfo("Bug");
	*/

	try {
      	   builder->AddDataMember(type, var_name.c_str (), offset, modifiers);
           builder->AddProperty(fgClangDeclKey, ToDeclProp(D));
   	} catch (std::exception& e) {
           ret = false;
      	   ShowReflexWarning(TString(e.what()) + " ... var member " + var_name, GetLocation(D));
           printf("\n\tError in adding data member to the class buider");
   	}
   }
   else{
	var_name = D->getQualifiedNameAsString();

	/* What to do with this?
	if (! Reflex::Scope::ByName("").IsNamespace())
      	   ShowInfo("Bug");
	*/

   	try {
      	   Reflex::VariableBuilder builder =
               Reflex::VariableBuilder(var_name.c_str (), type, offset, modifiers);

           builder.AddProperty(fgClangDeclKey, ToDeclProp(D));
   	} catch (std::exception& e) {
           ret = false; // Is it OK?
           ShowReflexWarning(TString(e.what()) + " ... variable " + var_name, GetLocation(D));
           printf("\n\tError in creating a VariableBuilder");
   	}

   }
   if(ret) printf("\n\tReturning true ...");
   else printf("\n\tReturning false ...");
   return ret;
}

bool TScanner::VisitFieldDecl(clang::FieldDecl* D)
{
   DumpDecl(D, "VisitFieldDecl()");

   bool ret = true;

   Reflex::Type type = ExploreQualType(D->getType());
   std::string var_name =  D->getNameAsString();;
   //unsigned int modifiers = Visibility(D) | VarModifiers(D);
   unsigned int modifiers = Visibility(D);
   int offset = 0;

   Reflex::ClassBuilder* builder = fClassBuilders.top();
	
   //var_name = D->getNameAsString();

	/* What to do with this?
	if (! Reflex::Scope::ByName("").IsNamespace())
      	   ShowInfo("Bug");
	*/

   try {
      builder->AddDataMember(type, var_name.c_str (), offset, modifiers);
      builder->AddProperty(fgClangDeclKey, ToDeclProp(D));
   } catch (std::exception& e) {
      ret = false;
      ShowReflexWarning(TString(e.what()) + " ... var member " + var_name, GetLocation(D));
      printf("\n\tError in adding data member to the class buider");
   }

   if(ret) printf("\n\tReturning true ...");
   else printf("\n\tReturning false ...");
   return ret;
}


// This method visits a function declaration - if the function is part of a class 
// the corresponding class builder is used; if the function is standalone - 
// it uses the FunctionBuilder from Reflex
bool TScanner::VisitFunctionDecl(clang::FunctionDecl* D)
{
   DumpDecl(D, "VisitFunctionDecl()");
   bool ret = true;

   Reflex::Type type = FuncType(D);
   TString name;
   std::string func_name = D->getQualifiedNameAsString() + FuncParameterList(D);

   TString params = FuncParameters(D);
   int modifiers = FuncModifiers(D);

   TContext* context = AllocateContext ();
   context->name = func_name;

   Reflex::StubFunction stub = ClrStubFunction;
   void* stub_ctx = context;

   clang::DeclContext * ctx = D->getDeclContext();

   if (ctx->isRecord()){
	Reflex::ClassBuilder* builder = fClassBuilders.top();  
     
   	name = D->getNameAsString();
	try {
	   builder->AddFunctionMember(type, name, stub, stub_ctx, params, modifiers);
   	   builder->AddProperty(fgClangDeclKey, ToDeclProp(D));
   	   builder->AddProperty(fgClangFuncKey, ToFuncProp (func_name));
	} catch (std::exception& e) {
           ret = false; // Is it OK? 
      	   ShowReflexWarning(TString(e.what()) + " ... function member " + func_name, GetLocation(D));
           printf("\n\tError in adding function member to the class builder");
   	}
   }
   else{
        name = D->getQualifiedNameAsString();

  	try {
      	   Reflex::FunctionBuilder builder =
         	Reflex::FunctionBuilder(type, name, stub, stub_ctx, params, modifiers);

           builder.AddProperty(fgClangDeclKey, ToDeclProp(D));
           builder.AddProperty(fgClangFuncKey, ToFuncProp(func_name));

   	} catch (std::exception& e) {
           ret = false; // Is it OK?
      	   ShowReflexWarning(TString(e.what()) + " ... function " + func_name, GetLocation(D));
           printf("\n\tError in creating the function builder");
   	}
   }
   if(ret) printf("\n\tReturning true ...");
   else printf("\n\tReturning false ...");
   return ret;
}
                                                                          
bool TScanner::TraverseDeclContextHelper(DeclContext *DC)
{
   
   bool ret=true;
   
 
   if (!DC)
     return true;

   clang::Decl* D = dyn_cast<clang::Decl>(DC);
   // skip implicit decls
   if (D && D->isImplicit()){
      printf("\nDEBUG - Implicit declaration in TraverseDeclContextHelper()");
      return true;
   }

   DumpDecl(D, "TraverseDeclContextHelper()");

   for (DeclContext::decl_iterator Child = DC->decls_begin(), ChildEnd = DC->decls_end(); 
        ret && (Child != ChildEnd); ++Child) {
      
      ret=TraverseDecl(*Child);
     
   }
 
   RemoveBuilder(DC); 
   return ret;
   
}


void TScanner::RemoveBuilder(clang::DeclContext *DC){
   if(DC->isRecord()){

        //printf("\nDEBUG - Removing the builder");
	TString className = getClassName(DC);
        if(className.Length() > 0)
	  printf("\nDEBUG - removing buider for %s", className.Data());
        else 
           printf("\nDEBUG - removing a builder");
        printf("\n\tWe have %d elements left in the stack",(int) fClassBuilders.size());

	fClassBuilders.pop();
   }
}
TString TScanner::getClassName(clang::DeclContext* DC){
   
   clang::NamedDecl* N=dyn_cast<clang::NamedDecl>(DC);
   TString ret;
   if(N && (N->getIdentifier()!=NULL))
      ret = N->getNameAsString().c_str();
   return ret;
}

void TScanner::DumpDecl(clang::Decl* D, const char* msg) {
   std::string name;
   
   if (!D) {
      printf("\nDEBUG - DECL is NULL: %s", msg);
      return;
   }

   //clang::DeclContext *ctx = D->getDeclContext();

   //if(ctx && (ctx->isRecord() || ctx->isFunctionOrMethod())){
      clang::NamedDecl* N = dyn_cast<clang::NamedDecl> (D);
      if(N && N->getIdentifier()) name = N->getNameAsString();
      //}
      else name = "UNNAMED";
      
   /*
   clang::NamedDecl* N = dyn_cast<clang::NamedDecl> (D);
   printf("\nDecl %s (%s): %s",
          N->getIdentifier() ? N->getNameAsCString() : "(UNNAMED)",
          N->getDeclKindName(),
          msg);
   */
   printf("\nDEBUG - Decl %s of kind %s: %s", name.c_str(), D->getDeclKindName(), msg);
}

//______________________________________________________________________________
void TScanner::Scan(clang::ASTContext* C, clang::Decl* D)
{
   fCtx = C;
   printf("\nDEBUG from Velislava - into the Scan() function!!!");
   TraverseDecl(D);
}



