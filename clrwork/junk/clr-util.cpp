// clr-util.cpp
// Author: Zdenek Culik   16/04/2010

#include "clr-util.h"

//______________________________________________________________________________
TTextHelper::TTextHelper()
   : fIndent(0)
   , fStartLine(true)
   , fFileStream(0)
   , fStringStream(0)
   , fStream(0)
{
}

//______________________________________________________________________________
TTextHelper::~TTextHelper()
{
   delete fStream;
}

//______________________________________________________________________________
void TTextHelper::Open(const TString file_name)
{
   fFileStream = new std::ofstream();
   fFileStream->open(file_name.Data());
   fStream = fFileStream;
}

//______________________________________________________________________________
void TTextHelper::Close()
{
   fFileStream->close();
}

//______________________________________________________________________________
void TTextHelper::OpenString()
{
   fStringStream = new std::ostringstream();
   fStream = fStringStream;
}

//______________________________________________________________________________
TString TTextHelper::CloseString()
{
   return fStringStream->str();
}

//______________________________________________________________________________
void TTextHelper::SetIndent(int i)
{
   fIndent = i;
}

//______________________________________________________________________________
void TTextHelper::IncIndent()
{
   fIndent += 3;
}

//______________________________________________________________________________
void TTextHelper::DecIndent()
{
   fIndent -= 3;
}

//______________________________________________________________________________
void TTextHelper::PutPlainStr(const TString s)
{
   (*fStream) << s;
}

//______________________________________________________________________________
void TTextHelper::PutPlainChr(char c)
{
   (*fStream) << c;
}

//______________________________________________________________________________
void TTextHelper::OpenLine()
{
   TString spaces(' ', fIndent);
   // TString (character, count)
   // std::string (count, character)
   PutPlainStr(spaces);
   fStartLine = false;
}

//______________________________________________________________________________
void TTextHelper::CloseLine()
{
   fStartLine = true;
}

//______________________________________________________________________________
void TTextHelper::Put(const TString s)
{
   if (fStartLine) {
      OpenLine();
   }
   PutPlainStr(s);
}

//______________________________________________________________________________
void TTextHelper::PutChr(char c)
{
   if (fStartLine) {
      OpenLine();
   }
   PutPlainChr(c);
}

//______________________________________________________________________________
void TTextHelper::PutEol()
{
   PutPlainStr("\r\n");
   CloseLine();
}

//______________________________________________________________________________
void TTextHelper::PutLn(const TString s)
{
   Put(s);
   PutEol();
}

//______________________________________________________________________________
TString QuoteStrContent(const TString value)
{
   int len = value.Length();
   bool simple = true;
   for (int i = 0; (i < len) && simple; ++i) {
      unsigned char ch = value [i];
      if ((ch < ' ') || (ch == '\\') || (ch == '"') || (ch == 128) || (ch > 254)) {
         simple = false;
      }
   }
   if (simple) {
      return value;
   }
   TString result = "";
   const char* hex = "0123456789abcdef";
   for (int i = 0; i < len; ++i) {
      unsigned char ch = value[i];
      if ((ch < ' ') || (ch == 128) || (ch > 254)) {
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
               result = result + 'x' + hex[ch>>4] + hex[ch&15];
               break;
         }
      }
      else {
         if ((ch == '\\') || (ch == '"')) {
            result = result + '\\';
         }
         result = result + value[i]; // variable ch is unsigned char
      }
   }
   return result;
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
{
   // name = "value"
   if (!fUseHtml) {
      return;
   }
   Put(name);
   PutChr('=');
   PutChr(quote2);
   Put(QuoteStrContent(value));
   PutChr(quote2);
}

//______________________________________________________________________________
void THtmlHelper::Href(const TString url, const TString style, const TString text)
{
   // <a href="url" class="style"> text </a>
   if (!fUseHtml) {
      return;
   }
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

//______________________________________________________________________________
void THtmlHelper::Name(const TString name, const TString text)
{
   // <a name="name"> text </a>
   if (!fUseHtml) {
      return;
   }
   Put("<a ");
   Attr("name", name);
   if (text == "") {
      Put("/>");
   }
   else {
      Put(">");
      Put(text);
      Put("</a>");
   }
}

//______________________________________________________________________________
void THtmlHelper::StyleBegin(const TString style)
{
   if (!fUseHtml) {
      return;
   }
   Put("<span ");
   Attr("class", style);
   Put(">");
}

//______________________________________________________________________________
void THtmlHelper::StyleEnd()
{
   if (!fUseHtml) {
      return;
   }
   Put("</span>");
}

//______________________________________________________________________________
void THtmlHelper::StyledText(const TString text, const TString style)
{
   if (!fUseHtml) {
      Put(text);
      return;
   }
   Put("<span ");
   Attr("class", style);
   Put(">");
   Put(text);
   Put("</span>");
}

//______________________________________________________________________________
TString THtmlHelper::FceHtmlEscape(const TString s)
{
   int len = s.Length();
   bool ok = true;
   for (int i = 0; i < len; ++i) {
      char c = s[i];
      if ((c == '<') || (c == '>') || (c == '&')) {
         ok = false;
      }
   }
   if (ok) {
      return s;
   }
   TString result = "";
   for (int i = 0; i < len; ++i) {
      char c = s[i];
      if (c == '<') {
         result += "&lt;";
      }
      else if (c == '>') {
         result += "&gt;";
      }
      else if (c == '&') {
         result += "&amp;";
      }
      else {
         result += c;
      }
   }
   return result;
}

//______________________________________________________________________________
void THtmlHelper::PutHtmlEscape(const TString s)
{
   // Replace < > & characters
   if (!fUseHtml) {
      Put(s);
      return;
   }
   int len = s.Length();
   bool ok = true;
   for (int i = 0; i < len; ++i) {
      char c = s[i];
      if ((c == '<') || (c == '>') || (c == '&')) {
         ok = false;
      }
   }
   if (ok) {
      Put(s);
      return;
   }
   for (int i = 0; i < len; ++i) {
      char c = s[i];
      if (c == '<') {
         Put("&lt;");
      }
      else if (c == '>') {
         Put("&gt;");
      }
      else if (c == '&') {
         Put("&amp;");
      }
      else {
         PutChr(c);
      }
   }
}

//______________________________________________________________________________
void THtmlHelper::Head(const TString title)
{
   if (!fUseHtml) {
      return;
   }
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
   PutLn("</Head>");
   PutLn("<body>");
}

//______________________________________________________________________________
void THtmlHelper::Tail()
{
   if (!fUseHtml) {
      return;
   }
   PutLn("</body>");
   PutLn("</html>");
}

