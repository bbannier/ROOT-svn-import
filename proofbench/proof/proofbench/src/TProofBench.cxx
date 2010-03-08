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
// TProofBench is a steering class for PROOF benchmark suite.           //
// The primary goal of benchmark suite is to determine the optimal      //
// configuration parameters for a set of machines to be used as PROOF   //
// cluster. The suite measures the performance of the cluster for a set //
// of standard tasks as a function of the number of effective processes.//
// From these results, indications about the optimal number of          //
// concurrent processes could be derived. For large facilities,         //
// the suite should also give indictions about the optimal number of    //
// of sub-masters into which the cluster should be partitioned.         //
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
#include "TSortedList.h"

#include <stdlib.h>

ClassImp(TProofBench)

TProofBench::TProofBench(TString filename, 
                         Option_t* foption,
                         TProof* proof, 
                         TString basedir, 
                         ERunType runtype, 
                         EBenchmarkMode benchmarkmode,
                         Long_t nhists, 
                         EHistType histtype,
                         Int_t maxnworkers, 
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
fNTries(10),
fNFilesAWorker(1),
fNFilesANode(-1),
fNEvents(10000), 
fStepSize(1),
fStart(1), 
fNTracksBench(3),
fNTracksCleanup(100),
fDraw(0), 
fDebug(0),
fRegenerate(0),
fDataSetGeneratedBench(0), 
fDataSetGeneratedCleanup(0), 
fCPerfProfiles(0), 
fDirProofBench(0),
//fProfCPUTestEvent(0),
//fProfOptDataReadEvent(0),
//fProfOptDataReadIO(0),
//fProfFullDataReadEvent(0),
//fProfFullDataReadIO(0),
fNodes(0)
{
//Default constructor

   if (filename.Length()){
      OpenFile(filename.Data(), foption);
   }
   SetBaseDir(basedir);
   SetRunType("TProofBench", runtype);
   SetNHists(nhists);
   SetHistType(histtype);

   if (maxnworkers>0){ 
      SetMaxNWorkers(maxnworkers);
   }
   else{
      SetMaxNWorkers("1x");
   }

   SetNTries(ntries);
   SetNEvents(nevents);
   SetStepSize(stepsize);
   SetStart(start);
   SetNTracksBench(ntracksbench);
   SetNTracksCleanup(ntrackscleanup);
   SetDraw(draw);
   SetDebug(debug);
   SetRegenerate(regenerate);
   FillNodeInfo();

   //default fNFilesANode is maximum number of workers on a node in the cluster
   Int_t maxnw=0;

   if (fNodes){
      TIter nn(fNodes);
      Int_t nw=0;
      TProofNode* pn;
      while ((pn=(TProofNode*)nn())){
         nw=pn->GetNWrks();
         maxnw=nw>maxnw?nw:maxnw;
      }
   }
   fNFilesANode=maxnw;

   switch (benchmarkmode){
      case kModeNotSpecified:
      break;
      case kModeConstNFilesNode: //fixed number of files for each node 
                            //regardless of the number of workers on the node
         if (nfilesanode>0){
            SetBenchmarkMode(kModeConstNFilesNode, nfilesanode);
         }
         else{
            //Default: max number of workers on a node in the cluster
            SetBenchmarkMode(kModeConstNFilesNode, maxnw);
         }
      break;
      case kModeConstNFilesWorker:
         if (nfilesaworker>0){
            SetBenchmarkMode(kModeConstNFilesWorker, nfilesaworker);
         }
         else{ //default; 1 file a worker
            SetBenchmarkMode(kModeConstNFilesWorker, 1);
         } 
      break;
      case kModeVaryingNFilesWorker:
         if (nfilesaworker>0){
            SetBenchmarkMode(kModeVaryingNFilesWorker, nfilesaworker);
         }
         else{ //default; 1 file a worker
            SetBenchmarkMode(kModeVaryingNFilesWorker, 1);
         } 
      break;
      default:
      break;
   }

   //gEnv->SetValue("Proof.StatsHist",0);
   gEnv->SetValue("Proof.StatsTrace",1);
   //gEnv->SetValue("Proof.SlaveStatsTrace",0);
}

void TProofBench::SetBaseDir(TString basedir)
{
//Set base directory, where files for benchmark and cleaning up are to be placed.
//Input parameters:
//   basedir: base directory name
//   When string length is zero, there is no change in the base directory name
//Returns:
//   Nothing

   basedir.Remove(TString::kBoth, ' '); //remove leading and trailing white space(s)
   basedir.Remove(TString::kBoth, '\t');//remove leading and trailing tab character(s)
   basedir.Remove(TString::kTrailing, '/'); //remove trailing /

   fBaseDir=basedir.Length()>0?basedir:fBaseDir;
}

void TProofBench::SetRunType(TString where, ERunType runtype)
{
//Set run type and name stem
   switch (runtype){
   case kRunNotSpecified:
      fRunType=runtype;
      fNameStem="RunNotSpecified";
      break;
   case kRunCPUTest:
      fRunType=runtype;
      fNameStem="CPUTest";
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
   case kRunCleanup:
      fRunType=runtype;
      fNameStem="Cleanup";
      break;
   case kRunAll:
      fRunType=runtype;
      fNameStem="All";
      break;
   default:
      Error(where.Data(), "Invalid run type: use one of ("
                   "TProofBench::kRunCPUTest, TProofBench::kRunFullDataRead, "
                   "TProofBench::kRunOptDataRead, TProofBench::kRunNoDataRead, "
                   "TProofBench::kRunCleanup, or kRunAll)");
      return;
      break;
   }

}

Int_t TProofBench::SetBenchmarkMode(EBenchmarkMode benchmarkmode, Int_t par)
{
//Set benchmark mode and relevant parameter as a set 
//((kModeConstNFilesNode and fNFilesANode), (kModeConstNFilesWorker and fNFilesAWorker)
// or (kModeVaryingNFilesWorker and fNFilesAWorker)).
//Input Parameters:
//   benchmarkmode:
//      benchmark mode
//   par:
//      when par>0, it is used as relevant data member.
//      when par<0, there is no change in the relevant data member.
//Returns:
//   0 when ok
//   <0 when anything is wrong


   switch (benchmarkmode){
      case kModeConstNFilesNode:
         if (par<=0 && fNFilesANode<=0){
            Error("SetBenchmarkMode", "BenchmarkMode (kModeConstNFilesNode) is not compatible"
                  " with its parameter (fNFilesANode=%d)", fNFilesANode);
         }
         else{
            fBenchmarkMode=benchmarkmode;
            fNFilesANode=par>0?par:fNFilesANode;
            return 0;
         }
      break;
      case kModeConstNFilesWorker:
         if (par<=0 && fNFilesAWorker<=0){
            Error("SetBenchmarkMode", "BenchmarkMode (kModeConstNFilesWorker) is not compatible"
                  " with its parameter (fNFilesAWorker=%d)", fNFilesAWorker);
         }
         else{
            fBenchmarkMode=benchmarkmode;
            fNFilesAWorker=par>0?par:fNFilesAWorker;
            return 0;
         }
      break;
      case kModeVaryingNFilesWorker:
         if (par<=0 && fNFilesAWorker<=0){
            Error("SetBenchmarkMode", "BenchmarkMode (kModeVaryingNFilesWorker) is not compatible"
                  " with its parameter (fNFilesAWorker=%d)", fNFilesAWorker);
         }
         else{
            fBenchmarkMode=benchmarkmode;
            fNFilesAWorker=par>0?par:fNFilesAWorker;
            return 0;
         }
      break;
      default:
      break;
   }
    
   return -1;
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
//Set the maximum number of workers for benchmark test
//Input parameters:
//   sworkers: can be "1x", "2x" and so on, where total number of workers is set 
//             to 1*no_total_workers, 2*no_total_workers respectively.
//             For now only "1x" is supported
//Returns:
//   Nothing
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

   if (fDataSetGeneratedBench){ delete fDataSetGeneratedBench;}
   if (fDataSetGeneratedCleanup){ delete fDataSetGeneratedCleanup;}
   if (fFile){ 
      fFile->Close(); 
      delete fFile;
   }
   fProof=0;
   if (fNodes) delete fNodes;
} 

/* void TProofBench::RunBenchmarkAll()
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
*/

void TProofBench::RunBenchmark(ERunType whattorun, EBenchmarkMode mode, Long64_t nevents)
{
//Run benchmark
//Input parameters:
//   whattorun: 
//      kRunNotSpecified:        run mode is set to fRunType
//      kRunCPUTest:             runs cpu intensive test
//      kRunFullDataRead:        reads tree in files to perform IO-extensive test
//      kRunOptDataRead:         reads only part of tree to perform IO-extensive test
//      kRunCleanup:             reads in files for cleaning up memory between benchmark runs
//      kRunAll:                 = kRunCPUTest | kRunFullDataRead | kRunOptDataRead | kRunNoDataRead
//   mode:
//      kModeNotSpecified:       mode is set to fBenchmarkMode
//      kModeConstNFilesNode:    Processes data set DataSetEventConstNFilesNode_<#workers>_<nfilesanode>
//      kModeConstNFilesWorker:  Processes data set DataSetEventConstNFilesWorker_<#workers>_<nfilesaworker>
//      kModeVaryingNFilesWorker:Processes data set DataSetEventVaryingNFilesWorker_<#workers>_<nfilesaworker>
//      kModeCleanup:            Processes data set DataSetEventCleanup
//   nevents:   Number of events to run per file (-1 for all entries in files) 
//              when whattorun==kRunFullDataRead or whattorun==kRunOptDataRead or whattorun==kRunAll.
//              Total number of events to process when whattorun==kRunCPUTest.
//              Ignored when whattorun==kRunCleanup or whattorun==kRunNotSpecified
//Returns: Nothing
//  
   if (!fProof){
      Error("RunBenchmark", "Proof not set");
      return;
   }

   if (whattorun==kRunAll){
      RunBenchmark(kRunCPUTest, mode, nevents);
      RunBenchmark(kRunFullDataRead, mode, nevents);
      RunBenchmark(kRunOptDataRead, mode, nevents);
      RunBenchmark(kRunNoDataRead, mode, nevents);
   }

   if (whattorun==kRunNotSpecified){
      if (fRunType==kRunNotSpecified){
         Error("RunBenchmark", "Run type not specified; doing nothing and returning");
         return;
      }
      whattorun=fRunType;
      Info("RunBenchmark", "Run type is set to %d", whattorun);
   }
   if (mode==kModeNotSpecified
      && (whattorun==kRunFullDataRead || whattorun==kRunOptDataRead || whattorun==kRunNoDataRead)){
      if (fBenchmarkMode==kModeNotSpecified){
         Error("RunBenchmark", "Mode not specified; doing nothing and returning");
         return;
      }
      mode=fBenchmarkMode;
      Info("RunBenchmark", "Benchmark mode is set to %d", mode);
   }

   if (whattorun==kRunCPUTest && nevents<0){
      nevents=fNEvents;
      Info("RunBenchmark", "Number of events to process is set to %lld", nevents);
   }

   TString perfstats_name = "PROOF_PerfStats";

   Int_t filewritable=0;
   if (fFile){
      TString soption=fFile->GetOption(); 
      soption.ToLower(); 
      if (soption.Contains("create") || soption.Contains("update")){
         filewritable=1;
      }
   }
   switch (whattorun){
   case kRunNotSpecified:

      Info("RunBenchmark", "kRunNotSpecified requested; doing nothing and returning");
      SetRunType("RunBenchmark", kRunNotSpecified);
      return;
      break;

   case kRunCPUTest:
      Info("RunBenchmark", "kRunCPUTest");
      //SetRunType("RunBenchmark", kRunCPUTest);
      //if (CheckParameters("RunBenchmark")) return;
      //SetInputParameters();
      if (fProof){
         fProof->SetParameter("PROOF_BenchmarkHistType", fHistType);
         fProof->SetParameter("PROOF_BenchmarkNHists", fNHists);
         fProof->SetParameter("PROOF_BenchmarkDraw", Int_t(fDraw));
      }
      else{
         Error("RunBenchmark", "Proof not set, doing noting");
      }

      for (Int_t nactive=fStart; nactive<=fMaxNWorkers; nactive+=fStepSize) {
         fProof->SetParallel(nactive);
         for (Int_t j=0; j<fNTries; j++) {

            TTime starttime = gSystem->Now();
            fProof->Process("TSelHist", nevents);
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
               newname+="CPUTest";
               newname+="_";
               newname+=nactive;
               newname+="Slaves_Run";
               newname+=j;
               t->SetName(newname);

               if (fDebug && filewritable){
                  fDirProofBench->cd();
                  t->Write();
                  //t->SetName(origname);
                  //t->SetDirectory(trdir);
                  //t->Delete("");
               }
                  //dirsav->cd();
            } else {
               Error("RunBenchmark", "tree %s not found", perfstats_name.Data());
            }
         }//for iterations
      }//for number of workers

      break;
   case kRunFullDataRead:
   case kRunOptDataRead:
   case kRunNoDataRead:
   {
      const Int_t np=(fMaxNWorkers-fStart)/fStepSize+1;
      Int_t wp[np];
      Int_t i=0;
      for (Int_t nactive=fStart; nactive<=fMaxNWorkers; nactive+=fStepSize) {
         wp[i++]=nactive;
      }
      
      Int_t nf=0;
      TString smode;
      switch (mode){
         case kModeConstNFilesNode:
            smode="ConstNFilesNode";
            nf=fNFilesANode;
         break;
         case kModeConstNFilesWorker:
            nf=fNFilesAWorker;
            smode="ConstNFilesWorker";
         break;
         case kModeVaryingNFilesWorker:
            nf=fNFilesAWorker;
            smode="VaryingNFilesWorker";
         break;
         default:
            Error("RunBenchmark", "Benchmark mode not supported; %d", mode);
            return;
         break;
      }

      for (Int_t nactive=fStart; nactive<=fMaxNWorkers; nactive+=fStepSize) {
         for (Int_t j=0; j<fNTries; j++) {

            //cleanup run
            RunBenchmark(kRunCleanup);

            TString namestem;
            if (whattorun==kRunFullDataRead){
               namestem="FullDataRead";
               Info("RunBenchmark", "kRunFullDataRead, kMode%s", smode.Data());
            }
            else if (whattorun==kRunOptDataRead){
               namestem="OptDataRead";
               Info("RunBenchmark", "kRunOptDataRead, kMode%s", smode.Data());
            }
            else if (whattorun==kRunNoDataRead){
               namestem="NoDataRead";
               Info("RunBenchmark", "kRunNoDataRead, kMode%s", smode.Data());
            }

            //SetRunType("RunBenchmark", whattorun);
            //if (CheckParameters("RunBenchmark")) return;

            fProof->SetParallel(nactive);

            //SetInputParameters();
            if (fProof){
               fProof->SetParameter("PROOF_BenchmarkRunType", Int_t(whattorun));
               fProof->SetParameter("PROOF_BenchmarkDraw", Int_t(fDraw));
               fProof->SetParameter("PROOF_BenchmarkDebug", Int_t(fDebug));
            }
            else{
               Error("RunBenchmark", "Proof not set, doing noting");
            }

            TString dsname; 
  	    dsname.Form("DataSetEvent%s_%d_%d", smode.Data(), nactive, nf);
 
            TFileCollection* fc=0;
            Long64_t nfiles=0;
            Long64_t nevents_total=-1; //-1 for all events in the group of files
            if ((fc=fProof->GetDataSet(dsname.Data()))){
               nfiles=fc->GetNFiles();
               if (nevents>0){
                  nevents_total=nfiles*nevents;
               }
            }
            else{
               Error("RunBenchmark", "no such data set found; %s", dsname.Data());
               continue;
            }
            Info("RunBenchmark", "Total number of events to process is %lld", nevents_total); 
            TTime starttime = gSystem->Now();
            fProof->Process(dsname.Data(), "TSelEvent", "", nevents_total);

            TTime endtime = gSystem->Now();

            TList* l = fProof->GetOutputList();

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
               newname+=namestem;
               newname+="_";
               newname+=smode;
               newname+="_";
               newname+=nactive;
               newname+="Slaves_Run";
               newname+=j;
               t->SetName(newname);
               if (fDebug && filewritable){
                  fDirProofBench->cd();
                  t->Write();
                  //t->SetName(origname);
                  //t->SetDirectory(trdir);
                  //t->Delete("");
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
                  newname+=namestem;
                  newname+="_";
                  newname+=smode;
                  newname+="_";
                  newname+=nactive;
                  newname+="Slaves_Run";
                  newname+=j;
                  hnew->SetName(newname);
                  if (filewritable){
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
                  newname+=namestem;
                  newname+="_";
                  newname+=smode;
                  newname+="_";
                  newname+=nactive;
                  newname+="Slaves_Run";
                  newname+=j;
                  hnew->SetName(newname);
                  if (filewritable){
                     fDirProofBench->cd();
                     hnew->Write();
                     //hnew->SetName(origname);
                     //hnew->SetDirectory(hdir);
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
   }
      break;
   case kRunCleanup:
      {
         //make sure files are generated
         //RunBenchmark(kRunGenerateFileCleanup);

         Info("RunBenchmark", "kRunCleanup");
         //SetRunType("RunBenchmark", kRunCleanup);
         //if (CheckParameters("RunBenchmark")) return;
         Int_t nactive_sav=fProof->GetParallel();
         fProof->SetParallel(fMaxNWorkers);

         if (fProof){
            fProof->SetParameter("PROOF_BenchmarkRunType", Int_t(whattorun));
            fProof->SetParameter("PROOF_BenchmarkDraw", Int_t(fDraw));
            fProof->SetParameter("PROOF_BenchmarkDebug", Int_t(fDebug));
         }
         else{
            Error("RunBenchmark", "Proof not set, doing noting");
         }
         //fProof->Process(fDataSetGeneratedCleanup, "TSelEvent", "", -1);

         //TString dsname="ds_event_cleanup";
         TString dsname="DataSetEventCleanup";

         TFileCollection* fc_cleanup=fProof->GetDataSet(dsname.Data());
         TTime starttime = gSystem->Now();
         if (fc_cleanup){
            fProof->Process(fc_cleanup, "TSelEvent", "", -1);
         }
         else{
            Error("RunBenchmark", "File Collection does not exist; returning", dsname.Data());
            return;
         }

         if (fDebug){

            TList* l = fProof->GetOutputList();

            //save perfstats
            TTree* t = dynamic_cast<TTree*>(l->FindObject(perfstats_name.Data()));

            static Long64_t ncalls=0;

            if (t) {
               //TDirectory* trdir = t->GetDirectory();
               //TDirectory* dirsav = gDirectory;
               //fFile->cd();
               t->SetDirectory(fDirProofBench);
               //TString origname = t->GetName();

               //build up new name
               TString newname = perfstats_name;
               newname+="_";
               newname+="Cleanup";
               newname+=Form("%lld", ncalls);
               t->SetName(newname);

               if (filewritable){
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
               newname+="Cleanup";
               newname+=Form("%lld", ncalls);
               hnew->SetName(newname);

               if (filewritable){
                  fDirProofBench->cd();
                  hnew->Write();
                  //hnew->SetName(origname);
                  //hnew->SetDirectory(hdir);
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
               if (filewritable){
                  fDirProofBench->cd();
                  hnew->Write();
                  //hnew->SetName(origname);
                  //hnew->SetDirectory(hdir);
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
   case kRunAll:
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
            fc->Add(TString::Format("%s/event_%d.root", basedir, kk++));
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
   //SetInputParameters();
   if (fProof){
      fProof->SetParameter("PROOF_BenchmarkRunType", fRunType);
      fProof->SetParameter("PROOF_BenchmarkNEvents", fNEvents);
      fProof->SetParameter("PROOF_BenchmarkNTracksBench", fNTracksBench);
      fProof->SetParameter("PROOF_BenchmarkNTracksCleanup", fNTracksCleanup);
   }
   else{
      Error("GenerateFilesN", "Proof not set, doing noting");
   }

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

Int_t TProofBench::MakeDataSets(EBenchmarkMode mode, const char* option, Int_t nfp, Int_t start, Int_t stop, Int_t step, const TDSet* tdset)
{
//Make data sets (file collection) and register them for benchmark test
//
//Input parameters
//   basedir:
//   mode: kModeConstNFilesNode : fixed number of files on each node 
//         kModeConstNFilesWorker : files as many as workers on each node
//         kModeVaryingNFilesWorker : number of files is set as number of workers
//   option: is used as option to TProof::RegisterDataSet
//   nfp: when mode==kModeConstNFilesNode, number of files on each node
//        when mode==kModeConstNFilesWorker, number of files for each worker,
//                   the number of files in each data set will be
//                  sum of (nfp * (number of workers on the node))
//        when mode==kModeVaryingNFilesWorker, number of files for each worker
//   start: Starting number of workers during the benchmark test
//   stop: Ending number of workers during the benchmark test
//   step: Incremental number of workers during the benchmark test
//   tdset: data set (TDSet*) from which file collection is built from. 
//          This is usually the data set built when GenerateFiles function is called but user can supply one.
//Returns: 0 when ok, <0 when anything is wrong


   //check input parameters
   switch (mode){
      case kModeConstNFilesNode:
      case kModeConstNFilesWorker:
      case kModeVaryingNFilesWorker:
         if (start<=0){
             start=1;
             Info("MakeDataSets", "Start number of workers is set to %d", start);
         }
         if (stop<=0){
             stop=fMaxNWorkers;
             Info("MakeDataSets", "End number of workers is set to %d", stop);
         }
         if (step<=0){
             step=1;
             Info("MakeDataSets", "Step number of workers is set to %d", step);
         }
         if (start>stop){
            Error("MakeDataSets", "Wrong inputs; start(%d)>stop(%d)", start, stop);
            return -1;
         }
         break;
      case kModeCleanup:
      break;
      default:
         Error("MakeDataSets", "Mode not recognized; %d", mode);
         return -1;
      break;
   }

   switch (mode){
      case kModeConstNFilesNode:
      {
         if (nfp<=0){
            nfp=fNFilesANode;
            Info("MakeDataSets", "Number of files a node is set to %d for mode kModeConstNFilesNode", nfp);
         }
         const Int_t np=(stop-start)/step+1;
         Int_t wp[np];
         Int_t i=0;
         for (Int_t nactive=start; nactive<=stop; nactive+=step) {
            wp[i]=nactive;
            i++;
         }
         if (!tdset){
            if (fDataSetGeneratedBench){
               tdset=fDataSetGeneratedBench;
            }
            else{
               Error("MakeDataSets", "Empty data set; Files not generated");
               return -1;
            }
         }
         MakeDataSets(mode, np, wp, nfp, option, tdset);
      }
      break;
      case kModeConstNFilesWorker:
      {
         if (nfp<=0){
            nfp=fNFilesAWorker;
            Info("MakeDataSets", "Number of files a worker is set to %d "
                                 "for mode kModeConstNFilesWorker", nfp);
         }
         const Int_t np=(stop-start)/step+1;
         Int_t wp[np];
         Int_t i=0;
         for (Int_t nactive=start; nactive<=stop; nactive+=step) {
            wp[i]=nactive;
            i++;
         }
         if (!tdset){
            if (fDataSetGeneratedBench){
               tdset=fDataSetGeneratedBench;
            }
            else{
               Error("MakeDataSets", "Empty data set; Files not generated");
               return -1;
            }
         }
         MakeDataSets(mode, np, wp, nfp, option, tdset);
      }
      break;
      case kModeVaryingNFilesWorker:
      {
         if (nfp<=0){
            nfp=fNFilesAWorker;
            Info("MakeDataSets", "Number of files a worker is set to %d "
                                 "for mode kModeVaryingNFilesWorker", nfp);
         }
         const Int_t np=(stop-start)/step+1;
         Int_t wp[np];
         Int_t i=0;
         for (Int_t nactive=start; nactive<=stop; nactive+=step) {
            wp[i]=nactive;
            i++;
         }
         if (!tdset){
            if (fDataSetGeneratedBench){
               tdset=fDataSetGeneratedBench;
            }
            else{
               Error("MakeDataSets", "Empty data set; Files not generated");
               return -1;
            }
         }
         MakeDataSets(mode, np, wp, nfp, option, tdset);
      }
      break;
      case kModeCleanup:
      {
         if (!tdset){
            if (fDataSetGeneratedCleanup){
               tdset=fDataSetGeneratedCleanup;
            }
            else{
               Error("MakeDataSets", "Empty data set; Files not generated");
               return -1;
            }
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
         fProof->RegisterDataSet(dsname, fc, option);
      }
      break;
      default:
         Error("MakeDataSets", "mode not recognized; %d", mode);
         return -1;
      break;
   }
   return 0;
}

//_________________________________________________________________________________
Int_t TProofBench::MakeDataSets(EBenchmarkMode mode, Int_t np, const Int_t *wp, const Int_t nf,
                                const char* opt, const TDSet* tdset)
{
//Create the data sets (file collection) and register them to Proof
//Input parameters:
//   mode: benchmark mode
//   np: Number of test points
//   wp: Array of number of workers for test points (should contain np points)
//   nf: Number of files 
//       It corresponds to fNFilesANode when mode is kModeConstNFilesNode,
//       fNFilesAWorker when mode=kModeConstNFilesWorker,
//       fNFilesAWorker when mode=kModeVaryingNFilesWorker
//   opt: Option to TProof::RegisterDataSet
//   tdset: Data set (TDSet*) from which file collections are to be built from. 
//          This is usually the data set built when GenerateFiles function is called but user can supply one.
//Returns:
//   0 when ok
//  <0 when anything is wrong

   // Check inputs
   if (np <= 0 || !wp || !fProof) {
      Error("MakeDataSets", "wrong inputs (%d, %p, %p)", np, wp, fProof);
      return -1;
   }

   TString smode, stem;
   switch (mode){
      case kModeConstNFilesNode:
         if (!tdset){
            if (fDataSetGeneratedBench){
               tdset=fDataSetGeneratedBench;
            }
            else{
               Error("MakeDataSets", "Empty data set; Files not generated");
               return -1;
            }
         }
         smode="ConstNFilesNode";
         stem="_Benchmark_";
      break;
      case kModeConstNFilesWorker:
         if (!tdset){
            if (fDataSetGeneratedBench){
               tdset=fDataSetGeneratedBench;
            }
            else{
               Error("MakeDataSets", "Empty data set; Files not generated");
               return -1;
            }
         }
         smode="ConstNFilesWorker";
         stem="_Benchmark_";
      break;
      case kModeVaryingNFilesWorker:
         if (!tdset){
            if (fDataSetGeneratedBench){
               tdset=fDataSetGeneratedBench;
            }
            else{
               Error("MakeDataSets", "Empty data set; Files not generated");
               return -1;
            }
         }
         smode="VaryingNFilesWorker";
         stem="_Benchmark_";
      break;
      case kModeCleanup:
         Error("MakeDataSets", "Nothing to be done for this mode; %d", mode);
      break;
      default:
         Error("MakeDataSets", "Benchmark mode not recognized; %d", mode);
         return -1;
      break;
   }
   // There will be 'nr' datasets per point, rotating the files
   // Dataset naming:
   //                   ds_event_[lab]_[wrks_point]_[run]
   TString dsname;

   if (mode==kModeConstNFilesNode|| mode==kModeConstNFilesWorker){
      Int_t kp;
      for (kp = 0; kp < np; kp++) {
         // Dataset name
 	 dsname.Form("DataSetEvent%s_%d_%d", smode.Data(), wp[kp], nf);
         Info("MakeDataSets", "creating dataset '%s' ...", dsname.Data());
         // Create the TFileCollection
         TFileCollection *fc = new TFileCollection;

         TIter nxni(fNodes);
         TProofNode *ni = 0;
         while ((ni = (TProofNode *) nxni())) {
            TString nodename=ni->GetName();
            Int_t nworkers=ni->GetNWrks();

            //set number of files to add for each node
            Int_t nfiles=0;
            if (mode==kModeConstNFilesNode){
                  nfiles=nf;
            }
            else if (kModeConstNFilesWorker){
                  nfiles=nf*nworkers;
            }
            Int_t nfilesadded=0;
            Int_t nfile;
            TList* lelement=tdset->GetListOfElements();
            TIter nxtelement(lelement);
            TDSetElement *tdelement;
            TFileInfo* fileinfo;
            TUrl* url;
            TString hostname, filename, tmpstring;
      
            while ((tdelement=(TDSetElement*)nxtelement())){
      
               fileinfo=tdelement->GetFileInfo();
               url=fileinfo->GetCurrentUrl();
               hostname=url->GetHost();
               filename=url->GetFile();
      
               if (hostname!=nodename) continue;
      
               //filename=root://hostname//directory/EventTree_Benchmark_filenumber_serial.root
               //remove upto "Benchmark_"
               tmpstring=filename;
               tmpstring.Remove(0, filename.Index(stem)+stem.Length());
      
               TObjArray* token=tmpstring.Tokenize("_");
      
               if (token){
                  nfile=TString((*token)[0]->GetName()).Atoi();
                  token=TString((*token)[1]->GetName()).Tokenize(".");
                  Int_t serial=TString((*token)[0]->GetName()).Atoi();
      
                  //ok found, add it to the dataset
                  //count only once for set of split files
                  if (serial==0){
                     if (nfilesadded>=nfiles){
                        break;
                     }
                     nfilesadded++;
                  }
                  fc->Add(fileinfo);
                  //Info ("CreateDataSetsN", "added");
               }
               else{
                  Error("MakeDataSets", "File name not recognized: %s", fileinfo->GetName());
                  return -1;
               }
            }
            if (nfilesadded<nfiles){
               Warning("MakeDataSets", "Not enough number of files; "
                                       "%d files out of %d files requested on node %s "
                                       "are added to data set %s",
                        nfilesadded, nfiles, nodename.Data(), dsname.Data());
            }
         }

         fc->Update();
         // Register dataset with verification
         fProof->RegisterDataSet(dsname, fc, opt);
         SafeDelete(fc);
      }
   }
   else if (mode==kModeVaryingNFilesWorker){
      Int_t kp;
      Int_t nfiles=0;
      for (kp = 0; kp < np; kp++) {
         TFileCollection *fc = new TFileCollection;
 	 dsname.Form("DataSetEvent%s_%d_%d", smode.Data(), wp[kp], nf);
         Info("MakeDataSets", "creating dataset '%s' ...", dsname.Data());

         nfiles=nf*wp[kp]; //number of files on all nodes for wp[kp] workers

         //Check if we have enough number of files
         TList* le=tdset->GetListOfElements();
         Int_t nfilesavailable=le->GetSize(); 
         if (nfilesavailable<nfiles){
            Warning("MakeDataSets", "Number of available files (%d) is smaller than needed (%d)"
                   ,nfilesavailable, nfiles);
         }
         //Check if number of requested workers are not greater than number of available workers
         TList *wl = fProof->GetListOfSlaveInfos();
         if (!wl) {
            Error("MakeDataSets", "Could not get information about workers!");
            return -2;
         }

         Int_t nworkersavailable=wl->GetSize();
         if (nworkersavailable<wp[kp]){
            Warning("MakeDataSets", "Number of available workers (%d) is smaller than needed (%d); "
                                    "Only upto %d(=%d files/worker * %d workers) out of %d(=%d files/worker* %d workers) files will be added to the data set"
                   ,nworkersavailable, wp[kp], nf*nworkersavailable, nf, nworkersavailable, nfiles, nf, wp[kp]);
         }
         
         //copy tdset
         TList* lecopy=new TList;
         TIter nxte(le);
         TDSetElement *tde;
         while (tde=(TDSetElement*)nxte()){
            lecopy->Add(new TDSetElement(*tde));
         }
         //lecopy->Print("A");

         TIter nxwi(wl);
         TSlaveInfo *si = 0;
         TString nodename;
         Int_t nfilesadded=0;
         Int_t nfile;
         TFileInfo* fileinfo;
         TUrl* url;
         TString hostname, filename, tmpstring;
         while ((si = (TSlaveInfo *) nxwi())) {
            nodename=si->GetName();
            
            //start over for this worker
            TIter nxtelement(lecopy);
            Int_t nfilesadded_worker=0;
            while ((tde=(TDSetElement*)nxtelement())){
      
               fileinfo=tde->GetFileInfo();
               url=fileinfo->GetCurrentUrl();
               hostname=url->GetHost();
               filename=url->GetFile();
      
               if (hostname!=nodename) continue;
      
               //Info("MakeDataSets", "filename=%s", fileinfo->GetName());
               //filename=root://hostname//directory/EventTree_Benchmark_filenumber_serial.root
               //remove upto "Benchmark_"
               tmpstring=filename;
               tmpstring.Remove(0, filename.Index(stem)+stem.Length());
      
               TObjArray* token=tmpstring.Tokenize("_");
      
               //Info ("CreateDataSetsN", "file %s", url->GetUrl());
               if (token){
                  nfile=TString((*token)[0]->GetName()).Atoi();
                  token=TString((*token)[1]->GetName()).Tokenize(".");
                  Int_t serial=TString((*token)[0]->GetName()).Atoi();
      
                  //ok found, add it to the dataset
                  //count only once for set of split files
                  if (serial==0){
                     if (nfilesadded_worker>=nf){
                        break;
                     }
                     nfilesadded_worker++;
                  }
                  fc->Add(fileinfo);
                  lecopy->Remove(tde);
               }
               else{
                  Error("MakeDataSets", "File name not recognized: %s", fileinfo->GetName());
                  return -1;
               }
            }
            nfilesadded+=nfilesadded_worker;
            if (nfilesadded>=nfiles){
               break;
            }
         }
         if (nfilesadded<nfiles){
            Warning("MakeDataSets", "Only %d files out of %d files requested "
                                    "are added to data set %s",
                     nfilesadded, nfiles, dsname.Data());
         }
         fc->Update();
         // Register dataset with verification
         fProof->RegisterDataSet(dsname, fc, opt);
         lecopy->SetOwner(kTRUE);
         SafeDelete(lecopy);
         SafeDelete(fc);
      }
   }
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
   //SetInputParameters();
   if (fProof){
      fProof->SetParameter("PROOF_BenchmarkBaseDir", fBaseDir.Data());
      fProof->SetParameter("PROOF_BenchmarkRunType", fRunType);
      fProof->SetParameter("PROOF_BenchmarkNHists", fNHists);
      fProof->SetParameter("PROOF_BenchmarkHistType", fHistType);
      fProof->SetParameter("PROOF_BenchmarkNTries", fNTries);
      fProof->SetParameter("PROOF_BenchmarkNEvents", fNEvents);
      fProof->SetParameter("PROOF_BenchmarkDraw", Int_t(fDraw));
      fProof->SetParameter("PROOF_BenchmarkRegenerate", Int_t(fRegenerate));
      fProof->SetParameter("PROOF_BenchmarkMode", fBenchmarkMode);
      fProof->SetParameter("PROOF_BenchmarkNFilesAWorker", fNFilesAWorker);
      fProof->SetParameter("PROOF_BenchmarkNFilesANode", fNFilesANode);
   }
   else{
      Error("GenerateFilesN", "Proof not set, doing noting");
   }

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

Int_t TProofBench::GenerateFiles(EFileType filetype, Int_t nf, Long64_t nevents, const char* basedir, Bool_t regenerate)
{

//Generates files on worker nodes for I/O test or for cleanup run
//Input parameters do not change corresponding data members
//Data set member (fDataSetGeneratedBench or fDataSetGeneratedCleanup) gets filled up
//with generated data set elements
//
//Input parameters
//   filetype: kFileBenchmark : Generate files for benchmark run
//             kFileCleanup: Generate files for cleanup run
//   nf: Number of files per node when filetype==kFileBenchmark.
//       Ignored when filetype==kFileCleanup
//   nevents: Number of events in a file when filetype==kFileBenchmark.
//            Igonired when filetype==kFileCleanup 
//   basedir: Base directory for the files to be generated on the worker nodes. 
//   regenerate: Regenerate files when kTRUE,
//               Reuse files if they exist, when kFALSE (default)
//Returns: 
//   0 when ok
//  <0 when anything is wrong

   if (filetype==kFileBenchmark && nf<=0){
      Error("GenerateFiles", "Number of files to generate per node should be positive integer; %d", nf);
      return -1;
   }
   if (nevents<=0){
      nevents=fNEvents;
      Info("GenerateFiles", "Number of events to generate per file is set to %d", nevents);
   }
      
   TString bd=basedir;
   if (!bd.Length()){
      bd=fBaseDir.Data();
   }

   if (filetype==kFileBenchmark){
      if (fDataSetGeneratedBench) delete fDataSetGeneratedBench;
      fDataSetGeneratedBench = new TDSet("TTree","EventTree");
   }
   else if (filetype==kFileCleanup){
      if (fDataSetGeneratedCleanup) delete fDataSetGeneratedCleanup;
      fDataSetGeneratedCleanup = new TDSet("TTree","EventTree");
   }
   
   Int_t nactive_sav;

   //Generate files on all nodes, active or inactive
   if (fProof){
      nactive_sav=fProof->GetParallel();
      fProof->SetParallel(99999);
   }
   else{
      Error("GenerateFiles", "Proof not set, doing noting");
      return -1;
   }

   TList* wl=fProof->GetListOfSlaveInfos();

   TSortedList* wlcopy=new TSortedList;
   wlcopy->SetName("PROOF_SlaveInfos");
   //fProof->AddInputData(wl, kTRUE);

   TIter nwl(wl);
   TSlaveInfo *si=0;
   while (si=(TSlaveInfo*)nwl()){
      wlcopy->Add(new TSlaveInfo(*si));
   }
   fProof->AddInput(wlcopy);

   // Create the file names and the map {worker,files}
   // Naming:
   //         <basedir>/EventTree_Benchmark_nfile_serial.root
   TMap *filesmap = new TMap;
   filesmap->SetName("PROOF_FilesToProcess");
   Long64_t entries = 0;
   TIter nxni(fNodes);
   TProofNode *ni = 0;

   if (filetype==TProofBench::kFileBenchmark){
            while ((ni = (TProofNode *) nxni())) {
               TList *files = new TList;
               files->SetName(ni->GetName());
               for (Int_t i = 0; i <nf; i++) {
                  files->Add(new TObjString(TString::Format("%s/EventTree_Benchmark_%d_0.root", 
                                                            bd.Data(), i)));
                  entries++;
               }
               filesmap->Add(new TObjString(ni->GetName()), files);
               //files->Print();
            }
   }
   else if (filetype==TProofBench::kFileCleanup){
      while ((ni = (TProofNode *) nxni())) {
         TList *files = new TList;
         Long64_t memorytotal=ni->GetPhysRam()*1024*1024;
         Int_t nworkers=ni->GetNWrks();
         files->SetName(ni->GetName());
         
         for (Int_t i = 0; i <nworkers; i++) { //split load across the workers on the node
            files->Add(new TObjString(TString::Format("%s/EventTree_Cleanup_%d_0.root", 
                                                      bd.Data(), i)));
            entries++;
         }
         filesmap->Add(new TObjString(ni->GetName()), files);
         //files->Print();
      }
   }

   //fProof->SetParameter("PROOF_BenchmarkBaseDir", bd);
   fProof->SetParameter("PROOF_BenchmarkFileType", filetype);
   fProof->SetParameter("PROOF_BenchmarkNEvents", nevents);
   fProof->SetParameter("PROOF_BenchmarkRegenerate", Int_t(regenerate));
   fProof->SetParameter("PROOF_BenchmarkMode", fBenchmarkMode);
   // Add the file map in the input list
   fProof->AddInput(filesmap);

   // Set the packetizer to be the one on test
   fProof->SetParameter("PROOF_Packetizer", "TPacketizerFile");

   // Run
   fProof->Process("TSelEventGen", entries);

   TList* l = fProof->GetOutputList();

   if (!l){
      Error("GenerateFiles", "list of output not found");
      //Put it back to old configuration
      fProof->SetParallel(nactive_sav);
      return -1;
   }

   //l->Print("A");

   //merge outputs

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
               newfilename="root://"+hostname+"/"+filename;
               tdelement->SetName(newfilename.Data());
            }
            if (filetype==kFileBenchmark){
               fDataSetGeneratedBench->Add(tdset);
            }
            else if (filetype==kFileCleanup){
               fDataSetGeneratedCleanup->Add(tdset);
            }
         }
         else{
            Error("GenerateFiles", "%s not type TList*; moving to the next list", outputname.Data());
            continue;
         }
      }
   }

   // Clear the input parameters
   //ClearInputParameters();
   //fProof->DeleteParameters("PROOF_BenchmarkBaseDir");
   fProof->DeleteParameters("PROOF_BenchmarkFileType");
   fProof->DeleteParameters("PROOF_BenchmarkNEvents");
   fProof->DeleteParameters("PROOF_BenchmarkRegenerate");
   fProof->DeleteParameters("PROOF_BenchmarkBenchmarkMode");

   fProof->DeleteParameters("PROOF_Packetizer");
   fProof->GetInputList()->Remove(filesmap);
   filesmap->SetOwner(kTRUE);
   SafeDelete(filesmap);
   fProof->GetInputList()->Remove(wlcopy);
   wlcopy->SetOwner(kTRUE);
   SafeDelete(wlcopy);

   // Restore previous value
   //fNEvents = oldNEvents;

   //Put it back to old configuration
   fProof->SetParallel(nactive_sav);

   TList* lds=0;
   if (filetype==kFileBenchmark){
      lds=fDataSetGeneratedBench->GetListOfElements();
      lds->Sort();
   }
   else if (filetype==kFileCleanup){
      lds=fDataSetGeneratedCleanup->GetListOfElements();
      lds->Sort();
   }
   TIter nds(lds);
   TDSetElement* dse=0;
   Info("GenerateFiles", "List of files generarted:");
   while (dse=(TDSetElement*)nds()){
      dse->Print("A");
   }
   return 0;
}

void TProofBench::BuildPerfProfiles(ERunType runtype, EBenchmarkMode mode) {
//Build performance profiles
//Input parameters:
//   runtype: Run type to build performance profiles for.
//            When kRunAll, this function is recursively called with runtype=kRunFullDataRead, 
//            kRunOptDataRead, kRunNoDataRead in turn.
//   mode: Benchmark mode to build performance profiles for.
//         Ignored when runtype==kRunCPUTest.
//Returns:
//   Nothing

   if (!(runtype==kRunNotSpecified
      || runtype==kRunCPUTest
      || runtype==kRunFullDataRead
      || runtype==kRunOptDataRead
      || runtype==kRunNoDataRead
      || runtype==kRunAll)){
      Error("BuildPerfProfiles", "Invalid run request");
      return; 
   }

   if (runtype==kRunAll){
      BuildPerfProfiles(kRunCPUTest, mode);
      BuildPerfProfiles(kRunFullDataRead, mode);
      BuildPerfProfiles(kRunOptDataRead, mode);
      BuildPerfProfiles(kRunNoDataRead, mode);
      return;
   }

   if (runtype==kRunNotSpecified){
      if (fRunType==kRunNotSpecified){
         Error("BuildPerfProfiles", "Run type not specified; doing nothing and returning");
         return;
      }
      runtype=fRunType;
      Info("BuildPerfProfiles", "Run type is set to %d", runtype);
   }
   if (mode==kModeNotSpecified
      && (runtype==kRunFullDataRead || runtype==kRunOptDataRead || runtype==kRunNoDataRead)){
      if (fBenchmarkMode==kModeNotSpecified){
         Error("BuildPerfProfiles", "Mode not specified; doing nothing and returning");
         return;
      }
      mode=fBenchmarkMode;
      Info("BuildPerfProfiles", "Benchmark mode is set to %d", mode);
   }

   TString pattern;
   TString hname_event, hname_io, htitle_event, htitle_io;
   switch (runtype){
   case kRunCPUTest:
      pattern="PROOF_PerfStats_CPUTest"; 
      hname_event="hProfCPUTest";
      htitle_event="CPU Test";
      break;
   case kRunFullDataRead:
      pattern="PROOF_PerfStats_FullDataRead"; 
      hname_event=hname_io="hProfFullDataRead";
      htitle_event=htitle_io="Full Data Read";
      break;
   case kRunOptDataRead:
      pattern="PROOF_PerfStats_OptDataRead"; 
      hname_event=hname_io="hProfOptDataRead";
      htitle_event=htitle_io="Opt Data Read";
      break;
   case kRunNoDataRead:
      pattern="PROOF_PerfStats_NoDataRead"; 
      hname_event=hname_io="hProfNoDataRead";
      htitle_event=htitle_io="No Data Read";
      break;
   default:
      break; //we should never get here
   }

   if (runtype!=kRunCPUTest){
      switch (mode){
      case kModeConstNFilesNode:
         pattern+="_ConstNFilesNode"; 
         hname_event+="ConstNFilesNode";
         hname_io+="ConstNFilesNode";
         htitle_event+=", ConstNFilesNode";
         htitle_io+=", ConstNFilesNode";
      break;
      case kModeConstNFilesWorker:
         pattern+="_ConstNFilesWorker"; 
         hname_event+="ConstNFilesWorker";
         hname_io+="ConstNFilesWorker";
         htitle_event+=", ConstNFilesWorker";
         htitle_io+=", ConstNFilesWorker";
      break;
      case kModeVaryingNFilesWorker:
         pattern+="_VaryingNFilesWorker"; 
         hname_event+="VaryingNFilesWorker";
         hname_io+="VaryingNFilesWorker";
         htitle_event+=", VaryingNFilesWorker";
         htitle_io+=", VaryingNFilesWorker";
      break;
      default:
         break; //we should never get here
      }
   }

   hname_event+="Event";
   if (runtype!=kRunCPUTest){
      hname_io+="IO";
   }
   htitle_event+=" (Event Rate)";
   if (runtype!=kRunCPUTest){
      htitle_io+=" (I/O Rate)";
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

   Int_t fromkey=1;
   TList* list=0;
   TObject* obj=0; 

   TString sdir(fDirProofBench->GetPath()); 
   fDirProofBench->cd();

   list=fDirProofBench->GetList(); //list of object

   if (sdir.Contains("Rint:")){//memory
      fromkey=0;
   }
   else{//file
      //try object first
      TIter nxt(list);
      while ((obj=nxt())){
         if (!TString(obj->GetName()).Contains(TRegexp(pattern))) continue;
         fromkey=0;
         break;
      }
   }

   if (fromkey){
      list=fDirProofBench->GetListOfKeys(); //list of object
   }

   //see if file is open for write
   Int_t filewritable=0;
   if (fFile){
      TString soption=fFile->GetOption(); 
      soption.ToLower(); 
      if (soption.Contains("create") || soption.Contains("update")){
         filewritable=1;
      }
   }
      
   TIter nxt(list);
   TKey* key=0;

   Int_t max_slaves=0;
      
   while ((obj=nxt())){
      if (fromkey){
         key=dynamic_cast<TKey*>(obj);
         obj = key->ReadObj();
      }

      if (!TString(obj->GetName()).Contains(TRegexp(pattern))){
         if (fromkey) delete obj;
         continue;
      }

      TTree* t = dynamic_cast<TTree*>(obj);
      if (!t) {
         if (fromkey) delete obj;
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
         if (fromkey) delete t;
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
         if (fromkey) delete t;
         continue;
      } else {
         // this should be the run number
         run_holder = atoi(name+Index);
      }

      if(!t->FindBranch("PerfEvents")) {
         if (fromkey) delete t;
         continue;
      }

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

   }

   //book and fill profiles
   TProfile* hprof_event=0;
   TProfile* hprof_io=0;

   Double_t event_rate, IO_rate;
   Int_t entries=timing_tree->GetEntries();

   hprof_event= new TProfile(hname_event, htitle_event, max_slaves, 0.5, max_slaves+0.5);
   if (runtype!=kRunCPUTest){
      hprof_io= new TProfile(hname_io, htitle_io, max_slaves, 0.5, max_slaves+0.5);
   }
   for (int i=0; i<entries; i++){
      timing_tree->GetEntry(i); 
      //printf("ns_holder=%d, run_holder=%d time_holder=%f\n", ns_holder, run_holder, time_holder);
      event_rate=nevents_holder/time_holder/1000.; 
      hprof_event->Fill(Double_t(ns_holder), event_rate);
      if (runtype!=kRunCPUTest){
         IO_rate=bytes_holder/time_holder/(1024.*1024.); 
         hprof_io->Fill(Double_t(ns_holder), IO_rate);
      }
   }
   hprof_event->GetXaxis()->SetTitle("Number of Slaves");
   hprof_event->GetYaxis()->SetTitle("#times10^{3} Events/sec");
   hprof_event->SetMarkerStyle(21);
   if (filewritable){
      hprof_event->Write();
   }
   if (runtype!=kRunCPUTest){
      hprof_io->GetXaxis()->SetTitle("Number of Slaves");
      hprof_io->GetYaxis()->SetTitle("MB/sec");
      hprof_io->SetMarkerStyle(22);
      if (filewritable){
         hprof_io->Write();
      }
   }

   delete timing_tree;
   return;
}

void TProofBench::Print(Option_t* option)const{

   if (fFile){
       fFile->Print(option);
       fFile->ls(option);
   }
   else{
      Printf("No file open"); 
   }

   if (fProof) fProof->Print(option);
   Printf("fBaseDir=\"%s\"", fBaseDir.Data()); 
   Printf("fRunType=%s%s", "k",fNameStem.Data());

   TString sbenchmarkmode;
   switch (fBenchmarkMode) {
   case kModeNotSpecified:
      sbenchmarkmode="kModeNotSpecified";
      break;
   case kModeConstNFilesNode:
      sbenchmarkmode="kModeConstNFilesNode";
      break;
   case kModeConstNFilesWorker:
      sbenchmarkmode="kModeConstNFilesWorker";
      break;
   case kModeVaryingNFilesWorker:
      sbenchmarkmode="kModeVaryingNFilesWorker";
      break;
   case kModeCleanup:
      sbenchmarkmode="kModeCleanup";
      break;
   default:
      break;
   }

   Printf("fBenchmarkMode=%s", sbenchmarkmode.Data());

   Printf("fNHists=%ld", fNHists);

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

   Printf("fHistType=%s", sbenchhisttype.Data());
   Printf("fMaxNWorkers=%d", fMaxNWorkers);
   Printf("fNTries=%d", fNTries);
   Printf("fNFilesAWorker=%d", fNFilesAWorker);
   Printf("fNFilesANode=%d", fNFilesANode);
   Printf("fNEvents=%lld", fNEvents);
   Printf("fStepSize=%d", fStepSize);
   Printf("fStart=%d", fStart);
   Printf("fNTracksBench=%d", fNTracksBench);
   Printf("fNTracksCleanup=%d", fNTracksCleanup);
   Printf("fDraw=%d", fDraw);
   Printf("fDebug=%d", fDebug);
   Printf("fRegenerate=%d", fRegenerate);

   if (fDataSetGeneratedBench) fDataSetGeneratedBench->Print(option);
   if (fDataSetGeneratedCleanup) fDataSetGeneratedCleanup->Print(option);
   if (fCPerfProfiles){
      Printf("Performance Profiles Canvas: Name=%s Title=%s", 
              fCPerfProfiles->GetName(), fCPerfProfiles->GetTitle());
   }
   if (fDirProofBench){
      Printf("fDirProofBench=%s", fDirProofBench->GetPath());
   }

   if (fNodes) fNodes->Print(option);
}

TFile* TProofBench::OpenFile(const char* filename, 
                             Option_t* option, 
                             const char* ftitle, 
                             Int_t compress)
{
//Opens a file which output profiles and/or intermediate files (trees, histograms when debug is set)
//are to be written to. Makes a directory named "ProofBench" if possible and changes to the directory.
//If directory ProofBench already exists, change to the directory. If the directory can not be created,
//make a directory Rint:/ProofBench and change to the directory.
//Input parameters:
//   filename: Name of the file to open
//   option: Option to TFile::Open(...) function
//   ftitle: Input to TFile::Open(...) function
//Returns:
//   Open file if a file is already open
//   New file just opened
//   0 when open fails;

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
   if (!fFile){
      Error("OpenFile", "Cannot open file; %s", sfilename.Data());
      return 0;
   }
      
   if (fFile->IsZombie()){//open failed
      Error("FileOpen", "Cannot open file: %s", sfilename.Data());
      fFile->Close();
      fFile=0;
      dirsav->cd();
      return 0;
   }
   else{//open succeeded
      if (!soption.Contains("read")){
          fFile->mkdir("ProofBench");
      }
      if (fFile->cd("ProofBench")){
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

/* TFile* TProofBench::SetFile(TFile* ftmp){
//set file to ftmp
//always return fFile

   if (!ftmp){
       Error("OpenFile", "Cannot set file to empty file");
       return fFile;
   }
   if (fFile){
       Error("OpenFile", "file alaredy open; %s; Close it before open another file", fFile->GetName());
       return fFile;
   }
   fFile=ftmp;

   fFile->cd();
   TString soption=fFile->GetOption();
   soption.ToLower();
   if (!soption.Contains("read")){
      fFile->mkdir("ProofBench");
   }
   if (fFile->cd("ProofBench")){
      fDirProofBench=gDirectory;
   }
   else{
      gDirectory->cd("Rint:/");
      gDirectory->mkdir("ProofBench");
      gDirectory->cd("ProofBench");
      fDirProofBench=gDirectory;
   }
   return fFile;
}*/

/*void TProofBench::SetInputParameters(){

   if (fProof){
      fProof->SetParameter("PROOF_BenchmarkBaseDir", fBaseDir.Data());
      fProof->SetParameter("PROOF_BenchmarkRunType", fRunType);
      fProof->SetParameter("PROOF_BenchmarkNHists", fNHists);
      fProof->SetParameter("PROOF_BenchmarkHistType", fHistType);
      fProof->SetParameter("PROOF_BenchmarkNTries", fNTries);
      fProof->SetParameter("PROOF_BenchmarkNEvents", fNEvents);
      fProof->SetParameter("PROOF_BenchmarkDraw", Int_t(fDraw));
      fProof->SetParameter("PROOF_BenchmarkRegenerate", Int_t(fRegenerate));
      fProof->SetParameter("PROOF_BenchmarkMode", fBenchmarkMode);
      fProof->SetParameter("PROOF_BenchmarkNFilesAWorker", fNFilesAWorker);
      fProof->SetParameter("PROOF_BenchmarkNFilesANode", fNFilesANode);
   }
   else{
      Error("SetInputParameters", "Proof not set, doing noting");
   }
   return;
}
*/
 
void TProofBench::ClearInputParameters(){

   if (fProof){
      fProof->DeleteParameters("PROOF_BenchmarkBaseDir");
      fProof->DeleteParameters("PROOF_BenchmarkRunType");
      fProof->DeleteParameters("PROOF_BenchmarkNHists");
      fProof->DeleteParameters("PROOF_BenchmarkHistType");
      fProof->DeleteParameters("PROOF_BenchmarkNTries");
      fProof->DeleteParameters("PROOF_BenchmarkNEvents");
      fProof->DeleteParameters("PROOF_BenchmarkDraw");
      fProof->DeleteParameters("PROOF_BenchmarkRegenerate");
//      fProof->DeleteParameters("fBenchmarkMode");
//      fProof->DeleteParameters("fNFilesAWorker");
//      fProof->DeleteParameters("fNFilesANode");
   }
   else{
      Error("ClearInputParameters", "Proof not set, doing noting");
   }
   return;
}

/*
Int_t TProofBench::CheckParameters(TString where){
   //TString fFilename,
   //TString basedir,

   Int_t val=0; //return value
   
   if(!fProof){
      Error(where.Data(), "Proof not set");
      return 1;
   }

   if ( fRunType==kRunCPUTest
//     || fRunType==kRunGenerateFileBench 
//     || fRunType==kRunGenerateFileCleanup 
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

   if (fRunType==kRunCleanup){
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
}*/

void TProofBench::DrawPerfProfiles(ERunType runtype, EBenchmarkMode mode)
{
//Draw performance profiles
//Input parameters:
//   runtype: Run type to build performance profiles for.
//            When kRunAll, this function is recursively called with runtype=kRunFullDataRead, 
//            kRunOptDataRead, kRunNoDataRead in turn.
//   mode: Benchmark mode to build performance profiles for.
//         Ignored when runtype==kRunCPUTest.
//Returns:
//   Nothing

   if (!(runtype==kRunNotSpecified
      || runtype==kRunCPUTest
      || runtype==kRunFullDataRead
      || runtype==kRunOptDataRead
      || runtype==kRunNoDataRead
      || runtype==kRunAll)){
      Error("BuildPerfProfiles", "Invalid run request");
      return; 
   }

   //split a single do-everything request into multiple single requests
   if (runtype==kRunAll){
      DrawPerfProfiles(kRunCPUTest, mode);
      DrawPerfProfiles(kRunFullDataRead, mode);
      DrawPerfProfiles(kRunOptDataRead, mode);
      DrawPerfProfiles(kRunNoDataRead, mode);
      return;
   }

   //continue checking input parameters
   if (runtype==kRunNotSpecified){
      if (fRunType==kRunNotSpecified){
         Error("DrawPerfProfiles", "Run type not specified; doing nothing and returning");
         return;
      }
      runtype=fRunType;
      Info("DrawPerfProfiles", "Run type is set to %d", runtype);
   }
   if (mode==kModeNotSpecified
      && (runtype==kRunFullDataRead || runtype==kRunOptDataRead || runtype==kRunNoDataRead)){
      if (fBenchmarkMode==kModeNotSpecified){
         Error("DrawPerfProfiles", "Mode not specified; doing nothing and returning");
         return;
      }
      mode=fBenchmarkMode;
      Info("DrawPerfProfiles", "Benchmark mode is set to %d", mode);
   }

   //build up profile name from runtype and mode
   TString pattern;
   TString hname_event, hname_io;
   switch (runtype){
   case kRunCPUTest:
      hname_event="hProfCPUTest";
      break;
   case kRunFullDataRead:
      hname_event=hname_io="hProfFullDataRead";
      break;
   case kRunOptDataRead:
      hname_event=hname_io="hProfOptDataRead";
      break;
   case kRunNoDataRead:
      hname_event=hname_io="hProfNoDataRead";
      break;
   default:
      break; //we should never get here
   }

   if (runtype!=kRunCPUTest){
      switch (mode){
      case kModeConstNFilesNode:
         hname_event+="ConstNFilesNode";
         hname_io+="ConstNFilesNode";
         break;
      case kModeConstNFilesWorker:
         hname_event+="ConstNFilesWorker";
         hname_io+="ConstNFilesWorker";
         break;
      case kModeVaryingNFilesWorker:
         hname_event+="VaryingNFilesWorker";
         hname_io+="VaryingNFilesWorker";
         break;
      default:
         break;
      }
   }
   hname_event+="Event";
   if (runtype!=kRunCPUTest){
      hname_io+="IO";
   }

   //create canvas
   if (!fCPerfProfiles){
      fCPerfProfiles=new TCanvas("CPerfProfiles", "Performance Profiles");
      fCPerfProfiles->Divide(2, 1);
   }
   
   fDirProofBench->cd();

   //event rate profile
   TProfile* hprof_event=dynamic_cast<TProfile*>(fDirProofBench->FindObject(hname_event.Data())); //list of object
   if (!hprof_event){
      //try reading key
      TKey* key=fDirProofBench->FindKey(hname_event.Data()); 
      if (key){
         hprof_event=dynamic_cast<TProfile*>(key->ReadObj());
      }
   }
   if (hprof_event){
      fCPerfProfiles->cd(1);
      hprof_event->Draw();
   }
   else{
      Error("DrawPerfProfiles", "Profile %s not found", hname_event.Data());
   }

   //IO rate profile
   if (runtype!=kRunCPUTest){
      TProfile* hprof_io=dynamic_cast<TProfile*>(fDirProofBench->FindObject(hname_io.Data())); //list of object
      if (!hprof_io){
         //try reading key
         TKey* key=fDirProofBench->FindKey(hname_io.Data()); 
         if (key){
            hprof_io=dynamic_cast<TProfile*>(key->ReadObj());
         }
      }
      if (hprof_io){
         fCPerfProfiles->cd(2);
         hprof_io->Draw();
      }
      else{
         Error("DrawPerfProfiles", "Profile %s not found", hname_io.Data());
      }
   }
   return;
}
