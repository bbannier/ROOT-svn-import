// @(#)root/tree:$Id$
// Author: Axel Naumann, 2010-08-02

/*************************************************************************
 * Copyright (C) 1995-2010, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TTreeReader
#define ROOT_TTreeReader


////////////////////////////////////////////////////////////////////////////
//                                                                        //
// TTreeReader                                                            //
//                                                                        //
// A simple interface for reading trees or chains.                        //
//                                                                        //
//                                                                        //
////////////////////////////////////////////////////////////////////////////

#ifndef ROOT_THashTable
#include "THashTable.h"
#endif

class TDirectory;
class TTree;

namespace ROOT {
   class TBranchProxyDirector;
   class TTreeReaderValuePtrBase;
}

class TTreeReader: public TObject {
public:

   enum EEntryStatus {
      kEntryValid = 0, // data read okay
      kEntryNotLoaded, // no entry has been loaded yet
      kEntryNotFound, // the tree entry number does not exist
      kEntryChainSetupError, // problem in accessing a chain element, e.g. file without the tree
      kEntryChainFileError // problem in opening a chain's file
   };

   TTreeReader(TTree* tree);
   TTreeReader(const char* keyname, TDirectory* dir = NULL);

   EEntryStatus GetEntry(Long64_t entry);
   Bool_t GetNextEntry() { return GetEntry(GetCurrentEntry() + 1) == kEntryValid; }

   EEntryStatus GetEntryStatus() const { return fEntryStatus; }

   TTree* GetTree() const { return fTree; }
   Long64_t GetEntries(Bool_t force) const;
   Long64_t GetCurrentEntry() const;

protected:
   enum EMakeClassMode {
      kMakeClassUndefined,
      kMakeClassOff,
      kMakeClassOn
   };

   EMakeClassMode GetMakeClassMode() const { return fMakeClassMode; }
   void UpdateAddresses();
   void RegisterReader(ROOT::TTreeReaderValuePtrBase& reader);
   void UnregisterReader(ROOT::TTreeReaderValuePtrBase& reader);
   void InitializeMakeClassMode();
   Long64_t GetLocalEntryNumber() const { return fLocalEntryNumber; }

private:
   TTree*      fTree;
   TDirectory* fDirectory; // directory (or current file for chains)
   THashTable  fReaders; // readers
   Long64_t    fLocalEntryNumber; // entry number in the current tree
   EMakeClassMode fMakeClassMode; // whether makeclass mode is turned on
   EEntryStatus   fEntryStatus; // result of most recent GetEntry() entry is available
   ROOT::TBranchProxyDirector* fDirector; // proxying director

   friend class ROOT::TTreeReaderValuePtrBase;

   ClassDef(TTreeReader, 0); // A simple interface to read trees
};

#endif // defined TTreeReader
