// @(#)root/html:$Id$
// Author: Axel Naumann 2009-10-26

/*************************************************************************
 * Copyright (C) 1995-2009, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TDocString
#define ROOT_TDocString

////////////////////////////////////////////////////////////////////////////
//                                                                        //
// TDocString                                                             //
//                                                                        //
// String with a collection of documentation references.                  //
//                                                                        //
////////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TString
#include "TString.h"
#endif
#ifndef ROOT_TList
#include "TList.h"
#endif
#ifndef ROOT_TDocStringTable
#include "TDocStringTable.h"
#endif

namespace Doc {
class TDocStringTable;
class TDocumented;

class TDocString: public TObject {
public:
   class TDocRef: public TObject {
   public:
      enum ERefType {
         kClass = BIT(16),
         kMember,
         kTypedef,
         kNumRefTypes,
         kRefTypeMask = BIT(16) | BIT(17)
      };
      TDocRef(TDocStringTable* table, const char* refname,
              Ssiz_t from, Ssiz_t to, ERefType reftype):
         fStrId(table->GetIndex(refname)),
         fFrom(from),
         fTo(to) {
         SetBit(reftype);
      }
      virtual ~TDocRef();

      ERefType GetRefType() const {
         // Return the type of the reference; for all but
         // kClass, the documented sub string determines
         // the reference member.
         return (ERefType) TestBits(kRefTypeMask);
      }

      TSubString GetSubString(const TString& doc) const {
         // Return the documented sub string
         return doc(fFrom, fTo);
      }

      const TString& GetRefString(TDocStringTable* table) const {
         // Return the string corresponding to the documented entity
         return table->GetString(fStrId);
      }

      Ssiz_t GetFrom() const { return fFrom; }
      Ssiz_t GetTo() const { return fTo; }

   private:
      UInt_t fStrId; // index of reference in TDocStringTable
      Ssiz_t fFrom; // start pos of documentation reference
      Ssiz_t fTo; // end pos of documentation reference

      ClassDef(TDocRef, 1); // reference in a documentation string
   };

   TDocString(const char* str = ""): fStr(str), fRefs(0) {}
   virtual ~TDocString();

   void AddRef(TDocStringTable* table, Ssiz_t from, Ssiz_t to, TDocumented* doc);
   TCollection* GetListOfRefs() const { return fRefs; }

   operator const char*() const { return fStr; }
   operator const TString&() const { return fStr; }

private:
   TString fStr; // the string
   TList*  fRefs; // references on the string

   ClassDef(TDocString, 1); // Documentation base class
};
} // namespace Doc

#endif // ROOT_TDocString
