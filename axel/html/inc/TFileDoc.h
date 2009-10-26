// @(#)root/html:$Id$
// Author: Axel Naumann 2009-10-26

/*************************************************************************
 * Copyright (C) 1995-2009, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TFileDoc
#define ROOT_TFileDoc

////////////////////////////////////////////////////////////////////////////
//                                                                        //
// TFileDo  c                                                             //
//                                                                        //
// Documentation for a file                                               //
//                                                                        //
////////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TDocumented
#include "TDocumented.h"
#endif

namespace Doc {
class TFileDoc: public TDocumented {
public:
   TFileDoc() {}
   TFileDoc(const char* name, const char* inclAs):
      TDocumented(name), fInclAs(inclAs) {}
   virtual ~TFileDoc();

   Int_t Compare(const TObject* obj) const;

   virtual const char* GetURL() const;

protected:
   TDictionary* FindDictionary() const { return 0; }

private:
   TString fInclAs; // How this file gets #include'd
   ClassDef(TFileDoc, 1); // Documentation for a file
};
} // namespace Doc

#endif // ROOT_TFileDoc

