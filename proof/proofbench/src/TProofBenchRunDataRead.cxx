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
// TProofBenchRunDataRead                                               //
//                                                                      //
// I/O-intensive PROOF benchmark test reads in event files distributed  //
// on the cluster. Number of events processed per second and size of    //
// events processed per second are plotted against number of active     //
// workers. Performance rate for unit packets and performance rate      //
// for query are plotted.                                               //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "TProofBenchRunDataRead.h"
#include "TProofBenchRunCleanup.h"
#include "TProofBenchMode.h"
#include "TProofNode.h"
#include "TFile.h"
#include "TFileCollection.h"
#include "TFileInfo.h"
#include "TProof.h"
#include "TString.h"
#include "TDSet.h"
#include "Riostream.h"
#include "TMap.h"
#include "TTree.h"
#include "TH1.h"
#include "TCanvas.h"
#include "TProfile.h"
#include "TKey.h"
#include "TRegexp.h"
#include "TPerfStats.h"
#include "TSortedList.h"
#include "TPad.h"
#include "TEnv.h"
#include "TLeaf.h"
#include "TQueryResult.h"
#include "TMath.h"
#include "TStyle.h"

ClassImp(TProofBenchRunDataRead)

//______________________________________________________________________________
TProofBenchRunDataRead::TProofBenchRunDataRead(TProofBenchMode* mode,
                                               TProofBenchRunCleanup* runcleanup,
                                               TProofBenchRun::EReadType readtype,
                                               TString filename,
                                                Option_t* foption,
                                                TProof* proof,
                                                Int_t maxnworkers,
                                                Long64_t nevents,
                                                Int_t ntries,
                                                Int_t start,
                                                Int_t stop,
                                                Int_t step,
                                                Int_t draw,
                                                Int_t debug):
fProof(0),
fReadType(readtype),
fMode(mode),
fRunCleanup(runcleanup),
fNEvents(nevents),
fNTries(ntries),
fMaxNWorkers(0),
fStart(start),
fStop(stop),
fStep(step),
fDraw(draw),
fDebug(debug),
fFile(0),
fDirProofBench(0),
fWritable(0),
fNodes(0),
fPerfStats(0),
fListPerfProfiles(0),
fCPerfProfiles(0),
fName(0)
{

   //Default constructor

   fProof=proof?proof:gProof;

   //if (filename.Length()){
      if(!OpenFile(filename.Data(), foption)){//file open failed
         gDirectory->mkdir("ProofBench");
         gDirectory->cd("ProofBench");
         fDirProofBench=gDirectory; 
      }
   //}

   //set name
   fName="DataRead"+GetNameStem();

   if (maxnworkers>0){
      SetMaxNWorkers(maxnworkers);
   }
   else{
      SetMaxNWorkers("1x");
   }

   if (stop==-1){
      fStop=fMaxNWorkers;
   }

   FillNodeInfo();

   fPerfStats=new TList();

   gEnv->SetValue("Proof.StatsTrace",1);
   gStyle->SetOptStat(0);
}

//______________________________________________________________________________
TProofBenchRunDataRead::~TProofBenchRunDataRead()
{
   // Destructor
   fProof=0;
   fDirProofBench=0;
   if (fFile){
      fFile->Close();
      delete fFile;
   }
   if (fNodes){
      fNodes->SetOwner(kTRUE);
      SafeDelete(fNodes);
   }
   if (fPerfStats){
      fPerfStats->SetOwner(kTRUE);
      SafeDelete(fPerfStats);
   }
   if (fListPerfProfiles){
      fListPerfProfiles->SetOwner(kTRUE);
      SafeDelete(fListPerfProfiles);
   }
   if (fCPerfProfiles) delete fCPerfProfiles;
} 

//______________________________________________________________________________
void TProofBenchRunDataRead::Run(Long64_t nevents,
                                 Int_t ntries,
                                 Int_t start,
                                 Int_t stop,
                                 Int_t step,
                                 Int_t debug,
                                 Int_t draw)
{
   // Run benchmark
   // Input parameters
   //    nevents: Number of events to run per file. When it is -1, data member fNEvents is used. 
   //    ntries: Number of tries. When it is -1, data member fNTries is used.
   //    start: Start scan with 'start' workers.
   //    stop: Stop scan at 'stop workers.
   //    step: Scan every 'step' workers.
   //    debug: debug switch.
   //    draw: draw switch.
   // Returns
   //    Nothing

   if (!fProof){
      Error("RunBenchmark", "Proof not set");
      return;
   }

   nevents=-1?fNEvents:nevents;
   ntries=-1?fNTries:ntries;
   start=-1?fStart:start;
   stop=-1?fStop:stop;
   step=-1?fStep:step;
   debug=-1?fDebug:debug;
   draw=-1?fDraw:draw;

   if (!fListPerfProfiles){
      fListPerfProfiles=new TList();
   }

   Int_t quotient=(stop-start)/step;
   Int_t ndiv=quotient+1;
   Int_t ns_min=start;
   Int_t ns_max=quotient*step+start;

   //find perfstat event profile
   TString profile_perfstat_event_name=BuildProfileName("hProf", "PerfStat_Event");
   TProfile* profile_perfstat_event=(TProfile*)(fListPerfProfiles->FindObject(profile_perfstat_event_name.Data()));

   //book one if one does not exists yet or reset the existing one
   if (!profile_perfstat_event){
      TString profile_perfstat_event_title=BuildProfileTitle("Profile", "PerfStat Event");
      profile_perfstat_event= new TProfile(profile_perfstat_event_name, profile_perfstat_event_title, ndiv, ns_min-0.5, ns_max+0.5);

      profile_perfstat_event->GetXaxis()->SetTitle("Number of Slaves");
      profile_perfstat_event->GetYaxis()->SetTitle("#times10^{3} Events/sec");
      profile_perfstat_event->SetMarkerStyle(21);

      fListPerfProfiles->Add(profile_perfstat_event);
      Info("Run", "profile %s added", profile_perfstat_event_name.Data());
   }
   else{
      profile_perfstat_event->Reset();
   }
   //find perfstat IO profile
   TString profile_perfstat_IO_name=BuildProfileName("hProf", "PerfStat_IO");
   TProfile* profile_perfstat_IO=(TProfile*)(fListPerfProfiles->FindObject(profile_perfstat_IO_name.Data()));

   //book one if one does not exists yet or reset the existing one
   if (!profile_perfstat_IO){
      TString profile_perfstat_IO_title=BuildProfileTitle("Profile", "PerfStat IO");
      profile_perfstat_IO= new TProfile(profile_perfstat_IO_name, profile_perfstat_IO_title, ndiv, ns_min-0.5, ns_max+0.5);

      profile_perfstat_IO->GetXaxis()->SetTitle("Number of Slaves");
      profile_perfstat_IO->GetYaxis()->SetTitle("MB/sec");
      profile_perfstat_IO->SetMarkerStyle(21);

      fListPerfProfiles->Add(profile_perfstat_IO);
      Info("Run", "profile %s added", profile_perfstat_IO_name.Data());
   }
   else{
      profile_perfstat_IO->Reset();
   }

   //find queryresult profile
   TString profile_queryresult_event_name=BuildProfileName("hProf", "QueryResult_Event");
   Info("Run", "profile query result event name=%s", profile_queryresult_event_name.Data());

   TProfile* profile_queryresult_event=(TProfile*)(fListPerfProfiles->FindObject(profile_queryresult_event_name.Data()));

   if (!profile_queryresult_event){
      TString profile_queryresult_event_title=BuildProfileTitle("Profile", "QueryResult Event");
      profile_queryresult_event=new TProfile(profile_queryresult_event_name, profile_queryresult_event_title, ndiv, ns_min-0.5, ns_max+0.5);

      profile_queryresult_event->GetXaxis()->SetTitle("Number of Slaves");
      profile_queryresult_event->GetYaxis()->SetTitle("#times10^{3} Events/sec");
      profile_queryresult_event->SetMarkerStyle(22);

      fListPerfProfiles->Add(profile_queryresult_event);
      Info("Run", "profile %s added", profile_queryresult_event_name.Data());
   }
   else{
       profile_queryresult_event->Reset();
   }

   //find queryresult profile
   TString profile_queryresult_IO_name=BuildProfileName("hProf", "QueryResult_IO");
   Info("Run", "profile query result IO name=%s", profile_queryresult_IO_name.Data());
   TProfile* profile_queryresult_IO=(TProfile*)(fListPerfProfiles->FindObject(profile_queryresult_IO_name.Data()));

   if (!profile_queryresult_IO){
      TString profile_queryresult_IO_title=BuildProfileTitle("Profile", "QueryResult IO");
      profile_queryresult_IO=new TProfile(profile_queryresult_IO_name, profile_queryresult_IO_title, ndiv, ns_min-0.5, ns_max+0.5);

      profile_queryresult_IO->GetXaxis()->SetTitle("Number of Slaves");
      profile_queryresult_IO->GetYaxis()->SetTitle("MB/sec");
      profile_queryresult_IO->SetMarkerStyle(22);

      fListPerfProfiles->Add(profile_queryresult_IO);
      Info("Run", "profile %s added", profile_queryresult_IO_name.Data());
   }
   else{
       profile_queryresult_IO->Reset();
   }

   //get pad
   if (!fCPerfProfiles){
      TString canvasname=TString("Performance Profiles ")+GetName();
      fCPerfProfiles=new TCanvas(canvasname.Data(), canvasname.Data());
   }
   //divide the canvas as many as the number of profiles in the list
   Int_t nprofiles=fListPerfProfiles->GetSize();

   Info("Run", "# profiles=%d", nprofiles);
   if (nprofiles<=2){
      fCPerfProfiles->Divide(nprofiles);
   }
   else{
      Int_t nside = (Int_t)TMath::Sqrt((Float_t)nprofiles);
      nside = (nside*nside<nprofiles)?nside+1:nside;
      fCPerfProfiles->Divide(nside,nside);
   }

   //reset the list of performance statistics trees
   fPerfStats->Delete();

   for (Int_t nactive=start; nactive<=stop; nactive+=step) {
      for (Int_t j=0; j<ntries; j++) {

         Int_t npad=1; //pad number

         TString dsname; 
         Int_t nf=fMode->GetNFiles();
         TString smode=fMode->GetName();
   
         //cleanup run
         dsname.Form("DataSetEvent%s_%d_%d", smode.Data(), nactive, nf);
         fRunCleanup->SetDataSetCleanup(dsname);
         fRunCleanup->Run(nevents, 0, 0, 0, 0, debug, draw);

         //TString namestem=GetNameStem();
         DeleteParameters();
         SetParameters();
         fProof->SetParallel(nactive);

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

         Info("RunBenchmark", "Processing data set : %s", dsname.Data()); 
         Info("RunBenchmark", "Total number of events to process is %lld", nevents_total); 
         TTime starttime = gSystem->Now();
         fProof->Process(dsname.Data(), "TSelEvent", "", nevents_total);
         DeleteParameters();

         TTime endtime = gSystem->Now();

         TList* l = fProof->GetOutputList();

         Info("Run", "Printing out output list");
         l->Print("A");

         //save perfstats
         TString perfstats_name = "PROOF_PerfStats";
         TTree* t = dynamic_cast<TTree*>(l->FindObject(perfstats_name.Data()));
         if (t) {
            FillPerfStatProfiles(t, profile_perfstat_event, profile_perfstat_IO, nactive);
            fCPerfProfiles->cd(npad++);
            profile_perfstat_event->Draw();
            gPad->Update();
            fCPerfProfiles->cd(npad++);
            profile_perfstat_IO->Draw();
            gPad->Update();

            t->SetDirectory(fDirProofBench);

            //change the name
            TString newname=BuildNewPatternName(perfstats_name, nactive, j);
            t->SetName(newname);
            fPerfStats->Add(t);
            if (debug && fWritable){
               t->Write();
            }
         } else {
            cout << perfstats_name.Data() << " tree not found" << endl << flush;
         }

         //performance measures from TQueryResult

         TQueryResult* queryresult=fProof->GetQueryResult();
         TDatime qr_start=queryresult->GetStartTime();
         TDatime qr_end=queryresult->GetEndTime();
         Float_t qr_init=queryresult->GetInitTime(); 
         Float_t qr_proc=queryresult->GetProcTime();
         Float_t qr_usedcpu=queryresult->GetUsedCPU();
         Long64_t qr_bytes=queryresult->GetBytes();

         Long64_t qr_entries=queryresult->GetEntries();

         //calculate event rate, fill and draw
         Long64_t qr_entriesinkilo=0;
         Double_t qr_eventrate=0;

         const Long64_t Lkilobytes=1024;
         const Double_t Dkilobytes=1024;

         if (qr_entries>1024*Lkilobytes){
            qr_entriesinkilo=qr_entries/Lkilobytes;
            qr_eventrate=qr_entriesinkilo/Double_t(qr_init+qr_proc);
         }
         else{
            qr_eventrate=qr_entries/Double_t(qr_init+qr_proc)/Dkilobytes;
         }

         profile_queryresult_event->Fill(nactive, qr_eventrate);
         fCPerfProfiles->cd(npad++);
         profile_queryresult_event->Draw();
         gPad->Update();

         //calculate IO rate, fill and draw
         Long64_t qr_bytesinmega=0;
         Double_t qr_IOrate=0;

         const Long64_t Lmegabytes=1024*1024;
         const Double_t Dmegabytes=1024*1024;

         if (qr_IOrate>1024*Lmegabytes){
            qr_bytesinmega=qr_bytes/Lmegabytes;
            qr_IOrate=qr_bytesinmega/Double_t(qr_init+qr_proc);
         }
         else{
            qr_IOrate=qr_bytes/Double_t(qr_init+qr_proc)/Dmegabytes;
         }

         profile_queryresult_IO->Fill(nactive, qr_IOrate);
         fCPerfProfiles->cd(npad++);
         profile_queryresult_IO->Draw();
         gPad->Update();

         //save outputhistos 
         if (debug){
            TString ptdist_name = "pt_dist";
            TH1* h = dynamic_cast<TH1*>(l->FindObject(ptdist_name.Data()));
            if (h) {
               TH1 *hnew = (TH1*)h->Clone("hnew");

               hnew->SetDirectory(fDirProofBench);
               TString origname = h->GetName();

               TString newname=BuildNewPatternName(ptdist_name, nactive, j);

               hnew->SetName(newname);
               if (fWritable){
                  hnew->Write();
                  delete hnew;
               }
            } else {
               Error("Run", "histogram %s not found", ptdist_name.Data());
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
               TString newname=BuildNewPatternName(tracksdist_name, nactive, j);
 
               hnew->SetName(newname);
               if (fWritable){
                  hnew->Write();
                  delete hnew;
               }
            } 
            else {
               Error("RunBenchmark", "histogram %s not found", tracksdist_name.Data());
            }
         }
      }//for ntries
   }//for number of workers
   
   if (fWritable){
      profile_perfstat_event->Write();
      profile_perfstat_IO->Write();
      profile_queryresult_event->Write();
      profile_queryresult_IO->Write();
   }
}

//______________________________________________________________________________
void TProofBenchRunDataRead::FillPerfStatProfiles(TTree* t, TProfile* profile_event, TProfile* profile_IO, Int_t nactive)
{

   // Fill performance profiles using tree 't'(PROOF_PerfStats).
   // Input parameters
   //    t: Proof output tree (PROOF_PerfStat) containing performance statistics.
   //    profile_event: Event-rate profile to be filled up with information from tree 't'.
   //    profile_IO: IO-rate profile to be filled up with information from tree 't'.
   //    nactive: Number of active workers processed the query.
   // Return
   //    Nothing

   Int_t nevents_holder;
   Int_t bytes_holder;
   Float_t time_holder;

   Int_t max_slaves=0;

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

      //Printf("k:%lld fTimeStamp=%lf fEvtNode=%s pe.fType=%d fSlaveName=%s fNodeName=%s fFileName=%s fFileClass=%s fSlave=%s fEventsProcessed=%lld fBytesRead=%lld fLen=%lld fLatency=%lf fProcTime=%lf fCpuTime=%lf fIsStart=%d fIsOk=%d",k, pe.fTimeStamp.GetSec() + 1e-9*pe.fTimeStamp.GetNanoSec(), pe.fEvtNode.Data(), pe.fType, pe.fSlaveName.Data(), pe.fNodeName.Data(), pe.fFileName.Data(), pe.fFileClass.Data(), pe.fSlave.Data(), pe.fEventsProcessed, pe.fBytesRead, pe.fLen, pe.fLatency, pe.fProcTime, pe.fCpuTime, pe.fIsStart, pe.fIsOk);

      if (pe.fType==TVirtualPerfStats::kPacket){
         nevents_kPacket+=pe.fEventsProcessed;
         bytesread_kPacket+=pe.fBytesRead;
      }
      if (pe.fType==TVirtualPerfStats::kRate){
         //printf("adding pe.fEventsProcessed=%lld\n", pe.fEventsProcessed);
         nevents_kRate+=pe.fEventsProcessed;
         bytesread_kRate+=pe.fBytesRead;
      }
     
      ///if (!started) {
      //  if (pe.fType==TVirtualPerfStats::kPacket) {
      //     start = pe.fTimeStamp.GetSec()
       //             + 1e-9*pe.fTimeStamp.GetNanoSec()
       //             - pe.fProcTime;
       //     started=kTRUE;
       //  }
      //} else {
      //   if (pe.fType==TVirtualPerfStats::kPacket) {
      //      end = pe.fTimeStamp.GetSec()
      //            + 1e-9*pe.fTimeStamp.GetNanoSec();
      //   }
      //}
      //skip information from workers
      if (pe.fEvtNode.Contains(".")) continue;
      if (pe.fType==TVirtualPerfStats::kStart) start= pe.fTimeStamp.GetSec()+1e-9*pe.fTimeStamp.GetNanoSec();
      if (pe.fType==TVirtualPerfStats::kStop) end= pe.fTimeStamp.GetSec()+1e-9*pe.fTimeStamp.GetNanoSec();
   }
     
   //if (nevents_kPacket!=fNEvents){
    //  Error("BuildTimingTree", "Number of events processed is different from the number of events in the file");
     // return 0;
   //}

   nevents_holder=nevents_kPacket;
   bytes_holder=bytesread_kPacket;
   time_holder=end-start;

   Double_t event_rate, IO_rate;

   event_rate=nevents_holder/time_holder/1000.; 
   profile_event->Fill(Double_t(nactive), event_rate);

   IO_rate=bytes_holder/time_holder/(1024.*1024.); 
   profile_IO->Fill(Double_t(nactive), IO_rate);

   return;
}

//______________________________________________________________________________
void TProofBenchRunDataRead::Print(Option_t* option)const{

   Printf("Name=%s", fName.Data());
   if (fProof) fProof->Print(option);
   if (fMode) fMode->Print();
   Printf("fReadType=%s%s", "k", GetNameStem().Data());
   if (fRunCleanup) fRunCleanup->Print(option);
   Printf("fNEvents=%lld", fNEvents);
   Printf("fNTries=%d", fNTries);
   Printf("fMaxNWorkers=%d", fMaxNWorkers);
   Printf("fStart=%d", fStart);
   Printf("fStop=%d", fStop);
   Printf("fStep=%d", fStep);
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
   if (fNodes) fNodes->Print(option);
   if (fPerfStats) fPerfStats->Print(option);
   if (fListPerfProfiles) fListPerfProfiles->Print(option);

   if (fCPerfProfiles){
      Printf("Performance Profiles Canvas: Name=%s Title=%s", 
              fCPerfProfiles->GetName(), fCPerfProfiles->GetTitle());
   }
}

//______________________________________________________________________________
void TProofBenchRunDataRead::DrawPerfProfiles()
{
   // Get canvas
   if (!fCPerfProfiles){
      TString canvasname=TString("Performance Profiles ")+GetName();
      fCPerfProfiles=new TCanvas(canvasname.Data(), canvasname.Data());
   }

   fCPerfProfiles->Clear();

   // Divide the canvas as many as the number of profiles in the list
   Int_t nprofiles=fListPerfProfiles->GetSize();
   if (nprofiles<=2){
      fCPerfProfiles->Divide(nprofiles);
   }
   else{
      Int_t nside = (Int_t)TMath::Sqrt((Float_t)nprofiles);
      nside = (nside*nside<nprofiles)?nside+1:nside;
      fCPerfProfiles->Divide(nside,nside);
   }

   Int_t npad=1;
   TIter nxt(fListPerfProfiles);
   TProfile* profile=0;
   while ((profile=(TProfile*)(nxt()))){
      fCPerfProfiles->cd(npad++);
      profile->Draw();
      gPad->Update();
   }
   return;
}

//______________________________________________________________________________
void TProofBenchRunDataRead::SetMode(TProofBenchMode* mode)
{
   fMode=mode;
}

//______________________________________________________________________________
void TProofBenchRunDataRead::SetRunCleanup(TProofBenchRunCleanup* runcleanup)
{
   fRunCleanup=runcleanup;
}

//______________________________________________________________________________
void TProofBenchRunDataRead::SetReadType(TProofBenchRun::EReadType readtype)
{
   fReadType=readtype;
}

//______________________________________________________________________________
void TProofBenchRunDataRead::SetNEvents(Long64_t nevents)
{
   fNEvents=nevents;
}

//______________________________________________________________________________
void TProofBenchRunDataRead::SetNTries(Int_t ntries)
{
   fNTries=ntries;
}

//______________________________________________________________________________
void TProofBenchRunDataRead::SetMaxNWorkers(Int_t maxnworkers)
{
  fMaxNWorkers=maxnworkers;
}

//______________________________________________________________________________
void TProofBenchRunDataRead::SetMaxNWorkers(TString sworkers)
{
   //Set the maximum number of workers for benchmark test
   //Input parameters
   //    sworkers: can be "1x", "2x" and so on, where total number of workers is set 
   //              to 1*no_total_workers, 2*no_total_workers respectively.
   //              For now only "1x" is supported
   // Return
   //    Nothing

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

//______________________________________________________________________________
void TProofBenchRunDataRead::SetStart(Int_t start)
{
   fStart=start;
}

//______________________________________________________________________________
void TProofBenchRunDataRead::SetStop(Int_t stop)
{
   fStop=stop;
}

//______________________________________________________________________________
void TProofBenchRunDataRead::SetStep(Int_t step)
{
   fStep=step;
}

//______________________________________________________________________________
void TProofBenchRunDataRead::SetDraw(Int_t draw)
{
   fDraw=draw;
}

//______________________________________________________________________________
void TProofBenchRunDataRead::SetDebug(Int_t debug)
{
   fDebug=debug;
}

//______________________________________________________________________________
TFile* TProofBenchRunDataRead::OpenFile(const char* filename,
                                        Option_t* option,
                                        const char* ftitle,
                                        Int_t compress)
{
   // Opens a file which output profiles and/or intermediate files (trees, histograms when debug is set)
   // are to be written to. Makes a directory named "ProofBench" if possible and changes to the directory.
   // If directory ProofBench already exists, change to the directory. If the directory can not be created,
   // make a directory Rint:/ProofBench and change to the directory.
   // Input parameters
   //    filename: Name of the file to open
   //    option: Option for TFile::Open(...) function
   //    ftitle: Title parameter for TFile::Open(...) function
   //    compress: Compression parameter for TFile::Open(...) function
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

   if (fFile->IsZombie()){//open failed
      Error("FileOpen", "Cannot open file: %s", sfilename.Data());
      fFile->Close();
      fFile=0;
      dirsav->cd();
      return 0;
   }
   else{//open succeeded
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
void TProofBenchRunDataRead::SetDirProofBench(TDirectory* dir)
{
   fDirProofBench=dir;
}

//______________________________________________________________________________
TProofBenchMode* TProofBenchRunDataRead::GetMode()const
{
   return fMode;
}

//______________________________________________________________________________
TProofBenchRunCleanup* TProofBenchRunDataRead::GetRunCleanup()const
{
   return fRunCleanup;
}

//______________________________________________________________________________
TProofBenchRun::EReadType TProofBenchRunDataRead::GetReadType()const
{
   return fReadType;
}

//______________________________________________________________________________
Long64_t TProofBenchRunDataRead::GetNEvents()const
{
   return fNEvents;
}

//______________________________________________________________________________
Int_t TProofBenchRunDataRead::GetNTries()const
{
   return fNTries;
}

//______________________________________________________________________________
Int_t TProofBenchRunDataRead::GetMaxNWorkers()const
{
   return fMaxNWorkers;
}

//______________________________________________________________________________
Int_t TProofBenchRunDataRead::GetStart()const
{
   return fStart;
}

//______________________________________________________________________________
Int_t TProofBenchRunDataRead::GetStop()const
{
   return fStop;
}

//______________________________________________________________________________
Int_t TProofBenchRunDataRead::GetStep()const
{
   return fStep;
}

//______________________________________________________________________________
Int_t TProofBenchRunDataRead::GetDraw()const
{
   return fDraw;
}

//______________________________________________________________________________
Int_t TProofBenchRunDataRead::GetDebug()const
{
   return fDebug;
}

//______________________________________________________________________________
TFile* TProofBenchRunDataRead::GetFile()const
{
   return fFile;
}

//______________________________________________________________________________
TDirectory* TProofBenchRunDataRead::GetDirProofBench()const
{
   return fDirProofBench;
}

//______________________________________________________________________________
TList* TProofBenchRunDataRead::GetListOfPerfStats()const
{
   return fPerfStats;
}

//______________________________________________________________________________
TList* TProofBenchRunDataRead::GetListPerfProfiles()const
{
   return fListPerfProfiles;
}

//______________________________________________________________________________
TCanvas* TProofBenchRunDataRead::GetCPerfProfiles()const
{
   return fCPerfProfiles;
}

//______________________________________________________________________________
const char* TProofBenchRunDataRead::GetName()const
{
   return fName.Data();
}

//______________________________________________________________________________
TString TProofBenchRunDataRead::GetNameStem()const
{
   TString namestem;
   switch (fReadType){
   case TProofBenchRun::kReadFull:
      namestem="Full";
      break;
   case TProofBenchRun::kReadOpt:
      namestem="Opt";
      break;
   case TProofBenchRun::kReadNo:
      namestem="No";
      break;
   default:
      break;
   }
    return namestem;
}

//______________________________________________________________________________
Int_t TProofBenchRunDataRead::FillNodeInfo()
{
   // Re-Generate the list of worker node info (fNodes)
   // The existing info is always removed.
   // Return
   //    0 if ok
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

//______________________________________________________________________________
Int_t TProofBenchRunDataRead::SetParameters()
{
   if (!fProof){
      Error("SetParameters", "Proof not set; Doing nothing");
      return 1;
   }
   
   fProof->SetParameter("PROOF_BenchmarkReadType", Int_t(fReadType));
   fProof->SetParameter("PROOF_BenchmarkDraw", Int_t(fDraw));
   fProof->SetParameter("PROOF_BenchmarkDebug", Int_t(fDebug));
   return 0;
}

//______________________________________________________________________________
Int_t TProofBenchRunDataRead::DeleteParameters(){
   if (!fProof){
      Error("DeleteParameters", "Proof not set; Doing nothing");
      return 1;
   }
   fProof->DeleteParameters("PROOF_BenchmarkReadType");
   fProof->DeleteParameters("PROOF_BenchmarkDraw");
   fProof->DeleteParameters("PROOF_BenchmarkDebug");
   return 0;
}

//______________________________________________________________________________
TString TProofBenchRunDataRead::BuildPatternName(const TString& objname, const TString& delimiter){
  
   if (!fMode){
      Error("BuildPatternName", "Mode is not set");
      //return 0;
      return "";
   }
   TString newname(objname);
   newname+=delimiter;
   newname+=GetName();
   newname+=delimiter;
   newname+=fMode->GetName();
   //return newname.Data();
   return newname;
}

//______________________________________________________________________________
TString TProofBenchRunDataRead::BuildNewPatternName(const TString& objname, Int_t nactive, Int_t tries, const TString& delimiter){
  
   if (!fMode){
      Error("BuildNewPatternName", "Mode is not set");
      //return 0;
      return "";
   }
   TString newname(BuildPatternName(objname, delimiter));
   newname+=delimiter;
   newname+=nactive;
   newname+="Slaves_Run";
   newname+=tries;
   //return newname.Data();
   return newname;
}

//______________________________________________________________________________
TString TProofBenchRunDataRead::BuildProfileName(const TString& objname, const TString& type, const TString& delimiter){
   if (!fMode){
      Error("BuildProfileName", "Mode is not set");
      //return 0;
      return "";
   }
   TString newname(BuildPatternName(objname, delimiter));
   newname+=delimiter;
   newname+=type;
   //return newname.Data();
   return newname;
}

//______________________________________________________________________________
TString TProofBenchRunDataRead::BuildProfileTitle(const TString& objname, const TString& type, const TString& delimiter){
   if (!fMode){
      Error("BuildProfileTitle", "Mode is not set");
      //return 0;
      return "";
   }
   TString newname(BuildPatternName(objname, delimiter));
   newname+=delimiter;
   newname+=type;
   //return newname.Data();
   return newname;
}
