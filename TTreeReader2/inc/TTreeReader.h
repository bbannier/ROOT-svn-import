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

#ifndef ROOT_TBranchProxyDirector
#include "TBranchProxyDirector.h"
#endif
#ifndef ROOT_TObjArray
#include "TObjArray.h"
#endif
#ifndef ROOT_TTree
#include "TTree.h"
#endif

class TDictionary;
class TDirectory;
class TFileCollection;
class TTree;

namespace ROOT {
   class TBranchProxy;
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

   TTreeReader():
      fDirectory(0),
      fEntryStatus(kEntryNoTree),
      fProxyGenerator(0)
   {}

   TTreeReader(TTree* tree);
   TTreeReader(const char* keyname, TDirectory* dir = NULL);
   TTreeReader(const char* keyname, TFileCollection* files);

   ~TTreeReader();

   void SetTree(TTree* tree);
   void SetTree(const char* keyname, TDirectory* dir = NULL);
   void SetChain(const char* keyname, TFileCollection* files);

   Bool_t IsChain() const { return TestBit(kBitIsChain); }

   Bool_t SetNextEntry() { return SetEntry(GetCurrentEntry() + 1) == kEntryValid; }
   EEntryStatus SetEntry(Long64_t entry);

   EEntryStatus GetEntryStatus() const { return fEntryStatus; }

   TTree* GetTree() const { return fTree; }
   Long64_t GetEntries(Bool_t force) const { return fTree ? (force ? fTree->GetEntries() : fTree->GetEntriesFast() ) : -1; }
   Long64_t GetCurrentEntry() const { return fDirector ? fDirector->GetReadEntry() : 0; }

protected:
   void InitializeProxyGenerator();
   ROOT::TBranchProxy* CreateProxy(const char* branchname,
                                   TDictionary* dict);
   void RegisterValueReader(ROOT::TTreeReaderValuePtrBase* reader);
   void DeregisterValueReader(ROOT::TTreeReaderValuePtrBase* reader);

private:

   enum EPropertyBits {
      kBitIsChain = BIT(14) // our tree is a chain
   };

   TTree* fTree; // tree that's read
   TDirectory* fDirectory; // directory (or current file for chains)
   EEntryStatus fEntryStatus; // status of most recent read request
   ROOT::TTreeProxyGenerator* fProxyGenerator; // generator for proxy objects
   ROOT::TBranchProxyDirector* fDirector; // proxying director
   TObjArray    fValues; // TTreeReaderValuePtrBase objects that use our director

   friend class ROOT::TTreeReaderValuePtrBase;

   ClassDef(TTreeReader, 0); // A simple interface to read trees
};

#endif // defined TTreeReader
