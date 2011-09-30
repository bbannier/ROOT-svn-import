// @(#)root/treeplayer:$Id$
// Author: Axel Naumann, 2011-09-21

/*************************************************************************
 * Copyright (C) 1995-2011, Rene Brun and Fons Rademakers and al.        *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include "TTreeReader.h"
#include "TBranchProxyDirector.h"
#include "TBranchProxyDescriptor.h"
#include "TChain.h"
#include "TDirectory.h"
#include "TTreeProxyGenerator.h"
#include "TTreeReaderValuePtr.h"

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// TTreeReader                                                                //
//                                                                            //
// Connects TTreeReaderValuePtr to a TTree.                                   //
//                                                                            //
//                                                                            //
//                                                                            //
//                                                                            //
//                                                                            //
//                                                                            //
//                                                                            //
//                                                                            //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

ClassImp(TTreeReader)

//______________________________________________________________________________
TTreeReader::TTreeReader(TTree* tree):
   fTree(tree),
   fDirectory(0),
   fEntryStatus(kEntryNotLoaded),
   fProxyGenerator(0),
   fDirector(0)
{
   // Access data from tree.
   InitializeProxyGenerator();
}

//______________________________________________________________________________
TTreeReader::TTreeReader(const char* keyname, TDirectory* dir /*= NULL*/):
   fTree(0),
   fDirectory(dir),
   fEntryStatus(kEntryNotLoaded),
   fProxyGenerator(0),
   fDirector(0)
{
   // Access data from the tree called keyname in the directory (e.g. TFile)
   // dir, or the current directory if dir is NULL. If keyname cannot be
   // found, or if it is not a TTree, IsZombie() will return true.
   if (!fDirectory) fDirectory = gDirectory;
   fDirectory->GetObject(keyname, fTree);
   InitializeProxyGenerator();
}

//______________________________________________________________________________
TTreeReader::~TTreeReader()
{
   // Tell all value readers that the tree reader does not exist anymore.
   TIter iValue(&fValues);
   ROOT::TTreeReaderValuePtrBase* valueReader = 0;
   while ((valueReader = (ROOT::TTreeReaderValuePtrBase*)iValue())) {
      valueReader->MarkTreeReaderUnavailable();
   }
   delete fProxyGenerator;
   delete fDirector;
}

//______________________________________________________________________________
void TTreeReader::InitializeProxyGenerator()
{
   // Initialization of the TTreeProxyGenerator.
   if (!fTree) {
      MakeZombie();
      //fEntryStatus = kEntryNoTree;
   } else {
      fProxyGenerator = new ROOT::TTreeProxyGenerator(fTree);
      fDirector = new ROOT::TBranchProxyDirector(fTree, -1);
   }
}

//______________________________________________________________________________
TTreeReader::EEntryStatus TTreeReader::SetEntry(Long64_t entry)
{
   // Load an entry into the tree, return the status of the read.
   // For chains, entry is the global (i.e. not tree-local) entry number.

   if (!fTree) {
      fEntryStatus = kEntryNoTree;
      return fEntryStatus;
   }
   TTree* prevTree = fDirector->GetTree();

   int loadResult = fTree->LoadTree(entry);
   if (loadResult == -2) {
      fEntryStatus = kEntryNotFound;
      return fEntryStatus;
   }

   TTree* currentTree = fTree->GetTree();
   if (prevTree != currentTree) {
      fDirector->SetTree(currentTree);
   }
   if (!prevTree || fDirector->GetReadEntry() == -1) {
      // Tell readers we now have a tree
      TIter iValue(&fValues);
      ROOT::TTreeReaderValuePtrBase* valueReader = 0;
      while ((valueReader = (ROOT::TTreeReaderValuePtrBase*)iValue())) {
         valueReader->CreateProxy();
      }
   }
   fDirector->SetReadEntry(entry);
   fEntryStatus = kEntryValid;
   return fEntryStatus;
}

//______________________________________________________________________________
void TTreeReader::SetTree(TTree* tree)
{
   // Set (or update) the which tree to reader from. tree can be
   // a TTree or a TChain.
   fTree = tree;
   if (fTree) {
      ResetBit(kZombie);
      if (fTree->InheritsFrom(TChain::Class())) {
         SetBit(kBitIsChain);
      }
   }

   if (!fDirector) {
      InitializeProxyGenerator();
   }
}

//______________________________________________________________________________
ROOT::TBranchProxy* TTreeReader::CreateProxy(const char* branchname,
                                             TDictionary* dict)
{
   // Ask the TTreeProxyGenerator whether branchname exists,
   // what branchname contains, and wire the TBranchProxy representing it
   // to us so we can access its data.

   if (!fProxyGenerator) {
      Error("CreateProxy()", "No tree has been provided! First pass the tree to the TTreeReader e.g. using TTreeReader::SetTree(), then access its data.");
      return 0;
   }


   TBranch* branch = fTree->GetBranch(branchname);
   if (!branch) {
      Error("CreateProxy()", "The tree does not have a branch called %s. You could check with TTree::Print() for available branches.", branchname);
      return 0;
   }

   fProxyGenerator->fCurrentListOfTopProxies->Clear();
   fProxyGenerator->AnalyzeTopBranch(branch);

   ROOT::TBranchProxyDescriptor* branchDescr
      = (ROOT::TBranchProxyDescriptor*)fProxyGenerator->fCurrentListOfTopProxies->Last();
   if (!branchDescr) {
      Error("CreateProxy()", "Failed to generate data proxy for branch %s.",
            branchname);
      return 0;
   }

   TDictionary* branchActualType = TDictionary::GetDictionary(branchDescr->GetDataType()->GetName());

   if (!branchActualType) {
      // The proxy generator will have complained already if we cannot find
      // the branch's type.
      Error("CreateProxy()", "The branch %s contains data of type %s, which does not have a dictionary.",
            branchname, branchDescr->GetDataType()->GetName());
      return 0;
   }

   if (!dict) {
      Error("CreateProxy()", "The type T of the TTreeReaderValuePtr<T> accessing branch %s (which contains data of type %s) is not known to ROOT. You will need to create a dictionary for it.",
            branchname, branchActualType->GetName());
      return 0;
   }

   if (dict != branchActualType) {
      Error("CreateProxy()", "The branch %s contains data of type %s. It cannot be accessed by a TTreeReaderValuePtr<%s>",
            branchname, branchActualType->GetName(), dict->GetName());
   }

   ROOT::TBranchProxy* proxy = 0;
   return proxy;
}

//______________________________________________________________________________
void TTreeReader::RegisterValueReader(ROOT::TTreeReaderValuePtrBase* reader)
{
      fValues.AddLast(reader);
}

//______________________________________________________________________________
void TTreeReader::DeregisterValueReader(ROOT::TTreeReaderValuePtrBase* reader)
{
      fValues.Remove(reader);
}
