// @(#)root/html:$Id$
// Author: Axel Naumann 2009-10-26

/*************************************************************************
 * Copyright (C) 1995-2009, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include "TDocStringTable.h"

#include "TObjString.h"

//______________________________________________________________________________
////////////////////////////////////////////////////////////////////////////////
//
// Store a collection of (ideally often reused) strings, that can be retrieved
// by a unique index; the index can be retrieved by a hashed lookup on the
// string.
// The index is stored as the unique ID of the TObjString. Each strings is thus
// referenced both by the THashTable and the TObjArray member. Because the
// collections contain pointers, each TObjString is only written once.
////////////////////////////////////////////////////////////////////////////////

ClassImp(Doc::TDocStringTable);


//______________________________________________________________________________
Doc::TDocStringTable::~TDocStringTable()
{
   // Destructor
}

//______________________________________________________________________________
UInt_t Doc::TDocStringTable::GetIndexOrAdd(const char* str)
{
   // Get the index for the given string, add it if it isn't found.
   if (!str) {
      Error("GetIndexOrAdd()", "string is NULL!");
      return (UInt_t) -1;
   }
   TObjString* os = (TObjString*) fHashedStrings.FindObject(str);
   if (!os) {
      os = new TObjString(str);
      os->SetUniqueID(fIndexedStrings.GetEntries());
      fHashedStrings.Add(os);
      fIndexedStrings.Add(os);
   }
   return os->GetUniqueID();
}

//______________________________________________________________________________
const TString& Doc::TDocStringTable::GetString(UInt_t idx) const
{
   // Get the string for a given index
   static const TString sNullRef;

   TObjString* os = (TObjString*)fIndexedStrings[idx];
   if (!os) {
      return sNullRef;
   }
   return os->String();
}

//______________________________________________________________________________
Int_t Doc::TDocStringTable::GetIndex(const char* str) const
{
   // Determine whether the tablew contains str
   if (!str) {
      Error("Contains()", "string is NULL!");
      return -1;
   }
   TObjString* os = (TObjString*) fHashedStrings.FindObject(str);
   if (!os) return -1;
   return os->GetUniqueID();
}

//______________________________________________________________________________
void Doc::TDocStringTable::Remove(const char* str)
{
   // Remove a string
   if (!str) {
      Error("Remove()", "string is NULL!");
      return;
   }
   Int_t idx = GetIndex(str);
   if (idx == -1) {
      Error("Remove()", "string is not found!");
      return;
   }
   fHashedStrings.Remove(fIndexedStrings[idx]);
   fIndexedStrings.RemoveAt(idx);
}

