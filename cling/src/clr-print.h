// @(#)root/cint:$Id$
// Author: Zdenek Culik   16/04/2010

#ifndef __CLR_PRINT_H__
#define __CLR_PRINT_H__

#include "Reflex/Reflex.h"
#include "TString.h"
#include "clr-util.h" // class THtmlHelper

/* -------------------------------------------------------------------------- */

class TReflexPrinter : public THtmlHelper {
private:
   void PrintLine(TString key, TString name);
   void PrintNote(TString s);

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
