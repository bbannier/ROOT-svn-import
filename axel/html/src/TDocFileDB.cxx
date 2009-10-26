// @(#)root/html:$Id$
// Author: Axel Naumann 2009-10-26

/*************************************************************************
 * Copyright (C) 1995-2009, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include "TDocFileDB.h"

#include "TPRegexp.h"
#include "TSystem.h"
#include "THtml.h"

//______________________________________________________________________________
Doc::TFileSysEntry::TFileSysEntry(const char* name, TFileSysDir* parent):
   fName(name), fParent(parent), fLevel(parent ? parent->GetLevel() + 1 : 0)
{
   // Construct a file system entry.
}

//______________________________________________________________________________
void Doc::TFileSysEntry::GetFullName(TString& fullname, Bool_t asIncluded) const {
   // Retrieve the fully qualified (i.e. with all parent directories) name.
   if (fParent) {
      fParent->GetFullName(fullname, asIncluded);
      if (fullname[0])
         fullname += "/";
   } else
      fullname = "";
   fullname += fName;
}


//______________________________________________________________________________
void Doc::TFileSysDir::Recurse(TFileSysDB* db, const char* path)
{
   // Recursively fill entries by parsing the contents of path.

   TString dir(path);
   if (gDebug > 0 || GetLevel() < 2)
      Info("Recurse", "scanning %s...", path);
   TPMERegexp regexp(db->GetIgnore());
   dir += "/";
   void* hDir = gSystem->OpenDirectory(dir);
   const char* direntry = 0;
   while ((direntry = gSystem->GetDirEntry(hDir))) {
      if (!direntry[0] || direntry[0] == '.' || regexp.Match(direntry)) continue;
      TString entryPath(dir + direntry);
      if (gSystem->AccessPathName(entryPath, kReadPermission))
         continue;
      FileStat_t buf;
      gSystem->GetPathInfo(entryPath, buf);
      if (R_ISDIR(buf.fMode)) {
         // skip if we would nest too deeply,  and skip soft links:
         if (GetLevel() > db->GetMaxLevel()
#ifndef R__WIN32
             || db->GetMapIno().GetValue(buf.fIno)
#endif
             ) continue;
         TFileSysDir* subdir = new TFileSysDir(direntry, this);
         fDirs.Add(subdir);
#ifndef R__WIN32
         db->GetMapIno().Add(buf.fIno, (Long_t)subdir);
#endif
         subdir->Recurse(db, entryPath);
      } else {
         int delen = strlen(direntry);
         // only .cxx and .h are taken
         if (strcmp(direntry + delen - 4, ".cxx")
             && strcmp(direntry + delen - 2, ".h"))
            continue;
         TFileSysEntry* entry = new TFileSysEntry(direntry, this);
         db->GetEntries().Add(entry);
         fFiles.Add(entry);
      }
   } // while dir entry
   gSystem->FreeDirectory(hDir);
}


//______________________________________________________________________________
void Doc::TFileSysDB::Fill()
{
   // Recursively fill entries by parsing the path specified in GetName();
   // can be a THtml::GetDirDelimiter() delimited list of paths.

   TString dir;
   Ssiz_t posPath = 0;
   while (fName.Tokenize(dir, posPath, THtml::GetDirDelimiter())) {
      if (gSystem->AccessPathName(dir, kReadPermission)) {
         Warning("Fill", "Cannot read InputPath \"%s\"!", dir.Data());
         continue;
      }
      FileStat_t buf;
      gSystem->GetPathInfo(dir, buf);
      if (R_ISDIR(buf.fMode)) {
#ifndef R__WIN32
         TFileSysRoot* prevroot = (TFileSysRoot*) (Long_t)GetMapIno().GetValue(buf.fIno);
         if (prevroot != 0) {
            Warning("Fill", "InputPath \"%s\" already present as \"%s\"!", dir.Data(), prevroot->GetName());
            continue;
         }
#endif
         TFileSysRoot* root = new TFileSysRoot(dir, this);
         fDirs.Add(root);
#ifndef R__WIN32
         GetMapIno().Add(buf.fIno, (Long_t)root);
#endif
         root->Recurse(this, dir);
      } else {
         Warning("Fill", "Cannot read InputPath \"%s\"!", dir.Data());
      }
   }
}

//______________________________________________________________________________
Doc::TFileSysRoot::TFileSysRoot(const char* name, TFileSysDB* parent):
         TFileSysDir(name, parent)
{
   // Construct a search root directory: it doesn't have any parents.
   // It might not correspond to the file system's root directory but
   // instead be a path from THtml::GetInputPath().
}
