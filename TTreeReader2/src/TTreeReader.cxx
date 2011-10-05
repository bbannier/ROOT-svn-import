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

#include "TBranchClones.h"
#include "TBranchElement.h"
#include "TBranchRef.h"
#include "TBranchSTL.h"
#include "TChain.h"
#include "TDirectory.h"
#include "TLeaf.h"
#include "TROOT.h"
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
   fDirector(0)
{
   // Access data from tree.
   Initialize();
}

//______________________________________________________________________________
TTreeReader::TTreeReader(const char* keyname, TDirectory* dir /*= NULL*/):
   fTree(0),
   fDirectory(dir),
   fEntryStatus(kEntryNotLoaded),
   fDirector(0)
{
   // Access data from the tree called keyname in the directory (e.g. TFile)
   // dir, or the current directory if dir is NULL. If keyname cannot be
   // found, or if it is not a TTree, IsZombie() will return true.
   if (!fDirectory) fDirectory = gDirectory;
   fDirectory->GetObject(keyname, fTree);
   Initialize();
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
   delete fDirector;
   fProxies.SetOwner();
}

//______________________________________________________________________________
void TTreeReader::Initialize()
{
   // Initialization of the director.
   if (!fTree) {
      MakeZombie();
      fEntryStatus = kEntryNoTree;
   } else {
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
      Initialize();
   }
}

//______________________________________________________________________________
ROOT::TBranchProxy* TTreeReader::CreateProxy(const char* branchname,
                                             TDictionary* dict)
{
   // Search for the branchname, determine what it contains, and wire the
   // TBranchProxy representing it to us so we can access its data.

   ROOT::TNamedBranchProxy* namedProxy
      = (ROOT::TNamedBranchProxy*)fProxies.FindObject(branchname);
   if (namedProxy && namedProxy->GetDict() == dict) {
      return namedProxy->GetProxy();
   }

   TBranch* branch = fTree->GetBranch(branchname);
   if (!branch) {
      Error("CreateProxy()", "The tree does not have a branch called %s. You could check with TTree::Print() for available branches.", branchname);
      return 0;
   }

   TDictionary* branchActualType = 0;
   const char* branchActualTypeName = GetBranchDataType(branch, branchActualType);

   if (!branchActualType) {
      Error("CreateProxy()", "The branch %s contains data of type %s, which does not have a dictionary.",
            branchname, branchActualTypeName ? branchActualTypeName : "{UNDETERMINED TYPE}");
      return 0;
   }

   if (dict != branchActualType) {
      Error("CreateProxy()", "The branch %s contains data of type %s. It cannot be accessed by a TTreeReaderValuePtr<%s>",
            branchname, branchActualType->GetName(), dict->GetName());
   }

   // Update named proxy's dictionary
   if (namedProxy && !namedProxy->GetDict()) {
      namedProxy->SetDict(dict);
      return namedProxy->GetProxy();
   }
   return CreateProxy(branch);
}


//______________________________________________________________________________
ROOT::TBranchProxy* TTreeReader::CreateProxy(TBranch* branch)
{
   // Search for the branchname, determine what it contains, and wire the
   // TBranchProxy representing it to us so we can access its data.
   // A proxy for branch must not have been created before (i.e. check
   // fProxies before calling this function!)

   if (branch->IsA() == TBranchElement::Class()) {
   } else if (branch->IsA() == TBranch::Class()) {
   } else if (branch->IsA() == TBranchClones::Class()) {
   } else if (branch->IsA() == TBranchObject::Class()) {
   } else if (branch->IsA() == TBranchSTL::Class()) {
   } else if (branch->IsA() == TBranchRef::Class()) {
   }

   TString membername;

   bool isTopLevel = branch->GetMother() == branch;
   if (!isTopLevel) {
      membername = strrchr(branch->GetName(), '.');
      if (membername.IsNull()) {
         membername = branch->GetName();
      }
   }
   ROOT::TNamedBranchProxy* namedProxy
      = new ROOT::TNamedBranchProxy(fDirector, branch, membername);
   fProxies.Add(namedProxy);
   return namedProxy->GetProxy();
}

//______________________________________________________________________________
const char* TTreeReader::GetBranchDataType(TBranch* branch,
                                           TDictionary* &dict) const
{
   // Retrieve the type of data stored by branch; put its dictionary into
   // dict, return its type name. If no dictionary is available, at least
   // its type name should be returned.

   dict = 0;
   if (branch->IsA() == TBranchElement::Class()) {
      TBranchElement* brElement = (TBranchElement*)branch;
      if (brElement->GetType() == 4) {
         dict = brElement->GetClass();
         return brElement->GetClassName();
      } else if (brElement->GetType() == 3) {
         dict = TClonesArray::Class();
         return "TClonesArray";
      } else if (brElement->GetType() == 31
                 || brElement->GetType() == 41) {
         // it's a member, extract from GetClass()'s streamer info
         Error("GetBranchDataType()", "Must use TTreeReaderValueArray to access a member of an object that is stored in a collection.");
      }
      return 0;
   } else if (branch->IsA() == TBranch::Class()
              || branch->IsA() == TBranchObject::Class()
              || branch->IsA() == TBranchSTL::Class()) {
      const char* dataTypeName = branch->GetClassName();
      if ((!dataTypeName || !dataTypeName[0])
          && branch->IsA() == TBranch::Class()) {
         // leaflist. Can't represent.
         Error("GetBranchDataType()", "The branch %s was created using a leaf list and cannot be represented as a C++ type. Please access one of its siblings using a TTreeReaderValueArray:", branch->GetName());
         TIter iLeaves(branch->GetListOfLeaves());
         TLeaf* leaf = 0;
         while ((leaf = (TLeaf*) iLeaves())) {
            Error("GetBranchDataType()", "   %s.%s", branch->GetName(), leaf->GetName());
         }
         return 0;
      }
      dict = TDictionary::GetDictionary(dataTypeName);
      return dataTypeName;
   } else if (branch->IsA() == TBranchClones::Class()) {
      dict = TClonesArray::Class();
      return "TClonesArray";
   } else if (branch->IsA() == TBranchRef::Class()) {
      // Can't represent.
      Error("GetBranchDataType()", "The branch %s is a TBranchRef and cannot be represented as a C++ type.", branch->GetName());
      return 0;
   } else {
      Error("GetBranchDataType()", "The branch %s is of type %s - something that is not handled yet.", branch->GetName(), branch->IsA()->GetName());
      return 0;
   }

   return 0;
}

//______________________________________________________________________________
void TTreeReader::RegisterValueReader(ROOT::TTreeReaderValuePtrBase* reader)
{
   // Add a value reader for this tree.
   fValues.AddLast(reader);
}

//______________________________________________________________________________
void TTreeReader::DeregisterValueReader(ROOT::TTreeReaderValuePtrBase* reader)
{
   // Remove a value reader for this tree.
   fValues.Remove(reader);
}
