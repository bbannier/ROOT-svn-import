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
   class TTreeProxyGenerator;
   class TTreeReaderValuePtrBase;
}

class TTreeReader: public TObject {
public:

   enum EEntryStatus {
      kEntryValid = 0, // data read okay
      kEntryNotLoaded, // no entry has been loaded yet
      kEntryNoTree, // the tree does not exist
      kEntryNotFound, // the tree entry number does not exist
      kEntryChainSetupError, // problem in accessing a chain element, e.g. file without the tree
      kEntryChainFileError // problem in opening a chain's file
   };

   TTreeReader(const char* keyname, TDirectory* dir = NULL);
   TTreeReader(TTree* tree);
   TTreeReader(TFileCollection* files);

   Bool_t GetNextEntry() { return GetEntry(GetCurrentEntry() + 1) == kEntryValid; }
   void GetEntry(Long64_t entry);

   EEntryStatus GetEntryStatus() const { return fEntryStatus; }

   TTree* GetTree() const { return fTree; }
   Long64_t GetEntries(Bool_t force) const;
   Long64_t GetCurrentEntry() const;

   // Internal
   Bool_t Notify(); 

protected:
   void InitializeProxyGenerator();
   void UpdateAddresses();
   void RegisterReader(ROOT::TTreeReaderValuePtrBase& reader);
   void UnregisterReader(ROOT::TTreeReaderValuePtrBase& reader);
   void InitializeMakeClassMode();

private:
   TTree*      fTree;
   TDirectory* fDirectory; // directory (or current file for chains)
   THashTable  fReaders; // readers
   EEntryStatus fEntryStatus; // status of most recent read request
   EMakeClassMode fMakeClassMode; // whether makeclass mode is turned on
   ROOT::TTreeProxyGenerator* fProxyGenerator; // generator for proxy objects
   ROOT::TBranchProxyDirector* fDirector; // proxying director

   friend class ROOT::TTreeReaderValuePtrBase;

   ClassDef(TTreeReader, 0); // A simple interface to read trees
};

#endif // defined TTreeReader
