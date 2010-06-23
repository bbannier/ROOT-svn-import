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
#include "TFile.h"
#include "TFileCollection.h"
#include "TFileInfo.h"
#include "TProof.h"

#include "TString.h"

#include "TDSet.h"
#include "Riostream.h"
#include "THashList.h"
#include "TMap.h"
#include "TEnv.h"
#include "TTree.h"
#include "TH1.h"
#include "TLeaf.h"
#include "TCanvas.h"
#include "TROOT.h"
#include "TStyle.h"
#include "TPaveText.h"
#include "TProfile.h"
#include "TLegend.h"
#include "TKey.h"
#include "TMap.h"
#include "TRegexp.h"
#include "TPerfStats.h"
#include "TParameter.h"
#include "TSelectorList.h"
#include "TDrawFeedback.h"
#include "TSortedList.h"

#include <stdlib.h>
#include "TDSet.h"
#include "TFileCollection.h"
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

//Generates files on worker nodes for cleanup run
//Input parameters do not change corresponding data members
//
//Input parameters
//  ignored
//Returns: 
//  map with files to be generated on the worker nodes

   // Create the file names and the map {worker,files}
   // Naming:        <basedir>/EventTree_Cleanup_nfile_serial.root

   TMap *filesmap = new TMap;
   filesmap->SetName("PROOF_FilesToProcess");
   TIter nxni(fNodes);
   TProofNode *ni = 0;

   while ((ni = (TProofNode *) nxni())) {
      TList *files = new TList;
      Int_t nworkers=ni->GetNWrks();
      files->SetName(ni->GetName());

      for (Int_t i = 0; i <nworkers; i++) { //split load across the workers on the node
         files->Add(new TObjString(TString::Format("EventTree_Cleanup_%d_0.root", i)));
      }
      filesmap->Add(new TObjString(ni->GetName()), files);
      //files->Print();
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
/*   if (!tdset){
      if (fDataSetGeneratedCleanup){
         tdset=fDataSetGeneratedCleanup;
      }
      else{
         Error("MakeDataSets", "Empty data set; Files not generated");
         return -1;
      }
   }
*/
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
   return 0;
}

//______________________________________________________________________________
Int_t TProofBenchModeCleanup::MakeDataSets(Int_t,
                                      Int_t,
                                      const Int_t*,
                                      const TDSet*,
                                      const char*,
                                      TProof*)
{
   return 0;
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
   // Re-Generate the list of worker node info (fNodes)
   // Return 0 if OK, -1 if proof not set, -2 if info could not be retrieved
   // (the existing info is always removed)

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
