// @(#)root/cint:$Id$
// Author: Zdenek Culik   16/04/2010

#ifndef __CLR_UTIL_H__
#define __CLR_UTIL_H__

#include "TString.h"
#include <fstream>
#include <sstream>

/* -------------------------------------------------------------------------- */

class TTextHelper {
private:
   int  fIndent;
   bool fStartLine;

   std::ofstream* fFileStream;
   std::ostringstream* fStringStream;
   std::ostream* fStream;

protected:
   void PutPlainStr(const TString s);
   void PutPlainChr(char c);

   virtual void OpenLine();
   virtual void CloseLine();

public:
   void Open(const TString name);
   void Close();

   void OpenString();
   TString CloseString();

   void Put(const TString s);
   void PutChr(char c);
   void PutEol();
   void PutLn(const TString s);

   void SetIndent(int i);
   void IncIndent();
   void DecIndent();

   TTextHelper();
   virtual ~ TTextHelper();
};

/* -------------------------------------------------------------------------- */

class THtmlHelper : public TTextHelper {
protected:
   bool fUseHtml;
   TString fStyleFileName;

public:
   void EnableHtml(bool param_use_html) {
      fUseHtml = param_use_html;
   }
   bool HtmlEnabled() {
      return fUseHtml;
   }

   void SetStyleFileName(const TString fileName) {
      fStyleFileName = fileName;
   }

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

public:
   THtmlHelper() : fUseHtml(true) {  }
   virtual ~ THtmlHelper() { }
};

/* -------------------------------------------------------------------------- */

#endif /* __CLR_UTIL_H__ */
