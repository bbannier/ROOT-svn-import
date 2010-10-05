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
#include "TMap.h"
#include "TProofNodes.h"

ClassImp(TProofBenchModeCleanup)

//______________________________________________________________________________
TProofBenchModeCleanup::TProofBenchModeCleanup(TProof* proof,
                                               TProofNodes* nodes)
:fProof(proof), fNodes(nodes), fName("Cleanup")
{
   if (!fProof){
      fProof=gProof;
   }
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

   TList* nodes=fNodes->GetListOfNodes();
   TIter nxtnode(nodes);

   TList *node = 0;

   while ((node = (TList*) nxtnode())) {
      TList *files = new TList;
      files->SetName(node->GetName());
      Int_t nwrks=node->GetSize();

      //split load across the workers on the node
      for (Int_t i = 0; i <nwrks; i++) {
         files->Add(new TObjString(TString::Format("%s_EventTree_Cleanup_%d_0.root", node->GetName(), i)));
      }
      filesmap->Add(new TObjString(node->GetName()), files);
   }

   Info("FilesToProcess", "Map of files to be generated:");
   filesmap->Print("A", -1);

   return filesmap;
}

//______________________________________________________________________________
Int_t TProofBenchModeCleanup::MakeDataSets(Int_t, Int_t, Int_t, Int_t,
                                     const TList* listfiles, const char* option,
                                     TProof* proof)
{
   // Make data set from list of file 'listfiles'and register it.
   // Input parameters
   //    Int_t Ignored.
   //    Int_t Ignored.
   //    Int_t Ignored.
   //    Int_t Ignored.
   //    listfiles List of files (TFileInfo*) from which data set is built and registered.
   //    option Option to TProof::RegisterDataSet(...).
   //    proof Proof
   // Return
   //    0 when ok
   //   <0 otherwise

   if (!listfiles){
      Error("MakeDataSets", "Empty data set; Files not generated");
      return -1;
   }

   TString dsname="DataSetEventCleanup";
   Info("MakeDataSetDataSets", "Creating dataset '%s'.", dsname.Data());
   // Create the TFileCollection
   TFileCollection *fc = new TFileCollection;

   TIter nxtfileinfo(listfiles);
   TFileInfo* fileinfo;

   while ((fileinfo=(TFileInfo*)nxtfileinfo())){
      fc->Add((TFileInfo*)(fileinfo->Clone()));
   }
   fc->Update();
   proof->RegisterDataSet(dsname, fc, option);
   delete fc;
   return 0;
}

//______________________________________________________________________________
Int_t TProofBenchModeCleanup::MakeDataSets(Int_t, Int_t, const Int_t*,
                                    const TList* listfiles, const char* option,
                                    TProof* proof)
{
   // Make data set from list of files 'listfiles' and register it.
   // Input parameters
   //    Int_t: Ignored.
   //    Int_t: Ignored.
   //    const Int_t*: Ignored.
   //    listfiles: List of files (TFileInfo*) from which data set is built and
   //               registered.
   //    option: Option to TProof::RegisterDataSet(...).
   //    proof: Proof
   // Return
   //    0 when ok
   //   <0 otherwise

   Info("MakeDataSets", "Making data sets for mode %s.", GetName()); 

   return MakeDataSets(0, 0, 0, 0, listfiles, option, proof);
}

//______________________________________________________________________________
void TProofBenchModeCleanup::Print(Option_t* option) const
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
TProof* TProofBenchModeCleanup::GetProof() const
{
   return fProof;
}

//______________________________________________________________________________
Int_t TProofBenchModeCleanup::GetNFiles() const
{
   return 0;
}

//______________________________________________________________________________
const char* TProofBenchModeCleanup::GetName() const
{
   return fName.Data();
}
