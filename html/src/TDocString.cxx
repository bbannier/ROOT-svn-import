// @(#)root/html:$Id$
// Author: Axel Naumann 2009-10-26

/*************************************************************************
 * Copyright (C) 1995-2009, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include "TDocString.h"

#include "TClassDoc.h"
#include "TTypedefDoc.h"
#include "TFunctionDoc.h"
#include "TDataDoc.h"

//______________________________________________________________________________
////////////////////////////////////////////////////////////////////////////////
//
// A reference at a given position and length in a string. The reference is an
// index into the parent's TDocStringTable.
//
////////////////////////////////////////////////////////////////////////////////

ClassImp(Doc::TDocString::TDocRef);

Doc::TDocString::TDocRef::~TDocRef()
{
   // Destructor
}

//______________________________________________________________________________
////////////////////////////////////////////////////////////////////////////////
//
// A string with references. The references span multiple characters; they are
// described by TDocString::TDocRef.
//
////////////////////////////////////////////////////////////////////////////////

ClassImp(Doc::TDocString);

//______________________________________________________________________________
Doc::TDocString::~TDocString()
{
   // Destructor
   delete fRefs;
}

//______________________________________________________________________________
void Doc::TDocString::AddRef(TDocStringTable* table, Ssiz_t from, Ssiz_t to,
                             TDocumented* doc)
{
   // Add a reference to a documented string.
   TDocRef::ERefType type = TDocRef::kNumRefTypes;
   if (doc->InheritsFrom(TMemberDoc::Class())) {
      type = TDocRef::kMember;
   } else if (doc->InheritsFrom(TClassDoc::Class())) {
      type = TDocRef::kClass;
   } else if (doc->IsA() == TTypedefDoc::Class()) {
      type = TDocRef::kTypedef;
   }

   if (type == TDocRef::kNumRefTypes) {
      Error("AddRef", "Cannot add reference to entity %s\n",
            doc->ClassName());
      return;
   }

   if (!fRefs) {
      fRefs = new TList();
      fRefs->SetOwner();
   }
   fRefs->Add(new TDocRef(table, doc->GetName(), from, to, type));
}
