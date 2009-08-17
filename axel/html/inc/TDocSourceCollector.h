// @(#)root/html:$Id: TDocInfo.h 23937 2008-05-20 16:44:59Z axel $
// Author: Axel Naumann, 2009

/*************************************************************************
 * Copyright (C) 1995-2009, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TDocSourceCollector
#define ROOT_TDocSourceCollector

#ifndef ROOT_TObject
#include "TObject.h"
#endif

class THtml;
namespace RootDoc {

   class TFileSysDir;
   class TFileSysDB;
   //______________________________________________________________
   // Utility class representing a directory entry
   class TFileSysEntry: public TObject {
   public:
      TFileSysEntry(const char* name, TFileSysDir* parent):
         fName(name), fParent(parent), fLevel(parent ? parent->GetLevel() + 1 : 0) {}
      const char* GetName() const { return fName; }
      virtual ULong_t Hash() const { return fName.Hash(); }
      virtual void GetFullName(TString& fullname, Bool_t asIncluded) const {
         if (fParent) {
            fParent->GetFullName(fullname, asIncluded);
            if (fullname[0])
               fullname += "/";
         } else
            fullname = "";
         fullname += fName;
      }

      TFileSysDir* GetParent() const { return fParent; }
      Int_t GetLevel() const { return fLevel; }
   protected:
      TString      fName; // name of the element
      TFileSysDir* fParent; // parent directory
      Int_t        fLevel; // level of directory
      ClassDef(TFileSysEntry, 0); // an entry of the local file system
   };

   //______________________________________________________________
   // Utility class representing a directory
   class TFileSysDir: public TFileSysEntry {
   public:
      TFileSysDir(const char* name, TFileSysDir* parent):
         TFileSysEntry(name, parent)
      { fFiles.SetOwner(); fDirs.SetOwner(); }
      const TList* GetFiles() const { return &fFiles; }
      const TList* GetSubDirs() const { return &fDirs; }

      void Recurse(TFileSysDB* db, const char* path);

   protected:
      TList fFiles;
      TList fDirs;
      ClassDef(TFileSysDir, 0); // an directory of the local file system
   };

   //______________________________________________________________
   // Utility class representing a root directory as specified in
   // THtml::GetInputPath()
   class TFileSysRoot: public TFileSysDir {
   public:
      TFileSysRoot(const char* name, TFileSysDB* parent):
         TFileSysDir(name, parent) {}
      void GetFullName(TString& fullname, Bool_t asIncluded) const {
         // prepend directory part of THtml::GetInputPath() only
         // if !asIncluded
         fullname = "";
         if (!asIncluded)
            fullname += fName;
      }

      ClassDef(TFileSysRoot, 0); // an root directory of the local file system
   };

   //______________________________________________________________
   // Utility class representing a directory
   class TFileSysDB: public TFileSysDir {
   public:
      TFileSysDB(const char* path, const char* ignore, Int_t maxdirlevel):
         TFileSysDir(path, 0), fEntries(1009, 5), fIgnorePath(ignore), fMaxLevel(maxdirlevel)
      { Fill(); }

      TExMap& GetMapIno() { return fMapIno; }
      THashTable& GetEntries() { return fEntries; }
      const TString& GetIgnore() const { return fIgnorePath; }
      Int_t   GetMaxLevel() const { return fMaxLevel; }

   protected:
      void Fill();

   private:
      TExMap   fMapIno; // inode to TFileSysDir map, to detect softlinks
      THashTable fEntries; // hash map of all filenames without paths
      TString  fIgnorePath; // regexp of path to ignore while building entry tree
      Int_t    fMaxLevel; // maximum level of directory nesting
      ClassDef(TFileSysDB, 0); // instance of file system data
   };


   class TDocSourceCollector: public TObject {
   public:
      TDocSourceCollector(THtml* html): fHtml(html) {}
      virtual ~TDocSourceCollector() {}
      
      TSourceFileDocInfo* GetFileFromInclude(const char* incl) const;

   private:
      THtml* fHtml;
      ClassDef(TDocSourceCollector, 0); // Finds source files for THtml
   };
} // namespace RootDoc
#endif // ROOT_TDocSourceCollector
