// @(#)root/html:$Id$
// Author: Axel Naumann 2009-10-26

/*************************************************************************
 * Copyright (C) 1995-2009, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TModuleDoc
#define ROOT_TModuleDoc

////////////////////////////////////////////////////////////////////////////
//                                                                        //
// TModuleDoc                                                             //
//                                                                        //
// Documentation for a set of classes                                     //
//                                                                        //
////////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TDocumented
#include "TDocumented.h"
#endif
#ifndef ROOT_TDocStringTable
#include "TDocStringTable.h"
#endif

namespace Doc {
class TModuleDoc: public TDocumented {
public:
   TModuleDoc() {}
   TModuleDoc(const char* name):
      TDocumented(name) {}
   virtual ~TModuleDoc();

   void AddClass(const char* name);
   const TObjArray* GetClasses() const { return fClasses.GetArray(); }

   Int_t Compare(const TObject* obj) const;

   virtual const char* GetURL() const;

protected:
   TDocStringTable fClasses; // classes contained in this module
   TDictionary* FindDictionary() const { return 0; }

private:
   ClassDef(TModuleDoc, 1); // Documentation for a file
};
} // namespace Doc

#endif // ROOT_TModuleDoc

