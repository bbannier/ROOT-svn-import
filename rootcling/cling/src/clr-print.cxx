
/* clr-print.cc */

/* pseudo code from Reflex dictionaty */

#include "clr-print.h"
#include "clr-scan.h" // const clang_key
#include "clr-util.h" // class HtmlOutput

#include <sstream> // class ostringstream

/* ---------------------------------------------------------------------- */

using std::string;

/********************* HTML FROM REFLEX DICTIONARIES **********************/

const string key_style = "key";
const string name_style = "name";

const string namespace_style = "namespace";
const string class_style = "class";
const string enum_style = "enum";
const string function_style = "function";
const string type_style = "type";
const string variable_style = "variable";
const string template_style = "template";

const string error_style = "error";
const string comment_style = "comment";

/* ---------------------------------------------------------------------- */

const int qual_opt = Reflex::FINAL | Reflex::SCOPED | Reflex::QUALIFIED;
const int mini_opt = 0;

/* ---------------------------------------------------------------------- */

class ReflexPrinter : public clr_util::HtmlOutput
{
  private:
     void print_line (string key, string name);

     void print_type_name (Reflex::Type t);
     void print_type_notes (Reflex::Type t);
     void print_type (Reflex::Type t);
     void print_src_type (Reflex::Type t);

     void print_namespace (Reflex::Scope s);
     void print_class (Reflex::Scope s);
     void print_enum (Reflex::Scope s);
     void print_variable (Reflex::Member m);
     void print_function (Reflex::Member m);

     void print_member (Reflex::Member m);
     void print_access (Reflex::Member m, int & acs);
     void print_scope (Reflex::Scope s, bool show_access = false);
     #if 0
     void print_type_template (Reflex::TypeTemplate s);
     void print_member_template (Reflex::MemberTemplate s);
     #endif

  public:
     void print_src_scope (string text, Reflex::Scope s);
};

/* ---------------------------------------------------------------------- */

inline string NumToStr (size_t num)
{
   std::ostringstream stream;
   stream << num;
   return stream.str ();
}

/* ---------------------------------------------------------------------- */

void ReflexPrinter::print_line (string key, string name)
{
   styled_text (key, key_style);
   PutChr (' ');
   styled_text (name, name_style);
   PutEol ();
}

/* ---------------------------------------------------------------------- */

void ReflexPrinter::print_type_notes (Reflex::Type t)
{
   if (t.IsVirtual ())
      Put (" // virtual");

   if (t.IsConst ())
      Put (" // const");

   if (t.IsVolatile ())
      Put (" // volatile");

   if (t.IsConstVolatile ())
      Put (" // const volatile");

   if (t.IsAbstract ())
      Put (" // abstract");

   if (t.IsArray ())
   {
      Put (" // array [");
      Put (NumToStr (t.ArrayLength ()));
      Put ("]");
   }

   if (t.IsClass ())
      Put (" // class");

   if (t.IsComplete ())
      Put (" // complete");

   if (t.IsEnum ())
      Put (" // enum");

   if (t.IsFundamental ())
      Put (" // fundamental");

   if (t.IsPointer ())
      Put (" // pointer");

   if (t.IsPointerToMember ())
      Put (" // pointer to member");

   if (t.IsReference ())
      Put (" // reference");

   if (t.IsStruct ())
      Put (" // struct");

   if (t.IsTypedef ())
      Put (" // typedef");

   if (t.IsUnion ())
      Put (" // union");

   if (t.IsUnqualified ())
      Put (" // unqualified");
}

void ReflexPrinter::print_type (Reflex::Type t)
{
   print_line ("sub-type", t.Name (mini_opt)); // %%
   IncIndent ();

   if (t.IsTemplateInstance ())
      print_line ("TEMPLATE INSTANCE", "");

   /* qualifiers */

   /*
   if (t.IsVirtual ())
      Put ("virtual ");

   if (t.IsConst ())
      Put ("const ");

   if (t.IsVolatile ())
      Put ("volatile ");

   if (t.IsConstVolatile ())
      Put ("const volatile ");
   */

   /* declaration */

   Put (t.TypeTypeAsString ());
   Put (" ");
   Put (t.Name ());
   Put (";");

   /* comments */

   print_type_notes (t);

   PutEol ();

   #if 0
   for (Reflex::Member_Iterator i = t.Member_Begin (); i != t.Member_End (); ++i)
   {
      Reflex::Member m = *i;
      print_member (m);
   }
   #endif

   #if 0
   Reflex::Scope s = t;
   if (s)
      print_src_scope ("inner-scope", s);
   #endif

   #if 0
   Reflex::Type tt = t.TypeType ();
   if (tt)
      print_src_type (tt);
   #endif

   DecIndent ();
   print_line ("end of sub-type", t.Name (qual_opt));
}

void ReflexPrinter::print_src_type (Reflex::Type t)
{
   if (t.IsTypedef ())
   {
      styled_text ("typedef", type_style);
      Put (" ");
      Put (t.ToType().Name (qual_opt));
      Put (" ");
      Put (t.Name ());
      Put (";");
      print_type_notes (t);
      PutEol ();
   }
   else
   {
      // print_type (t);
   }
}

void ReflexPrinter::print_type_name (Reflex::Type t)
{
   if (! t)
   {
      styled_text ("no-type", error_style);
   }
   #if 0
   else if (t.IsPointerToMember ())
   {
      styled_text ("pointer-to-member", error_style);
   }
   #endif
   else
   {
      Put (t.Name (qual_opt));
   }
}

/* ---------------------------------------------------------------------- */

void ReflexPrinter::print_namespace (Reflex::Scope s)
{
   styled_text ("namespace", namespace_style);
   Put (" ");
   styled_text (s.Name (mini_opt), namespace_style);
   PutEol ();

   styled_text ("{", namespace_style);
   PutEol ();

   IncIndent ();
   print_scope (s);
   DecIndent ();

   styled_text ("};", namespace_style);
   Put (" ");
   styled_text ("// end of " + s.Name (qual_opt), comment_style);
   PutEol ();
}

/* ---------------------------------------------------------------------- */

void ReflexPrinter::print_class (Reflex::Scope s)
{
   if (s.IsTemplateInstance ())
   {
      styled_text ("template", template_style);
      Put (" ");
      put_html_escape ("<");

      bool any = false;

      for (Reflex::Type_Iterator i = s.TemplateArgument_Begin (); i != s.TemplateArgument_End (); ++i)
      {
         if (any)
            Put (", ");
         any = true;

         Reflex::Type m = *i;
         Put (m.Name (mini_opt));
      }

      put_html_escape (">");
      PutEol ();
      IncIndent ();
   }

   if (s.IsUnion ())
      styled_text ("union", class_style);
   else
      styled_text ("class", class_style);

   Put (" ");
   styled_text (s.Name (mini_opt), class_style);

   bool any = false;
   for (Reflex::Base_Iterator i = s.Base_Begin (); i != s.Base_End (); ++i)
   {
      Reflex::Base m = *i;

      if (! any)
         Put (" : ");
      else
         Put (", ");

      any = true;

      Put (m.Name (qual_opt));
   }
   PutEol ();

   styled_text ("{", class_style);
   PutEol ();

   IncIndent ();
   print_scope (s, true); // second parameter ... show acceess
   DecIndent ();

   styled_text ("};", class_style);
   Put (" ");
   styled_text ("// end of " + s.Name (qual_opt), comment_style);
   PutEol ();

   if (s.IsTemplateInstance ())
      DecIndent ();
}

/* ---------------------------------------------------------------------- */

void ReflexPrinter::print_enum (Reflex::Scope s)
{
   styled_text ("enum", enum_style);
   Put (" ");
   styled_text (s.Name (mini_opt), enum_style);
   PutEol ();

   styled_text ("{", enum_style);
   PutEol ();
   IncIndent ();

   Reflex::Member_Iterator i = s.Member_Begin ();
   while (i != s.Member_End ())
   {
      Reflex::Member m = *i;

      styled_text (m.Name (mini_opt), enum_style);

      Put ("=");
      Put (NumToStr (m.Offset ()));

      ++ i;
      if (i != s.Member_End ())
         Put (",");

      PutEol ();
   }

   DecIndent ();
   styled_text ("};", enum_style);
   Put (" ");
   styled_text ("// end of " + s.Name (qual_opt), comment_style);
   PutEol ();
}

/* ---------------------------------------------------------------------- */

void ReflexPrinter::print_variable (Reflex::Member m)
{
   print_type_name (m.TypeOf ());

   Put (" ");

   styled_text (m.Name (mini_opt), variable_style);
}

/* ---------------------------------------------------------------------- */

void ReflexPrinter::print_function (Reflex::Member m)
{
   print_type_name (m.TypeOf ().ReturnType ());

   Put (" ");

   styled_text (m.Name (mini_opt), function_style);
   Put (" (");

   bool any = false;
   Reflex::StdString_Iterator j = m.FunctionParameterDefault_Begin ();

   for (Reflex::StdString_Iterator i = m.FunctionParameterName_Begin (); i != m.FunctionParameterName_End (); ++i)
   {
      if (any)
         Put (", ");

      // Put (" [: ");
      Put (*i);
      // Put (" :] ");

      std::string value = *j;
      if (value != "")
      {
         Put (" = ");
         Put (value);
      }

      j ++;
      any = true;
   }

   Put (")");

   if (m.IsPureVirtual ())
      Put (" = 0");
}

/* ---------------------------------------------------------------------- */

void ReflexPrinter::print_member (Reflex::Member m)
{
   /* template instance */

   if (m.IsTemplateInstance ())
   {
      styled_text ("template", template_style);
      Put (" ");
      put_html_escape ("<");

      /* TemplateArgument */

      bool any = false;

      for (Reflex::Type_Iterator i = m.TemplateArgument_Begin (); i != m.TemplateArgument_End (); ++i)
      {
         Reflex::Type t = *i;

         if (any)
            Put (", ");

         Put (t.Name (qual_opt));

         any = true;
      }

      put_html_escape (">");
      PutEol ();

      IncIndent ();
   }

   /* qualifiers */

   if (m.IsExtern ())
      Put ("extern ");

   if (m.IsStatic ())
      Put ("static ");

   if (m.IsAuto ())
      Put ("auto ");

   if (m.IsRegister ())
      Put ("register ");

   if (m.IsInline ())
      Put ("inline ");

   if (m.IsVirtual ())
      Put ("virtual ");

   if (m.IsExplicit ())
      Put ("explicit ");

   if (m.IsMutable ())
      Put ("mutable ");

   if (m.IsConst ())
      Put ("const ");

   if (m.IsVolatile ())
      Put ("volatile ");

   /* declaration */

   if (m.IsFunctionMember () || m.TypeOf ().IsFunction ()) // !? for template function IsFunctionMember returns false
      print_function (m);
   else
      print_variable (m);

   Put (";");

   /* comments */

   if (m.IsArtificial ())
      Put (" // artificial");

   if (m.IsConstructor ())
      Put (" // constructor");

   if (m.IsCopyConstructor ())
      Put (" // copy constructor");

   if (m.IsConverter ())
      Put (" // converter");

   if (m.IsOperator ())
      Put (" // operator");

   if (m.IsDestructor ())
      Put (" // destructor");

   if (m.IsAbstract ())
      Put (" // abstract");

   if (m.IsPureVirtual ())
      Put (" // pure virtual");

   if (m.IsTransient ())
      Put (" // transient");

   PutEol ();

   if (m.IsTemplateInstance ())
      DecIndent ();
}

/* ---------------------------------------------------------------------- */

void ReflexPrinter::print_access (Reflex::Member m, int & acs)
{
   /* visibility */

   DecIndent ();

   if (m.IsPrivate () && acs != Reflex::PRIVATE)
   {
      PutLn ("private:");
      acs = Reflex::PRIVATE;
  }

   if (m.IsProtected () && acs != Reflex::PROTECTED)
   {
      PutLn ("protected:");
      acs = Reflex::PROTECTED;
   }

   if (m.IsPublic () && acs != Reflex::PUBLIC)
   {
      PutLn ("public:");
      acs = Reflex::PUBLIC;
   }

   IncIndent ();
}

/* ---------------------------------------------------------------------- */

void ReflexPrinter::print_scope (Reflex::Scope s, bool show_access)
{
   #if 0
   if (s.IsTemplateInstance ())
      print_line ("TEMPLATE INSTANCE", "");
   #endif

   if (show_access)
      IncIndent ();

   /* TemplateArgument */

   #if 0
   for (Reflex::Type_Iterator i = s.TemplateArgument_Begin (); i != s.TemplateArgument_End (); ++i)
   {
      Reflex::Type m = *i;
      print_line ("template-argument", m.Name (mini_opt));
      // print_src_scope ("template-argument", m, sub_types);
   }
   #endif

   /* UsingDirective */

   for (Reflex::Scope_Iterator i = s.UsingDirective_Begin (); i != s.UsingDirective_End (); ++i)
   {
      Reflex::Scope m = *i;
      print_line ("using", m.Name (qual_opt));
   }

   /* Member */

   int acs = -1; // value different from private, protected, public

   for (Reflex::Member_Iterator i = s.Member_Begin (); i != s.Member_End (); ++i)
   {
      Reflex::Member m = *i;

      if (show_access)
         print_access (m, acs);

      print_member (m);
   }

   /* DataMember */

   #if 0
   // again data memmbers
   for (Reflex::Member_Iterator i = s.DataMember_Begin (); i != s.DataMember_End (); ++i)
   {
      Reflex::Member m = *i;
      print_variable (m);
   }
   #endif

   /* FunctionMember */

   #if 0
   // again function memmbers
   for (Reflex::Member_Iterator i = s.FunctionMember_Begin (); i != s.FunctionMember_End (); ++i)
   {
      Reflex::Member m = *i;
      print_function (m);
   }
   #endif

   /* MemberTemplate */

   #if 0
   // already in members
   for (Reflex::MemberTemplate_Iterator i = s.MemberTemplate_Begin (); i != s.MemberTemplate_End (); ++i)
   {
      Reflex::MemberTemplate m = *i;
      print_member_template (m);
   }
   #endif

   /* SubTypeTemplate */

   #if 0
   // already in members
   for (Reflex::TypeTemplate_Iterator i = s.SubTypeTemplate_Begin (); i != s.SubTypeTemplate_End (); ++i)
   {
      Reflex::TypeTemplate m = *i;
      print_type_template (m);
   }
   #endif

   /* Scope */

   #if 0
   // all scopes
   for ( Reflex::Scope_Iterator i = s.Scope_Begin (); i != s.Scope_End (); ++i )
   {
      Reflex::Scope m = *i;
      print_line ("scope", m.Name ());
   }
   #endif

   /* SubScope */

   for (Reflex::Scope_Iterator i = s.SubScope_Begin (); i != s.SubScope_End (); ++i)
   {
      Reflex::Scope m = *i;
      print_src_scope ("sub-scope", m); // %%
   }

   /* SubType */

   for (Reflex::Type_Iterator i = s.SubType_Begin (); i != s.SubType_End (); ++i)
   {
      Reflex::Type t = *i;
      Reflex::Scope sub_scope = t;
      if (! sub_scope)
         print_src_type (t);
   }

   if (show_access)
      DecIndent ();
}

/* ---------------------------------------------------------------------- */

void ReflexPrinter::print_src_scope (string text, Reflex::Scope s)
{
   if (s.IsTopScope ())
   {
      print_scope (s);
   }
   else if (s.IsNamespace ())
   {
      print_namespace (s);
   }
   else if (s.IsClass () /* || s.IsStruct ()  */ || s.IsUnion ())
   {
      print_class (s);
   }
   else if (s.IsEnum ())
   {
      print_enum (s);
   }
   else
   {
      print_line (text, s.Name (mini_opt)); // %%

      IncIndent ();
      print_scope (s);
      DecIndent ();

      print_line ("end of " + text, s.Name (qual_opt));
   }
}

/* ---------------------------------------------------------------------- */

#if 0
void ReflexPrinter::print_type_template (Reflex::TypeTemplate s)
{
   print_line ("TYPE-TEMPLATE", s.Name (mini_opt));
   IncIndent ();

   /* TemplateParameterName */

   for (Reflex::StdString_Iterator i = s.TemplateParameterName_Begin (); i != s.TemplateParameterName_End (); ++i)
   {
      std::string m = *i;
      print_line ("template-parameter-name ", m);
   }

   /* TemplateParameterDefault */

   for (Reflex::StdString_Iterator i = s.TemplateParameterDefault_Begin (); i != s.TemplateParameterDefault_End (); ++i)
   {
      std::string m = *i;
      print_line ("template-parameter-default", m);
   }

   /* TemplateInstance */

   for (Reflex::Type_Iterator i = s.TemplateInstance_Begin (); i != s.TemplateInstance_End (); ++i)
   {
      Reflex::Type m = *i;
      print_line ("template-instance", m.Name (mini_opt));
      // print_src_scope ("template-instance", m, sub_types);
   }

   /* TypeTemplate */

   #if 0
   // all templates
   for (Reflex::TypeTemplate_Iterator i = s.TypeTemplate_Begin (); i != s.TypeTemplate_End (); ++i)
   {
      Reflex::TypeTemplate m = *i;
      print_line ("type-template", m.Name (mini_opt));
   }
   #endif

   DecIndent ();
   print_line ("end of type-template ", s.Name (qual_opt));
}
#endif

/* ---------------------------------------------------------------------- */

#if 0
void ReflexPrinter::print_member_template (Reflex::MemberTemplate s)
{
   print_line ("begin of member-template", s.Name (qual_opt));

   styled_text ("template", template_style);
   Put (" ");
   put_html_escape ("<");

   /* TemplateParameterName and TemplateParameterDefault */

   bool any = false;
   Reflex::StdString_Iterator j = s.TemplateParameterDefault_Begin ();

   for (Reflex::StdString_Iterator i = s.TemplateParameterName_Begin (); i != s.TemplateParameterName_End (); ++i)
   {
      std::string name = *i;
      std::string value = "";
      if (j != s.TemplateParameterDefault_End ())
         value = *j;

      if (any)
         Put (", ");

      Put (name);

      if (value != "")
         Put (" = " + value);

      if (j != s.TemplateParameterDefault_End ())
         ++ j;

      any = true;
   }

   put_html_escape (">");
   PutEol ();

   IncIndent ();

   /* TemplateInstance */

   print_line ("begin of template instances", s.Name (qual_opt));
   IncIndent ();
   IncIndent (); // visibility

   int acs = -1;
   for (Reflex::Member_Iterator i = s.TemplateInstance_Begin (); i != s.TemplateInstance_End (); ++i)
   {
      Reflex::Member m = *i;
      print_access (m, acs);
      print_member (m);
   }

   DecIndent (); // visibility
   DecIndent ();
   print_line ("end of template instances", s.Name (qual_opt));

   /* MemberTemplate */

   #if 0
   // all templates
   for (Reflex::MemberTemplate_Iterator i = s.MemberTemplate_Begin (); i != s.MemberTemplate_End (); ++i)
   {
      Reflex::MemberTemplate m = *i;
      print_line ("member-template", m.Name (mini_opt));
   }
   #endif

   DecIndent ();

   print_line ("end of member-template", s.Name (qual_opt));
}
#endif

/* ---------------------------------------------------------------------- */

void ClrPrint (string file_name,
               string style_file_name,
               bool use_html)
{
   Reflex::Scope scope = Reflex::Scope::GlobalScope();

   ReflexPrinter io;

   io.Open (file_name);
   io.enable_html (use_html);

   if (use_html)
   {
      string title = "pseudo code from Reflex dictionary " + scope.Name (qual_opt);

      io.head (title, style_file_name);

      io.PutLn ("<h1>" + title + "</h1>");

      io.PutLn ("<pre>");
   }

   // io.IncIndent ();
   io.print_src_scope ("top-scope", scope);
   // io.DecIndent ();

   if (use_html)
   {
      io.PutLn ("</pre>");
      io.tail ();
   }

   io.Close ();
}

/* ---------------------------------------------------------------------- */

string ClrPrintScope (Reflex::Scope scope,
                      string style_file_name)
{
   ReflexPrinter io;

   bool use_html = true;

   io.OpenString ();

   io.enable_html (use_html);

   if (use_html)
   {
      string title = "pseudo code from Reflex dictionary " + scope.Name (qual_opt);

      io.head (title, style_file_name);

      io.PutLn ("<h1>" + title + "</h1>");

      io.PutLn ("<pre>");
   }

   if (! scope.IsTopScope())
      io.IncIndent ();

   io.print_src_scope ("top-scope", scope);

   if (! scope.IsTopScope())
      io.DecIndent ();

   if (use_html)
   {
      io.PutLn ("</pre>");
      io.tail ();
   }

   return io.CloseString ();
}

/* ---------------------------------------------------------------------- */


