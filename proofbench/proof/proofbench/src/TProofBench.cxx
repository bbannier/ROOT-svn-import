// @(#)root/proofx:$Id$
// Author:

/*************************************************************************
 * Copyright (C) 1995-2005, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TProofBench                                                          //
//                                                                      //
// TProofBench is ...                                                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "TProofBench.h"
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

#include <stdlib.h>

ClassImp(TProofBench)

TProofBench::TProofBench(
                         TString fFilename, 
                         TProof* proof, 
                         TString basedir, 
                         ERunType runtype, 
                         EBenchmarkMode benchmarkmode,
                         Long_t nhists, 
                         EHistType histtype,
                         Int_t maxnworkers, 
                         Int_t nnodes, 
                         Int_t ntries, 
                         Int_t nfilesaworker,
                         Int_t nfilesanode,
                         Long64_t nevents, 
                         Int_t stepsize, 
                         Int_t start, 
                         Int_t ntracksbench,
                         Int_t ntrackscleanup,
                         Int_t draw,
                         Int_t debug,
                         Int_t regenerate):
fFile(0), 
fProof(proof), 
fBaseDir(""),
fRunType(kRunNotSpecified),
fNameStem(""),
fBenchmarkMode(benchmarkmode),
fNHists(16),
fHistType(kHistAll),
fMaxNWorkers(-1), 
fNNodes(-1), 
fNTries(10),
fNFilesAWorker(-1),
fNFilesANode(-1),
fNEvents(10000), 
fStepSize(1),
fStart(1), 
fNTracksBench(10),
fNTracksCleanup(100),
fDraw(0), 
fDebug(0),
fRegenerate(0),
fNEventsGenerated(0),
fNFilesGeneratedBench(0), 
fFilesGeneratedCleanup(kFALSE),
fDataSet(0), 
fDataSetGeneratedBench(0), 
fDataSetGeneratedCleanup(0), 
fCPerfProfiles(0), 
fDirProofBench(0),
fProfCPUTestEvent(0),
fProfOptDataReadEvent(0),
fProfOptDataReadIO(0),
fProfFullDataReadEvent(0),
fProfFullDataReadIO(0),
fNodes(0),
fNodeInfo(0)
//fListRunType(0)
{

   //Default constructor

   FileOpen(fFilename, "new");
   SetBaseDir(basedir);
   SetRunType("TProofBench", runtype);
   SetNHists(nhists);
   SetHistType(histtype);

   if (maxnworkers>0){ 
      SetMaxNWorkers(maxnworkers);
   }
   else{
      SetMaxNWorkers("2x");
   }

   SetNNodes(nnodes);
   SetNTries(ntries);
   SetNEvents(nevents);
   SetStepSize(stepsize);
   SetStart(start);
   SetNTracksBench(ntracksbench);
   SetNTracksCleanup(ntrackscleanup);
   SetDraw(draw);
   SetDebug(debug);
   SetRegenerate(regenerate);
   BuildNodesInfo();

   switch (benchmarkmode){
   //fixed number of files for each node regardless of the number of workers on the node
   case kModeStaticNode: 
      if (nfilesanode>0){
         SetBenchmarkMode(kModeStaticNode, nfilesanode);
      }
      else{
      //default: max number of workers on a node in the cluster
         TIterator* nxn=fNodeInfo->MakeIterator();
         TObjString *nk = 0;
         Int_t maxnw=0;
         Int_t nw=0;
         TString snw;
         while (nk=(TObjString*)nxn->Next()){
            snw=((TObjString*)fNodeInfo->GetValue(nk))->String();
            nw=snw.Atoi();
            maxnw=nw>maxnw?nw:maxnw;
         }
         SetBenchmarkMode(kModeStaticNode, maxnw);
      }
      break;
   case kModeStaticWorkersNode:
      if (nfilesaworker>0){
         SetBenchmarkMode(kModeStaticWorkersNode, nfilesaworker);
      }
      else{ //default; 1 file a worker
         SetBenchmarkMode(kModeStaticWorkersNode, 1);
      } 
      break;
   case kModeStaticCluster:
      break;
   case kModeDynamicWorkers:
      break;
   default:
      break;
   }

   //fListRunType=new TList;
   //fListRunType->Add(new TParameter<Int_t>("kRunNotSpecified", Int_t(kRunNotSpecified))); 
   ////fListRunType->Add(new TParameter<Int_t>("kRunNothing", Int_t(kRunNothing))); 
   //fListRunType->Add(new TParameter<Int_t>("kRunCPUTest", Int_t(kRunCPUTest))); 
   //fListRunType->Add(new TParameter<Int_t>("kRunFullDataRead", Int_t(kRunFullDataRead))); 
   //fListRunType->Add(new TParameter<Int_t>("kRunOptDataRead", Int_t(kRunOptDataRead))); 
   //fListRunType->Add(new TParameter<Int_t>("kRunNoDataRead", Int_t(kRunNoDataRead))); 
   //fListRunType->Add(new TParameter<Int_t>("kRunGenerateFileBench", Int_t(kRunGenerateFileBench))); 
   //fListRunType->Add(new TParameter<Int_t>("kRunGenerateFileCleanup",Int_t(kRunGenerateFileCleanup))); 
   //fListRunType->Add(new TParameter<Int_t>("kRunCleanup", Int_t(kRunCleanup))); 

   //SetParameters(basedir, nhists, maxnworkers, nnodes, ntries, nevents, stepsize, start);
   // switch logging on
   // Probably changing environment variable is not a good idea. This should be fixed
   gEnv->SetValue("Proof.StatsHist",0);
   gEnv->SetValue("Proof.StatsTrace",1);
   gEnv->SetValue("Proof.SlaveStatsTrace",0);
}

void TProofBench::SetBaseDir(TString basedir)
{
   basedir.Remove(TString::kBoth, ' '); //remove leading and trailing white space(s)
   basedir.Remove(TString::kBoth, '\t');//remove leading and trailing tab character(s)
   basedir.Remove(TString::kTrailing, '/'); //remove trailing /

   //see if files were already generated
   //if ((basedir.Length()<=0 || basedir==fBaseDir) //no base directory change
   //number of files requested is smaller than files already generated
   //&& ((ntries<=0 && fNTriesGenerated>0) || (ntries>0 && ntries<=fNTriesGenerated))
   //number of events/file is smaller than that in files already generated
   //&& ((nevents<=0 && fNEventsGenerated>0) || (nevents>0 && nevents<=fNEventsGenerated))){
   //}
   //else{ //file generation is needed)
   //returnval=1;
   //}
   fBaseDir=basedir.Length()>0?basedir:fBaseDir;
}

void TProofBench::SetRunType(TString where, ERunType runtype)
{
   switch (runtype){
   case kRunNotSpecified:
      fRunType=runtype;
      fNameStem="RunNotSpecified";
      break;
   //case kRunNothing:
   //   fRunType=runtype;
   //   fNameStem="RunNothing";
   //   return;
   //   break;
   case kRunCPUTest:
      fRunType=runtype;
      fNameStem="CPUTest";
      break;
   case kRunGenerateFileBench:
      fRunType=runtype;
      fNameStem="GenerateFileBench";
      break;
   case kRunGenerateFileCleanup:
      fRunType=runtype;
      fNameStem="GenerateFileCleanup";
      break;
   case kRunCleanup:
      fRunType=runtype;
      fNameStem="Cleanup";
      break;
   case kRunFullDataRead:
      fRunType=runtype;
      fNameStem="FullDataRead";
      break;
   case kRunOptDataRead:
      fRunType=runtype;
      fNameStem="OptDataRead";
      break;
   case kRunNoDataRead:
      fRunType=runtype;
      fNameStem="NoDataRead";
      break;
   default:
      Error(where.Data(), "Invalid run type: use one of (" //TProofBench::kRunNothing,"
                   "TProofBench::kRunCPUTest, TProofBench::kRunGenerateFileBench,"
                   "TProofBench::kRunGenerateFileCleanup, TProofBench::kRunCleanup,"
                   "TProofBench::kRunFullDataRead, TProofBench::kRunOptDataRead,"
                   " or TProofBench::kRunNoDataRead)");
      return;
      break;
   }

}

void TProofBench::SetBenchmarkMode(EBenchmarkMode benchmarkmode, Int_t par)
{
   if (par<=0){ //second argument not given
      switch (benchmarkmode){
         case kModeStaticNode:
            if (fNFilesANode>0){
               fBenchmarkMode=benchmarkmode;
               return;
            }
         break;
         case kModeStaticWorkersNode:
            if (fNFilesAWorker>0){
               fBenchmarkMode=benchmarkmode;
               return;
            }
         break;
         case kModeStaticCluster:
         break;
         case kModeDynamicWorkers:
         break;
         default:
         break;
      }
    
      Error("SetBenchmarkMode", "BenchmarkMode is not compatible with its parameter");
      return;
   }
   else{
      switch (benchmarkmode){
         case kModeStaticNode:
            fBenchmarkMode=benchmarkmode;
            fNFilesANode=par;
            return;
         break;
         case kModeStaticWorkersNode:
            fBenchmarkMode=benchmarkmode;
            fNFilesAWorker=par;
            return;
         break;
         case kModeStaticCluster:
         break;
         case kModeDynamicWorkers:
         break;
         default:
         break;

         return;
      }
   }
}

void TProofBench::SetNHists(Long_t nhists)
{
   fNHists=nhists>-1?nhists: fNHists;
}

void TProofBench::SetMaxNWorkers(Int_t maxnworkers)
{
  fMaxNWorkers=maxnworkers>-1?maxnworkers:fMaxNWorkers;
}

void TProofBench::SetMaxNWorkers(TString sworkers)
{
   sworkers.ToLower();
   sworkers.Remove(TString::kTrailing, ' ');
   if (fProof){
      if (sworkers.Contains("x")){//nx
         TList* lslave=fProof->GetListOfSlaveInfos();
         Int_t nslaves=lslave->GetSize();  //number of slave workers regardless of its status, active or inactive
         sworkers.Remove(TString::kTrailing, 'x');
         Int_t mult=sworkers.Atoi();
         fMaxNWorkers=mult*nslaves; //this number to be parameterized in the future
      }
   }
   else{
      Error("SetMaxNWorkers", "Proof not set, doing nothing");
   }
   return;
}

void TProofBench::SetNTries(Int_t ntries)
{
   fNTries=ntries>-1?ntries:fNTries;
}

void TProofBench::SetNFilesAWorker(Int_t nfilesaworker)
{
   fNFilesAWorker=nfilesaworker>-1?nfilesaworker:fNFilesAWorker;
}

void TProofBench::SetNFilesANode(Int_t nfilesanode)
{
   fNFilesANode=nfilesanode>-1?nfilesanode:fNFilesANode;
}

void TProofBench::SetNEvents(Long64_t nevents)
{
   fNEvents=nevents>-1?nevents:fNEvents;
}

void TProofBench::SetStepSize(Int_t stepsize)
{
   fStepSize=stepsize>-1?stepsize:fStepSize;
}

void TProofBench::SetStart(Int_t start)
{
   fStart=start>-1?start:fStart;
}

void TProofBench::SetNTracksBench(Int_t ntracksbench){
   fNTracksBench=ntracksbench>-1?ntracksbench:fNTracksBench;
}

void TProofBench::SetNTracksCleanup(Int_t ntrackscleanup){
   fNTracksCleanup=ntrackscleanup>-1?ntrackscleanup:fNTracksCleanup;
}

void TProofBench::SetDraw(Int_t draw)
{
   fDraw=draw>-1?draw:fDraw;
}

void TProofBench::SetDebug(Int_t debug)
{
   fDebug=debug>-1?debug:fDebug;
}

void TProofBench::SetRegenerate(Int_t regenerate)
{
   fRegenerate=regenerate>-1?regenerate:fRegenerate;
}

TProofBench::~TProofBench()
{
//destructor

   if (fDataSet){ delete fDataSet;}
   if (fDataSetGeneratedBench){ delete fDataSetGeneratedBench;}
   if (fDataSetGeneratedCleanup){ delete fDataSetGeneratedCleanup;}
   if (fFile){ 
      TString soption=fFile->GetOption(); 
      soption.ToLower(); 
      if (soption.Contains("create") || soption.Contains("update")){
         fFile->Write(0, TFile::kWriteDelete);
      }
      fFile->Close(); 
      delete fFile;
   }
   fProof=0;
   //if (fListRunType){
   //   delete fListRunType;
   //   fListRunType=0;
   //}
   if (fNodeInfo) delete fNodeInfo;
} 

void TProofBench::RunBenchmarkAll()
{

   if (!fProof){
      Error("RunBenchmarkAll", "Proof not set");
      return;
   }

   RunBenchmark(kRunCPUTest);
   BuildPerfProfiles(kRunCPUTest);
   RunBenchmark(kRunOptDataRead);
   BuildPerfProfiles(kRunOptDataRead);
   RunBenchmark(kRunFullDataRead);
   BuildPerfProfiles(kRunFullDataRead);
}

void TProofBench::RunBenchmark(ERunType whattorun, Bool_t regenerate)
{
   if (!fProof){
      Error("RunBenchmark", "Proof not set");
      return;
   }

   TString perfstats_name = "PROOF_PerfStats";

   switch (whattorun){
   //case kRunNothing:
   //   Info("RunBenchmark", "kRunNothing requested; doing nothing and returning");
   //   SetRunType("RunBenchmark", kRunNothing);
   //   return;
   //   break;
   case kRunCPUTest:
 
      Info("RunBenchmark", "kRunCPUTest");
      SetRunType("RunBenchmark", kRunCPUTest);
      if (CheckParameters("RunBenchmark")) return;
      SetInputParameters();

      for (Int_t nactive=fStart; nactive<=fMaxNWorkers; nactive+=fStepSize) {
         fProof->SetParallel(nactive);
         for (Int_t j=0; j<fNTries; j++) {

            TTime starttime = gSystem->Now();
            fProof->Process("TSelHist", fNEvents);
            TTime endtime = gSystem->Now();

            //TList* l = fDataSet->GetOutputList();
            TList* l = fProof->GetOutputList();

            //save perfstats
            TTree* t = dynamic_cast<TTree*>(l->FindObject(perfstats_name.Data()));
            if (t) {
               //TDirectory* trdir = t->GetDirectory();
               //TDirectory* dirsav = gDirectory;

               t->SetDirectory(fDirProofBench);

               TString origname = t->GetName();

               //build up new name
               TString newname = perfstats_name;
               newname+="_";
               newname+=fNameStem;
               newname+="_";
               newname+=nactive;
               newname+="slaves_run";
               newname+=j;
               t->SetName(newname);

               if (fDebug){
                  if (fFile && !fFile->IsZombie()){
                     fDirProofBench->cd();
                     t->Write();
                     //t->SetName(origname);
                     //t->SetDirectory(trdir);
                     //t->Delete("");
                  }
               }
                  //dirsav->cd();
            } else {
               Error("RunBenchmark", "tree %s not found", perfstats_name.Data());
            }
         }//for iterations
      }//for number of workers

      break;
   case kRunGenerateFileBench:
      Info("RunBenchmark", "kRunGenerateFileBench");
      SetRunType("RunBenchmark", kRunGenerateFileBench);
      fRegenerate=regenerate;
      if (CheckParameters("RunBenchmark")) return;
      GenerateFiles();
      break;
   case kRunGenerateFileCleanup:
      Info("RunBenchmark", "kRunGenerateFileCleanup");
      SetRunType("RunBenchmark", kRunGenerateFileCleanup);
      fRegenerate=regenerate;
      if (CheckParameters("RunBenchmark")) return;
      GenerateFiles();
      break;
   case kRunCleanup:
      {
         //make sure files are generated
         RunBenchmark(kRunGenerateFileCleanup);

         Info("RunBenchmark", "kRunCleanup");
         SetRunType("RunBenchmark", kRunCleanup);
         if (CheckParameters("RunBenchmark")) return;
         Int_t nactive_sav=fProof->GetParallel();
         fProof->SetParallel(fMaxNWorkers);

         SetInputParameters();
         fDataSetGeneratedCleanup->Print("a");

         TTime starttime = gSystem->Now();

         fProof->Process(fDataSetGeneratedCleanup, "TSelEvent", "", -1);

         if (fDebug){

            TList* l = fDataSetGeneratedCleanup->GetOutputList();

            //Info("RunBenchmark", "Printing output list");
            //l->Print("a");
            //Info("RunBenchmark", "Printing output list done");

            //save perfstats
            TTree* t = dynamic_cast<TTree*>(l->FindObject(perfstats_name.Data()));

            static int ncalls=0;
            if (t) {
               //TDirectory* trdir = t->GetDirectory();
               //TDirectory* dirsav = gDirectory;
               //fFile->cd();
               t->SetDirectory(fDirProofBench);
               //TString origname = t->GetName();

               //build up new name
               TString newname = perfstats_name;
               newname+="_";
               newname+=fNameStem;
               newname+=Form("%d", ncalls);
               t->SetName(newname);

               if (fFile && !fFile->IsZombie()) {
                  fDirProofBench->cd();
                  t->Write();
                  //tnew->SetName(origname);
                  //tnew->SetDirectory(trdir);
                  //t->Delete("");
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
               newname+=fNameStem;
               newname+=Form("%d", ncalls);
               hnew->SetName(newname);

               if (fFile && !fFile->IsZombie()) {
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
               newname+=fNameStem;
               newname+=Form("%d", ncalls);
               hnew->SetName(newname);
               if (fFile && !fFile->IsZombie()) {
                  fDirProofBench->cd();
                  hnew->Write();
                  delete hnew;
               }
            } 
            else {
               Error("RunBenchmark", "histogram %s not found",  tracksdist_name.Data());
            }
            ncalls++;
            fProof->SetParallel(nactive_sav);
         }
      } 
      break;
   case kRunFullDataRead:
   case kRunOptDataRead:
   case kRunNoDataRead:

      //make sure files are there
      RunBenchmark(kRunGenerateFileCleanup);
      RunBenchmark(kRunGenerateFileBench);

      for (Int_t nactive=fStart; nactive<=fMaxNWorkers; nactive+=fStepSize) {
         for (Int_t j=0; j<fNTries; j++) {

            //cleanup run
            RunBenchmark(kRunCleanup);

            if (whattorun==kRunFullDataRead) Info("RunBenchmark", "kRunFullDataRead");
            else if (whattorun==kRunOptDataRead) Info("RunBenchmark", "kRunOptDataRead");
            else if (whattorun==kRunNoDataRead) Info("RunBenchmark", "kRunNoDataRead");

            SetRunType("RunBenchmark", whattorun);
            if (CheckParameters("RunBenchmark")) return;

            MakeDataSet();
            fDataSet->Print("a");

            fProof->SetParallel(nactive);

            SetInputParameters();
            TTime starttime = gSystem->Now();
            fProof->Process(fDataSet, "TSelEvent", "", -1);

            TTime endtime = gSystem->Now();

            TList* l = fDataSet->GetOutputList();

            //Info("RunBenchmark", "Printing output list");
            //l->Print("a");
            //Info("RunBenchmark", "Printing output list done");

            //save perfstats
            TTree* t = dynamic_cast<TTree*>(l->FindObject(perfstats_name.Data()));
            if (t) {
               //TDirectory* trdir = t->GetDirectory();
               //TDirectory* dirsav = gDirectory;
               //fFile->cd();
               t->SetDirectory(fDirProofBench);
               //TString origname = t->GetName();

               //build up new name
               TString newname = perfstats_name;
               newname+="_";
               newname+=fNameStem;
               newname+="_";
               newname+=nactive;
               newname+="slaves_run";
               newname+=j;
               t->SetName(newname);
               if (fDebug){
                  if (fFile && !fFile->IsZombie()) {
                     fDirProofBench->cd();
                     t->Write();
                     //tnew->SetName(origname);
                     //tnew->SetDirectory(trdir);
                     //t->Delete("");
                  }
               }
               //dirsav->cd();
            } else {
               cout << perfstats_name.Data() << " tree not found" << endl << flush;
            }

            //save outputhistos 
            if (fDebug){
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
                  newname+=fNameStem;
                  newname+="_";
                  newname+=nactive;
                  newname+="slaves_run";
                  newname+=j;
                  hnew->SetName(newname);
                  if (fFile && !fFile->IsZombie()) {
                     fDirProofBench->cd();
                     hnew->Write();
                     //hnew->SetName(origname);
                     //hnew->SetDirectory(hdir);
                     delete hnew;
                  }
               
                  //dirsav->cd();
               } else {
                  Error("RunBenchmark", "histogram %s not found", ptdist_name.Data());
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
                  newname+=fNameStem;
                  newname+="_";
                  newname+=nactive;
                  newname+="slaves_run";
                  newname+=j;
                  hnew->SetName(newname);
                  if (fFile && !fFile->IsZombie()) {
                    fDirProofBench->cd();
                    hnew->Write();
                    //h2->SetName(origname);
                    //h2->SetDirectory(hdir);
                    delete hnew;
                  }
                  //dirsav->cd();
               } 
               else {
                  Error("RunBenchmark", "histogram %s not found", tracksdist_name.Data());
               }
            }
         }//for ntries
      }//for number of workers
      break;
   default:
      Error("RunBenchmark", "Invalid run type: use one of kRunFullDataRead, kRunOptDataRead or kRunNoDataRead");
      return;
      break;
   }
}

//_________________________________________________________________________________
Int_t TProofBench::FillNodeInfo()
{
   // Re-Generate the list of worker node info (fNodes)
   // Return 0 if OK, -1 if info could not be retrieved
   // (the existing info is always removed)

   if (fNodes) {
      fNodes->SetOwner(kTRUE);
      SafeDelete(fNodes);
   }
   fNodes = new TList;

   // Get info
   TList *wl = fProof->GetListOfSlaveInfos();
   if (!wl) {
      Error("FillNodeInfo", "could not get information about workers!");
      return -1;
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
//_________________________________________________________________________________
Int_t TProofBench::CreateDataSetsN(const char *basedir, const char *lab,
                                   Int_t np, const Int_t *wp,
                                   Int_t nr, Int_t nfw, Int_t nfmx)
{
   // Create the datasets for tests wth PROOF-Lite
   // NB: Should be extended to the case the files are not local

   // Check inputs
   if (np <= 0 || !wp || !fProof) {
      Error("CreateDataSetsN", "wrong inputs (%d, %p, %p)", np, wp, fProof);
      return -1;
   }

   // There will be 'nr' datasets per point, rotating the files
   // Dataset naming:
   //                   ds_event_[lab]_[wrks_point]_[run]
   //
   TString dsname, slab("");
   if (lab && strlen(lab) > 0) slab.Form("_%s", lab);
   Int_t kp, kr, kf, kk = 0;
   for (kp = 0; kp < np; kp++) {
      for (kr = 0; kr < nr; kr++) {
         // Dataset name
 	 dsname.Form("ds_event%s_%d_%d", slab.Data(), wp[kp], kr);
         Info("CreateDataSetsN", "creating dataset '%s' ...", dsname.Data());
         // Create the TFileCollection
         TFileCollection *fc = new TFileCollection;
         Int_t nf = nfw * wp[kp];
         for (kf = 0; kf < nf; kf++) {
            Info("CreateDataSetsN", "adding file '%s/event_%d.root' ...", basedir, kk);
            fc->Add(new TFileInfo(TString::Format("%s/event_%d.root", basedir, kk++)));
            if (kk >= nfmx) kk = 0;
         }
         fc->Update();
         // Register dataset with verification
         fProof->RegisterDataSet(dsname, fc, "OV");
         // Cleanup
         delete fc;
      }
   }
   // Done
   return 0;
}

//_________________________________________________________________________________
Int_t TProofBench::GenerateFilesN(Int_t nf, Long64_t fileent)
{
   // Generate the files needed for the test using TPacketizerFile
   // *** This is only to show how it works ***

   // Find out the number of physically different machines
   if (FillNodeInfo()) {
      Error("GenerateFilesN", "could not get information about workers!");
      return -1;
   }

   // Number of events per file
   Long64_t oldNEvents = fNEvents;
   fNEvents = fileent;

   // Create the file names and the map {worker,files}
   // Naming:
   //         <basedir>/event_<file>.root
   TMap *filesmap = new TMap;
   filesmap->SetName("PROOF_FilesToProcess");
   Long64_t entries = 0;
   TIter nxni(fNodes);
   TProofNode *ni = 0;
   while ((ni = (TProofNode *) nxni())) {
      TList *files = new TList;
      files->SetName(ni->GetName());
      Int_t i = 0;
      for (i = 0; i < nf; i++) {
         files->Add(new TObjString(TString::Format("%s/event_%d.root", fBaseDir.Data(), i)));
         entries++;
      }
      filesmap->Add(new TObjString(ni->GetName()), files);
      files->Print();
   }

   // Set the relevant input parameters
   SetInputParameters();

   // Add the file map in the input list
   fProof->AddInput(filesmap);

   // Set the packetizer to be the one on test
   fProof->SetParameter("PROOF_Packetizer", "TPacketizerFile");

   // Run
   fProof->Process("TSelEventGenN", entries);

   // Clear the input parameters
   ClearInputParameters();
   fProof->DeleteParameters("PROOF_Packetizer");
   fProof->GetInputList()->Remove(filesmap);
   filesmap->SetOwner(kTRUE);
   SafeDelete(filesmap);

   // Restore previous value
   fNEvents = oldNEvents;

   // Done
   return 0;
}

//_________________________________________________________________________________
Int_t TProofBench::GenerateDataSetN(const char *dset,
                                    Int_t nw, Int_t nfw, Long64_t fileent)
{
   // Generate the files needed for a test on PROOF-Lite with nw workers.
   // It will generate nfw files per worker for 'nr' different runs.
   // Each file will have 'fileent' entries.
   // A set of datasets 'dset_[nw]_[run]' are automatically registered and verified
   // Uses TPacketizerFile.
   // *** This is only to show how it works ***

   // Number of events per file
   Long64_t oldNEvents = fNEvents;
   fNEvents = fileent;

   // Create dataset containers
   Int_t nr = 4;
   TFileCollection fcs[4];
   Int_t ir = 0;
   for (ir = 0; ir < nr ; ir++) {
     fcs[ir].SetName(TString::Format("%s_%d_%d", dset, nw, ir));
   }

   // Number of files
   Int_t nf = nw * nfw;

   // Create the file names and the map {worker,files}
   // Naming:
   //         <basedir>/event_<file>.root
   TMap *filesmap = new TMap;
   filesmap->SetName("PROOF_FilesToProcess");
   TList *files = new TList;
   files->SetName(gSystem->HostName());
   Long64_t entries = 0;
   Int_t kk = 0;
   for (ir = 0; ir < nr ; ir++) {
      Int_t i = 0;
      for (i = 0; i < nf; i++) {
         fcs[ir].Add(new TFileInfo(TString::Format("%s/event_%d.root", fBaseDir.Data(), kk)));
         files->Add(new TObjString(TString::Format("%s/event_%d.root", fBaseDir.Data(), kk++)));
         entries++;
      }
   }
   filesmap->Add(new TObjString((gSystem->HostName())), files);
   files->Print();

   // Set the relevant input parameters
   SetInputParameters();

   // Add the file map in the input list
   fProof->AddInput(filesmap);

   // Set the packetizer to be the one on test
   fProof->SetParameter("PROOF_Packetizer", "TPacketizerFile");

   // Run
   fProof->Process("TSelEventGenN", entries);

   // Clear the input parameters
   ClearInputParameters();
   fProof->DeleteParameters("PROOF_Packetizer");
   fProof->GetInputList()->Remove(filesmap);
   filesmap->SetOwner(kTRUE);
   SafeDelete(filesmap);

   // Restore previous value
   fNEvents = oldNEvents;

   // Register and verify the datasets
   for (ir = 0; ir < nr ; ir++) {
     fProof->RegisterDataSet(fcs[ir].GetName(), &fcs[ir], "OV");
   }

   // Done
   return 0;
}

Int_t TProofBench::GenerateFiles()
{

//Generate files on worker nodes for I/O test or for cleanup run
//basedir: base directory for the files to be generated on the worker node. 
//Use "" (empty string) not to change the current base directory
//ntries: number of files per node to be generated. Use negative number not to change the current number
//nevents: number of events to generate per file. Use negative number not to change the current number
//Returns number of successfully generated files

   if (fRunType==kRunGenerateFileBench){
      if (fNFilesGeneratedBench){
         Info("GenerateFiles", "Files already generated, returning");
         return 0;
      }

      if (fDataSetGeneratedBench) delete fDataSetGeneratedBench;
      fDataSetGeneratedBench = new TDSet("TTree","EventTree");
   }
   else if (fRunType==kRunGenerateFileCleanup){
      if (fFilesGeneratedCleanup && !fRegenerate){
         return 0;  
      }
      if (fDataSetGeneratedCleanup) delete fDataSetGeneratedCleanup;
      fDataSetGeneratedCleanup = new TDSet("TTree","EventTree");
   }
   
   Int_t nactive_sav;

   //Generate files on all nodes, active or inactive
   nactive_sav=fProof->GetParallel();
   fProof->SetParallel(99999);

   SetInputParameters();

   TList* wl=fProof->GetListOfSlaveInfos();
   wl->SetName("PROOF_SlaveInfos");
   fProof->AddInputData(wl, kTRUE);

   if (fRunType==kRunGenerateFileBench){
      switch (fBenchmarkMode){
      case kModeStaticNode:
      Info("GenerateFiles", "Generating %d file(s)/node with %lld event(s)/file at %s on the cluster. "
           "It may take a while...",
        fNFilesANode, fNEvents, fBaseDir==""?"default directory":fBaseDir.Data());
      break;
      case kModeStaticWorkersNode:
      Info("GenerateFiles", "Generating %d file(s)/worker with %lld event(s)/file at %s on the cluster. "
           "It may take a while...",
        fNFilesAWorker, fNEvents, fBaseDir==""?"default directory":fBaseDir.Data());
      break;
      case kModeStaticCluster:
      Info("GenerateFiles", "Generating %d file(s) with %lld event(s)/file at %s on the cluster. "
           "It may take a while...",
        fNFilesAWorker, fNEvents, fBaseDir==""?"default directory":fBaseDir.Data());
      break;
      case kModeDynamicWorkers:
      Info("GenerateFiles", "Generating %d file(s)/worker with %lld event(s)/file at %s on the cluster. "
           "It may take a while...",
        fNFilesAWorker, fNEvents, fBaseDir==""?"default directory":fBaseDir.Data());
      break;
      default:
      break;
      }
   }
   else if (fRunType==kRunGenerateFileCleanup){
      Info("GenerateFiles", "Generating file(s) at each node for cleaning up memory. "
           "It may take a while...");
   }

   fProof->Process("TSelEventGen", Long64_t(0));


   TList* l = fProof->GetOutputList();

   if (!l){
      Error("GenerateFiles", "list of output not found");
      return 0;
   } 


   Info("GenerateFiles", "Printing out list of outputs");
   l->Print("a");
   Info("GenerateFiles", "Printing out list of outputs; done");

   //build up dataset out of outputs of workers
   TObject* obj;
   TList* lfilesgenerated;
   TList* ltdelement;
   TDSet* tdset;
   TString outputname, hostname, filename, newfilename;
   TDSetElement* tdelement;
 
   TIter nxt(l);

   while((obj=nxt())){
      outputname=obj->GetName(); 
      if (outputname.Contains("PROOF_FilesGenerated")){
         lfilesgenerated=dynamic_cast<TList*>(obj);
         if (lfilesgenerated){
            TObjArray* token=outputname.Tokenize("_"); //filename=PROOF_FilesGenerated_hostname_ordinal
            hostname=((*token)[2])->GetName();

            tdset=dynamic_cast<TDSet*>(lfilesgenerated->At(0)); //lfilesgenerated is 1-element list
            ltdelement=tdset->GetListOfElements();
            TIter nxtelement(ltdelement); 
            while((tdelement=(TDSetElement*)nxtelement())){
               filename=tdelement->GetName();
               //printf("filename=%s\n", filename.Data());
               newfilename="root://"+hostname+"/"+filename;
               tdelement->SetName(newfilename.Data());
            }
            //tdset->Print("a");
            if (fRunType==kRunGenerateFileBench){
               fDataSetGeneratedBench->Add(tdset);  
            }
            else if (fRunType==kRunGenerateFileCleanup){
               fDataSetGeneratedCleanup->Add(tdset);  
            }
         }
         else{
            Error("GenerateFiles", "%s not type TList*", outputname.Data());
            return 0; 
         }
      }
   }

   if (fRunType==kRunGenerateFileBench){
      //fDataSetGenerated->Validate();
      fNFilesGeneratedBench=fNTries;
      fNEventsGenerated=fNEvents;
      fDataSetGeneratedBench->Print("a"); 
   }
   else if (fRunType==kRunGenerateFileCleanup){
      fFilesGeneratedCleanup=kTRUE;
      fDataSetGeneratedCleanup->Print("a"); 
   }

   //Put it back to old configuration
   fProof->SetParallel(nactive_sav);

   return fNFilesGeneratedBench;
}

void TProofBench::BuildNodesInfo(){

   if (fNodeInfo) return;

   if (!fProof){
      Error("BuildNodesInfo", "Proof not set");
      return;
   }

   TList* l = fProof->GetListOfSlaveInfos();
   if (l) {
      //build node info
      fNodeInfo=new TMap;
      TIter nxw(l);
      TSlaveInfo *si = 0;
      TString nodename, ordinal;

      while ((si=(TSlaveInfo*)nxw())){
         //obj->Print(); 
         nodename=si->GetName();
         ordinal=si->GetOrdinal();
         //printf("nodename=%s\n", nodename.Data());
         //printf("ordinal=%s\n", ordinal.Data());

         //See if proof is running on localhost
         if (nodename.Contains("localhost")){
            //hostname="localhost.localdomain";
            nodename=gSystem->HostName();
         }

         if (TPair* pair=(TPair*)fNodeInfo->FindObject(nodename)){
            TObjString* value=(TObjString*)pair->Value();
            TString svalue(value->String());
            Int_t nworkers=svalue.Atoi();
            char snw[10];
            sprintf(snw, "%d", nworkers+1);
            value->SetString(snw);
         }
         else{
            TObjString* key=new TObjString(nodename);
            TObjString* value=new TObjString("1");
            fNodeInfo->Add(key, value);
         }
      }
      fNodeInfo->Print("A");
   }
   else {
      Error("MakeDataSet", "No list of workers received!");
      return;
   }
}

void TProofBench::MakeDataSet()
{
// Build a TDSet object out of the dataset of all generated files (fDataSetGeneratedBench)
// The built dataset will be used for the IO-bound test
// the files generated with GenerateFiles function.
// ntry: file number
// Typical data file looks like this: 
// EventTree_proof0.kisti.re.kr_0_1.root

   if (!fProof){
      Error("MakeDataSet", "Proof not set");
      return;
   }

   if (fDataSet){
      delete fDataSet;
      fDataSet=0;
   }

   fDataSet = new TDSet("TTree","EventTree");

   TList nodelist;

   TIterator* nxn=fNodeInfo->MakeIterator();
   TObjString *nk = 0;
   TString nodename, ordinal;

   while (nk=(TObjString*)nxn->Next()){
      nodename=nk->String();
      TString snw=((TObjString*)fNodeInfo->GetValue(nk))->String();
      Int_t nw=snw.Atoi();
      Info("MakeDataSet", "# workers on %s is %d", nodename.Data(), nw);

      Int_t nfilesnode=0;
      if (fBenchmarkMode==kModeStaticNode){ //fixed number of files per node during running
         nfilesnode=fNFilesANode;
      }
      else if (fBenchmarkMode==kModeStaticWorkersNode){ //fixed number of files per node during running
         nfilesnode=fNFilesAWorker*nw;
      }

      Int_t nfilesadded=0;
      Int_t nfile;
      TList* lelement=fDataSetGeneratedBench->GetListOfElements();
      TIter nxtelement(lelement);
      TDSetElement *tdelement;
      TFileInfo* fileinfo;
      TUrl* url;
      TString hostname, filename, tmpstring;

      while (nfilesadded<=nfilesnode && (tdelement=(TDSetElement*)nxtelement())){

         fileinfo=tdelement->GetFileInfo();
         url=fileinfo->GetCurrentUrl();
         hostname=url->GetHost();
         filename=url->GetFile();

         if (hostname!=nodename) continue;

         //filename=...hostname_nfile_serial.root
         //remove upto name_stem and leading "_"
         tmpstring=filename;
         const TString stem="_Benchmark_";
         tmpstring.Remove(0, filename.Index(stem)+stem.Length());

         TObjArray* token=tmpstring.Tokenize("_");

         if (token){
            nfile=TString((*token)[0]->GetName()).Atoi();
            token=TString((*token)[1]->GetName()).Tokenize(".");
            Int_t serial=TString((*token)[0]->GetName()).Atoi();

            //ok found, add it to the dataset
            //count only once for set of split files
            if (serial==0){
               if (nfilesadded>=nfilesnode){
                  break;
               }
               nfilesadded++;
            }
            fDataSet->Add(fileinfo);
         }
         else{
            Error("BuildNodesInfo", "File name not recognized: %s", tmpstring.Data());
            return;
         }
      }
   }

   if (fDataSet){
      fDataSet->Lookup();
      fDataSet->Validate();
   }
   else{
      Error("BuildNodesInfo", "Dataset empty");
      return;
   }
   return;
}

/* void TProofBench::DrawPerfProfiles(TString filename, Int_t draw) {
// Plots total processing time as a function of number of slaves
// using each of the 3 selectors.
// filename: input file name with performance statistics

   fDraw=draw>=0?draw:fDraw;

   TDirectory* dirsav=gDirectory;

   fDirProofBench->cd();

   //TFile* fFilelocal;

   filename.Remove(TString::kBoth, ' '); //remove leading and trailing white space(s)
   filename.Remove(TString::kBoth, '\t');//remove leading and trailing tab character(s)

   //FileOpen(filename, "READ");
   //fFilelocal->cd();

   TString perfstats_name = "PROOF_PerfStats";

   Int_t ns_holder;
   Int_t run_holder;
   Int_t nevents_holder;
   Int_t bytes_holder;
   Float_t time_holder;

   Int_t procmax_slaves = 0;

   TTree* tt_cpu    = BuildTimingTree(perfstats_name+"_CPUTest",
                                      procmax_slaves);
   //tt_proc->SetMarkerStyle(4);
   //set branch addresses
   tt_cpu->GetBranch("perfproctime")->GetLeaf("nslaves")->SetAddress(&ns_holder);
   tt_cpu->GetBranch("perfproctime")->GetLeaf("run")->SetAddress(&run_holder);
   tt_cpu->GetBranch("perfproctime")->GetLeaf("nevents")->SetAddress(&nevents_holder);
   tt_cpu->GetBranch("perfproctime")->GetLeaf("bytes")->SetAddress(&bytes_holder);
   tt_cpu->GetBranch("perfproctime")->GetLeaf("time")->SetAddress(&time_holder);

   tt_cpu->Print("a");

   TTree* tt_proc    = BuildTimingTree(perfstats_name+"_FullDataRead",
                                       procmax_slaves);
   //tt_proc->SetMarkerStyle(4);
   //set branch addresses
   tt_proc->GetBranch("perfproctime")->GetLeaf("nslaves")->SetAddress(&ns_holder);
   tt_proc->GetBranch("perfproctime")->GetLeaf("run")->SetAddress(&run_holder);
   tt_proc->GetBranch("perfproctime")->GetLeaf("nevents")->SetAddress(&nevents_holder);
   tt_proc->GetBranch("perfproctime")->GetLeaf("bytes")->SetAddress(&bytes_holder);
   tt_proc->GetBranch("perfproctime")->GetLeaf("time")->SetAddress(&time_holder);

   tt_proc->Print("a");

   Int_t procoptmax_slaves = 0;
   TTree* tt_procopt = BuildTimingTree(perfstats_name+"_OptDataRead",
                                       procoptmax_slaves);
   //tt_procopt->SetMarkerStyle(5);
   //set branch addresses
   tt_procopt->GetBranch("perfproctime")->GetLeaf("nslaves")->SetAddress(&ns_holder);
   tt_procopt->GetBranch("perfproctime")->GetLeaf("run")->SetAddress(&run_holder);
   tt_procopt->GetBranch("perfproctime")->GetLeaf("nevents")->SetAddress(&nevents_holder);
   tt_procopt->GetBranch("perfproctime")->GetLeaf("bytes")->SetAddress(&bytes_holder);
   tt_procopt->GetBranch("perfproctime")->GetLeaf("time")->SetAddress(&time_holder);

   tt_procopt->Print("a");

   Int_t nslaves = procmax_slaves>procoptmax_slaves?procmax_slaves:procoptmax_slaves;
   //if (nslaves<noprocmax_slaves) nslaves=noprocmax_slaves;
   
   TProfile* cpuprof_event = new TProfile("cpuprof_event", "CPU Test Event Rate", nslaves, 0.5, nslaves+0.5);

   Double_t event_rate, IO_rate;
   Int_t entries=tt_cpu->GetEntries();
   for (int i=0; i<entries; i++){
      tt_cpu->GetEntry(i); 
      //printf("ns_holder=%d, run_holder=%d time_holder=%f\n", ns_holder, run_holder, time_holder);
      event_rate=nevents_holder/time_holder; 
      cpuprof_event->Fill(Double_t(ns_holder), event_rate);
   }
   
   cpuprof_event->GetXaxis()->SetTitle("Number of Slaves");
   cpuprof_event->GetYaxis()->SetTitle("Events/sec");

   cpuprof_event->SetMarkerStyle(20);

   entries=tt_proc->GetEntries();
   for (int i=0; i<entries; i++){
      tt_proc->GetEntry(i); 
      //printf("ns_holder=%d, run_holder=%d time_holder=%f\n", ns_holder, run_holder, time_holder);
      event_rate=nevents_holder/time_holder; 
      IO_rate=bytes_holder/time_holder/(1024.*1024.); 
      procprof_event->Fill(Double_t(ns_holder), event_rate);
      procprof_IO->Fill(Double_t(ns_holder), IO_rate);
   }
   
   Double_t ymax=procprof_event->GetMaximum();
   procprof_event->GetYaxis()->SetRangeUser(0, ymax*1.2);

   procprof_event->GetXaxis()->SetTitle("Number of Slaves");
   procprof_event->GetYaxis()->SetTitle("Events/sec");
   procprof_IO->GetXaxis()->SetTitle("Number of Slaves");
   procprof_IO->GetYaxis()->SetTitle("MB/sec");

   procprof_event->SetMarkerStyle(21);
   procprof_IO->SetMarkerStyle(22);

   TProfile* procoptprof_event = new TProfile("procoptprof_event", "Event Rate", nslaves, 0.5, nslaves+0.5);
   TProfile* procoptprof_IO = new TProfile("procoptprof_IO", "I/O Rate", nslaves, 0.5, nslaves+0.5);

   procoptprof_event->GetXaxis()->SetTitle("Number of Slaves");
   procoptprof_event->GetYaxis()->SetTitle("Events/sec");
   procoptprof_IO->GetXaxis()->SetTitle("Number of Slaves");
   procoptprof_IO->GetYaxis()->SetTitle("MB/sec");

   procoptprof_event->SetMarkerStyle(23);
   procoptprof_IO->SetMarkerStyle(24);

   entries=tt_procopt->GetEntries();
   for (int i=0; i<entries; i++){
      tt_procopt->GetEntry(i); 
      event_rate=nevents_holder/time_holder; 
      IO_rate=bytes_holder/time_holder/(1024.*1024.); 
      procoptprof_event->Fill(Double_t(ns_holder), event_rate);
      procoptprof_IO->Fill(Double_t(ns_holder), IO_rate);
   }

   if (fFile && !fFile->IsZombie() && !TString(fFile->GetOption()).Contains("READ")){
       fDirProofBench->cd();
       cpuprof_event->Write();
       procprof_event->Write();
       procprof_IO->Write();
       procoptprof_event->Write();
       procoptprof_IO->Write();
       if (gROOT->IsBatch() || !fDraw){
          delete cpuprof_event;
          delete procprof_event;
          delete procprof_IO;
          delete procoptprof_event;
          delete procoptprof_IO;
       }
   }

   if (!gROOT->IsBatch() && fDraw){
      //save current pad
      //TVirtualPad* pad_sav=gPad;

      //if (!fCPerfProfiles){
      //   fCPerfProfiles=new TCanvas("Performance_Profiles", "Performance Profiles");
      //}
      //else{
      //   fCPerfProfiles->cd();
      //}

      fCPerfProfiles=dynamic_cast<TCanvas*>(gROOT->FindObject("Performance_Profiles"));

      if (!fCPerfProfiles){
         fCPerfProfiles = new TCanvas("Performance_Profiles","Performance Profiles",800,600);
         fCPerfProfiles->SetBorderMode(0);
      }

      fCPerfProfiles->Clear();
      fCPerfProfiles->Divide(2, 2);
     

      TStyle* style_sav=gStyle;

      gROOT->SetStyle("Plain");
      gStyle->SetOptStat(0);
      gStyle->SetNdivisions(505);
      gStyle->SetTitleFontSize(0.07);

      fCPerfProfiles->cd(1);
      cpuprof_event->Draw();
      gPad->Update();

      fCPerfProfiles->cd(2);
      procoptprof_event->GetYaxis()->SetRangeUser(0, procoptprof_event->GetMaximum()*1.2);
      procoptprof_event->Draw();
      procprof_event->Draw("same");
      gPad->Update();

      fCPerfProfiles->cd(3);
      procoptprof_IO->GetYaxis()->SetRangeUser(0, procoptprof_IO->GetMaximum()*1.2);
      procoptprof_IO->Draw();
      procprof_IO->Draw("same");
      gPad->Update();

      //legend for pad 2
      fCPerfProfiles->cd(2);

      Float_t legxoffset = 0.1;
      Float_t legwidth = 0.2;
      Float_t legyoffset = 0.02;
      Float_t legheight = 0.15;
   
      Float_t lm = gPad->GetLeftMargin();
      Float_t rm = gPad->GetRightMargin();
      Float_t tm = gPad->GetTopMargin();
      Float_t bm = gPad->GetBottomMargin();
   
      TLegend* leg_event = new TLegend(lm+legxoffset*(1.0-lm-rm),
                                 1.0-tm-(legyoffset+legheight)*(1.0-tm-bm),
                                 lm+(legxoffset+legwidth)*(1.0-lm-rm),
                                 1.0-tm-legyoffset*(1.0-tm-bm));
      leg_event->SetBorderSize(1); 
      leg_event->SetFillColor(0);
      leg_event->AddEntry(procoptprof_event,"Partial Event","p");
      leg_event->AddEntry(procprof_event,"Full Event","p");

      leg_event->Draw();
      gPad->Update();

      //legends for pad 3
      fCPerfProfiles->cd(3);

      lm = gPad->GetLeftMargin();
      rm = gPad->GetRightMargin();
      tm = gPad->GetTopMargin();
      bm = gPad->GetBottomMargin();
   
      TLegend* leg_IO = new TLegend(lm+legxoffset*(1.0-lm-rm),
                                 1.0-tm-(legyoffset+legheight)*(1.0-tm-bm),
                                 lm+(legxoffset+legwidth)*(1.0-lm-rm),
                                 1.0-tm-legyoffset*(1.0-tm-bm));
      leg_IO->SetBorderSize(1); 
      leg_IO->SetFillColor(0);
      leg_IO->AddEntry(procoptprof_IO,"Partial Event","p");
      leg_IO->AddEntry(procprof_IO,"Full Event","p");

      leg_IO->Draw();
      gPad->Update();
   
      fCPerfProfiles->Update();

//      TPaveText* titlepave = dynamic_cast<TPaveText*>(gPad->GetListOfPrimitives()->FindObject("title"));
//
//      if (titlepave) {
//         Double_t x1ndc = titlepave->GetX1NDC();
//         Double_t x2ndc = titlepave->GetX2NDC();
//         titlepave->SetX1NDC((1.0-x2ndc+x1ndc)/2.);
//         titlepave->SetX2NDC((1.0+x2ndc-x1ndc)/2.);
//         titlepave->SetBorderSize(0);
//
//         gPad->Update();
//      }
//      gPad->Modified();
//      fCPerfProfiles->Update();
//      //pad_sav->cd();

      gStyle=style_sav;
   }
   dirsav->cd();
}*/

TTree* TProofBench::BuildPerfProfiles(ERunType runtype) {

   if (!(runtype==kRunNotSpecified
      || runtype==kRunCPUTest
      || runtype==kRunFullDataRead
      || runtype==kRunOptDataRead
      || runtype==kRunNoDataRead
      || runtype==kRunAll)){
      Error("BuildPerfProfiles", "Invalid run request");
      return 0; 
   }

   if (runtype==kRunAll){
      BuildPerfProfiles(kRunCPUTest);
      BuildPerfProfiles(kRunFullDataRead);
      BuildPerfProfiles(kRunOptDataRead);
   }

   //assume fRunType if runtype is not specified
   if (runtype==kRunNotSpecified){
      runtype=fRunType;
   }

   TString pattern;
   switch (runtype){
   case kRunCPUTest:
      pattern="CPUTest"; 
      break;
   case kRunFullDataRead:
      pattern="FullDataRead"; 
      break;
   case kRunOptDataRead:
      pattern="OptDataRead"; 
      break;
   case kRunNoDataRead:
      pattern="NoDataRead"; 
      break;
   default:
      break; //we should never get here
   }

   fDirProofBench->cd();

   TTree* timing_tree = new TTree("Timing Tree", "Timing Tree");
   timing_tree->SetDirectory(0);

   Int_t ns_holder;
   Int_t run_holder;
   Int_t nevents_holder;
   Int_t bytes_holder;
   Float_t time_holder;

   TBranch* br = timing_tree->Branch("perfproctime", &ns_holder,
                                     "nslaves/I:run/I:nevents/I:bytes/I:time/F");
   br->GetLeaf("nslaves")->SetAddress(&ns_holder);
   br->GetLeaf("run")->SetAddress(&run_holder);
   br->GetLeaf("nevents")->SetAddress(&nevents_holder);
   br->GetLeaf("bytes")->SetAddress(&bytes_holder);
   br->GetLeaf("time")->SetAddress(&time_holder);

   // extract timing info
/*   TIter NextKey(fDirProofBench->GetListOfKeys());
   TKey* key = 0;
   while ((key = dynamic_cast<TKey*>(NextKey()))) {
       printf("key in the list: %s\n", key->GetName());
      if(!TString(key->GetName()).Contains(TRegexp(pattern)))
         continue;

      TObject* obj = key->ReadObj();
*/

   Int_t isfile=0;
   TList* list=0;
   TString sdir(fDirProofBench->GetPath()); 
   if (sdir.Contains("Rint:")){//memory
      list=fDirProofBench->GetList(); //list of object
      isfile=0;//memory
   }
   else{//file
      if (fDebug){ //it is written to the disk
         list=fDirProofBench->GetListOfKeys();
         isfile=1;//file
      }
      else{
         list=fDirProofBench->GetList(); //list of object
         isfile=0;//memory
      }
   }

   TIter nxt(list);
   TObject* obj; 
   TKey* key;

   Int_t max_slaves=0;

   while ((obj=nxt())){
       if (TString(obj->GetName()).Contains(TRegexp(pattern))){
          printf("key in the list: %s\n", obj->GetName());
       }

      if (!TString(obj->GetName()).Contains(TRegexp(pattern))) continue;

      if (isfile){
         key=dynamic_cast<TKey*>(obj);
         obj = key->ReadObj();
      }

      TTree* t = dynamic_cast<TTree*>(obj);
      if (!t) {
         if (isfile) delete obj;
         continue;
      }

      //parse name to get number of slaves and run
      UInt_t Index = 0;
      const Char_t *name = t->GetName();
      while (Index<strlen(name)) {
        if ( name[Index]>='0' && name[Index]<='9')
        break;
        Index++;
      }

      if (Index == strlen(name)) {
         if (isfile) delete t;
         continue;
      } else {
         // this should be the number of slaves
         ns_holder = atoi(name+Index);
      }

      // get past number of slaves
      while (Index<strlen(name)) {
        if ( name[Index]<'0' || name[Index]>'9')
        break;
        Index++;
      }

      if (Index == strlen(name)) {
         if (isfile) delete t;
         continue;
      }

      while (Index<strlen(name)) {
        if ( name[Index]>='0' && name[Index]<='9')
        break;
        Index++;
      }

      if (Index == strlen(name)) {
         if (isfile) delete t;
         continue;
      } else {
         // this should be the run number
         run_holder = atoi(name+Index);
      }

      if(!t->FindBranch("PerfEvents")) {
         if (isfile) delete t;
         continue;
      }

      //printf("name=%s\n", name);
      // extract timing information
      TPerfEvent pe;
      TPerfEvent* pep = &pe;
      t->SetBranchAddress("PerfEvents",&pep);
      Long64_t entries = t->GetEntries();
      Double_t start(0), end(0);
      //Bool_t started=kFALSE;
      
      Long64_t nevents_kPacket=0;
      Long64_t nevents_kRate=0;
      Long64_t bytesread_kPacket=0;
      Long64_t bytesread_kRate=0;

      for (Long64_t k=0; k<entries; k++) {
         t->GetEntry(k);

         printf("k:%lld fTimeStamp=%lf fEvtNode=%s pe.fType=%d fSlaveName=%s fNodeName=%s fFileName=%s fFileClass=%s fSlave=%s fEventsProcessed=%lld fBytesRead=%lld fLen=%lld fLatency=%lf fProcTime=%lf fCpuTime=%lf fIsStart=%d fIsOk=%d\n",k, pe.fTimeStamp.GetSec() + 1e-9*pe.fTimeStamp.GetNanoSec(), pe.fEvtNode.Data(), pe.fType, pe.fSlaveName.Data(), pe.fNodeName.Data(), pe.fFileName.Data(), pe.fFileClass.Data(), pe.fSlave.Data(), pe.fEventsProcessed, pe.fBytesRead, pe.fLen, pe.fLatency, pe.fProcTime, pe.fCpuTime, pe.fIsStart, pe.fIsOk);

         if (pe.fType==TVirtualPerfStats::kPacket){
            nevents_kPacket+=pe.fEventsProcessed;
            bytesread_kPacket+=pe.fBytesRead;
         }
         if (pe.fType==TVirtualPerfStats::kRate){
            //printf("adding pe.fEventsProcessed=%lld\n", pe.fEventsProcessed);
            nevents_kRate+=pe.fEventsProcessed;
            bytesread_kRate+=pe.fBytesRead;
         }
        
         /*if (!started) {
            if (pe.fType==TVirtualPerfStats::kPacket) {
               start = pe.fTimeStamp.GetSec()
                       + 1e-9*pe.fTimeStamp.GetNanoSec()
                       - pe.fProcTime;
               started=kTRUE;
            }
         } else {
            if (pe.fType==TVirtualPerfStats::kPacket) {
               end = pe.fTimeStamp.GetSec()
                     + 1e-9*pe.fTimeStamp.GetNanoSec();
            }
         }*/
         //skip information from workers
         if (pe.fEvtNode.Contains(".")) continue;
         if (pe.fType==TVirtualPerfStats::kStart) start= pe.fTimeStamp.GetSec()+1e-9*pe.fTimeStamp.GetNanoSec();
         if (pe.fType==TVirtualPerfStats::kStop) end= pe.fTimeStamp.GetSec()+1e-9*pe.fTimeStamp.GetNanoSec();
      }
     
      //printf("nevents_kPacket=%lld, nevents_kRate=%lld\n", nevents_kPacket, nevents_kRate);
      //printf("bytesread_kPacket=%lld, bytesread_kRate=%lld\n", bytesread_kPacket, bytesread_kRate);

      //if (nevents_kPacket!=fNEvents){
       //  Error("BuildTimingTree", "Number of events processed is different from the number of events in the file");
        // return 0;
      //}

      nevents_holder=nevents_kPacket;
      bytes_holder=bytesread_kPacket;
      time_holder = end-start;

      timing_tree->Fill();

      if (max_slaves<ns_holder) max_slaves=ns_holder;

      if (isfile) delete t;
//printf("printing gDirectory\n");
//      gDirectory->ls();
//printf("printing gDirectory, done\n");
   }

   Double_t event_rate, IO_rate;
   Int_t entries=timing_tree->GetEntries();

   if (runtype & kRunCPUTest){
      if (fProfCPUTestEvent) delete fProfCPUTestEvent;
      fProfCPUTestEvent = new TProfile("hProfCPUTestEvent", "CPU Test (Event Rate)", max_slaves, 0.5, max_slaves+0.5);
      for (int i=0; i<entries; i++){
         timing_tree->GetEntry(i); 
         //printf("ns_holder=%d, run_holder=%d time_holder=%f\n", ns_holder, run_holder, time_holder);
         event_rate=nevents_holder/time_holder; 
         fProfCPUTestEvent->Fill(Double_t(ns_holder), event_rate);
      }
      fProfCPUTestEvent->GetXaxis()->SetTitle("Number of Slaves");
      fProfCPUTestEvent->GetYaxis()->SetTitle("Events/sec");
      fProfCPUTestEvent->SetMarkerStyle(20);
   }

   if(runtype & kRunOptDataRead){
      if (fProfOptDataReadEvent) delete fProfOptDataReadEvent;
      if (fProfOptDataReadIO) delete fProfOptDataReadIO;
      fProfOptDataReadEvent= new TProfile("hProfOptDataReadEvent", "Opt Data Read (Event Rate)", max_slaves, 0.5, max_slaves+0.5);
      fProfOptDataReadIO= new TProfile("hProfOptDataReadIO", "Opt Data Read (I/O Rate)", max_slaves, 0.5, max_slaves+0.5);
      for (int i=0; i<entries; i++){
         timing_tree->GetEntry(i); 
         //printf("ns_holder=%d, run_holder=%d time_holder=%f\n", ns_holder, run_holder, time_holder);
         event_rate=nevents_holder/time_holder; 
         IO_rate=bytes_holder/time_holder/(1024.*1024.); 
         fProfOptDataReadEvent->Fill(Double_t(ns_holder), event_rate);
         fProfOptDataReadIO->Fill(Double_t(ns_holder), IO_rate);
      }
      fProfOptDataReadEvent->GetXaxis()->SetTitle("Number of Slaves");
      fProfOptDataReadEvent->GetYaxis()->SetTitle("Events/sec");
      fProfOptDataReadIO->GetXaxis()->SetTitle("Number of Slaves");
      fProfOptDataReadIO->GetYaxis()->SetTitle("MB/sec");

      fProfOptDataReadEvent->SetMarkerStyle(21);
      fProfOptDataReadIO->SetMarkerStyle(22);

   }

   if (runtype & kRunFullDataRead){
      if (fProfFullDataReadEvent) delete fProfFullDataReadEvent;
      if (fProfFullDataReadIO) delete fProfFullDataReadIO;
      fProfFullDataReadEvent= new TProfile("hProfFullDataReadEvent", "Full Data Read (Event Rate)", max_slaves, 0.5, max_slaves+0.5);
      fProfFullDataReadIO= new TProfile("hProfFullDataReadIO", "Full Data Read (I/O Rate)", max_slaves, 0.5, max_slaves+0.5);
      for (int i=0; i<entries; i++){
         timing_tree->GetEntry(i); 
         //printf("ns_holder=%d, run_holder=%d time_holder=%f\n", ns_holder, run_holder, time_holder);
         event_rate=nevents_holder/time_holder; 
         IO_rate=bytes_holder/time_holder/(1024.*1024.); 
         fProfFullDataReadEvent->Fill(Double_t(ns_holder), event_rate);
         fProfFullDataReadIO->Fill(Double_t(ns_holder), IO_rate);
      }
      fProfFullDataReadEvent->GetXaxis()->SetTitle("Number of Slaves");
      fProfFullDataReadEvent->GetYaxis()->SetTitle("Events/sec");
      fProfFullDataReadIO->GetXaxis()->SetTitle("Number of Slaves");
      fProfFullDataReadIO->GetYaxis()->SetTitle("MB/sec");

      fProfFullDataReadEvent->SetMarkerStyle(23);
      fProfFullDataReadIO->SetMarkerStyle(24);
   }

   return timing_tree;
}

void TProofBench::Print(Option_t* option)const{

   if (fFile){
       fFile->Print(option);
       fFile->ls(option);
   }
   else{
      printf("no file open\n"); 
   }

   if (fProof) fProof->Print(option);
   printf("fBaseDir=%s\n", fBaseDir.Data()); 
   printf("fRunType=%s%s\n", "k",fNameStem.Data());
   printf("fBenchmarkMode=%d\n", fBenchmarkMode);

   TString sbenchhisttype;
   switch (fHistType) {
   case kHistNotSpecified:
      sbenchhisttype="kHistNotSpecified";
      break;
   case kHist1D:
      sbenchhisttype="kHist1D";
      break;
   case kHist2D:
      sbenchhisttype="kHist2D";
      break;
   case kHist3D:
      sbenchhisttype="kHist3D";
      break;
   case kHistAll:
      sbenchhisttype="kHistAll";
      break;
   default:
      break;
   }

   printf("fNHists=%ld\n", fNHists);
   printf("fHistType=%s\n", sbenchhisttype.Data());
   printf("fMaxNWorkers=%d\n", fMaxNWorkers);
   printf("fNNodes=%d\n", fNNodes);
   printf("fNTries=%d\n", fNTries);
   printf("fNFilesAWorker=%d\n", fNFilesAWorker);
   printf("fNFilesANode=%d\n", fNFilesANode);
   printf("fNEvents=%lld\n", fNEvents);
   printf("fStepSize=%d\n", fStepSize);
   printf("fStart=%d\n", fStart);
   printf("fNTracksBench=%d\n", fNTracksBench);
   printf("fNTracksCleanup=%d\n", fNTracksCleanup);
   printf("fDraw=%d\n", fDraw);
   printf("fDebug=%d\n", fDebug);
   printf("fNEventsGenerated=%lld\n", fNEventsGenerated);
   printf("fNFilesGeneratedBench=%d\n", fNFilesGeneratedBench);
   printf("fFilesGenenratedCleanup=%d\n", fFilesGeneratedCleanup);

   if (fDataSet) fDataSet->Print(option);
   if (fDataSetGeneratedBench) fDataSetGeneratedBench->Print(option);
   if (fDataSetGeneratedCleanup) fDataSetGeneratedCleanup->Print(option);
   if (fCPerfProfiles){
      printf("Performance Profiles Canvas: Name=%s Title=%s\n", 
              fCPerfProfiles->GetName(), fCPerfProfiles->GetTitle());
   }
   printf("fDirProofBench=%s\n", fDirProofBench->GetPath());
}

TFile* TProofBench::FileOpen(const char* filename, 
                             Option_t* option, 
                             Bool_t force, 
                             const char* ftitle, 
                             Int_t compress)
{
//open file
//Current open file, if any, will be written and closed when force is true, 
//option is option to TFile::Open(...) function

   TString sfilename(filename);
   sfilename.Remove(TString::kBoth, ' '); //remove leading and trailing white space(s)
   sfilename.Remove(TString::kBoth, '\t');//remove leading and trailing tab character(s)

   //if (sfilename.Length()<1){
   //   return fFile;
   //}
   
   TFile* ftmp=0;

   TString soption(option);
   soption.ToLower();

   if (fFile && force){
      //see if the specified file is already open
      FileStat_t a, b;
      if (fFile && !gSystem->GetPathInfo(fFile->GetName(), a) && !gSystem->GetPathInfo(sfilename, b) && a.fDev==b.fDev && a.fIno==b.fIno){
         //file already open, check open mode
         if (soption.Contains("new") || (soption.Contains("create") && !soption.Contains("recreate"))){
            //trying to open new file which already exists
            Error("FileOpen", "File exists: %s, doing nothing", fFile->GetName());
         }
         else{//reopen a file with same name 
            fFile->Close();
            fFile=new TFile(sfilename, option, ftitle, compress);
            if (fFile->IsZombie()){//open failed
               Error("FileOpen", "Cannot open file: %s", sfilename.Data());
               fFile->Close();
               fFile=0;
            }
            if (!soption.Contains("read")){
               fFile->mkdir("ProofBench");
               fFile->cd("ProofBench");
               fDirProofBench=gDirectory;
            }
            else{
               gDirectory->cd("Rint:/");
               gDirectory->mkdir("ProofBench");
               gDirectory->cd("ProofBench");
               fDirProofBench=gDirectory;
            }
         }
         return fFile;
      }

      ftmp=new TFile(sfilename, option, ftitle, compress);
      if (ftmp->IsZombie()){//open failed
         Error("FileOpen", "Cannot open file: %s", sfilename.Data());
         ftmp->Close();
               gDirectory->cd("Rint:/");
               gDirectory->mkdir("ProofBench");
               gDirectory->cd("ProofBench");
               fDirProofBench=gDirectory;
      }
      else{//open succeeded
         fFile->Close();        
         fFile=ftmp;

            if (!soption.Contains("read")){
               fFile->mkdir("ProofBench");
               fFile->cd("ProofBench");
               fDirProofBench=gDirectory;
            }
            else{
               gDirectory->cd("Rint:/");
               gDirectory->mkdir("ProofBench");
               gDirectory->cd("ProofBench");
               fDirProofBench=gDirectory;
            }
      }
      return fFile;
   }
   else if (fFile && !force){
        Info("FileOpen", "File (%s) already open; doing nothing. To force use kTRUE", fFile->GetName());
        return fFile;
   }
   else{ //!fFile && (force || !force)
      if (sfilename.Length()<1){
         gDirectory->cd("Rint:/");
         gDirectory->mkdir("ProofBench");
         gDirectory->cd("ProofBench");
         fDirProofBench=gDirectory;
         return 0;
      } 
      ftmp=new TFile(sfilename, option, ftitle, compress);
      if (ftmp->IsZombie()){
         Error("FileOpen", "Cannot open file: %s", sfilename.Data());
         ftmp->Close();
               gDirectory->cd("Rint:/");
               fDirProofBench=gDirectory;
         return fFile;
      }
      else{
         fFile=ftmp;
            if (!soption.Contains("read")){
               fFile->mkdir("ProofBench");
               fFile->cd("ProofBench");
               fDirProofBench=gDirectory;
            }
            else{
               gDirectory->cd("Rint:/");
               gDirectory->mkdir("ProofBench");
               gDirectory->cd("ProofBench");
               fDirProofBench=gDirectory;
            }
         return fFile;
      }
   }
}

TFile* TProofBench::SetFile(TFile* ftmp){
//set file to ftmp
//if file is already open in writable mode, all objects are written before file is set to ftmp 

   if (fFile){
      TString soption=fFile->GetOption();
      soption.ToLower();
      if (soption.Contains("create") || soption.Contains("update")){
         fFile->Write();
      }
      fFile->Close();
      delete fFile;
   }
   fFile=ftmp;
   return fFile;
}

/*
Int_t TProofBench::SetParameters(TString basedir, 
                                      Long_t nhists, 
                                      Int_t maxnworkers, 
                                      Int_t nnodes, 
                                      Int_t ntries, 
                                      Long64_t nevents, 
                                      Int_t stepsize, 
                                      Int_t start)
{

   Int_t returnval=0;

   basedir.Remove(TString::kBoth, ' '); //remove leading and trailing white space(s)
   basedir.Remove(TString::kBoth, '\t');//remove leading and trailing tab character(s)
   basedir.Remove(TString::kTrailing, '/'); //remove trailing /

   //see if files were already generated
   if ((basedir.Length()<=0 || basedir==fBaseDir) //no base directory change
    //number of files requested is smaller than files already generated
    && ((ntries<=0 && fNTriesGenerated>0) || (ntries>0 && ntries<=fNTriesGenerated)) 
    //number of events/file is smaller than that in files already generated
    && ((nevents<=0 && fNEventsGenerated>0) || (nevents>0 && nevents<=fNEventsGenerated))){    
   }
   else{ //file generation is needed)
      returnval=1;
   }

   fBaseDir=basedir.Length()>0?basedir:fBaseDir;
   fNHists=nhists>-1?nhists: fNHists;
   fNNodes=nnodes>-1?nnodes: fNNodes;

   if (fProof){
      TList* lslave=fProof->GetListOfSlaveInfos();
      Int_t nslaves=lslave->GetSize();  //number of slave workers regardless of its status, active or inactive
      fMaxNWorkers=maxnworkers>-1?maxnworkers:2*nslaves; //this number to be parameterized in the future
   }
   else{
      fMaxNWorkers=maxnworkers>-1?maxnworkers:fMaxNWorkers;
   }

   fNTries=ntries>-1?ntries: fNTries;
   fNEvents=nevents>-1?nevents: fNEvents;
   fStepSize=stepsize>-1?stepsize:fStepSize;

   if (start>-1 && start>fMaxNWorkers){
      Info("SetParameters", "starting # workers cannot be greater than max workers;"
           "Setting start # workers to %d", fMaxNWorkers);
      fStepSize=fMaxNWorkers;
   }
   else{
      fStart=start>-1?start:fStart;
   }

   return returnval;
}*/
 
void TProofBench::SetInputParameters(){

   if (fProof){
      fProof->SetParameter("fBaseDir", fBaseDir.Data());
      fProof->SetParameter("fRunType", fRunType);
      fProof->SetParameter("fNHists", fNHists);
      fProof->SetParameter("fHistType", fHistType);
      fProof->SetParameter("fNTries", fNTries);
      fProof->SetParameter("fNEvents", fNEvents);
      fProof->SetParameter("fDraw", Int_t(fDraw));
      fProof->SetParameter("fRegenerate", Int_t(fRegenerate));
      fProof->SetParameter("fBenchmarkMode", fBenchmarkMode);
      fProof->SetParameter("fNFilesAWorker", fNFilesAWorker);
      fProof->SetParameter("fNFilesANode", fNFilesANode);
   }
   else{
      Error("SetInputParameters", "Proof not set, doing noting");
   }
   return;
}
 
void TProofBench::ClearInputParameters(){

   if (fProof){
      fProof->DeleteParameters("fBaseDir");
      fProof->DeleteParameters("fRunType");
      fProof->DeleteParameters("fNHists");
      fProof->DeleteParameters("fHistType");
      fProof->DeleteParameters("fNTries");
      fProof->DeleteParameters("fNEvents");
      fProof->DeleteParameters("fDraw");
      fProof->DeleteParameters("fRegenerate");
      fProof->DeleteParameters("fBenchmarkMode");
      fProof->DeleteParameters("fNFilesAWorker");
      fProof->DeleteParameters("fNFilesANode");
   }
   else{
      Error("ClearInputParameters", "Proof not set, doing noting");
   }
   return;
}

Int_t TProofBench::CheckParameters(TString where){
   //TString fFilename,
   //TString basedir,

   Int_t val=0; //return value
   
   if(!fProof){
      Error(where.Data(), "Proof not set");
      return 1;
   }

   if ( fRunType==kRunCPUTest
     || fRunType==kRunGenerateFileBench 
     || fRunType==kRunGenerateFileCleanup 
     || fRunType==kRunCleanup
     || fRunType==kRunFullDataRead
     || fRunType==kRunOptDataRead
     || fRunType==kRunNoDataRead){
   }
   else{
      Error(where.Data(), "fRunType not set; fRunType=%d",fRunType);
      val=1;
   }

   if (fRunType==kRunCPUTest){
      if (fNHists<=0){
         Error(where.Data(), "fNHists not set; fNHists=%d",fNHists);
         val=1;
      }
      if (!(fHistType & kHistAll)){
         Error(where.Data(), "fHistType not set: fHistType=%d",fHistType);
         val=1;
      }
      if (fMaxNWorkers<=0){
         Error(where.Data(), "fMaxNWorkers not set: fMaxNWorkers=%d",fMaxNWorkers); 
         val=1;
      }
      if (fNTries<=0){
         Error(where.Data(), "fNTries not set: fNTries=%d",fNTries);
         val=1;
      }
      if (fNEvents<=0){
         Error(where.Data(), "fNEvents not set: fNEvents=%d",fNEvents);
         val=1;
      }
      if (fStepSize<=0){
         Error(where.Data(), "fStepSize not set: fStepSize=%d",fStepSize);
         val=1;
      }
      if (fStart<=0){
         Error(where.Data(), "fStart not set: fStart=%d",fStart);
         val=1;
      }
      return val;
   }

   if (fRunType==kRunGenerateFileBench){
      if (fNTracksBench<=0){
         Error(where.Data(), "fNTracksBench not set: fNTracksBench=%d",fNTracksBench);
         val=1;
      }
      if (fMaxNWorkers<=0){
         Error(where.Data(), "fMaxNWorkers not set: fMaxNWorkers=%d",fMaxNWorkers); 
         val=1;
      }
      if (fNTries<=0){
         Error(where.Data(), "fNTries not set: fNTries=%d",fNTries);
         val=1;
      }
      if (fNEvents<=0){
         Error(where.Data(), "fNEvents not set: fNEvents=%d",fNEvents);
         val=1;
      }
      if (fStepSize<=0){
         Error(where.Data(), "fStepSize not set: fStepSize=%d",fStepSize);
         val=1;
      }
      if (fStart<=0){
         Error(where.Data(), "fStart not set: fStart=%d",fStart);
         val=1;
      }
      if (fBenchmarkMode==kModeStaticNode && fNFilesANode<=0){
         Error(where.Data(), "fNFilesANode not set: fNFilesANode=%d",fNFilesANode);
         val=1;
      }
      if (fBenchmarkMode==kModeStaticWorkersNode && fNFilesAWorker<=0){
         Error(where.Data(), "fNFilesAWorker not set: fNFilesAWorker=%d",fNFilesAWorker);
         val=1;
      }
      return val;
   }

   if (fRunType==kRunGenerateFileCleanup){
      if (fNTracksCleanup<=0){
         Error(where.Data(), "fNTracksCleanup not set: fNTracksCleanup=%d",fNTracksCleanup);
         val=1;
      }
      if (fNTries<=0){
         Error(where.Data(), "fNTries not set: fNTries=%d",fNTries);
         val=1;
      }
      if (fNEvents<=0){
         Error(where.Data(), "fNEvents not set: fNEvents=%d",fNEvents);
         val=1;
      }
      if (fStepSize<=0){
         Error(where.Data(), "fStepSize not set: fStepSize=%d",fStepSize);
         val=1;
      }
      if (fStart<=0){
         Error(where.Data(), "fStart not set: fStart=%d",fStart);
         val=1;
      }
      return val;
   }

   if (fRunType==kRunCleanup){
      if (!fFilesGeneratedCleanup){
         Error(where.Data(), "Cleanup files not generated");
         val=1;
      }
      if (fStepSize<=0){
         Error(where.Data(), "fStepSize not set: fStepSize=%d",fStepSize);
         val=1;
      }
      if (fStart<=0){
         Error(where.Data(), "fStart not set: fStart=%d",fStart);
         val=1;
      }
      return val;
   }

   if (fRunType==kRunFullDataRead
    || fRunType==kRunOptDataRead
    || fRunType==kRunNoDataRead){

      if (!fNFilesGeneratedBench){
         Error(where.Data(), "Files not generated");
         val=1;
      }
      if (fNTries<=0){
         Error(where.Data(), "fNTries not set: fNTries=%d",fNTries);
         val=1;
      }
      if (fNEvents<=0){
         Error(where.Data(), "fNEvents not set: fNEvents=%d",fNEvents);
         val=1;
      }
      if (fStepSize<=0){
         Error(where.Data(), "fStepSize not set: fStepSize=%d",fStepSize);
         val=1;
      }
      if (fStart<=0){
         Error(where.Data(), "fStart not set: fStart=%d",fStart);
         val=1;
      }
      return val;
   }

   return 0;
}
