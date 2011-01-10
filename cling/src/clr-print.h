// @(#)root/cint:$Id$
// Author: Zdenek Culik   16/04/2010

#ifndef __CLR_PRINT_H__
#define __CLR_PRINT_H__

#include "Reflex/Reflex.h"
#include "TString.h"
#include <fstream> // class ofstream
#include <sstream> // class ostringstream

/* -------------------------------------------------------------------------- */

class THtmlHelper {
private:
   int  fIndent;
   bool fStartLine;

   std::ofstream* fFileStream;
   std::ostringstream* fStringStream;
   std::ostream* fStream;

protected:
   bool fUseHtml;
   TString fStyleFileName;

   void PutPlainStr(const TString s);
   void PutPlainChr(char c);

   virtual void OpenLine();
   virtual void CloseLine();

public:
   void Open(const TString name);
   void Close();

   void OpenString();
   TString CloseString();

   void EnableHtml(bool param_use_html) { fUseHtml = param_use_html; }
   bool HtmlEnabled() { return fUseHtml; }

   void SetStyleFileName(const TString fileName) { fStyleFileName = fileName; }

   void Put(const TString s);
   void PutChr(char c);
   void PutEol();
   void PutLn(const TString s);

   void SetIndent(int i);
   void IncIndent();
   void DecIndent();

   void Style();

   void Head(const TString title);
   void Tail();

   static TString FceHtmlEscape(const TString s);
   void Attr(const TString name, const TString value);

   void Href(const TString url, const TString style, const TString text);
   void Name(const TString name, const TString text);

   void StyleBegin(const TString style);
   void StyleEnd();
   void StyledText(const TString text, const TString style);

   void PutHtmlEscape(const TString s);

   THtmlHelper();
   virtual ~ THtmlHelper();
};

/* -------------------------------------------------------------------------- */

class TReflexPrinter : public THtmlHelper {
private:
   void PrintLine(TString key, TString name);
   void PrintNote(TString s);
   void PrintProperties(const Reflex::PropertyList& list);

   void PrintTypeName(Reflex::Type t);
   void PrintTypeNotes(Reflex::Type t);
   void PrintType(Reflex::Type t);
   void PrintSrcType(Reflex::Type t);

   void PrintNamespace(Reflex::Scope s);
   void PrintClass(Reflex::Scope s);
   void PrintEnum(Reflex::Scope s);
   void PrintVariable(Reflex::Member m);
   void PrintFunction(Reflex::Member m);

   void PrintMember(Reflex::Member m);
   void PrintAccess(Reflex::Member m, int& acs);
   void PrintScope(Reflex::Scope s, bool show_access = false);
   void PrintSrcScope(TString text, Reflex::Scope s);

   #if 0
   void PrintTypeTemplate(Reflex::TypeTemplate s);
   void PrintMemberTemplate(Reflex::MemberTemplate s);
   #endif

public:
   TReflexPrinter () { }
   virtual ~ TReflexPrinter () { }

   void Print(Reflex::Scope scope);
};

/* -------------------------------------------------------------------------- */

#endif /* __CLR_PRINT_H__ */
