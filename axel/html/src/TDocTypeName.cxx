// @(#)root/html:$Id$
// Author: Axel Naumann 2009-10-26

/*************************************************************************
 * Copyright (C) 1995-2009, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include "TDocTypeName.h"

#include "TClassDoc.h"
#include "TDictionary.h"
#include "TDocFileDB.h"
#include "TKey.h"
#include "TSystem.h"
#include "TTypedefDoc.h"

//______________________________________________________________________________
Doc::TDocTypeName::TDocTypeName(TKey* key, TModuleDoc* module):
   fModule(module), fKey(key), fDict(0), fDoc(0)
{
   // Initialize a type name with documentation in a ROOT file.

   // The title represents the kind, to allow a EKind to be determined from
   // the TKey holding a TDocTypeEntry.
   static const char* sTitles = "csuent?";
   const char* posTitle = strchr(sTitles, key->GetTitle()[0]);
   EKind kind = kUnknownKind;
   if (posTitle) {
      const int ikind = (int)kClass + (posTitle - sTitles);
      kind = (EKind) ikind;
   }
   SetKind(kind);
}

//______________________________________________________________________________
Doc::TDocTypeName::TDocTypeName(TDictionary* dict, TDocumented* doc /*= 0*/, TModuleDoc* module /*= 0*/):
   fModule(module), fKey(0), fDict(dict), fDoc(doc)
{
   // Initialize a type name from ROOT's dictionaries

   EKind kind = kUnknownKind;
   if (dict) {
      if (dict->Property() & kIsClass) {
         kind = kClass;
      } else if (dict->Property() & kIsEnum) {
         kind = kEnum;
      } else if (dict->Property() & kIsNamespace) {
         kind = kNamespace;
      } else if (dict->Property() & kIsStruct) {
         kind = kStruct;
      } else if (dict->Property() & kIsTypedef) {
         kind = kTypedef;
      } else if (dict->Property() & kIsUnion) {
         kind = kUnion;
      }
      SetKind(kind);
   }
}

//______________________________________________________________________________
Doc::TDocTypeName::~TDocTypeName()
{
   // Destructor
}

//______________________________________________________________________________
Bool_t Doc::TDocTypeName::IsNewerThan(TKey* other) const
{
   // Determine whether this type name is newer than the key of
   // another TDocTypeName.
   // The key is newer than the files it parsed. If one of our fFiles
   // entries has a modification time after the key's write time then
   // we are newer than the key.

   Long_t keyTime = other->GetDatime().Convert();
   TIter iFile(&fFiles);
   TFileSysEntry* fse = 0;
   TString fseName;
   FileStat_t stat;
   TDatime fseDatime;
   while ((fse = (TFileSysEntry*) iFile())) {
      fse->GetFullName(fseName, kFALSE /* not as included */);
      if (!gSystem->GetPathInfo(fseName, stat)) {
         Error("IsNewerThan()",
               "Cannot find file %s!", fseName.Data());
         continue;
      }
      if (keyTime < stat.fMtime) {
         // the key is older than one of the modification times of our files,
         // so the key is older
         return kTRUE;
      }
   }

   // All files' modification times are older than the key's date:
   return kFALSE;
}

//______________________________________________________________________________
void Doc::TDocTypeName::SetDoc(TDocumented* doc)
{
   // Set the documentation object for thsi typename.
   // Also sets the kind of documented element (fKind).

   fDoc = doc;
   Doc::TClassDoc* cd = dynamic_cast<Doc::TClassDoc*>(doc);
   if (cd) {
      SetKind((EKind)cd->GetKind());
   } else {
      TTypedefDoc* td = dynamic_cast<TTypedefDoc*>(doc);
      if (td) {
         SetKind(kTypedef);
      } else {
         SetKind(kUnknownKind);
      }
   }
}

//______________________________________________________________________________
const char* Doc::TDocTypeName::GetName() const
{
   if (fKey) return fKey->GetName();
   if (fDoc) return fDoc->GetName();
   //if (fDict) return fDict->GetName();
   Error("GetName()",
         "Neither key nor documentation object are known, cannot determine name for %s",
         fDict ? fDict->GetName() : "{UNKNOWN}");
   return "UNKNOWN";
}
