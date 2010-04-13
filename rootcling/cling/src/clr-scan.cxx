
/* clr-scan.cc */

#include "clr-scan.h"
#include "clr-util.h"

#include "clr-print.h"

// #define DUMP
// #define LIST

#ifdef DUMP
   #include "clr-dump.h"
#endif

#ifdef LIST
   #include "clr-list.h"
#endif

#ifdef TRACE
   #include "clr-trace.h"
#endif

#include "clang/AST/ASTConsumer.h"

#include <Reflex/Reflex.h>
#include <Reflex/Builder/ReflexBuilder.h>
#include <Reflex/DictionaryGenerator.h>

#define TEMPLATES

/* ---------------------------------------------------------------------- */

using std::string;

using clr_util::info;
using clr_util::warning;
using clr_util::error;

using clr_util::RemoveSuffix;

/* ---------------------------------------------------------------------- */

/* global variables */

// context from ClrInit
clang::ASTContext * Ctx = NULL;

// Reflex class builders
// clr_util::Dictionary < string, Reflex::ClassBuilder * > class_builders;

// property key used for connection with Clang objects
const char * clang_key = "clang";

int unknown_class_counter = 0;
int unknown_enum_counter = 0;

/* ---------------------------------------------------------------------- */

/* forward declarations */

Reflex::Type ScanQualType (clang::QualType qual_type);

void ScanTypedef (clang::TypedefDecl * D);
void ScanEnum (clang::EnumDecl * D);

void ScanClass (clang::RecordDecl * D, string template_params = "");
void ScanFunction (clang::FunctionDecl * D, string template_params = "");

void ScanDecl (clang::Decl * D, Reflex::Scope Outer);

/* ---------------------------------------------------------------------- */

/* initialization */

void init_dict_variables ()
{
   // class_builders.clear ();
   unknown_class_counter = 0;
   unknown_enum_counter = 0;
}

/* conversion and type check used by AddProperty */

inline void * to_prop_item (clang::Decl * item)
{
   return item;
}

/********************************* NUMBERS ********************************/

inline size_t APInt_to_size (const llvm::APInt & num)
{
   return * num.getRawData ();
}

inline long APInt_to_long (const llvm::APInt & num)
{
   return * num.getRawData ();
}

inline string APInt_to_string (const llvm::APInt & num)
{
   return num.toString (10, true);
}

inline string IntToStr (int num)
{
   std::ostringstream stream;
   stream << num;
   return stream.str ();
}

/******************************** UNKNOWN *********************************/

string GetLocation (clang::Decl * D)
{
   string location = "";
   llvm::raw_string_ostream stream (location);
   clang::SourceManager & source_manager = Ctx->getSourceManager ();
   D->getLocation ().print (stream, source_manager);
   return stream.str ();
}

string GetName (clang::Decl * D)
{
   string name = "";
   string kind = D->getDeclKindName ();

   if (clang::NamedDecl * ND = dyn_cast <clang::NamedDecl> (D))
   {
      name = ND->getQualifiedNameAsString ();
   }

   return name;
}

inline string AddSpace (const string txt)
{
   if (txt == "")
      return "";
   else
      return txt + " ";
}

/* ---------------------------------------------------------------------- */

void ScanInfo (clang::Decl * D)
{
   string location = GetLocation (D);
   string kind = D->getDeclKindName ();
   string name = GetName (D);
   info ("Scan: " + kind + " declaration " + name, location);
}

void ScanUnknown (clang::Decl * D, string txt = "")
// unknown - this kind of declaration was not known to programmer
{
   string location = GetLocation (D);
   string kind = D->getDeclKindName ();
   string name = GetName (D);
   warning ("Unknown " + AddSpace (txt) + kind + " declaration " + name, location);
}

void ScanUnexpected (clang::Decl * D, string txt = "")
// unexpected - this kind of declaration is unexpected (in this place)
{
   string location = GetLocation (D);
   string kind = D->getDeclKindName ();
   string name = GetName (D);
   warning ("Unexpected " + kind + " declaration " + name, location);
}

void ScanUnsupported (clang::Decl * D, string txt = "")
// unsupported - this kind of declaration is probably not used
{
   string location = GetLocation (D);
   string kind = D->getDeclKindName ();
   string name = GetName (D);
   warning ("Unsupported " + AddSpace (txt) + kind + " declaration " + name, location);
}

void ScanUnimplemented (clang::Decl * D, string txt = "")
// to be implemented
{
   string location = GetLocation (D);
   string kind = D->getDeclKindName ();
   string name = GetName (D);
   warning ("Unimplemented " + AddSpace (txt) + kind + " declaration " + name, location);
}

/* ---------------------------------------------------------------------- */

void ScanUnknownType (clang::QualType qual_type)
{
   string kind = qual_type.getTypePtr ()->getTypeClassName ();
   warning ("Unknown " + kind + " type " + qual_type.getAsString ());
}

void ScanUnsupportedType (clang::QualType qual_type)
{
   string kind = qual_type.getTypePtr ()->getTypeClassName ();
   warning ("Unsupported" + kind + " type " + qual_type.getAsString ());
}

void ScanUnimplementedType (clang::QualType qual_type)
{
   string kind = qual_type.getTypePtr ()->getTypeClassName ();
   warning ("Unimplemented " + kind + " type " + qual_type.getAsString ());
}

/*
void ScanUnimplementedType (clang::Type * T)
{
   string kind = T->getTypeClassName ();
   warning ("Unimplemented " + kind + " type");
}
*/

/***************************** CLASS BUILDER ******************************/

Reflex::ClassBuilder * GetClassBuilder (clang::RecordDecl * D, string template_params = "")
{
   string full_name = D->getQualifiedNameAsString () + template_params;
   // info ("GetClassBuilder " + full_name);

   if (full_name == "")
   {
      unknown_class_counter ++;
      full_name = "_UNKNOWN_CLASS_" + IntToStr (unknown_class_counter) + "_"; // !?
   }

   Reflex::ClassBuilder * builder = NULL; // class_builders.get (full_name);
   if (builder == NULL)
   {
      clang::TagDecl::TagKind kind = D->getTagKind ();

      int style = 0;

      if (kind == clang::TagDecl::TK_class)
         style = Reflex::CLASS;
      else if (kind == clang::TagDecl::TK_struct)
         style = Reflex::STRUCT;
      else if (kind == clang::TagDecl::TK_union)
         style = Reflex::UNION;

      style = style | Reflex::PUBLIC; // !?

      try
      {
         builder = new Reflex::ClassBuilder (full_name.c_str (), typeid (Reflex::UnknownType), 0, style);
      }
      catch (std::exception & e)
      {
         warning (string (e.what ()) + " : class " + full_name);
         unknown_class_counter ++;
         full_name = "_UNKNOWN_CLASS_" + IntToStr (unknown_class_counter) + "_"; // !?
         builder = new Reflex::ClassBuilder (full_name.c_str (), typeid (Reflex::UnknownType), 0, Reflex::CLASS); // !?
      }

      builder->AddProperty (clang_key, to_prop_item (D));

      // class_builders.store (full_name, builder);
   }

   return builder;
}

/********************************** TYPE **********************************/

Reflex::Type ScanPointerType (clang::PointerType * T)
{
   Reflex::Type element = ScanQualType (T->getPointeeType ());
   return PointerBuilder (element);
}

Reflex::Type ScanLValueReferenceType (clang::LValueReferenceType * T)
{
   Reflex::Type element = ScanQualType (T->getPointeeType ());
   return ReferenceBuilder (element);
}

Reflex::Type ScanMemberPointerType (clang::MemberPointerType * T)
{
   Reflex::Type element = ScanQualType (T->getPointeeType ());

   Reflex::Scope base;

   const clang::Type * cls_type = T->getClass ();
   if (cls_type != NULL)
   {
      const clang::RecordType * rec_type = dyn_cast <clang::RecordType> (cls_type);
      if (rec_type != NULL)
      {
         clang::RecordDecl * rec_decl = rec_type->getDecl ();

         if (rec_decl != NULL)
         {
             string rec_name = rec_decl->getQualifiedNameAsString ();
             base = Reflex::Scope::ByName (rec_name);

             // info ("ScanMemberPointerType " + base.Name () + " " + element.Name ());

             return PointerToMemberBuilder (element, base);
          }
      }
   }

   // return PointerBuilder (element); // !? !!
   return Reflex::Type (); // !? !!
}

/* ---------------------------------------------------------------------- */

Reflex::Type ScanArrayType (clang::ArrayType * T)
{
   Reflex::Type element = ScanQualType (T->getElementType ());

   size_t size = 0;
   if (clang::ConstantArrayType * CT = dyn_cast <clang::ConstantArrayType> (T))
   {
      size = APInt_to_size (CT->getSize ()) ;
   }
   else
   {
      warning ("Unknown array size"); // !?
   }

   return ArrayBuilder (element, size);
}

/* ---------------------------------------------------------------------- */

Reflex::Type ScanFunctionType (clang::FunctionType * T)
{
   // parameters
   std::vector <Reflex::Type> vec;

   if (clang::FunctionProtoType * FT = dyn_cast <clang::FunctionProtoType> (T))
   {
      for (clang::FunctionProtoType::arg_type_iterator I = FT->arg_type_begin (), E = FT->arg_type_end (); I != E; ++I)
      {
          Reflex::Type param_type = ScanQualType (*I);
          vec.push_back (param_type);
      }
   }

   // result type
   Reflex::Type type = ScanQualType (T->getResultType ());

   return FunctionTypeBuilder (type, vec);
}

/* ---------------------------------------------------------------------- */

Reflex::Type ScanEnumType (clang::EnumType * T)
{
   clang::EnumDecl * D = T->getDecl ();
   string full_name = D->getQualifiedNameAsString ();

   if (full_name == "")
   {
      unknown_enum_counter ++;
      full_name = "_UNKNOWN_ENUM_" + IntToStr (unknown_enum_counter) + "_"; // !?
   }

   string items = "";
   bool any = false;

   for (clang::EnumDecl::enumerator_iterator I = D->enumerator_begin (), E = D->enumerator_end (); I != E; ++I)
   {
      clang::EnumConstantDecl * item = * I;

      if (! any)
         items += ",";
      any = true;

      items += item->getNameAsString ();

      if (item->getInitExpr () != NULL)
         items += "=" + APInt_to_string (item->getInitVal ());
   }

   return Reflex::EnumTypeBuilder (full_name.c_str (), items.c_str ());
}

/* ---------------------------------------------------------------------- */

Reflex::Type ScanRecordType (clang::RecordType * T)
{
   Reflex::ClassBuilder * builder = GetClassBuilder (T->getDecl ()); // !?
   return builder->ToType ();
}

/* ---------------------------------------------------------------------- */

Reflex::Type ScanElaboratedType (clang::ElaboratedType * T)
{
   return ScanQualType (T->getUnderlyingType ());
}

/* ---------------------------------------------------------------------- */

Reflex::Type ScanTypedefType (clang::TypedefType * T)
{
    clang::TypedefDecl * D = T->getDecl ();
    string name = D->getQualifiedNameAsString ();

    // !? !!
    return Reflex::TypeBuilder (name.c_str ());

    Reflex::Type type = ScanQualType (D->getUnderlyingType ());

    return Reflex::TypedefTypeBuilder (name.c_str (), type);
}

/* ---------------------------------------------------------------------- */

Reflex::Type ScanBuiltinType (clang::BuiltinType * T)
{
   clang::LangOptions lang_opts; // !?
   const char * name = T->getName (lang_opts);
   // info ("BuiltinType " + string (name));
   return Reflex::TypeBuilder (name);
}

Reflex::Type ScanQualifiedNameType (clang::QualifiedNameType * T)
{
   #if 0
   clang::IdentifierInfo * N = T->getQualifier ()->getAsIdentifier();
   const char * name = N->getNameStart ();
   info ("QualifiedNameType " + string (name));
   return Reflex::TypeBuilder (name);
   #endif

   #if 1
   string name = T->getNamedType ().getAsString (); // !?
   // info ("QualifiedNameType " + name);
   return Reflex::TypeBuilder (name.c_str ());
   #endif
}

Reflex::Type ScanTemplateTypeParmType (clang::TemplateTypeParmType * T)
{
   clang::IdentifierInfo * N = T->getName ();
   const char * name = N->getNameStart ();

   // info ("TemplateTypeParmType " + string (name));
   return Reflex::TypeBuilder (name);
}

Reflex::Type ScanSubstTemplateTypeParmType (clang::SubstTemplateTypeParmType * T)
{
   const char * name = T->getName ()->getNameStart ();
   // info ("SubstTemplateTypeParmType " + string (name));
   return Reflex::TypeBuilder (name);
}

/* ---------------------------------------------------------------------- */

string ConvTemplateName (clang::TemplateName & N)
{
   clang::LangOptions lang_opts;
   clang::PrintingPolicy print_opts (lang_opts); // !?

   string text = "";
   llvm::raw_string_ostream stream (text);

   N.print (stream, print_opts);

   return stream.str ();
}

Reflex::Type ScanTemplateSpecializationType (clang::TemplateSpecializationType * T)
{
   clang::TemplateName N = T->getTemplateName ();
   string name = ConvTemplateName (N);

   // info ("TemplateSpecializationType " + name);
   return Reflex::TypeBuilder (name.c_str ());
}

/* ---------------------------------------------------------------------- */

Reflex::Type ScanQualType (clang::QualType qual_type)
{
   Reflex::Type type;
   clang::Type * T = qual_type.getTypePtr ();

   if (clang::PointerType * S = dyn_cast <clang::PointerType> (T))
   {
       type = ScanPointerType (S);
   }
   else if (clang::MemberPointerType * S = dyn_cast <clang::MemberPointerType> (T))
   {
       type = ScanMemberPointerType (S);
   }
   else if (clang::LValueReferenceType * S = dyn_cast <clang::LValueReferenceType> (T))
   {
       type = ScanLValueReferenceType (S);
   }
   else if (clang::RValueReferenceType * S = dyn_cast <clang::RValueReferenceType> (T))
   {
       /* ... */
       ScanUnsupportedType (qual_type);
   }
   else if (clang::ArrayType * S = dyn_cast <clang::ArrayType> (T))
   {
       type = ScanArrayType (S);
   }
   else if (clang::EnumType * S = dyn_cast <clang::EnumType> (T))
   {
       type = ScanEnumType (S);
   }
   else if (clang::RecordType * S = dyn_cast <clang::RecordType> (T))
   {
       type = ScanRecordType (S);
   }
   else if (clang::ElaboratedType * S = dyn_cast <clang::ElaboratedType> (T))
   {
       type = ScanElaboratedType (S);
   }
   else if (clang::FunctionType * S = dyn_cast <clang::FunctionType> (T))
   {
       type = ScanFunctionType (S);
   }
   else if (clang::TypedefType * S = dyn_cast <clang::TypedefType> (T))
   {
       type = ScanTypedefType (S);
   }
   else if (clang::BuiltinType * S = dyn_cast <clang::BuiltinType> (T))
   {
       type = ScanBuiltinType (S);
   }
   else if (clang::ComplexType * S = dyn_cast <clang::ComplexType> (T))
   {
       /* ... */
       ScanUnsupportedType (qual_type);
   }
   else if (clang::QualifiedNameType * S = dyn_cast <clang::QualifiedNameType> (T))
   {
       ScanQualifiedNameType (S);
   }
   else if (clang::SubstTemplateTypeParmType * S = dyn_cast <clang::SubstTemplateTypeParmType> (T))
   {
       ScanSubstTemplateTypeParmType (S);
   }
   else if (clang::TemplateSpecializationType * S = dyn_cast <clang::TemplateSpecializationType> (T))
   {
       ScanTemplateSpecializationType (S);
   }
   else if (clang::TemplateTypeParmType * S = dyn_cast <clang::TemplateTypeParmType> (T))
   {
       ScanTemplateTypeParmType (S);
   }
   else
   {
       ScanUnknownType (qual_type);
   }

   // const and volatile

   if (qual_type.isConstQualified ())
      type = ConstBuilder (type);

   if (qual_type.isVolatileQualified ())
      type = VolatileBuilder (type);

   return type;
}

/******************************* EXPRESSION *******************************/

#if 0
string ExprToStr (clang::Expr * expr)
{
   string result = "";

   if (expr->isEvaluatable (*Ctx))
   {
      clang::Expr::EvalResult answer;
      expr->Evaluate (answer, *Ctx);

      if (answer.Val.isInt ())
      {
         result = APInt_to_string (answer.Val.getInt ());
      }
      else
      {
         string text = "";
         llvm::raw_string_ostream stream (text);
         answer.Val.print (stream);
         result = stream.str ();
      }
   }
   else
   {
      result = "unknown_value"; // !? !!
   }

   return result;
}
#endif

string ExprToStr (clang::Expr * expr)
{
   clang::LangOptions lang_opts;
   clang::PrintingPolicy print_opts (lang_opts); // !?

   string text = "";
   llvm::raw_string_ostream stream (text);

   expr->printPretty (stream, NULL, print_opts);

   return stream.str ();
}

/******************************** TEMPLATE *********************************/

string ConvTemplateParameterList (clang::TemplateParameterList * list)
{
   string result = "";
   bool any = false;

   for (clang::TemplateParameterList::iterator I = list->begin(), E = list->end(); I != E; ++I)
   {
      if (any)
         result += ",";
      any = true;

      if (clang::TemplateTemplateParmDecl * P = dyn_cast <clang::TemplateTemplateParmDecl> (*I))
      {
         ScanUnimplemented (*I, "template template parameter");
      }
      else if (clang::TemplateTypeParmDecl * P = dyn_cast <clang::TemplateTypeParmDecl> (*I))
      {
         if (P->wasDeclaredWithTypename ())
            result += "typename ";
         else
            result += "class ";

         if (P->isParameterPack ())
            result += "... ";

         result += P->getNameAsString ();
      }
      else if (clang::NonTypeTemplateParmDecl * P = dyn_cast <clang::NonTypeTemplateParmDecl> (*I))
      {
         result += P->getType().getAsString ();

         if (clang::IdentifierInfo * N = P->getIdentifier())
         {
            result += " ";
            result += N->getName();
         }

         if (P->hasDefaultArgument ())
            result += " = " + ExprToStr (P->getDefaultArgument ());
      }
      else
      {
         ScanUnknown (*I, "template parameter");
      }
   }

   // info ("template parameters <" + result + ">");

   return "<" + result + ">";
}

string ScanTemplateParams (clang::TemplateDecl * D)
{
   return ConvTemplateParameterList (D->getTemplateParameters ());
}

#if 0
string ConvTemplateArguments (const clang::TemplateArgumentList & list)
{
   string result = "";
   bool any = false;

   unsigned list_size = list.size ();
   for (int inx = 0; inx < list_size; inx ++)
   {
      if (any)
         result += ",";
      any = true;

      clang::TemplateArgument D = list.get (inx);

      switch (D.getKind ())
      {
         case clang::TemplateArgument::Type:
            result += D.getAsType ().getAsString ();
            break;

         case clang::TemplateArgument::Declaration:
            if (clang::NamedDecl * N = dyn_cast <clang::NamedDecl> (D.getAsDecl ()))
               result += N->getNameAsString();
            break;

         case clang::TemplateArgument::Integral:
            result += APInt_to_string (*D.getAsIntegral());
            break;

         case clang::TemplateArgument::Template:
            {
               clang::LangOptions lang_opts;
               clang::PrintingPolicy print_opts (lang_opts); // !?
               string text = "";
               llvm::raw_string_ostream stream (text);
               D.getAsTemplate ().print (stream, print_opts);
               result += stream.str ();
            }
            break;

         case clang::TemplateArgument::Expression:
            {
               result += ExprToStr (D.getAsExpr ());
            }
            break;

         case clang::TemplateArgument::Pack:
            result += "..."; // !?
            break;

         default:
            warning ("Unknown template argument");
      }
   }

   return "<" + result + ">";
}
#endif

#if 1
string ConvTemplateArguments (const clang::TemplateArgumentList & list)
{
   clang::LangOptions lang_opts;
   clang::PrintingPolicy print_opts (lang_opts); // !?
   return clang::TemplateSpecializationType::PrintTemplateArgumentList
             (list.getFlatArgumentList (), list.flat_size (), print_opts);
}

#endif

/* ---------------------------------------------------------------------- */

void ScanClassTemplate (clang::ClassTemplateDecl * D)
{
   #ifdef TEMPLATES
   // info ("ScanClassTemplate " + D->getQualifiedNameAsString ());
   string template_params = ScanTemplateParams (D);
   ScanClass (D->getTemplatedDecl (), template_params);
   #endif

   #if 0
   typedef llvm::FoldingSet < clang::ClassTemplateSpecializationDecl > container_t;
   container_t & list = D->getSpecializations ();
   for (container_t::iterator I = list.begin(), E = list.end(); I != E; ++I)
   {
      clang::ClassTemplateSpecializationDecl & S = *I;
      info (S.getSpecializedTemplate ()->getQualifiedNameAsString ());
   }
   #endif
}

void ScanFunctionTemplate (clang::FunctionTemplateDecl * D)
{
   #ifdef TEMPLATES
   // info ("ScanFunctionTemplate");
   string template_params = ScanTemplateParams (D);
   ScanFunction (D->getTemplatedDecl (), template_params);
   #else
      ScanUnimplemented (D);
   #endif
}

#if 0
void ScanTemplateParm (clang::TemplateTemplateParmDecl * D)
{
   ScanUnimplemented (D, "template parameter");
}

void ScanTemplateTypeParmDecl (clang::TemplateTypeParmDecl *D)
{
   ScanUnimplemented (D, "template type parameter");
}
#endif

/******************************** FUNCTION ********************************/

string FuncParameters (clang::FunctionDecl * D)
{
   string result = "";

   for (clang::FunctionDecl::param_iterator I = D->param_begin (), E = D->param_end (); I != E; ++I)
   {
      clang::ParmVarDecl * P = * I;

      if (result != "")
         result += ";";  // semicolon, not comma, important

      string type = P->getType().getAsString ();
      string name = P->getNameAsString ();

      result += type + " " + name;

      // NO if (P->hasDefaultArg ()) // check hasUnparsedDefaultArg () and hasUninstantiatedDefaultArg ()
      if (P->getInit ())
      {
         string init_value = ExprToStr (P->getDefaultArg ());
         result += "=" + init_value;
      }
   }

   return result;
}

Reflex::Type FuncType (clang::FunctionDecl * D)
{
   std::vector <Reflex::Type> vec;

   for (clang::FunctionDecl::param_iterator I = D->param_begin (), E = D->param_end (); I != E; ++I)
   {
      clang::ParmVarDecl * P = * I;
      Reflex::Type type = ScanQualType (P->getType ());
      vec.push_back (type);
   }

   Reflex::Type type = ScanQualType (D->getResultType());

   return Reflex::FunctionTypeBuilder (type, vec);
}

unsigned int FuncModifiers (clang::FunctionDecl * D)
{
   unsigned int modifiers = 0;

   switch (D->getStorageClass ())
   {
       case clang::FunctionDecl::Extern:
          modifiers |= Reflex::EXTERN;
          break;

       case clang::FunctionDecl::Static:
          modifiers |= Reflex::STATIC;
          break;

       case clang::VarDecl::PrivateExtern:
          // !?
          break;
   }

   if (D->isPure ())
       modifiers |= Reflex::ABSTRACT;

   if (D->isInlineSpecified ())
       modifiers |= Reflex::INLINE;

   if (D->isInlined ()) // !?
       modifiers |= Reflex::INLINE;

   if (clang::CXXMethodDecl * M = dyn_cast <clang::CXXMethodDecl> (D))
   {
       if (M->isVirtual ())
          modifiers |= Reflex::VIRTUAL;

       if (M->isStatic ())
          modifiers |= Reflex::STATIC;

       if (clang::CXXConstructorDecl * S = dyn_cast <clang::CXXConstructorDecl> (M))
       {
           modifiers |= Reflex::CONSTRUCTOR;

           if (S->isCopyConstructor ())
              modifiers |= Reflex::COPYCONSTRUCTOR;

           if (S->isExplicit ())
              modifiers |= Reflex::EXPLICIT;

           if (S->isConvertingConstructor (false)) // !? parameter allow explicit
              modifiers |= Reflex::CONVERTER;
       }
       else if (clang::CXXDestructorDecl * S = dyn_cast <clang::CXXDestructorDecl> (M))
       {
           modifiers |= Reflex::DESTRUCTOR;
       }
       else if (clang::CXXConversionDecl * S = dyn_cast <clang::CXXConversionDecl> (M))
       {
           modifiers |= Reflex::CONVERTER;

           if (S->isExplicit ())
              modifiers |= Reflex::EXPLICIT;
       }
   }

   return modifiers;
}

/********************************* CLASS **********************************/

unsigned int VisibilityModifiers (clang::AccessSpecifier access)
{
   unsigned int modifiers = 0;

   switch (access)
   {
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

unsigned int Visibility (clang::Decl * D)
{
   return VisibilityModifiers (D->getAccess ());
}

unsigned int VarModifiers (clang::VarDecl * D)
{
   unsigned int modifiers = 0;

   switch (D->getStorageClass ())
   {
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

   if (D->isStaticDataMember())
      modifiers |= Reflex::STATIC; // !?

   return modifiers;
}

/* ---------------------------------------------------------------------- */

void ScanFieldMember (clang::FieldDecl * D, Reflex::ClassBuilder * builder)
{
   // type

   Reflex::Type type = ScanQualType (D->getType());

   // name

   string name = D->getNameAsString ();

   // modifiers

   int modifiers = Visibility (D);

   if (D->isMutable ())
      modifiers |= Reflex::MUTABLE;

   // offset

   int offset = 0; // !?

   // add item

   try
   {
      builder->AddDataMember (type, name.c_str (), offset, modifiers);
      builder->AddProperty (clang_key, to_prop_item (D));
   }
   catch (std::exception & e)
   {
      warning (string (e.what ()) + " : field " + name);
   }
}

void ScanVarMember (clang::VarDecl * D, Reflex::ClassBuilder * builder)
{
   // type

   Reflex::Type type = ScanQualType (D->getType());

   // name

   string name = D->getNameAsString ();

   // modifiers

   int modifiers = Visibility (D) | VarModifiers (D);

   // offset

   int offset = 0; // !?

   // add item

   try
   {
      builder->AddDataMember (type, name.c_str (), offset, modifiers);
      builder->AddProperty (clang_key, to_prop_item (D));
   }
   catch (std::exception & e)
   {
      warning (string (e.what ()) + " : var member " + name);
   }
}

void ScanMethod (clang::CXXMethodDecl * D, Reflex::ClassBuilder * builder, string template_params = "")
{
   // type

   Reflex::Type type = FuncType (D);

   // paramers

   string params = FuncParameters (D);

   // modifiers

   int modifiers = Visibility (D) | FuncModifiers (D);

   // stub

   Reflex::StubFunction stub = NULL; // !?
   void * stub_ctx = NULL;

   // add item

   string name = D->getNameAsString () + template_params;
   builder->AddFunctionMember (type, name.c_str (), stub, stub_ctx, params.c_str(), modifiers);
   builder->AddProperty (clang_key, to_prop_item (D));
}

void ScanEnumMember (clang::EnumDecl * D, Reflex::ClassBuilder * builder)
{
   // name

   string full_name = D->getQualifiedNameAsString ();

   // modifiers

   unsigned int modifiers = Visibility (D); // !?

   // items

   string items = "";
   for (clang::EnumDecl::enumerator_iterator I = D->enumerator_begin (), E = D->enumerator_end (); I != E; ++I)
   {
      string item_name = I->getNameAsString ();
      long value = APInt_to_long (I->getInitVal ()); // !?
      if (items != "")
         items = items + ";";
      items = items + item_name ;  // !? + "=" + value;
   }

   // add item

   builder->AddEnum (full_name.c_str (), items.c_str(), & typeid (Reflex::UnknownType), modifiers);

   // no property
}

void ScanTypedefMember (clang::TypedefDecl * D, Reflex::ClassBuilder * builder)
{
   // !! !?
   return;

   // type

   Reflex::Type type = ScanQualType (D->getUnderlyingType ());

   // name

   string name = D->getNameAsString ();

   // add item

   info ("AddTypedef " + name);

   builder->AddTypedef (type, name.c_str ());

   // builder->AddProperty (clang_key, to_prop_item (D));
}

void ScanFunctionTemplateMember (clang::FunctionTemplateDecl * D, Reflex::ClassBuilder * builder)
{
   #ifdef TEMPLATES
   string template_params = ScanTemplateParams (D);

   clang::FunctionDecl * F = D->getTemplatedDecl ();

   if (clang::CXXMethodDecl * S = dyn_cast <clang::CXXMethodDecl> (F))
      ScanMethod (S, builder, template_params);
   else
      // ScanFunction (F, template_params); // !?
      ScanFunction (F); // !?
   #else
      ScanUnimplemented (D);
   #endif
}

void ScanFriendMember (clang::FriendDecl *D, Reflex::ClassBuilder * builder)
{
   ScanUnimplemented (D, "friend member");
}

/* ---------------------------------------------------------------------- */

void ScanMember (clang::Decl * D, Reflex::ClassBuilder * builder)
{
   if (clang::FieldDecl * S = dyn_cast <clang::FieldDecl> (D))
   {
      ScanFieldMember (S, builder);
   }
   else if (clang::VarDecl * S = dyn_cast <clang::VarDecl> (D))
   {
      ScanVarMember (S, builder);
   }
   else if (clang::CXXMethodDecl * S = dyn_cast <clang::CXXMethodDecl> (D))
   {
      ScanMethod (S, builder);
   }
   else if (clang::TypedefDecl * S = dyn_cast <clang::TypedefDecl> (D)) // before TypeDecl
   {
      ScanTypedefMember (S, builder);
   }
   else if (clang::EnumDecl * S = dyn_cast <clang::EnumDecl> (D)) // before TagDecl and TypeDecl
   {
      ScanEnumMember (S, builder);
   }
   else if (clang::RecordDecl * S = dyn_cast <clang::RecordDecl> (D))
   {
      ScanClass (S);
   }
   else if (clang::FriendDecl * S = dyn_cast <clang::FriendDecl> (D))
   {
      ScanFriendMember (S, builder);
   }
   else if (clang::ClassTemplateDecl * S = dyn_cast <clang::ClassTemplateDecl> (D))
   {
      ScanClassTemplate (S);
   }
   else if (clang::FunctionTemplateDecl * S = dyn_cast <clang::FunctionTemplateDecl> (D))
   {
      ScanFunctionTemplateMember (S, builder);
   }
   else
   {
      ScanUnknown (D, "member");
   }
}

void ScanClass (clang::RecordDecl * D, string template_params)
{
   if (clang::CXXRecordDecl * C = dyn_cast <clang::CXXRecordDecl> (D))
   {
      clang::ClassTemplateDecl * template_decl = C->getDescribedClassTemplate ();
      if (template_decl != NULL)
      {
         // info ("getDescribedClassTemplate");
         template_params += ScanTemplateParams (template_decl); // !? what if already has some template parameters
      }
   }

   if (clang::ClassTemplatePartialSpecializationDecl * P = dyn_cast <clang::ClassTemplatePartialSpecializationDecl> (D))
   {
      template_params += ConvTemplateParameterList (P->getTemplateParameters ());
   }
   else if (clang::ClassTemplateSpecializationDecl * S = dyn_cast <clang::ClassTemplateSpecializationDecl> (D))
   {
      #if 1
         template_params += ConvTemplateArguments (S->getTemplateArgs ()); // !? !!
      #endif
      if (template_params == "")
         template_params = "< >"; // specialization, but not partial specializatiion
   }

   Reflex::ClassBuilder * builder = GetClassBuilder (D, template_params);

   if (clang::CXXRecordDecl * C = dyn_cast <clang::CXXRecordDecl> (D))
   {
      if (C->getDefinition () != NULL) // important
         for (clang::CXXRecordDecl::base_class_iterator I = C->bases_begin (), E = C->bases_end (); I != E; ++I)
         {
             // clang::CXXBaseSpecifier  b = *I;

             Reflex::Type type = ScanQualType (I->getType ());

             Reflex::OffsetFunction offset = NULL; // !?

             unsigned int modifiers = VisibilityModifiers (I->getAccessSpecifier ());

             if (I->isVirtual ())
                modifiers |= Reflex::VIRTUAL;

             builder->AddBase (type, offset, modifiers);
         }
   }

   for (clang::DeclContext::decl_iterator I = D->decls_begin (), E = D->decls_end (); I != E; ++I)
      ScanMember (*I, builder);

   // return builder->ToType ();
}

/********************************** ENUM **********************************/

void ScanEnum (clang::EnumDecl * D)
{
   // name

   string full_name = D->getQualifiedNameAsString ();

   if (full_name == "")
      full_name = "_UNKNOWN_ENUM_"; // !?

   // modifiers

   unsigned int modifiers = Visibility (D); // !?

   // add item

   Reflex::EnumBuilder * builder =
       new Reflex::EnumBuilder (full_name.c_str (), typeid (Reflex::UnknownType), modifiers);

   builder->AddProperty (clang_key, to_prop_item (D));

   // items

   for (clang::EnumDecl::enumerator_iterator I = D->enumerator_begin (), E = D->enumerator_end (); I != E; ++I)
   {
      string item_name = I->getNameAsString ();
      long value = APInt_to_long (I->getInitVal ()); // !?
      builder->AddItem (item_name.c_str (), value);
   }
}

/********************************* USING **********************************/

void ScanUsingDecl (clang::UsingDecl * D, Reflex::Scope Outer)
{
   ScanUnimplemented (D);
   #if 0
   clang::IdentifierInfo * N = D->getTargetNestedNameDecl ()->getAsIdentifier ();
   Reflex::Scope Inner = Reflex::Scope::ByName (N->getName ());
   Outer.AddUsingDirective (Inner);
   #endif
}

void ScanUsingShadowDecl (clang::UsingShadowDecl * D, Reflex::Scope Outer)
{
   ScanUnimplemented (D);
}

/********************************* GLOBAL **********************************/

void ScanTranslationUnit (clang::TranslationUnitDecl * D, Reflex::Scope Outer)
{
     clang::DeclContext * DC = clang::TranslationUnitDecl::castToDeclContext (D);
     for (clang::DeclContext::decl_iterator I = DC->decls_begin(), E = DC->decls_end (); I != E; ++I )
     {
        ScanDecl (*I, Outer);
     }
}

void ScanNamespace (clang::NamespaceDecl * D)
{
   string full_name = D->getQualifiedNameAsString ();

   Reflex::NamespaceBuilder * builder =
      new Reflex::NamespaceBuilder (full_name.c_str ());

   builder->AddProperty (clang_key, to_prop_item (D));

   if (clang::DeclContext * DC = dyn_cast <clang::DeclContext> (D))
   {
      for (clang::DeclContext::decl_iterator I = DC->decls_begin (), E = DC->decls_end (); I != E; ++I)
         ScanDecl (*I, builder->ToScope ());
   }
}

void ScanLinkageSpec (clang::LinkageSpecDecl * D, Reflex::Scope Outer)
{
   for (clang::DeclContext::decl_iterator I = D->decls_begin (), E = D->decls_end (); I != E; ++I)
      ScanDecl (*I, Outer);
}

void ScanTypedef (clang::TypedefDecl * D)
{
   // type

   Reflex::Type type = ScanQualType (D->getUnderlyingType ());

   // name

   string full_name = D->getQualifiedNameAsString ();

   // !? !!
   return;

   // add item

   Reflex::Type result = Reflex::TypedefTypeBuilder (full_name.c_str (), type);

   result.Properties().AddProperty (clang_key, to_prop_item (D));
}

void ScanVariable (clang::VarDecl * D)
{
   // type

   Reflex::Type type = ScanQualType (D->getType());

   // name

   string full_name = D->getQualifiedNameAsString ();

   // modifiers

   unsigned int modifiers = Visibility (D) | VarModifiers (D);

   // offset

   int offset = 0; // !?

   // add item

   if (! Reflex::Scope::ByName ("").IsNamespace ())
      info ("Bug");

   try
   {
      Reflex::VariableBuilder builder =
         Reflex::VariableBuilder (full_name.c_str (), type, offset, modifiers);

      builder.AddProperty (clang_key, to_prop_item (D));
   }
   catch (std::exception & e)
   {
      warning (string (e.what ()) + " : variable " + full_name);
   }
}

void ScanFunction (clang::FunctionDecl * D, string template_params)
{
   clang::FunctionTemplateDecl * template_decl = D->getDescribedFunctionTemplate ();
   if (template_decl != NULL)
      template_params += ScanTemplateParams (template_decl); // !? what if function already has some template parameters

   #if 0
   if (D->isFunctionTemplateSpecialization ())
      template_params += ConvTemplateArguments (* D->getTemplateSpecializationArgs ());
   #endif

   // type

   Reflex::Type type = FuncType (D);

   // name

   string full_name = D->getQualifiedNameAsString () + template_params;

   // paramers

   string params = FuncParameters (D);

   // modifiers

   int modifiers = FuncModifiers (D);

   // stub

   Reflex::StubFunction stub = NULL; // !? !!
   void * stub_ctx = NULL;

   // add item

   try
   {
      Reflex::FunctionBuilder builder =
         Reflex::FunctionBuilder (type, full_name.c_str (), stub, stub_ctx, params.c_str (), modifiers);

      builder.AddProperty (clang_key, to_prop_item (D));
   }
   catch (std::exception & e)
   {
      warning (string (e.what ()) + " : function " + full_name);
   }
}

/********************************** DECL **********************************/

void ScanDecl (clang::Decl * D, Reflex::Scope Outer)
{
   // ScanInfo (D);

   if (clang::VarDecl * S = dyn_cast <clang::VarDecl> (D))
   {
      ScanVariable (S);
   }
   else if (clang::FunctionDecl * S = dyn_cast <clang::FunctionDecl> (D))
   {
      ScanFunction (S);
   }
   else if (clang::EnumDecl * S = dyn_cast <clang::EnumDecl> (D)) // before TagDecl and TypeDecl
   {
       ScanEnum (S);
   }
   else if (clang::RecordDecl * S = dyn_cast <clang::RecordDecl> (D)) // before TagDecl and TypeDecl
   {
      ScanClass (S);
   }
   else if (clang::TypedefDecl * S = dyn_cast <clang::TypedefDecl> (D)) // before TypeDecl
   {
      ScanTypedef (S);
   }
   else if (clang::ClassTemplateDecl * S = dyn_cast <clang::ClassTemplateDecl> (D))
   {
      ScanClassTemplate (S);
   }
   else if (clang::FunctionTemplateDecl * S = dyn_cast <clang::FunctionTemplateDecl> (D))
   {
      ScanFunctionTemplate (S);
   }
   else if (clang::LinkageSpecDecl * S = dyn_cast <clang::LinkageSpecDecl> (D))
   {
      ScanLinkageSpec (S, Outer);
   }
   else if (clang::TranslationUnitDecl * S = dyn_cast <clang::TranslationUnitDecl> (D))
   {
      ScanTranslationUnit (S, Outer);
   }
   else if (clang::NamespaceDecl * S = dyn_cast <clang::NamespaceDecl> (D))
   {
      ScanNamespace (S);
   }
   else if (clang::NamespaceAliasDecl * S = dyn_cast <clang::NamespaceAliasDecl> (D))
   {
      /* ... */
      ScanUnsupported (S);
   }
   else if (clang::UsingDecl * S = dyn_cast <clang::UsingDecl> (D))
   {
      ScanUsingDecl (S, Outer);
   }
   else if (clang::UsingDirectiveDecl * S = dyn_cast <clang::UsingDirectiveDecl> (D))
   {
      /* ... */
      ScanUnsupported (S);
   }
   else if (clang::UsingShadowDecl * S = dyn_cast <clang::UsingShadowDecl> (D))
   {
      ScanUsingShadowDecl (S, Outer);
   }
   else if (clang::StaticAssertDecl * S = dyn_cast <clang::StaticAssertDecl> (D))
   {
      /* ... */
      ScanUnsupported (S);
   }
   else if (clang::FriendDecl * S = dyn_cast <clang::FriendDecl> (D))
   {
      // should be used in class declaration
      ScanUnexpected (S);
   }
   else if (clang::FriendTemplateDecl * S = dyn_cast <clang::FriendTemplateDecl> (D))
   {
      // should be used in class declaration
      ScanUnexpected (S);
   }
   else if (clang::EnumConstantDecl * S = dyn_cast <clang::EnumConstantDecl> (D))
   {
      // should be used inside enumeration type declaration
      ScanUnexpected (S);
   }
   else if (clang::TemplateTemplateParmDecl * S = dyn_cast <clang::TemplateTemplateParmDecl> (D))
   {
      // should be used in template declaration
      ScanUnexpected (S);
   }
   else if (clang::TemplateTypeParmDecl * S = dyn_cast <clang::TemplateTypeParmDecl> (D))
   {
      // should be used in template declaration
      ScanUnexpected (S);
   }
   else
   {
      ScanUnknown (D);
   }
}

/********************************** SCAN **********************************/

/* global variables */

bool use_html = false;

string output_print_file_name;
string input_style_file_name;
string output_dict_file_name;

void ClrInit (clang::ASTContext * C,
              std::string print_file_name,
              std::string style_file_name,
              std::string dict_file_name)
{
   #ifdef TRACE
      set_error_handlers ();
   #endif

   Ctx = C;

   output_print_file_name = print_file_name;
   input_style_file_name = style_file_name;
   output_dict_file_name = dict_file_name;

   use_html = (input_style_file_name != "");

   init_dict_variables ();

   #ifdef LIST
      ClrListOpen (RemoveSuffix (output_print_file_name, ".html") + ".list.html", // !?
                   style_file_name,
                   use_html);
   #endif
}

void ClrScan (clang::Decl * D)
{
   ScanDecl (D, Reflex::Scope::GlobalScope ());

   #ifdef LIST
      ClrListDecl (D);
   #endif
}

void ClrOutput ()
{
   if (output_print_file_name != "")
   {
      info ("writing " + output_print_file_name);
      ClrPrint (output_print_file_name, input_style_file_name, use_html); // text or HTML output

      #ifdef DUMP
         ClrDump (RemoveSuffix (output_print_file_name, ".html") + ".dump.html", // !?
                  input_style_file_name,
                  use_html);
      #endif

      #ifdef LIST
         ClrListClose ();
      #endif
   }

   if (output_dict_file_name != "")
   {
      Reflex::DictionaryGenerator generator;
      Reflex::Scope::GlobalScope().GenerateDict (generator);
      info ("writing " + output_dict_file_name);
      generator.Print (output_dict_file_name.c_str ());
   }
}

/* ---------------------------------------------------------------------- */

void ClrStore (clang::ASTContext * C,
               clang::Decl * D)
{
   Ctx = C;

   output_print_file_name = "";
   input_style_file_name = "";
   output_dict_file_name = "";

   use_html = (input_style_file_name != "");

   ScanDecl (D, Reflex::Scope::GlobalScope ());
}

/* ---------------------------------------------------------------------- */

void ClrInfo ()
{
   output_print_file_name = "output.html";
   input_style_file_name = "style.css";
   output_dict_file_name = "output.code";

   use_html = (input_style_file_name != "");

   info ("writing " + output_print_file_name);
   ClrPrint (output_print_file_name, input_style_file_name, use_html); // text or HTML output
}

/* ---------------------------------------------------------------------- */

void ClrTest ()
{
   info ("Hello from ClrTest");
}

/* ---------------------------------------------------------------------- */

