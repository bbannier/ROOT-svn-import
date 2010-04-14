
/* clr-util.cc */

#include "clr-util.h"

#ifdef TRACE
   #include "clr-trace.h"
#endif

#include <string>
#include <iostream>
#include <stdexcept> // class std::runtime_error

using std::string;

namespace clr_util {

/* ---------------------------------------------------------------------- */

void show_message (const string msg, const string location)
{
   string txt = location;

   if (txt != "")
      txt = txt + " ";

   txt = txt + msg;

   std::cerr << "CLR-" << txt << std::endl;
}

void info (const string msg, const string location)
{
   show_message (msg, location);
}

void warning (const string msg, const string location)
{
   show_message ("warning: " + msg, location);
}

void error (const string msg, const string location)
{
   show_message ("error: " + msg, location);
   #ifdef TRACE
      trace ();
   #endif
   throw new std::runtime_error ("error: " + msg  + ", " +  location);
}

/* ---------------------------------------------------------------------- */

bool HasSuffix (const string value, const string suffix)
{
   int len = value.length ();
   int suffix_len = suffix.length ();

   return len > suffix_len &&
          value.substr (len-suffix_len) == suffix;
}

string RemoveSuffix (const string value, const string suffix)
{
   if (HasSuffix (value, suffix))
      return value.substr (0, value.length () - suffix.length ());
   else
      return value;
}

/****************************** TEXT OUTPUT *******************************/

TextOutput::TextOutput ():
   FileStream (NULL),
   StringStream (NULL),
   Stream (NULL),
   Indent (0),
   StartLine (true)
{
}

TextOutput::~TextOutput ()
{
   if (Stream != NULL)
      delete Stream;
}

void TextOutput::Open (const std::string file_name)
{
   FileStream = new std::ofstream ();
   FileStream->open (file_name.c_str ());
   Stream = FileStream;
}

void TextOutput::Close ()
{
   FileStream->close ();
}

/* ---------------------------------------------------------------------- */

void TextOutput::OpenString ()
{
   StringStream = new std::ostringstream ();
   Stream = StringStream;
}

string TextOutput::CloseString ()
{
   return StringStream->str ();
}

/* ---------------------------------------------------------------------- */

void TextOutput::SetIndent (int i)
{
   Indent = i;
}

void TextOutput::IncIndent ()
{
   Indent += 3;
}

void TextOutput::DecIndent ()
{
   Indent -= 3;
}

/* ---------------------------------------------------------------------- */

void TextOutput::PutPlainStr (const string s)
{
   (*Stream) << s;
}

void TextOutput::PutPlainChr (char c)
{
   (*Stream) << c;
}

/* ---------------------------------------------------------------------- */

void TextOutput::OpenLine ()
{
   string spaces (Indent, ' ');
   PutPlainStr (spaces);
   StartLine = false;
}

void TextOutput::CloseLine ()
{
   StartLine = true;
}

/* ---------------------------------------------------------------------- */

void TextOutput::Put (const string s)
{
   if (StartLine)
      OpenLine ();

   PutPlainStr (s);
}

void TextOutput::PutChr (char c)
{
   if (StartLine)
      OpenLine ();

   PutPlainChr (c);
}

void TextOutput::PutEol ()
{
   PutPlainStr ("\n");
   CloseLine ();
}

void TextOutput::PutLn (const string s)
{
   Put (s);
   PutEol ();
}


/****************************** QUOTE STRING ******************************/

string QuoteStrContent (const string value)
{
   int len = value.length ();

   bool simple = true;
   for (int i = 0; i < len && simple; i++)
   {
      unsigned char ch = value [i];
      if (ch < ' ' || ch == '\\' || ch == '"' || ch == 128 || ch >= 255)
         simple = false;
   }

   if (simple)
   {
      return value;
   }
   else
   {
      string result = "";
      const char hex [16+1] = "0123456789abcdef";

      for (int i = 0; i < len; i++)
      {
         unsigned char ch = value [i];

         if (ch < ' ' || ch == 128 || ch >= 255)
         {
            result = result + '\\';
            switch (ch)
            {
               case '\a': result = result + 'a'; break;
               case '\b': result = result + 'b'; break;
               case '\f': result = result + 'f'; break;
               case '\n': result = result + 'n'; break;
               case '\r': result = result + 'r'; break;
               case '\t': result = result + 't'; break;
               case '\v': result = result + 'v'; break;

               default:
                  result = result + 'x' + hex [ch >> 4] + hex [ch & 15];
                  break;
            }
         }
         else
         {
            if (ch == '\\' || ch == '"')
               result = result + '\\';
            result = result + value [i]; // variable ch is unsigned char
         }
      }

      return result;
   }
}

const char quote2 = '"';

string QuoteStr (const string value, char quote = quote2)
{
   return quote + QuoteStrContent (value) + quote;
}

/****************************** HTML OUTPUT *******************************/

inline string fce_attr (const string name, const string value)
{
   return name + "=" + quote2 + QuoteStrContent (value) + quote2;
}

void HtmlOutput::attr (const string name, const string value)
// name = "value"
{
   Put (name);
   PutChr ('=');
   PutChr (quote2);
   Put (QuoteStrContent (value));
   PutChr (quote2);
}

/* ---------------------------------------------------------------------- */

void HtmlOutput::a_href (const string url, const string style, const string text)
// <a href="url" class="style"> text </a>
{
   Put ("<a ");
   attr ("href", url);

   if (style != "")
   {
      Put (" ");
      attr ("class", style);
   }

   Put (">");
   Put (text);
   Put ("</a>");
}

void HtmlOutput::a_name (const string name, const string text)
// <a name="name"> text </a>
{
   Put ("<a ");
   attr ("name", name);

   if (text == "")
   {
      Put ("/>");
   }
   else
   {
      Put (">");
      Put (text);
      Put ("</a>");
   }
}

/* ---------------------------------------------------------------------- */

void HtmlOutput::style_begin (const string style)
{
   Put ("<span ");
   attr ("class", style);
   Put (">");
}

void HtmlOutput::style_end ()
{
   Put ("</span>");
}

void HtmlOutput::styled_text (const string text, const string style)
{
   if (use_html)
   {
      Put ("<span ");
      attr ("class", style);
      Put (">");
      Put (text);
      Put ("</span>");
   }
   else
   {
      Put (text);
   }
}

/* ---------------------------------------------------------------------- */

string HtmlOutput::fce_html_escape (const string s)
{
   int len = s.length ();

   bool ok = true;
   for (int i = 0; i < len; i++)
   {
       char c = s[i];
       if (c == '<' || c == '>' || c == '&')
          ok = false;
   }

   if (ok)
   {
      return s;
   }
   else
   {
      string result = "";

      for (int i = 0; i < len; i++)
      {
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

void HtmlOutput::put_html_escape (const string s)
// Replace < > & characters
{
   int len = s.length ();

   bool ok = true;
   for (int i = 0; i < len; i++)
   {
       char c = s[i];
       if (c == '<' || c == '>' || c == '&')
          ok = false;
   }

   if (ok)
   {
       Put (s);
   }
   else
   {
      for (int i = 0; i < len; i++)
      {
         char c = s[i];
         if (c == '<')
             Put ("&lt;");
         else if (c == '>')
             Put ("&gt;");
         else if (c == '&')
             Put ("&amp;");
         else
             PutChr (c);
      }
   }
}

/* ---------------------------------------------------------------------- */

void HtmlOutput::head (const string title, const string style_file_name)
{
   string charset = "ISO-8859-1";

   PutLn ("<?xml " + fce_attr ("version", "1.0") + "?>");

   PutLn ("<!DOCTYPE html PUBLIC " +
          QuoteStr ("-//W3C//DTD XHTML 1.0 Transitional//EN") + " " +
          QuoteStr ("http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd") + ">");

   PutLn ("<html " + fce_attr ("xmlns", "http://www.w3.org/1999/xhtml") + " " +
                     fce_attr ("xml:lang", "en") + " " +
                     fce_attr ("lang", "en") + ">");

   PutLn ("<head>");

   PutLn ("<meta " + fce_attr ("http-equiv", "Content-Type") + " " +
                     fce_attr ("content", "text/html; charset=" + charset) + " />");

   PutLn ("<title>" + title + "</title>");

   if (style_file_name != "")
   {
      PutLn ("<link " + fce_attr ("rel", "stylesheet") + " " +
                        fce_attr ("type", "text/css") + " " +
                        fce_attr ("href", style_file_name ) + " />");
   }

   PutLn ("</head>");
   PutLn ("<body>");
}

void HtmlOutput::tail ()
{
   PutLn ("</body>");
   PutLn ("</html>");
}

/* ---------------------------------------------------------------------- */

} // close namespace

