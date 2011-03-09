// @(#)root/cint:$Id$
// Author: Zdenek Culik   16/04/2010

/* pseudo code from Reflex dictionaty */

#include "TObject.h"
#include "clr-print.h"

/*********************** HTML FROM REFLEX DICTIONARIES ************************/

const int qual_opt = Reflex::FINAL | Reflex::SCOPED | Reflex::QUALIFIED;
const int mini_opt = 0;

//______________________________________________________________________________
inline TString NumToStr(size_t num)
{
   TString txt = "";
   txt += num;
   return txt;
}

/********************************* HTML STYLE **********************************/

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
const TString property_style = "property";

//______________________________________________________________________________
void THtmlHelper::Style()
{
   PutLn (".key { color: Blue; }");
   PutLn (".name { color: Orange; }");

   PutLn (".namespace { color: Gold; }");
   PutLn (".enum { color: DarkOrange; }");
   PutLn (".class { color: Blue; }");
   PutLn (".function { color: Green; }");
   PutLn (".type { color: Chocolate; }");
   PutLn (".variable { color: SaddleBrown; }");
   PutLn (".template { color: DarkViolet; }");

   PutLn (".error { color: Red; }");
   PutLn (".comment { color: Grey; /* font-style:italic; */ }");
   PutLn (".property { color: CornFlowerBlue; }");

   PutLn ("h1 {");
   IncIndent();
      PutLn ("margin: 0.5em auto 0.5em auto;");
      PutLn ("padding: 15px 10px 15px 55px;");
      PutLn ("color: Blue;");
      PutLn ("background: #ffff70;");
      PutLn ("border: 1px solid #c0c0e0;");
   DecIndent();
   PutLn ("}");
}

/********************************* HTML HELPER *********************************/

//______________________________________________________________________________
THtmlHelper::THtmlHelper():
   fIndent(0),
   fStartLine(true),
   fFileStream(NULL),
   fStringStream(NULL),
   fStream(NULL),
   fUseHtml(true)
{
}

//______________________________________________________________________________
THtmlHelper::~THtmlHelper()
{
   if (fStream != NULL)
      delete fStream;
}

//______________________________________________________________________________
void THtmlHelper::Open(const TString file_name)
{
   fFileStream = new std::ofstream();
   fFileStream->open(file_name.Data());
   fStream = fFileStream;
}

//______________________________________________________________________________
void THtmlHelper::Close()
{
   fFileStream->close();
}

//______________________________________________________________________________
void THtmlHelper::OpenString()
{
   fStringStream = new std::ostringstream();
   fStream = fStringStream;
}

//______________________________________________________________________________
TString THtmlHelper::CloseString()
{
   return fStringStream->str();
}

//______________________________________________________________________________
void THtmlHelper::SetIndent(int i)
{
   fIndent = i;
}

//______________________________________________________________________________
void THtmlHelper::IncIndent()
{
   fIndent += 3;
}

//______________________________________________________________________________
void THtmlHelper::DecIndent()
{
   fIndent -= 3;
}

//______________________________________________________________________________
void THtmlHelper::PutPlainStr(const TString s)
{
   (*fStream) << s;
}

//______________________________________________________________________________
void THtmlHelper::PutPlainChr(char c)
{
   (*fStream) << c;
}

//______________________________________________________________________________
void THtmlHelper::OpenLine()
{
   TString spaces(' ', fIndent);
   // TString (character, count)
   // std::string (count, character)
   PutPlainStr(spaces);
   fStartLine = false;
}

//______________________________________________________________________________
void THtmlHelper::CloseLine()
{
   fStartLine = true;
}

//______________________________________________________________________________
void THtmlHelper::Put(const TString s)
{
   if (fStartLine)
      OpenLine();

   PutPlainStr(s);
}

//______________________________________________________________________________
void THtmlHelper::PutChr(char c)
{
   if (fStartLine)
      OpenLine();

   PutPlainChr(c);
}

//______________________________________________________________________________
void THtmlHelper::PutEol()
{
   if (fUseHtml)
      PutPlainStr("\r\n");
   else
      PutPlainStr("\n");

   CloseLine();
}

//______________________________________________________________________________
void THtmlHelper::PutLn(const TString s)
{
   Put(s);
   PutEol();
}

//______________________________________________________________________________
TString QuoteStrContent(const TString value)
{
   int len = value.Length();

   bool simple = true;
   for (int i = 0; i < len && simple; i++) {
      unsigned char ch = value [i];
      if (ch < ' ' || ch == '\\' || ch == '"' || ch == 128 || ch >= 255)
         simple = false;
   }

   if (simple) {
      return value;
   } else {
      TString result = "";
      const char hex [16+1] = "0123456789abcdef";

      for (int i = 0; i < len; i++) {
         unsigned char ch = value [i];

         if (ch < ' ' || ch == 128 || ch >= 255) {
            result = result + '\\';
            switch (ch) {
               case '\a':
                  result = result + 'a';
                  break;
               case '\b':
                  result = result + 'b';
                  break;
               case '\f':
                  result = result + 'f';
                  break;
               case '\n':
                  result = result + 'n';
                  break;
               case '\r':
                  result = result + 'r';
                  break;
               case '\t':
                  result = result + 't';
                  break;
               case '\v':
                  result = result + 'v';
                  break;

               default:
                  result = result + 'x' + hex [ch >> 4] + hex [ch & 15];
                  break;
            }
         } else {
            if (ch == '\\' || ch == '"')
               result = result + '\\';
            result = result + value [i]; // variable ch is unsigned char
         }
      }

      return result;
   }
}

const char quote2 = '"';

//______________________________________________________________________________
TString QuoteStr(const TString value, char quote = quote2)
{
   return quote + QuoteStrContent(value) + quote;
}

//______________________________________________________________________________
inline TString FceAttr(const TString name, const TString value)
{
   return name + "=" + quote2 + QuoteStrContent(value) + quote2;
}

//______________________________________________________________________________
void THtmlHelper::Attr(const TString name, const TString value)
// name = "value"
{
   if (fUseHtml) {
      Put(name);
      PutChr('=');
      PutChr(quote2);
      Put(QuoteStrContent(value));
      PutChr(quote2);
   }
}

//______________________________________________________________________________
void THtmlHelper::Href(const TString url, const TString style, const TString text)
// <a href="url" class="style"> text </a>
{
   if (fUseHtml) {
      Put("<a ");
      Attr("href", url);

      if (style != "") {
         Put(" ");
         Attr("class", style);
      }

      Put(">");
      Put(text);
      Put("</a>");
   }
}

//______________________________________________________________________________
void THtmlHelper::Name(const TString name, const TString text)
// <a name="name"> text </a>
{
   if (fUseHtml) {
      Put("<a ");
      Attr("name", name);

      if (text == "") {
         Put("/>");
      } else {
         Put(">");
         Put(text);
         Put("</a>");
      }
   }
}

//______________________________________________________________________________
void THtmlHelper::StyleBegin(const TString style)
{
   if (fUseHtml) {
      Put("<span ");
      Attr("class", style);
      Put(">");
   }
}

//______________________________________________________________________________
void THtmlHelper::StyleEnd()
{
   if (fUseHtml)
      Put("</span>");
}

//______________________________________________________________________________
void THtmlHelper::StyledText(const TString text, const TString style)
{
   if (fUseHtml) {
      Put("<span ");
      Attr("class", style);
      Put(">");
      Put(text);
      Put("</span>");
   } else {
      Put(text);
   }
}

//______________________________________________________________________________
TString THtmlHelper::FceHtmlEscape(const TString s)
{
   int len = s.Length();

   bool ok = true;
   for (int i = 0; i < len; i++) {
      char c = s[i];
      if (c == '<' || c == '>' || c == '&')
         ok = false;
   }

   if (ok) {
      return s;
   } else {
      TString result = "";

      for (int i = 0; i < len; i++) {
         char c = s[i];
         if (c == '<')
            result += "&lt;";
         else if (c == '>')
            result += "&gt;";
         else if (c == '&')
            result += "&amp;";
         else
            result += c;
      }

      return result;
   }
}

//______________________________________________________________________________
void THtmlHelper::PutHtmlEscape(const TString s)
// Replace < > & characters
{
   if (fUseHtml) {
      int len = s.Length();

      bool ok = true;
      for (int i = 0; i < len; i++) {
         char c = s[i];
         if (c == '<' || c == '>' || c == '&')
            ok = false;
      }

      if (ok) {
         Put(s);
      } else {
         for (int i = 0; i < len; i++) {
            char c = s[i];
            if (c == '<')
               Put("&lt;");
            else if (c == '>')
               Put("&gt;");
            else if (c == '&')
               Put("&amp;");
            else
               PutChr(c);
         }
      }
   } else {
      Put (s);
   }
}

//______________________________________________________________________________
void THtmlHelper::Head(const TString title)
{
   if (fUseHtml) {
      TString charset = "ISO-8859-1";

      PutLn("<?xml " + FceAttr("version", "1.0") + "?>");

      PutLn("<!DOCTYPE html PUBLIC " +
            QuoteStr("-//W3C//DTD XHTML 1.0 Transitional//EN") + " " +
            QuoteStr("http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd") + ">");

      PutLn("<html " + FceAttr("xmlns", "http://www.w3.org/1999/xhtml") + " " +
            FceAttr("xml:lang", "en") + " " +
            FceAttr("lang", "en") + ">");

      PutLn("<Head>");

      PutLn("<meta " + FceAttr("http-equiv", "Content-Type") + " " +
            FceAttr("content", "text/html; charset=" + charset) + " />");

      PutLn("<title>" + title + "</title>");

      if (fStyleFileName != "") {
         PutLn("<link " + FceAttr("rel", "stylesheet") + " " +
               FceAttr("type", "text/css") + " " +
               FceAttr("href", fStyleFileName) + " />");
      }
      else
      {
         PutLn("<style " + FceAttr("type", "text/css") + ">");
	 IncIndent();
	 Style();
	 DecIndent();
         PutLn("</style>");
      }

      PutLn("</Head>");
      PutLn("<body>");
   }
}

//______________________________________________________________________________
void THtmlHelper::Tail()
{
   if (fUseHtml) {
      PutLn("</body>");
      PutLn("</html>");
   }
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
void TReflexPrinter::PrintNote(TString s)
{
   PutChr(' ');
   s = "// " + s;
   StyledText(s, comment_style);
}

//______________________________________________________________________________
void TReflexPrinter::PrintProperties(const Reflex::PropertyList& list)
{
   for (Reflex::StdString_Iterator i = list.Key_Begin(); i != list.Key_End(); ++i) {
       std::string key = *i;
       std::string value = list.PropertyAsString (key);

       TString s = " // property " + key + " = "+ value;
       StyledText(s, property_style);
   }
}

//______________________________________________________________________________
void TReflexPrinter::PrintTypeNotes(Reflex::Type t)
{
   if (t.IsVirtual())
      PrintNote("virtual");

   if (t.IsConst())
      PrintNote("const");

   if (t.IsVolatile())
      PrintNote("volatile");

   if (t.IsConstVolatile())
      PrintNote("const volatile");

   if (t.IsAbstract())
      PrintNote("abstract");

   if (t.IsArray())
      PrintNote("array [" + NumToStr(t.ArrayLength()) + "]");

   if (t.IsClass())
      PrintNote("class");

   if (t.IsComplete())
      PrintNote("complete");

   if (t.IsEnum())
      PrintNote("enum");

   if (t.IsFundamental())
      PrintNote("fundamental");

   if (t.IsPointer())
      PrintNote("pointer");

   if (t.IsPointerToMember())
      PrintNote("pointer to member");

   if (t.IsReference())
      PrintNote("reference");

   if (t.IsStruct())
      PrintNote("struct");

   if (t.IsTypedef())
      PrintNote("typedef");

   if (t.IsUnion())
      PrintNote("union");

   if (t.IsUnqualified())
      PrintNote("unqualified");
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
void TReflexPrinter::PrintTypeName(Reflex::Type t)
{
   if (! t) {
      StyledText( "no-type " + t.Name(qual_opt), error_style);
   }
   #if 0
   else if (t.IsPointerToMember()) {
      StyledText(t.Name(qual_opt), member_pointer_style);
   }
   #endif
   else {
      StyledText(t.Name(qual_opt), type_style);
   }
}

//______________________________________________________________________________
void TReflexPrinter::PrintSrcType(Reflex::Type t)
{
   if (t.IsTypedef()) {
      StyledText("typedef", type_style);
      Put(" ");
      // Put(t.ToType().Name(qual_opt));
      PrintTypeName(t.ToType());
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
   PrintProperties (s.Properties ());
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

      Put(" = ");
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

         // Put(t.Name(qual_opt));
         PrintTypeName(t);

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
      PrintNote("artificial");

   if (m.IsConstructor())
      PrintNote("constructor");

   if (m.IsCopyConstructor())
      PrintNote("copy constructor");

   if (m.IsConverter())
      PrintNote("converter");

   if (m.IsOperator())
      PrintNote("operator");

   if (m.IsDestructor())
      PrintNote("destructor");

   if (m.IsAbstract())
      PrintNote("abstract");

   if (m.IsPureVirtual())
      PrintNote("pure virtual");

   if (m.IsTransient())
      PrintNote("transient");

   /* properties */

   PrintProperties (m.Properties ());

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
      Reflex::Scope u = *i;
      PrintLine("using", u.Name(qual_opt));
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
      Reflex::Scope sub_scope = *i;
      PrintLine("scope", sub_scope.Name());
   }
   #endif

   /* SubType */

   for (Reflex::Type_Iterator i = s.SubType_Begin(); i != s.SubType_End(); ++i) {
      Reflex::Type sub_type = *i;
      Reflex::Scope sub_scope = sub_type;
      if (! sub_scope)
         PrintSrcType(sub_type);
   }

   /* SubScope */

   for (Reflex::Scope_Iterator i = s.SubScope_Begin(); i != s.SubScope_End(); ++i) {
      Reflex::Scope sub_scope = *i;
      PrintSrcScope("sub-scope", sub_scope);  // %%
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
