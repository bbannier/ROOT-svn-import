// @(#)root/html:$Id$
// Author: Axel Naumann 2009-10-26

/*************************************************************************
 * Copyright (C) 1995-2009, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TDocumented
#define ROOT_TDocumented

////////////////////////////////////////////////////////////////////////////
//                                                                        //
// TDocumented                                                            //
//                                                                        //
// Base class for documentation                                           //
//                                                                        //
////////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TObject
#include "TObject.h"
#endif
#ifndef ROOT_TDocString
#include "TDocString.h"
#endif

class TDictionary;

namespace Doc {
class TDocumented: public TObject {
public:
   // Access specification
   enum EAccess {
      kPublic = BIT(14),
      kProtected,
      kPrivate,
      kUnknownAccess,
      kNumAccess = kUnknownAccess,
      kAccessBitMask = BIT(14) | BIT(15)
   };

   // A file location, combination of file name idx in parent's
   // TClassDoc::fFiles and line number.
   class TFileLocation {
   public:
      TFileLocation(UInt_t fileNameIdx, UInt_t lineNo):
         fFileNameIdx(fileNameIdx), fLineNo(lineNo) {}

   private:
      UInt_t fFileNameIdx;
      UInt_t fLineNo;
   };

   enum ELocationSpec {
      kDeclaration,
      kDefinition,
      kNumLocationSpec
   };

   TDocumented(): fTDictionary(0) {}
   TDocumented(const char* name, TDictionary* dict = 0):
      fName(name), fTDictionary(dict) {}
   virtual ~TDocumented();

   void SetAccess(EAccess access) { SetBit(access); }
   EAccess GetAccess() const { return (EAccess)TestBits(kAccessBitMask); }

   Bool_t IsSortable() const { return kTRUE; }
   Int_t  Compare(const TObject* obj) const;

   TDictionary* GetDictionary() const {
      if (!fTDictionary) fTDictionary = FindDictionary();
      return fTDictionary;
   }

   virtual const char* GetURL() const = 0;

protected:
   virtual TDictionary* FindDictionary() const;

private:
   TDocumented(const TDocumented&); // intentionally not implemented

   TString fName; // name of the entity
   TDocString fDocumentation; // documentation of this entity
   UInt_t fFileLocations[kNumLocationSpec]; // locations of declaration and definition

   mutable TDictionary* fTDictionary; //! dictionary element documented by this object

   ClassDef(TDocumented, 1); // Documentation base class
};
} // namespace Doc

#endif // ROOT_TDocumented
