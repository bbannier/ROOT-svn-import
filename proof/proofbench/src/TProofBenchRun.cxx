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

#include "TProofBenchRun.h"
#include "TProofNode.h"
#include "TProofBenchMode.h"
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

ClassImp(TProofBenchRun)

//TProofBenchRun::TProofBenchRun()
//{
//Default constructor

   //if (filename.Length()){
   //   OpenFile(filename.Data(), foption);
   //}

   //SetBaseDir(basedir);

   /*if (maxnworkers>0){ 
      SetMaxNWorkers(maxnworkers);
   }
   else{
      SetMaxNWorkers("1x");
   }*/

   //SetNTries(ntries);
   //SetNEvents(nevents);
   //SetStepSize(stepsize);
   //SetStart(start);
   //SetDraw(draw);
   //SetDebug(debug);
   //FillNodeInfo();

   /*
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
   }*/

   //gEnv->SetValue("Proof.StatsHist",0);
   //gEnv->SetValue("Proof.StatsTrace",1);
   //gEnv->SetValue("Proof.SlaveStatsTrace",0);
//}

TProofBenchRun::~TProofBenchRun()
{
//destructor

   /*if (fFile){ 
      fFile->Close(); 
      delete fFile;
   }*/
   //fProof=0;
   //if (fNodes) delete fNodes;
} 

/* void TProofBenchRun::Run(Long64_t nevents,
                         TProof* proof,
                         Int_t start,
                         Int_t stop,
                         Int_t step,
                         Int_t ntries,
                         Int_t debug,
                         Int_t draw,
                         Int_t write,
                         TDirectory* dir)
{
//Run benchmark
//Input parameter:
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

   Int_t filewritable=0;
   if (fFile){
      TString soption=fFile->GetOption(); 
      soption.ToLower(); 
      if (soption.Contains("create") || soption.Contains("update")){
         filewritable=1;
      }
   }

   DoRun(nevents,
         fProof,
         start,
         stop,
         step,
         ntries,
         debug,
         draw,
         0,
         0);

   return;
}*/

//_________________________________________________________________________________
/*Int_t TProofBenchRun::FillNodeInfo()
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
}*/

/*void TProofBenchRun::BuildPerfProfiles() {
//Build performance profiles
//Input parameters:
//   runtype: Run type to build performance profiles for.
//            When kRunAll, this function is recursively called with runtype=kRunFullDataRead, 
//            kRunOptDataRead, kRunNoDataRead in turn.
//   mode: Benchmark mode to build performance profiles for.
//         Ignored when runtype==kRunCPUTest.
//Returns:
//   Nothing

   Int_t filewritable=0;
   if (fFile){
      TString soption=fFile->GetOption();
      soption.ToLower();
      if (soption.Contains("create") || soption.Contains("update")){
         filewritable=1;
      }
   }

   DoBuildPerfProfiles(filewritable, fDirProofBench, fMaxNWorkers);
   return;
}*/

/*void TProofBenchRun::Print(Option_t* option)const{

   if (fFile){
       fFile->Print(option);
       fFile->ls(option);
   }
   else{
      Printf("No file open"); 
   }

   if (fProof) fProof->Print(option);
   Printf("fBaseDir=\"%s\"", fBaseDir.Data()); 

   Printf("fMaxNWorkers=%d", fMaxNWorkers);

   if (fNodes) fNodes->Print(option);

   DoPrint(option);
}*/

/*TFile* TProofBenchRun::OpenFile(const char* filename, 
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
         SetDirProofBench(gDirectory);
      }
      else{
          gDirectory->cd("Rint:/");
          gDirectory->mkdir("ProofBench");
          gDirectory->cd("ProofBench");
          SetDirProofBench(gDirectory);
      }
      return fFile;
   }
}*/

/* TFile* TProofBenchRun::SetFile(TFile* ftmp){
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

/*void TProofBenchRun::SetInputParameters(){

   if (fProof){
      fProof->SetParameter("PROOF_BenchmarkBaseDir", fBaseDir.Data());
      fProof->SetParameter("PROOF_BenchmarkRunType", fRun);
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
 
/*void TProofBenchRun::ClearInputParameters(){

   if (fProof){
      fProof->DeleteParameters("PROOF_BenchmarkBaseDir");
      fProof->DeleteParameters("PROOF_BenchmarkRunType");
      fProof->DeleteParameters("PROOF_BenchmarkNHists");
      fProof->DeleteParameters("PROOF_BenchmarkHistType");
      fProof->DeleteParameters("PROOF_BenchmarkNTries");
      fProof->DeleteParameters("PROOF_BenchmarkNEvents");
      fProof->DeleteParameters("PROOF_BenchmarkDraw");
      //fProof->DeleteParameters("PROOF_BenchmarkRegenerate");
//      fProof->DeleteParameters("fBenchmarkMode");
//      fProof->DeleteParameters("fNFilesAWorker");
//      fProof->DeleteParameters("fNFilesANode");
   }
   else{
      Error("ClearInputParameters", "Proof not set, doing noting");
   }
   return;
}*/

/*
Int_t TProofBenchRun::CheckParameters(TString where){
   //TString fFilename,
   //TString basedir,

   Int_t val=0; //return value
   
   if(!fProof){
      Error(where.Data(), "Proof not set");
      return 1;
   }

   if ( fRun==kRunCPUTest
//     || fRun==kRunGenerateFileBench 
//     || fRun==kRunGenerateFileCleanup 
     || fRun==kRunCleanup
     || fRun==kRunFullDataRead
     || fRun==kRunOptDataRead
     || fRun==kRunNoDataRead){
   }
   else{
      Error(where.Data(), "fRun not set; fRun=%d",fRun);
      val=1;
   }

   if (fRun==kRunCPUTest){
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

   if (fRun==kRunCleanup){
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

   if (fRun==kRunFullDataRead
    || fRun==kRunOptDataRead
    || fRun==kRunNoDataRead){
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

//void TProofBenchRun::DrawPerfProfiles()
//{
//Draw performance profiles
//Input parameters:
//   runtype: Run type to build performance profiles for.
//            When kRunAll, this function is recursively called with runtype=kRunFullDataRead, 
//            kRunOptDataRead, kRunNoDataRead in turn.
//   mode: Benchmark mode to build performance profiles for.
//         Ignored when runtype==kRunCPUTest.
//Returns:
//   Nothing

   //build up profile name from runtype and mode
   /*
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
   }*/

   //hname_event=GetPProfileName("Event");
   //hname_event=GetPProfileName("IO");
   //create canvas
/*
   if (!fCPerfProfiles){
      fCPerfProfiles=new TCanvas("CPerfProfiles", "Performance Profiles");
      fCPerfProfiles->Divide(2, 1);
   }
   
   fDirProofBench->cd();

   //event rate profile
   TString profname_event=BuildName("Prof", "Event");
   TProfile* hprof_event=dynamic_cast<TProfile*>(fDirProofBench->FindObject(profname_event.Data())); //list of object
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
*/
   //DoDrawPerfProfiles();
//}

/*
const char* TProofBenchRun::BuildName(const char* objname, Int_t nactive, Int_t tries){
   TString newname(objname);
   newname+="_";
   newname+=fRun->GetName();
   if (fMode){
      newname+="_";
      newname+=fMode->GetName();
   }
   newname+="_";
   newname+=nactive;
   newname+="Slaves_Run";
   newname+=tries;
   return newname.Data();
}

const char* TProofBenchRun::BuildProfileName(const char* objname, const char* type){
   TString newname(objname);
   newname+="_";
   newname+=fRun->GetName();
   if (fMode){
      newname+="_";
      newname+=fMode->GetName();
   }
   newname+="_";
   newname+=type;
   return newname.Data();
}

const char* TProofBenchRun::BuildProfileTitle(const char* objname, const char* type){
   TString newname(objname);
   newname+=" ";
   newname+=fRun->GetName();
   if (fMode){
      newname+=" ";
      newname+=fMode->GetName();
   }
   newname+=" ";
   newname+=type;
   return newname.Data();
}
*/

/*
TCanvas* TProofBenchRun::GetCPerfProfiles(){
   //Create a canvas if it does not exist. Then return it.
   if (!fCPerfProfiles){
      fCPerfProfiles=new TCanvas("Performance Profiles");
   }
   return fCPerfProfiles;
}*/

/*void TProofBenchRun::SetBaseDir(TString basedir)
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
}*/

/* void TProofBenchRun::SetNHists(Long_t nhists)
{
   fNHists=nhists>-1?nhists: fNHists;
}*/

/*void TProofBenchRun::SetMaxNWorkers(Int_t maxnworkers)
{
  fMaxNWorkers=maxnworkers>-1?maxnworkers:fMaxNWorkers;
}

void TProofBenchRun::SetMaxNWorkers(TString sworkers)
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
}*/

/*void TProofBenchRun::SetNTries(Int_t ntries)
{
   fNTries=ntries>-1?ntries:fNTries;
}*/

/*void TProofBenchRun::SetNFilesAWorker(Int_t nfilesaworker)
{
   fNFilesAWorker=nfilesaworker>-1?nfilesaworker:fNFilesAWorker;
}*/

/*void TProofBenchRun::SetNEvents(Long64_t nevents)
{
   fNEvents=nevents>-1?nevents:fNEvents;
}*/

/*void TProofBenchRun::SetStart(Int_t start)
{
   fStart=start>-1?start:fStart;
}

void TProofBenchRun::SetStep(Int_t step)
{
   fStep=step>-1?step:fStep;
}

void TProofBenchRun::SetDraw(Int_t draw)
{
   fDraw=draw>-1?draw:fDraw;
}

void TProofBenchRun::SetDebug(Int_t debug)
{
   fDebug=debug>-1?debug:fDebug;
}
*/

//const TString TProofBenchRun::PERF_PATTERN="PROOF_PerfStat";
