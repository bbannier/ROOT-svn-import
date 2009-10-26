// @(#)root/html:$Id$
// Author: Axel Naumann 2009-10-26

/*************************************************************************
 * Copyright (C) 1995-2009, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TLibraryDoc
#define ROOT_TLibraryDoc

////////////////////////////////////////////////////////////////////////////
//                                                                        //
// TLibraryDoc                                                             //
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
class TLibraryDoc: public TDocumented {
public:
   TLibraryDoc() {}
   TLibraryDoc(const char* name):
      TDocumented(name) {}
   virtual ~TLibraryDoc();

   void AddModule(const char* name);
   const TObjArray* GetModules() const { return fModules.GetArray(); }

   void AddDependency(const char* name);
   const TObjArray* GetDependencies() const { return fDependencies.GetArray(); }

   Int_t Compare(const TObject* obj) const;

   virtual const char* GetURL() const;

protected:
   TDictionary* FindDictionary() const { return 0; }

private:
   TDocStringTable fModules; // modules contained in this library
   TDocStringTable fDependencies; // libraries that this library depends on

   ClassDef(TLibraryDoc, 1); // Documentation for a library
};
} // namespace Doc

#endif // ROOT_TLibraryDoc

