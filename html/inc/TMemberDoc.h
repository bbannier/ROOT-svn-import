// @(#)root/html:$Id$
// Author: Axel Naumann 2009-10-26

/*************************************************************************
 * Copyright (C) 1995-2009, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TMemberDoc
#define ROOT_TMemberDoc

////////////////////////////////////////////////////////////////////////////
//                                                                        //
// TMemberDoc                                                             //
//                                                                        //
// Documentation for a member (function or data)                          //
//                                                                        //
////////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TDocumented
#include "TDocumented.h"
#endif

namespace Doc {
class TMemberDoc: public TDocumented {
public:

   enum EStorageCV {
      kStatic = BIT(16),
      kMutable = BIT(17),
      kConst = BIT(18),
      kVolatile = BIT(19),
      kVirtual = BIT(20),
      kPure = BIT(21),
      kStorageCVBitMask = BIT(16) | BIT(17) | BIT(18) | BIT(19) | BIT(20) | BIT(21)
   };

   TMemberDoc();
   TMemberDoc(TClassDoc* owner, const char* name, const TDocString& type);
   virtual ~TMemberDoc();

   const char* GetType() const { return fType; }

   void SetStorageCV(UInt_t storageCV) { SetBit(storageCV); }
   UInt_t  GetStorageCV() const { return TestBits(kStorageCVBitMask); }

   Int_t Compare(const TObject* obj) const;

   virtual const char* GetAnchor() const = 0;

private:
   TDocString fType; // the member's type
   TDocumented* fOwner; // parent of this member
   ClassDef(TMemberDoc, 1); // Documentation for a member
};
} // namespace Doc

#endif // ROOT_TMemberDoc

