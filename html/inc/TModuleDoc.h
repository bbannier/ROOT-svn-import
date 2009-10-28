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

class TCollection;
class THashList;

namespace Doc {
class TModuleDoc: public TDocumented {
public:
   TModuleDoc() {}
   TModuleDoc(const char* name, const char* parent):
      TDocumented(name), fSuper(parent), fSub(0) {}
   virtual ~TModuleDoc();

   void AddClass(const char* name);
   const TObjArray* GetClasses() const { return fClasses.GetArray(); }

   Int_t Compare(const TObject* obj) const;

   virtual const char* GetURL() const;

   const TString& GetSuper() const { return fSuper; }
   TCollection*   GetSub() const;

   void        SetSelected(Bool_t sel = kTRUE) { fSelected = sel; }
   Bool_t      IsSelected() const { return kTRUE; }

protected:
   TDictionary* FindDictionary() const { return 0; }

private:
   TDocStringTable fClasses; // classes contained in this module
   TString fSuper; // name of module containing this module

   Bool_t  fSelected; //! whether selected for documentation output
   mutable THashList*  fSub; //! modules contained in this module

   ClassDef(TModuleDoc, 1); // Documentation for a file
};
} // namespace Doc

#endif // ROOT_TModuleDoc

