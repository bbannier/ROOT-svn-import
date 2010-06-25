// @(#)root/proofx:$Id$
// Author: Sangsu Ryu 22/06/2010

/*************************************************************************
 * Copyright (C) 1995-2005, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TProofBenchModeCleanup                                               //
//                                                                      //
// Cleaning-up mode for Proof benchmark test.                           //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "TProofBenchModeCleanup.h"
#include "TFileCollection.h"
#include "TFileInfo.h"
#include "TProof.h"
#include "TString.h"
#include "TDSet.h"
#include "TMap.h"
#include "TProofNode.h"

ClassImp(TProofBenchModeCleanup)

//______________________________________________________________________________
TProofBenchModeCleanup::TProofBenchModeCleanup(TProof* proof)
:fProof(proof), fNodes(0), fName("ConstNFilesNode")
{
   FillNodeInfo();
}

//______________________________________________________________________________
TProofBenchModeCleanup::~TProofBenchModeCleanup()
{
}

//______________________________________________________________________________
TMap* TProofBenchModeCleanup::FilesToProcess(Int_t)
{

   //Create map of files to be generated on each worker node for cleanup run
   //Input parameters do not change corresponding data members
   //
   //Input parameters
   //  Ignored
   //Returns: 
   //  map of files to be generated on each worker node

   TMap *filesmap = new TMap;
   filesmap->SetName("PROOF_FilesToProcess");
   TIter nxni(fNodes);
   TProofNode *ni = 0;

   while ((ni = (TProofNode *) nxni())) {
      TList *files = new TList;
      Int_t nworkers=ni->GetNWrks();
      files->SetName(ni->GetName());

      //split load across the workers on the node
      for (Int_t i = 0; i <nworkers; i++) {
         files->Add(new TObjString(TString::Format("EventTree_Cleanup_%d_0.root", i)));
      }
      filesmap->Add(new TObjString(ni->GetName()), files);
   }

   return filesmap;
}

//______________________________________________________________________________
Int_t TProofBenchModeCleanup::MakeDataSets(Int_t,
                                      Int_t,
                                      Int_t,
                                      Int_t,
                                      const TDSet* tdset,
                                      const char* option,
                                      TProof* proof)
{
   // Make data set from data set tdset and register it.
   // Input parameters
   //    Int_t Ignored.
   //    Int_t Ignored.
   //    Int_t Ignored.
   //    Int_t Ignored.
   //    tdset Dataset from which data set is built and registered.
   //    option Option to TProof::RegisterDataSet(...).
   //    proof Proof
   // Return
   //    0 when ok
   //   <0 otherwise

   if (!tdset){
      Error("MakeDataSets", "Empty data set; Files not generated");
      return -1;
   }

   TString dsname="DataSetEventCleanup";
   Info("MakeDataSetDataSets", "creating dataset '%s' ...", dsname.Data());
   // Create the TFileCollection
   TFileCollection *fc = new TFileCollection;

   TList* lelement=tdset->GetListOfElements();
   TIter nxtelement(lelement);
   TDSetElement *tdelement;
   TFileInfo* fileinfo;

   while ((tdelement=(TDSetElement*)nxtelement())){
      fileinfo=tdelement->GetFileInfo();
      //fileinfo->Print("A");
      fc->Add(fileinfo);
   }
   fc->Update();
   proof->RegisterDataSet(dsname, fc, option);
   delete fc;
   return 0;
}

//______________________________________________________________________________
Int_t TProofBenchModeCleanup::MakeDataSets(Int_t,
                                      Int_t,
                                      const Int_t*,
                                      const TDSet* tdset,
                                      const char* option,
                                      TProof* proof)
{
   // Make data set from data set tdset and register it.
   // Input parameters
   //    Int_t Ignored.
   //    Int_t Ignored.
   //    const Int_t* Ignored.
   //    tdset Dataset from which data set is built and registered.
   //    option Option to TProof::RegisterDataSet(...).
   //    proof Proof
   // Return
   //    0 when ok
   //   <0 otherwise

   return MakeDataSets(0, 0, 0, 0, tdset, option, proof);
}

//______________________________________________________________________________
void TProofBenchModeCleanup::Print(Option_t* option)const
{
   if (fProof) fProof->Print(option);
   if (fNodes) fNodes->Print(option);
   Printf("fName=%s", fName.Data());
}

//______________________________________________________________________________
TProofBenchMode::EFileType TProofBenchModeCleanup::GetFileType()
{
   return TProofBenchMode::kFileCleanup;
}

//______________________________________________________________________________
void TProofBenchModeCleanup::SetProof(TProof* proof)
{
   fProof=proof;
}

//______________________________________________________________________________
void TProofBenchModeCleanup::SetNFiles(Int_t)
{
}

//______________________________________________________________________________
TProof* TProofBenchModeCleanup::GetProof()const
{
   return fProof;
}

//______________________________________________________________________________
Int_t TProofBenchModeCleanup::GetNFiles()const
{
   return 0;
}

//______________________________________________________________________________
const char* TProofBenchModeCleanup::GetName()const
{
   return fName.Data();
}

//______________________________________________________________________________
Int_t TProofBenchModeCleanup::FillNodeInfo()
{
   // Re-Generate the list of worker node info (fNodes).
   // The existing info is always removed.
   // Return
   //    0 when ok
   //   <0 otherwise

   if (!fProof){
      Error("FillNodeInfo", "proof not set, doing nothing");
      return -1;
   }

   if (fNodes) {
      fNodes->SetOwner(kTRUE);
      SafeDelete(fNodes);
   }
   fNodes = new TList;
   fNodes->SetOwner();//fNodes is the owner of the members

   // Get info
   TList *wl = fProof->GetListOfSlaveInfos();
   if (!wl) {
      Error("FillNodeInfo", "could not get information about workers!");
      return -2;
   }

   TIter nxwi(wl);
   TSlaveInfo *si = 0;
   TProofNode *ni = 0;
   while ((si = (TSlaveInfo *) nxwi())) {
      if (!(ni = (TProofNode *) fNodes->FindObject(si->GetName()))) {
         ni = new TProofNode(si->GetName(), si->GetSysInfo().fPhysRam);
         fNodes->Add(ni);
      } else {
         ni->AddWrks(1);
      }
   }
   // Notify
   Info("FillNodeInfo","%d physically different mahcines found", fNodes->GetSize());
   // Done
   return 0;
}
