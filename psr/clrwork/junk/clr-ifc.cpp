// clr-ifc.cpp

#include "Reflex/DictionaryGenerator.h"
#include "TString.h"

#include "clr-ifc.h"
#include "clr-scan.h"
#include "clr-print.h"

#ifdef DUMP
#include "clr-dump.h"
#endif

#ifdef LIST
#include "clr-list.h"
#endif

void ClrPrint(TString file_name, TString style_file_name = "", bool useHtml = false);

/* ---------------------------------------------------------------------- */

bool HasSuffix(const std::string value, const std::string suffix)
{
   int len = value.length();
   int suffix_len = suffix.length();
   return len > suffix_len &&
          value.substr(len - suffix_len) == suffix;
}

std::string RemoveSuffix(const std::string value, const std::string suffix)
{
   if(HasSuffix(value, suffix))
      return value.substr(0, value.length() - suffix.length());
   else
      return value;
}

#if 0
TString RemoveSuffix(const TString value, const TString suffix)
{
   if(value.EndsWith(suffix))
      return value(0, value.Length() - suffix.Length());
   else
      return value;
}
#endif

/********************************** SCAN **********************************/

clang::ASTContext* Ctx = 0;

bool use_html = false;

std::string output_print_file_name;
std::string input_style_file_name;
std::string output_dict_file_name;

TScanner scanner;

/* ---------------------------------------------------------------------- */

void ClrInit(clang::ASTContext* C, std::string print_file_name,
   std::string style_file_name, std::string dict_file_name)
{
   Ctx = C;
   output_print_file_name = print_file_name;
   input_style_file_name = style_file_name;
   output_dict_file_name = dict_file_name;
   use_html = (input_style_file_name != "");
#ifdef LIST
   ClrListOpen(RemoveSuffix(output_print_file_name, "-print.html") + "-list.html",   // !?
               style_file_name,
               use_html);
#endif
   // --
}

/* ---------------------------------------------------------------------- */

void ClrSend(clang::Decl * D)
{
   scanner.Scan(Ctx, D);
#ifdef LIST
   ClrListDecl(D);
#endif
   // --
}

/* ---------------------------------------------------------------------- */

void ClrOutput()
{
   if(output_print_file_name != "") {
      ClrPrint(output_print_file_name, input_style_file_name, use_html);  // text or HTML output
#ifdef DUMP
      ClrDump(RemoveSuffix(output_print_file_name, "-print.html") + "-dump.html",   // !?
              input_style_file_name,
              use_html);
#endif
      // --
#ifdef LIST
      ClrListClose();
#endif
      // --
   }
   // if (false)
   if (output_dict_file_name != "") {
      Reflex::DictionaryGenerator generator;
#if 1
      Reflex::Scope::GlobalScope().GenerateDict(generator);
#else
      GlobalScope_GenerateDict(generator);
#endif
      generator.Print(output_dict_file_name.c_str());
   }
}

/*********************************** PRINT ************************************/

void ClrPrint(TString file_name, TString style_file_name, bool useHtml)
{
   Reflex::Scope scope = Reflex::Scope::GlobalScope();
   TReflexPrinter io;
   io.Open(file_name);
   io.EnableHtml(useHtml);
   io.SetStyleFileName(style_file_name);
   io.Print(scope);
   io.Close();
}

/********************************** TEST **********************************/

void ClrTest()
{
}

