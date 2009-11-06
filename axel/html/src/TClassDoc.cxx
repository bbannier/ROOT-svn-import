// @(#)root/html:$Id$
// Author: Axel Naumann 2009-10-26

/*************************************************************************
 * Copyright (C) 1995-2009, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include "TClassDoc.h"

#include "TClass.h"

//______________________________________________________________________________
Doc::TClassDoc::TClassDoc(const char* name, TClass* cl, const char* module):
   TDocumented(name, cl), fModule(module)
{
   EKind kind = kUnknownKind;
   // Constructor initializing all members
   if (cl->Property() & kIsClass) {
      kind = kClass;
   } else if (cl->Property() & kIsEnum) {
      kind = kEnum;
   } else if (cl->Property() & kIsNamespace) {
      kind = kNamespace;
   } else if (cl->Property() & kIsStruct) {
      kind = kStruct;
   } else if (cl->Property() & kIsTypedef) {
      kind = kTypedef;
   } else if (cl->Property() & kIsUnion) {
      kind = kUnion;
   }
   SetKind(kind);

   fMembers.SetOwner();
   fTypes.SetOwner();
   fSeeAlso.SetOwner();
}

//______________________________________________________________________________
Doc::TClassDoc::~TClassDoc()
{
   // Destructor
}

//______________________________________________________________________________
const char* Doc::TClassDoc::GetTitle() const {
   // The title represents the kind, to allow a EKind to be determined from
   // the TKey holding a TDocTypeEntry.
   static const char* sTitles[kNumKinds + 1] = {
      "c", "s", "u", "e", "n", "t", "?"
   };
   return sTitles[GetKind() - kClass];
}

