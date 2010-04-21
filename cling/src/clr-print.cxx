// @(#)root/cint:$Id$
// Author: Zdenek Culik   16/04/2010

/* pseudo code from Reflex dictionaty */

#include "TObject.h"
#include "clr-print.h"

/*********************** HTML FROM REFLEX DICTIONARIES ************************/

const TString key_style = "key";
const TString name_style = "name";

const TString namespace_style = "namespace";
const TString class_style = "class";
const TString enum_style = "enum";
const TString function_style = "function";
const TString type_style = "type";
const TString variable_style = "variable";
const TString template_style = "template";

const TString error_style = "error";
const TString comment_style = "comment";

/* -------------------------------------------------------------------------- */

const int qual_opt = Reflex::FINAL | Reflex::SCOPED | Reflex::QUALIFIED;
const int mini_opt = 0;

//______________________________________________________________________________
inline TString NumToStr(size_t num)
{
   TString txt = "";
   txt += num;
   return txt;
}

/******************************* REFLEX PRINTER *******************************/

//______________________________________________________________________________
void TReflexPrinter::PrintLine(TString key, TString name)
{
   StyledText(key, key_style);
   PutChr(' ');
   StyledText(name, name_style);
   PutEol();
}

//______________________________________________________________________________
void TReflexPrinter::PrintTypeNotes(Reflex::Type t)
{
   if (t.IsVirtual())
      Put(" // virtual");

   if (t.IsConst())
      Put(" // const");

   if (t.IsVolatile())
      Put(" // volatile");

   if (t.IsConstVolatile())
      Put(" // const volatile");

   if (t.IsAbstract())
      Put(" // abstract");

   if (t.IsArray()) {
      Put(" // array [");
      Put(NumToStr(t.ArrayLength()));
      Put("]");
   }

   if (t.IsClass())
      Put(" // class");

   if (t.IsComplete())
      Put(" // complete");

   if (t.IsEnum())
      Put(" // enum");

   if (t.IsFundamental())
      Put(" // fundamental");

   if (t.IsPointer())
      Put(" // pointer");

   if (t.IsPointerToMember())
      Put(" // pointer to member");

   if (t.IsReference())
      Put(" // reference");

   if (t.IsStruct())
      Put(" // struct");

   if (t.IsTypedef())
      Put(" // typedef");

   if (t.IsUnion())
      Put(" // union");

   if (t.IsUnqualified())
      Put(" // unqualified");
}

//______________________________________________________________________________
void TReflexPrinter::PrintType(Reflex::Type t)
{
   PrintLine("sub-type", t.Name(mini_opt));   // %%
   IncIndent();

   if (t.IsTemplateInstance())
      PrintLine("TEMPLATE INSTANCE", "");

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

   Put(t.TypeTypeAsString());
   Put(" ");
   Put(t.Name());
   Put(";");

   /* comments */

   PrintTypeNotes(t);

   PutEol();

   #if 0
   for (Reflex::Member_Iterator i = t.Member_Begin(); i != t.Member_End(); ++i) {
      Reflex::Member m = *i;
      PrintMember(m);
   }
   #endif

   #if 0
   Reflex::Scope s = t;
   if (s)
      PrintSrcScope("inner-scope", s);
   #endif

   #if 0
   Reflex::Type tt = t.TypeType();
   if (tt)
      PrintSrcType(tt);
   #endif

   DecIndent();
   PrintLine("end of sub-type", t.Name(qual_opt));
}

//______________________________________________________________________________
void TReflexPrinter::PrintSrcType(Reflex::Type t)
{
   if (t.IsTypedef()) {
      StyledText("typedef", type_style);
      Put(" ");
      Put(t.ToType().Name(qual_opt));
      Put(" ");
      Put(t.Name());
      Put(";");
      PrintTypeNotes(t);
      PutEol();
   } else {
      // PrintType (t);
   }
}

//______________________________________________________________________________
void TReflexPrinter::PrintTypeName(Reflex::Type t)
{
   if (! t) {
      StyledText("no-type", error_style);
   }
   #if 0
   else if (t.IsPointerToMember()) {
      StyledText("pointer-to-member", error_style);
   }
   #endif
   else {
      Put(t.Name(qual_opt));
   }
}

//______________________________________________________________________________
void TReflexPrinter::PrintNamespace(Reflex::Scope s)
{
   StyledText("namespace", namespace_style);
   Put(" ");
   StyledText(s.Name(mini_opt), namespace_style);
   PutEol();

   StyledText("{", namespace_style);
   PutEol();

   IncIndent();
   PrintScope(s);
   DecIndent();

   StyledText("};", namespace_style);
   Put(" ");
   StyledText("// end of " + s.Name(qual_opt), comment_style);
   PutEol();
}

//______________________________________________________________________________
void TReflexPrinter::PrintClass(Reflex::Scope s)
{
   if (s.IsTemplateInstance()) {
      StyledText("template", template_style);
      Put(" ");
      PutHtmlEscape("<");

      bool any = false;

      for (Reflex::Type_Iterator i = s.TemplateArgument_Begin(); i != s.TemplateArgument_End(); ++i) {
         if (any)
            Put(", ");
         any = true;

         Reflex::Type m = *i;
         Put(m.Name(mini_opt));
      }

      PutHtmlEscape(">");
      PutEol();
      IncIndent();
   }

   if (s.IsUnion())
      StyledText("union", class_style);
   else
      StyledText("class", class_style);

   Put(" ");
   StyledText(s.Name(mini_opt), class_style);

   bool any = false;
   for (Reflex::Base_Iterator i = s.Base_Begin(); i != s.Base_End(); ++i) {
      Reflex::Base m = *i;

      if (! any)
         Put(" : ");
      else
         Put(", ");

      any = true;

      Put(m.Name(qual_opt));
   }
   PutEol();

   StyledText("{", class_style);
   PutEol();

   IncIndent();
   PrintScope(s, true);  // second parameter ... show acceess
   DecIndent();

   StyledText("};", class_style);
   Put(" ");
   StyledText("// end of " + s.Name(qual_opt), comment_style);
   PutEol();

   if (s.IsTemplateInstance())
      DecIndent();
}

//______________________________________________________________________________
void TReflexPrinter::PrintEnum(Reflex::Scope s)
{
   StyledText("enum", enum_style);
   Put(" ");
   StyledText(s.Name(mini_opt), enum_style);
   PutEol();

   StyledText("{", enum_style);
   PutEol();
   IncIndent();

   Reflex::Member_Iterator i = s.Member_Begin();
   while (i != s.Member_End()) {
      Reflex::Member m = *i;

      StyledText(m.Name(mini_opt), enum_style);

      Put("=");
      Put(NumToStr(m.Offset()));

      ++ i;
      if (i != s.Member_End())
         Put(",");

      PutEol();
   }

   DecIndent();
   StyledText("};", enum_style);
   Put(" ");
   StyledText("// end of " + s.Name(qual_opt), comment_style);
   PutEol();
}

//______________________________________________________________________________
void TReflexPrinter::PrintVariable(Reflex::Member m)
{
   PrintTypeName(m.TypeOf());

   Put(" ");

   StyledText(m.Name(mini_opt), variable_style);
}

//______________________________________________________________________________
void TReflexPrinter::PrintFunction(Reflex::Member m)
{
   PrintTypeName(m.TypeOf().ReturnType());

   Put(" ");

   StyledText(m.Name(mini_opt), function_style);
   Put(" (");

   bool any = false;
   Reflex::StdString_Iterator j = m.FunctionParameterDefault_Begin();

   for (Reflex::StdString_Iterator i = m.FunctionParameterName_Begin(); i != m.FunctionParameterName_End(); ++i) {
      if (any)
         Put(", ");

      Put(*i);

      TString value = *j;
      if (value != "") {
         Put(" = ");
         Put(value);
      }

      j ++;
      any = true;
   }

   Put(")");

   if (m.IsPureVirtual())
      Put(" = 0");
}

//______________________________________________________________________________
void TReflexPrinter::PrintMember(Reflex::Member m)
{
   /* template instance */

   if (m.IsTemplateInstance()) {
      StyledText("template", template_style);
      Put(" ");
      PutHtmlEscape("<");

      /* TemplateArgument */

      bool any = false;

      for (Reflex::Type_Iterator i = m.TemplateArgument_Begin(); i != m.TemplateArgument_End(); ++i) {
         Reflex::Type t = *i;

         if (any)
            Put(", ");

         Put(t.Name(qual_opt));

         any = true;
      }

      PutHtmlEscape(">");
      PutEol();

      IncIndent();
   }

   /* qualifiers */

   if (m.IsExtern())
      Put("extern ");

   if (m.IsStatic())
      Put("static ");

   if (m.IsAuto())
      Put("auto ");

   if (m.IsRegister())
      Put("register ");

   if (m.IsInline())
      Put("inline ");

   if (m.IsVirtual())
      Put("virtual ");

   if (m.IsExplicit())
      Put("explicit ");

   if (m.IsMutable())
      Put("mutable ");

   if (m.IsConst())
      Put("const ");

   if (m.IsVolatile())
      Put("volatile ");

   /* declaration */

   if (m.IsFunctionMember() || m.TypeOf().IsFunction())    // !? for template function IsFunctionMember returns false
      PrintFunction(m);
   else
      PrintVariable(m);

   Put(";");

   /* comments */

   if (m.IsArtificial())
      Put(" // artificial");

   if (m.IsConstructor())
      Put(" // constructor");

   if (m.IsCopyConstructor())
      Put(" // copy constructor");

   if (m.IsConverter())
      Put(" // converter");

   if (m.IsOperator())
      Put(" // operator");

   if (m.IsDestructor())
      Put(" // destructor");

   if (m.IsAbstract())
      Put(" // abstract");

   if (m.IsPureVirtual())
      Put(" // pure virtual");

   if (m.IsTransient())
      Put(" // transient");

   PutEol();

   if (m.IsTemplateInstance())
      DecIndent();
}

//______________________________________________________________________________
void TReflexPrinter::PrintAccess(Reflex::Member m, int& acs)
{
   /* visibility */

   DecIndent();

   if (m.IsPrivate() && acs != Reflex::PRIVATE) {
      PutLn("private:");
      acs = Reflex::PRIVATE;
   }

   if (m.IsProtected() && acs != Reflex::PROTECTED) {
      PutLn("protected:");
      acs = Reflex::PROTECTED;
   }

   if (m.IsPublic() && acs != Reflex::PUBLIC) {
      PutLn("public:");
      acs = Reflex::PUBLIC;
   }

   IncIndent();
}

//______________________________________________________________________________
void TReflexPrinter::PrintScope(Reflex::Scope s, bool show_access)
{
   #if 0
   if (s.IsTemplateInstance())
      PrintLine("TEMPLATE INSTANCE", "");
   #endif

   if (show_access)
      IncIndent();

   /* TemplateArgument */

   #if 0
   for (Reflex::Type_Iterator i = s.TemplateArgument_Begin(); i != s.TemplateArgument_End(); ++i) {
      Reflex::Type m = *i;
      PrintLine("template-argument", m.Name(mini_opt));
      // PrintSrcScope ("template-argument", m, sub_types);
   }
   #endif

   /* UsingDirective */

   for (Reflex::Scope_Iterator i = s.UsingDirective_Begin(); i != s.UsingDirective_End(); ++i) {
      Reflex::Scope m = *i;
      PrintLine("using", m.Name(qual_opt));
   }

   /* Member */

   int acs = -1; // value different from private, protected, public

   for (Reflex::Member_Iterator i = s.Member_Begin(); i != s.Member_End(); ++i) {
      Reflex::Member m = *i;

      if (show_access)
         PrintAccess(m, acs);

      PrintMember(m);
   }

   /* DataMember */

   #if 0
   // again data memmbers
   for (Reflex::Member_Iterator i = s.DataMember_Begin(); i != s.DataMember_End(); ++i) {
      Reflex::Member m = *i;
      PrintVariable(m);
   }
   #endif

   /* FunctionMember */

   #if 0
   // again function memmbers
   for (Reflex::Member_Iterator i = s.FunctionMember_Begin(); i != s.FunctionMember_End(); ++i) {
      Reflex::Member m = *i;
      PrintFunction(m);
   }
   #endif

   /* MemberTemplate */

   #if 0
   // already in members
   for (Reflex::MemberTemplate_Iterator i = s.MemberTemplate_Begin(); i != s.MemberTemplate_End(); ++i) {
      Reflex::MemberTemplate m = *i;
      PrintMemberTemplate(m);
   }
   #endif

   /* SubTypeTemplate */

   #if 0
   // already in members
   for (Reflex::TypeTemplate_Iterator i = s.SubTypeTemplate_Begin(); i != s.SubTypeTemplate_End(); ++i) {
      Reflex::TypeTemplate m = *i;
      PrintTypeTemplate(m);
   }
   #endif

   /* Scope */

   #if 0
   // all scopes
   for (Reflex::Scope_Iterator i = s.Scope_Begin(); i != s.Scope_End(); ++i) {
      Reflex::Scope m = *i;
      PrintLine("scope", m.Name());
   }
   #endif

   /* SubScope */

   for (Reflex::Scope_Iterator i = s.SubScope_Begin(); i != s.SubScope_End(); ++i) {
      Reflex::Scope m = *i;
      PrintSrcScope("sub-scope", m);  // %%
   }

   /* SubType */

   for (Reflex::Type_Iterator i = s.SubType_Begin(); i != s.SubType_End(); ++i) {
      Reflex::Type t = *i;
      Reflex::Scope sub_scope = t;
      if (! sub_scope)
         PrintSrcType(t);
   }

   if (show_access)
      DecIndent();
}

//______________________________________________________________________________
void TReflexPrinter::PrintSrcScope(TString text, Reflex::Scope s)
{
   if (s.IsTopScope()) {
      PrintScope(s);
   } else if (s.IsNamespace()) {
      PrintNamespace(s);
   } else if (s.IsClass() /* || s.IsStruct ()  */ || s.IsUnion()) {
      PrintClass(s);
   } else if (s.IsEnum()) {
      PrintEnum(s);
   } else {
      PrintLine(text, s.Name(mini_opt));   // %%

      IncIndent();
      PrintScope(s);
      DecIndent();

      PrintLine("end of " + text, s.Name(qual_opt));
   }
}

#if 0
//______________________________________________________________________________
void TReflexPrinter::PrintTypeTemplate(Reflex::TypeTemplate s)
{
   PrintLine("TYPE-TEMPLATE", s.Name(mini_opt));
   IncIndent();

   /* TemplateParameterName */

   for (Reflex::StdString_Iterator i = s.TemplateParameterName_Begin(); i != s.TemplateParameterName_End(); ++i) {
      TString m = *i;
      PrintLine("template-parameter-name ", m);
   }

   /* TemplateParameterDefault */

   for (Reflex::StdString_Iterator i = s.TemplateParameterDefault_Begin(); i != s.TemplateParameterDefault_End(); ++i) {
      TString m = *i;
      PrintLine("template-parameter-default", m);
   }

   /* TemplateInstance */

   for (Reflex::Type_Iterator i = s.TemplateInstance_Begin(); i != s.TemplateInstance_End(); ++i) {
      Reflex::Type m = *i;
      PrintLine("template-instance", m.Name(mini_opt));
      // PrintSrcScope ("template-instance", m, sub_types);
   }

   /* TypeTemplate */

   #if 0
   // all templates
   for (Reflex::TypeTemplate_Iterator i = s.TypeTemplate_Begin(); i != s.TypeTemplate_End(); ++i) {
      Reflex::TypeTemplate m = *i;
      PrintLine("type-template", m.Name(mini_opt));
   }
   #endif

   DecIndent();
   PrintLine("end of type-template ", s.Name(qual_opt));
}
#endif

#if 0
//______________________________________________________________________________
void TReflexPrinter::PrintMemberTemplate(Reflex::MemberTemplate s)
{
   PrintLine("begin of member-template", s.Name(qual_opt));

   StyledText("template", template_style);
   Put(" ");
   PutHtmlEscape("<");

   /* TemplateParameterName and TemplateParameterDefault */

   bool any = false;
   Reflex::StdString_Iterator j = s.TemplateParameterDefault_Begin();

   for (Reflex::StdString_Iterator i = s.TemplateParameterName_Begin(); i != s.TemplateParameterName_End(); ++i) {
      TString name = *i;
      TString value = "";
      if (j != s.TemplateParameterDefault_End())
         value = *j;

      if (any)
         Put(", ");

      Put(name);

      if (value != "")
         Put(" = " + value);

      if (j != s.TemplateParameterDefault_End())
         ++ j;

      any = true;
   }

   PutHtmlEscape(">");
   PutEol();

   IncIndent();

   /* TemplateInstance */

   PrintLine("begin of template instances", s.Name(qual_opt));
   IncIndent();
   IncIndent();  // visibility

   int acs = -1;
   for (Reflex::Member_Iterator i = s.TemplateInstance_Begin(); i != s.TemplateInstance_End(); ++i) {
      Reflex::Member m = *i;
      PrintAccess(m, acs);
      PrintMember(m);
   }

   DecIndent();  // visibility
   DecIndent();
   PrintLine("end of template instances", s.Name(qual_opt));

   /* MemberTemplate */

   #if 0
   // all templates
   for (Reflex::MemberTemplate_Iterator i = s.MemberTemplate_Begin(); i != s.MemberTemplate_End(); ++i) {
      Reflex::MemberTemplate m = *i;
      PrintLine("member-template", m.Name(mini_opt));
   }
   #endif

   DecIndent();

   PrintLine("end of member-template", s.Name(qual_opt));
}
#endif

//______________________________________________________________________________
void TReflexPrinter::Print(Reflex::Scope scope)
{
   if (fUseHtml) {
      TString title = "pseudo code from Reflex dictionary " + scope.Name(qual_opt);
      // Head(title, style_file_name);
      Head(title);
      PutLn("<h1>" + title + "</h1>");
      PutLn("<pre>");
   }

   if (! scope.IsTopScope())
      IncIndent();

   PrintSrcScope("top-scope", scope);

   if (! scope.IsTopScope())
      DecIndent();

   if (fUseHtml) {
      PutLn("</pre>");
      Tail();
   }

}

/*********************************** PRINT ************************************/

void ClrPrint(TString file_name,
              TString style_file_name,
              bool useHtml)
{
   Reflex::Scope scope = Reflex::Scope::GlobalScope();

   TReflexPrinter io;

   io.Open(file_name);
   io.EnableHtml(useHtml);
   io.SetStyleFileName(style_file_name);

   io.Print(scope);

   io.Close();
}

TString ClrPrintScope(Reflex::Scope scope,
                      TString style_file_name,
                      bool useHtml)
{
   TReflexPrinter io;

   io.OpenString();
   io.EnableHtml(useHtml);
   io.SetStyleFileName(style_file_name);

   io.Print(scope);

   return io.CloseString();
}

/* -------------------------------------------------------------------------- */


