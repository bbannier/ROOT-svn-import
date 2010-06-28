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
#include "TProofNode.h"
#include "TFile.h"
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

ClassImp(TProofBenchRunCleanup)

//______________________________________________________________________________
TProofBenchRunCleanup::TProofBenchRunCleanup(TProofBenchRun::ECleanupType cleanuptype,
               TString filename,
               Option_t* foption,
               TProof* proof,
               Int_t maxnworkers,
               Long64_t nevents,
               Int_t draw,
               Int_t debug):
fProof(0),
fCleanupType(cleanuptype),
fNEvents(nevents),
fDraw(draw),
fDebug(debug),
fFile(0),
fDirProofBench(0),
fWritable(0)
{
   //Default constructor
 
   fProof=proof?proof:gProof;

   //if (filename.Length()){
   if(!OpenFile(filename.Data(), foption)){
      gDirectory->mkdir("ProofBench");
      gDirectory->cd("ProofBench");
      fDirProofBench=gDirectory;
   }

   TString name="Cleanup"+GetNameStem();

   fName=name;
   if (maxnworkers>0){
      SetMaxNWorkers(maxnworkers);
   }
   else{
      SetMaxNWorkers("1x");
   }

   gEnv->SetValue("Proof.StatsTrace",1);

}

//______________________________________________________________________________
TProofBenchRunCleanup::~TProofBenchRunCleanup()
{
   //destructor
   fProof=0;
   fDirProofBench=0;
   if (fFile){
      fFile->Close();
      delete fFile;
   }
} 

//______________________________________________________________________________
TString TProofBenchRunCleanup::GetNameStem()const
{
   TString namestem;
   switch (fCleanupType){
   case TProofBenchRun::kCleanupFile:
      namestem="File";
      break;
   case TProofBenchRun::kCleanupKernel:
      namestem="Kernel";
      break;
   default:
      break;
   }
    return namestem;
}

//______________________________________________________________________________
void TProofBenchRunCleanup::Run(Long64_t,
                                Int_t,
                                Int_t,
                                Int_t,
                                Int_t,
                                Int_t debug,
                                Int_t draw)
{
   // Clean up cache between bench mark runs. 
   // Input parameters
   //    Long64_t Ignored.
   //    Int_t    Ignored.
   //    Int_t    Ignored.
   //    Int_t    Ignored.
   //    Int_t    Ignored.
   //    debug    debug switch  
   //    draw     draw switch
   // Return
   //    Nothing

   if (!fProof){
      Error("RunBenchmark", "Proof not set");
      return;
   }

   Info("RunBenchmark", "kRunCleanup");

   fProof->SetParallel(99999);

   DeleteParameters();
   SetParameters();

   if (fCleanupType==TProofBenchRun::kCleanupFile){
      TString dsname="DataSetEventCleanup";
      TFileCollection* fc_cleanup=fProof->GetDataSet(dsname.Data());

      TTime starttime = gSystem->Now();
      if (fc_cleanup){
         fProof->Process(fc_cleanup, "TSelEvent", "", -1);
      }
      else{
         Error("Run", "File Collection does not exist; returning", dsname.Data());
         return;
      }

      if (debug){
   
         TList* l = fProof->GetOutputList();
   
         TString perfstats_name = "PROOF_PerfStats";
         //save perfstats
         TTree* t = dynamic_cast<TTree*>(l->FindObject(perfstats_name.Data()));
   
         static Long64_t ncalls=0;
   
         if (t) {
            t->SetDirectory(fDirProofBench);
   
            //build up new name
            TString newname = perfstats_name;
            newname+="_";
            newname+="Cleanup";
            newname+=Form("%lld", ncalls);
            t->SetName(newname);
   
            if (fWritable){
               fDirProofBench->cd();
               t->Write();
            }
         } else {
            Error("RunBenchmark", "tree %s not found",  perfstats_name.Data());
         }
   
         //save outputhistos
         TString ptdist_name = "pt_dist";
         TH1* h = dynamic_cast<TH1*>(l->FindObject(ptdist_name.Data()));
         if (h) {
            //TDirectory* hdir = h->GetDirectory();
            //TDirectory* dirsav = gDirectory;
            //fFile->cd();
            TH1 *hnew = (TH1*)h->Clone("hnew");
   
            hnew->SetDirectory(fDirProofBench);
            TString origname = h->GetName();
            TString newname = ptdist_name;
            newname+="_";
            newname+="Cleanup";
            newname+=Form("%lld", ncalls);
            hnew->SetName(newname);
   
            if (fWritable){
               fDirProofBench->cd();
               hnew->Write();
               delete hnew;
            }
         } else {
            Error("RunBenchmark", "histogram %s not found",  ptdist_name.Data());
         }
   
         TString tracksdist_name = "ntracks_dist";
         TH1* h2 = dynamic_cast<TH1*>(l->FindObject(tracksdist_name.Data()));
         if (h2) {
            //TDirectory* hdir = h2->GetDirectory();
            //TDirectory* dirsav = gDirectory;
            //fFile->cd();
            TH1 *hnew = (TH1*)h2->Clone("hnew");
            hnew->SetDirectory(fDirProofBench);
            //TString origname = h2->GetName();
            TString newname = tracksdist_name;
            newname+="_";
            newname+="Cleanup";
            newname+=Form("%lld", ncalls);
            hnew->SetName(newname);
            if (fWritable){
               fDirProofBench->cd();
               hnew->Write();
               delete hnew;
            }
         }
         else {
            Error("RunBenchmark", "histogram %s not found",  tracksdist_name.Data());
         }
         ncalls++;
      }
   }
   else if (fCleanupType==TProofBenchRun::kCleanupKernel){
      TString inputdataname="PROOF_BenchmarkFilesToCleanupCacheFor";
      //fProof->ClearInputData(inputdataname.Data());

      TFileCollection* fc=fProof->GetDataSet(fDataSetCleanup.Data());
      THashList* l=fc->GetList();

      THashList* lcopy=dynamic_cast<THashList*>(l->Clone());
 
      lcopy->SetName(inputdataname.Data());
      fProof->AddInputData(lcopy); 

      Info("Run", "Cleaning up files in data set: %s", fDataSetCleanup.Data());
      fProof->Process("TSelEvent", Long64_t(0));
      fProof->ClearInputData(inputdataname.Data()); 
      delete lcopy;
      //Wait a second or 2 because start time of TQueryResult has only 1-second precision.
      gSystem->Sleep(1500);
   }
   else if (fCleanupType==TProofBenchRun::kCleanupNotSpecified){
      Error("Run", "fCleanupType==kCleanupNotSpecified; try again"
                   " with either TProofBenchRun::kCleanupFile or TProofBenchRun::kCleanupKernel");
   }

   DeleteParameters();
}

//______________________________________________________________________________
void TProofBenchRunCleanup::Print(Option_t* option)const{
   Printf("Name=%s", fName.Data());
   if (fProof) fProof->Print(option);
   Printf("fCleanupType=%s%s", "k", fName.Data());
   Printf("fNEvents=%lld", fNEvents);
   Printf("fMaxNWorkers=%d", fMaxNWorkers);
   Printf("fDraw=%d", fDraw);
   Printf("fDebug=%d", fDebug);
   if (fFile){
       fFile->Print(option);
       fFile->ls(option);
   }
   else{
      Printf("No file open");
   }
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
void TProofBenchRunCleanup::SetCleanupType(TProofBenchRun::ECleanupType cleanuptype)
{
   fCleanupType=cleanuptype;
}

//______________________________________________________________________________
void TProofBenchRunCleanup::SetNEvents(Long64_t nevents)
{
   fNEvents=nevents;
}

//______________________________________________________________________________
void TProofBenchRunCleanup::SetMaxNWorkers(Int_t maxnworkers)
{
  fMaxNWorkers=maxnworkers;
}

//______________________________________________________________________________
void TProofBenchRunCleanup::SetMaxNWorkers(TString sworkers)
{

   // Set the maximum number of workers for benchmark test
   // Input parameters
   //    sworkers: Can be "1x", "2x" and so on, where total number of workers is set 
   //              to 1*no_total_workers, 2*no_total_workers respectively.
   //              For now only "1x" is supported
   // Return
   //    Nothing

   sworkers.ToLower();
   sworkers.Remove(TString::kTrailing, ' ');
   if (fProof){
      if (sworkers.Contains("x")){//nx
         TList* lslave=fProof->GetListOfSlaveInfos();
         // Number of slave workers regardless of its status, active or inactive
         Int_t nslaves=lslave->GetSize();  
         sworkers.Remove(TString::kTrailing, 'x');
         Int_t mult=sworkers.Atoi();
         fMaxNWorkers=mult*nslaves;
      }
   }
   else{
      Error("SetMaxNWorkers", "Proof not set, doing nothing");
   }
   return;
}

//______________________________________________________________________________
void TProofBenchRunCleanup::SetDataSetCleanup(const TString& dataset)
{
   fDataSetCleanup=dataset;
}

//______________________________________________________________________________
void TProofBenchRunCleanup::SetDraw(Int_t draw)
{
   fDraw=draw;
}

//______________________________________________________________________________
void TProofBenchRunCleanup::SetDebug(Int_t debug)
{
   fDebug=debug;
}

//______________________________________________________________________________
TFile* TProofBenchRunCleanup::OpenFile(const char* filename,
                                       Option_t* option,
                                       const char* ftitle,
                                       Int_t compress)
{
   // Opens a file which output profiles and/or intermediate files (trees, histograms when debug is set)
   // are to be written to. Makes a directory named "ProofBench" if possible and changes to the directory.
   // If directory ProofBench already exists, change to the directory. If the directory can not be created,
   // make a directory Rint:/ProofBench and change to the directory.
   // Input parameters:
   //    filename: Name of the file to open
   //    option: Option to TFile::Open(...) function
   //    ftitle: Title for TFile::Open(...) function
   //    compress: Compression for TFile::Open(...) function
   // Returns
   //    Open file if a file is already open
   //    New file just opened
   //    0 when open fails;

   TString sfilename(filename);
   sfilename.Remove(TString::kBoth, ' '); //remove leading and trailing white space(s)
   sfilename.Remove(TString::kBoth, '\t');//remove leading and trailing tab character(s)

   //if (sfilename.Length()<1){
   //   return fFile;
   //}

   TString soption(option);
   soption.ToLower();

   if (fFile){
      Error("OpenFile", "File alaredy open; %s; Close it before open another file", fFile->GetName());
      return fFile;
   }

   TDirectory* dirsav=gDirectory;
   fFile=new TFile(sfilename, option, ftitle, compress);

   if (fFile->IsZombie()){ // Open failed
      Error("FileOpen", "Cannot open file: %s", sfilename.Data());
      fFile->Close();
      fFile=0;
      dirsav->cd();
      return 0;
   }
   else{ // Open succeeded
      fFile->mkdir("ProofBench");

      fFile->cd("ProofBench");
      SetDirProofBench(gDirectory);

      TString soption=fFile->GetOption();
      soption.ToLower();
      if (soption.Contains("create") || soption.Contains("update")){
         fWritable=1;
      }

      return fFile;
   }
}

//______________________________________________________________________________
void TProofBenchRunCleanup::SetDirProofBench(TDirectory* dir)
{
   fDirProofBench=dir;
}

//______________________________________________________________________________
TProofBenchRun::ECleanupType TProofBenchRunCleanup::GetCleanupType()const
{
   return fCleanupType;
}

//______________________________________________________________________________
Int_t TProofBenchRunCleanup::GetMaxNWorkers()const
{
   return fMaxNWorkers;
}

//______________________________________________________________________________
TString TProofBenchRunCleanup::GetDataSetCleanup()const
{
   return fDataSetCleanup;
}

//______________________________________________________________________________
Int_t TProofBenchRunCleanup::GetDraw()const
{
   return fDraw;
}

//______________________________________________________________________________
Int_t TProofBenchRunCleanup::GetDebug()const
{
   return fDebug;
}

//______________________________________________________________________________
TFile* TProofBenchRunCleanup::GetFile()const
{
   return fFile;
}

//______________________________________________________________________________
TDirectory* TProofBenchRunCleanup::GetDirProofBench()const
{
   return fDirProofBench;
}

//______________________________________________________________________________
const char* TProofBenchRunCleanup::GetName()const
{
   return fName.Data();
}

//______________________________________________________________________________
Int_t TProofBenchRunCleanup::SetParameters(){
   if (!fProof){
      Error("SetParameters", "Proof not set; Doing nothing");
      return 1;
   }
   fProof->SetParameter("PROOF_BenchmarkCleanupType", Int_t(fCleanupType));
   fProof->SetParameter("PROOF_BenchmarkDraw", Int_t(fDraw));
   fProof->SetParameter("PROOF_BenchmarkDebug", Int_t(fDebug));

   return 0;
}

//______________________________________________________________________________
Int_t TProofBenchRunCleanup::DeleteParameters(){
   if (!fProof){
      Error("DeleteParameters", "Proof not set; Doing nothing");
      return 1;
   }
   fProof->DeleteParameters("PROOF_BenchmarkCleanupType");
   fProof->DeleteParameters("PROOF_BenchmarkDraw");
   fProof->DeleteParameters("PROOF_BenchmarkDebug");
   return 0;
}

//______________________________________________________________________________
const char* TProofBenchRunCleanup::BuildPatternName(const char* objname, const char* delimiter)
{

   TString newname(objname);
   newname+=delimiter;
   newname+=GetName();
   return newname.Data();
}
