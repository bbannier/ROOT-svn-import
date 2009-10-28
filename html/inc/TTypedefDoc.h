// @(#)root/html:$Id$
// Author: Axel Naumann 2007-01-09

/*************************************************************************
 * Copyright (C) 1995-2007, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TTypedefDoc
#define ROOT_TTypedefDoc

////////////////////////////////////////////////////////////////////////////
//                                                                        //
// TTypedefDoc                                                            //
//                                                                        //
// Documentation for a typedef                                            //
//                                                                        //
////////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TDocumented
#include "TDocumented.h"
#endif
#ifndef ROOT_TDocString
#include "TDocString.h"
#endif

namespace Doc {
class TTypedefDoc: public TDocumented {
public:

   TTypedefDoc();
   TTypedefDoc(TClassDoc* owner, const char* name,
               const TDocString& underlying, TDictionary* td = 0):
      TDocumented(name, td), fUnderlyingType(underlying), fOwner(owner) {}
   virtual ~TTypedefDoc();

   Int_t Compare(const TObject* obj) const;

   const char* GetURL() const;

private:
   TTypedefDoc(const TTypedefDoc&); // intentionally not implemented

   TDocString fUnderlyingType; // the underlying type of this typedef
   TDocumented* fOwner; // parent of this typedef
   ClassDef(TTypedefDoc, 1); // Documentation for a class
};
} // namespace Doc

#endif // ROOT_TTypedefDoc

