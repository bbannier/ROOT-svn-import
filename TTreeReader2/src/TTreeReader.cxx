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
#include "TClassEdit.h"
#include "TDirectory.h"
#include "TLeaf.h"
#include "TROOT.h"
#include "TStreamerElement.h"
#include "TStreamerInfo.h"
#include "TTreeReaderValue.h"

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// TTreeReader                                                                //
//                                                                            //
// Connects TTreeReaderValue to a TTree.                                   //
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
   ROOT::TTreeReaderValueBase* valueReader = 0;
   while ((valueReader = (ROOT::TTreeReaderValueBase*)iValue())) {
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
      ROOT::TTreeReaderValueBase* valueReader = 0;
      while ((valueReader = (ROOT::TTreeReaderValueBase*)iValue())) {
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
      Error("CreateProxy()", "The branch %s contains data of type %s. It cannot be accessed by a TTreeReaderValue<%s>",
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
ROOT::TBranchProxy* TTreeReader::CreateContentProxy(const char* branchname,
                                                    TDictionary* dict)
{
   // Access a branch's collection content (not the collection itself)
   // through a proxy.
   // Search for the branchname, determine what it contains, and wire the
   // TBranchProxy representing it to us so we can access its data.

   ROOT::TNamedBranchProxy* namedProxy
      = (ROOT::TNamedBranchProxy*)fProxies.FindObject(branchname);
   if (namedProxy && namedProxy->GetContentDict() == dict) {
      return namedProxy->GetProxy();
   }

   TBranch* branch = fTree->GetBranch(branchname);
   if (!branch) {
      Error("CreateContentProxy()", "The tree does not have a branch called %s. You could check with TTree::Print() for available branches.", branchname);
      return 0;
   }

   TDictionary* branchActualType = 0;
   TString branchActualTypeName;
   const char* nonCollTypeName = GetBranchContentDataType(branch, branchActualTypeName, branchActualType);
   if (nonCollTypeName) {
      Error("CreateContentProxy()", "The branch %s contains data of type %s, which should be accessed through a TTreeReaderValue< %s >.",
            branchname, nonCollTypeName, nonCollTypeName);
      return 0;
   }
   if (!branchActualType) {
      if (branchActualTypeName.IsNull()) {
         Error("CreateContentProxy()", "Cannot determine the type contained in the collection of branch %s. That's weird - please report!",
               branchname);
      } else {
         Error("CreateContentProxy()", "The branch %s contains data of type %s, which does not have a dictionary.",
               branchname, branchActualTypeName.Data());
      }
      return 0;
   }

   if (dict != branchActualType) {
      Error("CreateContentProxy()", "The branch %s contains data of type %s. It cannot be accessed by a TTreeReaderValue<%s>",
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
ROOT::TBranchProxy* TTreeReader::CreateContentProxy(TBranch* branch)
{
   // Access a branch's collection content (not the collection itself)
   // through a proxy.
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
const char* TTreeReader::GetBranchContentDataType(TBranch* branch,
                                                  TString& contentTypeName,
                                                  TDictionary* &dict) const
{
   // Access a branch's collection content (not the collection itself)
   // through a proxy.
   // Retrieve the type of data contained in the collection stored by branch;
   // put its dictionary into dict, If there is no dictionary, put its type
   // name into contentTypeName.
   // The contentTypeName is set to NULL if the branch does not
   // contain a collection; in that case, the type of the branch is returned.
   // In all other cases, NULL is returned.

   dict = 0;
   contentTypeName = "";
   if (branch->IsA() == TBranchElement::Class()) {
      TBranchElement* brElement = (TBranchElement*)branch;
      if (brElement->GetType() == 4
          || brElement->GetType() == 3) {
         TVirtualCollectionProxy* collProxy = brElement->GetCollectionProxy();
         dict = collProxy->GetValueClass();
         if (!dict) dict = TDataType::GetDataType(collProxy->GetType());
         if (!dict) {
            // We don't know the dictionary, thus we need the content's type name.
            // Determine it.
            if (brElement->GetType() == 3) {
               contentTypeName = brElement->GetClonesName();
               return 0;
            }
            // STL:
            TClassEdit::TSplitType splitType(brElement->GetClassName());
            int isSTLCont = splitType.IsSTLCont();
            if (!isSTLCont) {
               Error("GetBranchContentDataType()", "Cannot determine STL collection type of %s stored in branch %s", brElement->GetClassName(), branch->GetName());
               return brElement->GetClassName();
            }
            bool isMap = isSTLCont == TClassEdit::kMap
               || isSTLCont == TClassEdit::kMultiMap;
            if (isMap) contentTypeName = "std::pair< ";
            contentTypeName += splitType.fElements[1];
            if (isMap) {
               contentTypeName += splitType.fElements[2];
               contentTypeName += " >";
            }
            return 0;
         }
         return 0;
      } else if (brElement->GetType() == 31
                 || brElement->GetType() == 41) {
         // it's a member, extract from GetClass()'s streamer info
         TClass* clData = 0;
         EDataType dtData = kOther_t;
         int ExpectedTypeRet = brElement->GetExpectedType(clData, dtData);
         if (ExpectedTypeRet == 0) {
            dict = clData;
            if (!dict) {
               dict = TDataType::GetDataType(dtData);
            }
            if (!dict) {
               Error("GetBranchContentDataType()", "The branch %s contains a data type %d for which the dictionary cannot be retrieved.",
                     branch->GetName(), (int)dtData);
               contentTypeName = TDataType::GetTypeName(dtData);
               return 0;
            }
            return 0;
         } else if (ExpectedTypeRet == 1) {
            int brID = brElement->GetID();
            if (brID == -1) {
               // top
               Error("GetBranchContentDataType()", "The branch %s contains data of type %s for which the dictionary does not exist. It's needed.",
                     branch->GetName(), brElement->GetClassName());
               contentTypeName = brElement->GetClassName();
               return 0;
            }
            // Either the data type name doesn't have an EDataType entry
            // or the streamer info doesn't have a TClass* attached.
            TStreamerElement* element =
               (TStreamerElement*) brElement->GetInfo()->GetElems()[brID];
            contentTypeName = element->GetTypeName();
            return 0;
         }
         /* else (ExpectedTypeRet == 2)*/
         // The streamer info entry cannot be found.
         // TBranchElement::GetExpectedType() has already complained.
         return "{CANNOT DETERMINE TBranchElement DATA TYPE}";
      }
      return 0;
   } else if (branch->IsA() == TBranch::Class()
              || branch->IsA() == TBranchObject::Class()
              || branch->IsA() == TBranchSTL::Class()) {
      const char* dataTypeName = branch->GetClassName();
      if ((!dataTypeName || !dataTypeName[0])
          && branch->IsA() == TBranch::Class()) {
         // leaflist. Can't represent.
         Error("GetBranchContentDataType()", "The branch %s was created using a leaf list and cannot be represented as a C++ type. Please access one of its siblings using a TTreeReaderValueArray:", branch->GetName());
         TIter iLeaves(branch->GetListOfLeaves());
         TLeaf* leaf = 0;
         while ((leaf = (TLeaf*) iLeaves())) {
            Error("GetBranchContentDataType()", "   %s.%s", branch->GetName(), leaf->GetName());
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
      Error("GetBranchContentDataType()", "The branch %s is a TBranchRef and cannot be represented as a C++ type.", branch->GetName());
      return 0;
   } else {
      Error("GetBranchContentDataType()", "The branch %s is of type %s - something that is not handled yet.", branch->GetName(), branch->IsA()->GetName());
      return 0;
   }

   return 0;
}

//______________________________________________________________________________
void TTreeReader::RegisterValueReader(ROOT::TTreeReaderValueBase* reader)
{
   // Add a value reader for this tree.
   fValues.AddLast(reader);
}

//______________________________________________________________________________
void TTreeReader::DeregisterValueReader(ROOT::TTreeReaderValueBase* reader)
{
   // Remove a value reader for this tree.
   fValues.Remove(reader);
}
