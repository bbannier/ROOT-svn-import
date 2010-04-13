
/* clr-util.h */

#ifndef __CLR_UTIL_H__
#define __CLR_UTIL_H__

#include <string>
#include <map>
#include <fstream>
#include <sstream>

namespace clr_util {

using std::string;

/* ---------------------------------------------------------------------- */

void info    (const string msg, const string location = "");
void warning (const string msg, const string location = "");
void error   (const string msg, const string location = "");

/* ---------------------------------------------------------------------- */

string RemoveSuffix (const string value, const string suffix);

/* ---------------------------------------------------------------------- */

class TextOutput
{
   private:
      int  Indent;
      bool StartLine;

      std::ofstream * FileStream;
      std::ostringstream * StringStream;
      std::ostream * Stream;

   protected:
      void PutPlainStr (const string s);
      void PutPlainChr (char c);

      virtual void OpenLine ();
      virtual void CloseLine ();

   public:
      void Open (const string name);
      void Close ();

      void OpenString ();
      string CloseString ();

      void Put (const string s);
      void PutChr (char c);
      void PutEol ();
      void PutLn (const string s);

      void SetIndent (int i);
      void IncIndent ();
      void DecIndent ();

      TextOutput ();
      virtual ~ TextOutput ();
};

/* ---------------------------------------------------------------------- */

class HtmlOutput : public TextOutput
{
   protected:
      bool use_html;

   public:
      void enable_html (bool param_use_html) { use_html = param_use_html; }
      bool html_enabled () { return use_html; }

      void head (const string title, const string style_file_name = "");
      void tail ();

      static string fce_html_escape (const string s);
      void attr (const string name, const string value);

      void a_href (const string url, const string style, const string text);
      void a_name (const string name, const string text);

      void style_begin (const string style);
      void style_end ();
      void styled_text (const string text, const string style);

      void put_html_escape (const string s);

   public:
      HtmlOutput () : use_html (true) {  }
      virtual ~ HtmlOutput () { }
};

/* ---------------------------------------------------------------------- */

template <class Key, class Data>
class Dictionary
{
   private:
      typedef std::map <Key, Data> dict_type; // standard map, not MAP_TYPE, for types without hash<Key>
      typedef typename dict_type::iterator dict_iter;
      dict_type dict;

   public:
      bool contains (Key inx)          { return dict.find (inx) != dict.end (); }
      void store (Key inx, Data value) { dict [inx] = value; }
      void clear ()                    { dict.clear (); }

      Data get (Key inx)
      {
         dict_iter it = dict.find (inx);
         if (it == dict.end ())
            return Data ();
         else
            return it->second;
      }
};

/* ---------------------------------------------------------------------- */

} // close namespace

#endif /* __CLR_UTIL_H__ */
