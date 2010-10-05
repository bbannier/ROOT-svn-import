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
// TProofBenchRunCleanup                                                //
//                                                                      //
// Represents a memory cleaning-up run for PROOF benchmark.             //
// During I/O benchmark, files are repeatedly read.                     //
// Every new run should read file from disk, not from memory.           //
// 2 ways of clean up method are provided. One is brute force way       //
// in which dedicated files large enough to clean up memory             //
// on the machine are read in before every run. The other way is clean  //
// up files cache by calling posix_fadvise. It works only on Linux      //
// for now.                                                             //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "TProofBenchRunCleanup.h"
#include "TProofBenchMode.h"
#include "TFileCollection.h"
#include "TFileInfo.h"
#include "TProof.h"
#include "TString.h"
#include "Riostream.h"
#include "THashList.h"
#include "TMap.h"
#include "TEnv.h"
#include "TTree.h"
#include "TH1.h"
#include "TCanvas.h"
#include "TProfile.h"
#include "TKey.h"
#include "TPerfStats.h"
#include "TPad.h"
#include "TROOT.h"

ClassImp(TProofBenchRunCleanup)

//______________________________________________________________________________
TProofBenchRunCleanup::TProofBenchRunCleanup(
               TProofBenchRun::ECleanupType cleanuptype,
               TDirectory* dirproofbench, TProof* proof, Int_t debug):
fProof(proof), fCleanupType(cleanuptype), fDebug(debug),
fDirProofBench(dirproofbench)
{
   //Default constructor
 
   if (!fProof){
       fProof=gProof;
   }

   TString name="Cleanup"+GetNameStem();

   fName=name;
   gEnv->SetValue("Proof.StatsTrace",1);

}

//______________________________________________________________________________
TProofBenchRunCleanup::~TProofBenchRunCleanup()
{
   //destructor
   fProof=0;
   fDirProofBench=0;
} 

//______________________________________________________________________________
TString TProofBenchRunCleanup::GetNameStem() const
{
   TString namestem;
   switch (fCleanupType){
   case TProofBenchRun::kCleanupReadInFiles:
      namestem="ReadInFiles";
      break;
   case TProofBenchRun::kCleanupFileAdvise:
      namestem="FileAdvise";
      break;
   default:
      break;
   }
    return namestem;
}

//______________________________________________________________________________
void TProofBenchRunCleanup::Run(Long64_t, Int_t, Int_t, Int_t, Int_t,
                                Int_t debug, Int_t)
{
   // Clean up cache between bench mark runs. 
   // Input parameters
   //    Long64_t Ignored.
   //    Int_t    Ignored.
   //    Int_t    Ignored.
   //    Int_t    Ignored.
   //    Int_t    Ignored.
   //    debug    debug switch. When -1, data member fDebug is used.  
   //    Int_t    Ignored.
   // Return
   //    Nothing

   if (!fProof){
      Error("Run", "Proof not set");
      return;
   }

   debug=(debug==-1)?fDebug:debug;

   fProof->SetParallel(99999);

   DeleteParameters();
   SetParameters();

   if (fCleanupType==TProofBenchRun::kCleanupReadInFiles){
      TString dsname="DataSetEventCleanup";
      Info("Run", "Cleaning up file cache by reading in files of data set %s.",
                   dsname.Data());
      TFileCollection* fc_cleanup=fProof->GetDataSet(dsname.Data());

      TTime starttime = gSystem->Now();
      if (fc_cleanup){
         fProof->Process(fc_cleanup, "TSelEvent", "", -1);
      }
      else{
         Error("Run", "file Collection '%s' does not exist; returning",
                       dsname.Data());
         return;
      }

      if (debug){
   
         TList* l = fProof->GetOutputList();
   
         TString perfstats_name = "PROOF_PerfStats";
         //save perfstats
         TTree* t = dynamic_cast<TTree*>(l->FindObject(perfstats_name.Data()));
   
         static Long64_t ncalls=0;
   
         if (t) {
            TTree* tnew=(TTree*)t->Clone("tnew");
            tnew->SetDirectory(fDirProofBench);
   
            //build up new name
            TString newname = perfstats_name;
            newname+="_";
            newname+="Cleanup";
            newname+=Form("%lld", ncalls);
            tnew->SetName(newname);
   
            if (fDirProofBench->IsWritable()){
               fDirProofBench->cd();
               tnew->Write();
               tnew->Delete();
            }
         } else {
            Error("Run", "tree %s not found",  perfstats_name.Data());
         }
   
         //save outputhistos
         TString ptdist_name = "pt_dist";
         TH1* h = dynamic_cast<TH1*>(l->FindObject(ptdist_name.Data()));
         if (h) {
            TH1 *hnew = (TH1*)h->Clone("hnew");
            hnew->SetDirectory(fDirProofBench);

            //build up new name
            TString origname = h->GetName();
            TString newname = ptdist_name;
            newname+="_";
            newname+="Cleanup";
            newname+=Form("%lld", ncalls);
            hnew->SetName(newname);
   
            if (fDirProofBench->IsWritable()){
               fDirProofBench->cd();
               hnew->Write();
               delete hnew;
            }
         } else {
            Error("Run", "histogram %s not found",  ptdist_name.Data());
         }
   
         TString tracksdist_name = "ntracks_dist";
         TH1* h2 = dynamic_cast<TH1*>(l->FindObject(tracksdist_name.Data()));
         if (h2) {
            TH1 *hnew = (TH1*)h2->Clone("hnew");
            hnew->SetDirectory(fDirProofBench);

            //build up new name
            TString newname = tracksdist_name;
            newname+="_";
            newname+="Cleanup";
            newname+=Form("%lld", ncalls);
            hnew->SetName(newname);

            if (fDirProofBench->IsWritable()){
               fDirProofBench->cd();
               hnew->Write();
               delete hnew;
            }
         }
         else {
            Error("Run", "histogram %s not found",  tracksdist_name.Data());
         }
         ncalls++;
      }
   }
   else if (fCleanupType==TProofBenchRun::kCleanupFileAdvise){
      TString inputdataname="PROOF_BenchmarkFilesToCleanupCacheFor";
      //fProof->ClearInputData(inputdataname.Data());

      TFileCollection* fc=fProof->GetDataSet(fDataSetCleanup.Data());
 
      if (fc){
         THashList* l=fc->GetList();

         THashList* lcopy=dynamic_cast<THashList*>(l->Clone());
 
         lcopy->SetName(inputdataname.Data());
         fProof->AddInputData(lcopy); 

         Info("Run", "Cleaning up file cache of data set %s.",
                     fDataSetCleanup.Data());
         fProof->Process("TSelEvent", Long64_t(1));
         fProof->ClearInputData(inputdataname.Data()); 
         delete lcopy;
         //Wait a second or 2 because start time of TQueryResult has only 1-second precision.
         gSystem->Sleep(1500);
      }
      else{
         Error("Run", "DataSet not found: %s", fDataSetCleanup.Data());
      }
   }
   else if (fCleanupType==TProofBenchRun::kCleanupNotSpecified){
      Error("Run", "fCleanupType==kCleanupNotSpecified; try again"
                   " with either TProofBenchRun::kCleanupReadInFiles"
                   " or TProofBenchRun::kCleanupFileAdvise");
   }

   DeleteParameters();
}

//______________________________________________________________________________
void TProofBenchRunCleanup::Print(Option_t* option) const
{
   Printf("Name=%s", fName.Data());
   if (fProof) fProof->Print(option);
   Printf("fCleanupType=%s%s", "k", fName.Data());
   Printf("fDebug=%d", fDebug);
   if (fDirProofBench){
      Printf("fDirProofBench=%s", fDirProofBench->GetPath());
   }
}

//______________________________________________________________________________
void TProofBenchRunCleanup::DrawPerfProfiles()
{
   Info("DrawPerfProfiles", "There is nothing to be done");
   return; 
}

//______________________________________________________________________________
void TProofBenchRunCleanup::SetCleanupType
       (TProofBenchRun::ECleanupType cleanuptype)
{
   fCleanupType=cleanuptype;
}

//______________________________________________________________________________
void TProofBenchRunCleanup::SetDataSetCleanup(const TString& dataset)
{
   fDataSetCleanup=dataset;
}

//______________________________________________________________________________
void TProofBenchRunCleanup::SetDebug(Int_t debug)
{
   fDebug=debug;
}

//______________________________________________________________________________
void TProofBenchRunCleanup::SetDirProofBench(TDirectory* dir)
{
   fDirProofBench=dir;
}

//______________________________________________________________________________
TProofBenchRun::ECleanupType TProofBenchRunCleanup::GetCleanupType() const
{
   return fCleanupType;
}

//______________________________________________________________________________
TString TProofBenchRunCleanup::GetDataSetCleanup() const
{
   return fDataSetCleanup;
}

//______________________________________________________________________________
Int_t TProofBenchRunCleanup::GetDebug() const
{
   return fDebug;
}

//______________________________________________________________________________
TDirectory* TProofBenchRunCleanup::GetDirProofBench() const
{
   return fDirProofBench;
}

//______________________________________________________________________________
const char* TProofBenchRunCleanup::GetName() const
{
   return fName.Data();
}

//______________________________________________________________________________
Int_t TProofBenchRunCleanup::SetParameters()
{
   if (!fProof){
      Error("SetParameters", "Proof not set; Doing nothing");
      return 1;
   }
   fProof->SetParameter("PROOF_BenchmarkCleanupType", Int_t(fCleanupType));
   fProof->SetParameter("PROOF_BenchmarkDebug", Int_t(fDebug));

   return 0;
}

//______________________________________________________________________________
Int_t TProofBenchRunCleanup::DeleteParameters()
{
   if (!fProof){
      Error("DeleteParameters", "Proof not set; Doing nothing");
      return 1;
   }
   fProof->DeleteParameters("PROOF_BenchmarkCleanupType");
   fProof->DeleteParameters("PROOF_BenchmarkDebug");
   return 0;
}

//______________________________________________________________________________
const char* TProofBenchRunCleanup::BuildPatternName(const char* objname,
                                                    const char* delimiter)
{

   TString newname(objname);
   newname+=delimiter;
   newname+=GetName();
   return newname.Data();
}
