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
// TProofBenchRunDataRead                                               //
//                                                                      //
// TProofBenchRunDataRead is ...                                        //
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

TProofBenchRunDataRead::TProofBenchRunDataRead(TProofBenchMode* mode,
                                               TProofBenchRunCleanup* runcleanup,
                                               TProofBenchRun::EReadType readtype,
                                               TString filename,//output file where benchmark performance plot will be written to, 
                                                                     //user has to provide one
                                                Option_t* foption, //option to TFile() 
                                                TProof* proof,
                                                Int_t maxnworkers,//maximum number of workers to be tested. 
                                                                     //If not set (default), 2 times the number of total workers in the cluster available
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
fProfEvent(0),
fProfIO(0),
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

TProofBenchRunDataRead::~TProofBenchRunDataRead()
{
//destructor
   fProof=0;
   if (fProfEvent) delete fProfEvent;
   if (fProfIO) delete fProfIO;
   fDirProofBench=0;
   if (fFile){
      fFile->Close();
      delete fFile;
   }
   if (fNodes) delete fNodes;
   if (fCPerfProfiles) delete fCPerfProfiles;
} 

void TProofBenchRunDataRead::Run(Long64_t nevents,
                                 Int_t ntries,
                                 Int_t start,
                                 Int_t stop,
                                 Int_t step,
                                 Int_t debug,
                                 Int_t draw)
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
      fCPerfProfiles=new TCanvas("CPerfProfiles");
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

         //Info("Run", "start="); qr_start.Print();
         //Info("Run", "end="); qr_end.Print();
         //Info("Run", "init=%f proc=%f used cpu=%f", qr_init, qr_proc, qr_usedcpu);

         //calculate event rate, fill and draw
         Double_t qr_eventrate=qr_entries/Double_t(qr_init+qr_proc);

         profile_queryresult_event->Fill(nactive, qr_eventrate);
         fCPerfProfiles->cd(npad++);
         profile_queryresult_event->Draw();
         gPad->Update();

         //calculate IO rate, fill and draw
         Double_t qr_IOrate=qr_bytes/Double_t(qr_init+qr_proc);

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

void TProofBenchRunDataRead::FillPerfStatProfiles(TTree* t, TProfile* profile_event, TProfile* profile_IO, Int_t nactive)
{
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

      Printf("k:%lld fTimeStamp=%lf fEvtNode=%s pe.fType=%d fSlaveName=%s fNodeName=%s fFileName=%s fFileClass=%s fSlave=%s fEventsProcessed=%lld fBytesRead=%lld fLen=%lld fLatency=%lf fProcTime=%lf fCpuTime=%lf fIsStart=%d fIsOk=%d",k, pe.fTimeStamp.GetSec() + 1e-9*pe.fTimeStamp.GetNanoSec(), pe.fEvtNode.Data(), pe.fType, pe.fSlaveName.Data(), pe.fNodeName.Data(), pe.fFileName.Data(), pe.fFileClass.Data(), pe.fSlave.Data(), pe.fEventsProcessed, pe.fBytesRead, pe.fLen, pe.fLatency, pe.fProcTime, pe.fCpuTime, pe.fIsStart, pe.fIsOk);

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
     
   //printf("nevents_kPacket=%lld, nevents_kRate=%lld\n", nevents_kPacket, nevents_kRate);
   //printf("bytesread_kPacket=%lld, bytesread_kRate=%lld\n", bytesread_kPacket, bytesread_kRate);

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


void TProofBenchRunDataRead::BuildPerfProfiles(Int_t start,
                                               Int_t stop,
                                               Int_t step,
                                               Int_t debug)
{

//Build performance profiles
//Input parameters:
//   runtype: Run type to build performance profiles for.
//            When kRunAll, this function is recursively called with runtype=kRunFullDataRead, 
//            kRunOptDataRead, kRunNoDataRead in turn.
//   mode: Benchmark mode to build performance profiles for.
//         Ignored when runtype==kRunCPUTest.
//Returns:
//   Nothing

   start=-1?fStart:start;
   stop=-1?fStop:stop;
   step=-1?fStep:step;
   debug=-1?fDebug:debug;

   Int_t quotient=(stop-start)/step;
   Int_t ndiv=quotient+1;
   Int_t ns_min=start;
   Int_t ns_max=quotient*step+start;

   if (!fProfEvent){
      TString profilename=BuildProfileName("hProf", "Event");
      TString profiletitle=BuildProfileTitle("Profile", "Event");
      fProfEvent= new TProfile(profilename, profiletitle, ndiv, ns_min-0.5, ns_max+0.5);
   }
   else{
      fProfEvent->Reset();
   }

   if (!fProfIO){
      TString profilename=BuildProfileName("hProf", "IO");
      TString profiletitle=BuildProfileTitle("Profile", "IO");
      fProfIO= new TProfile(profilename, profiletitle, ndiv, ns_min-0.5, ns_max+0.5);
   }
   else{
      fProfIO->Reset();
   }

   TString hname_event, hname_io, htitle_event, htitle_io;
   TString smode=fMode->GetName();
   TString namestem=GetNameStem();
   TString perfstats_name = "PROOF_PerfStats";

   TString pattern=BuildPatternName(perfstats_name.Data());
   //pattern=perfstats_name+"_"+namestem+"DataRead"+"_"+smode;

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

   TIter nxt(fPerfStats);
   TTree* t=0;
   Int_t max_slaves=0;
      
   while ((t=dynamic_cast<TTree*>(nxt()))){

      TString treename=t->GetName();
      //Info("BuildPerfProfiles", "%s", treename.Data());

      if (!treename.Contains(TRegexp(pattern))){
         continue;
      }

      if(!t->FindBranch("PerfEvents")) {
         continue;
      }

      //parse name to get number of slaves and run
      treename.Remove(0, treename.Index(TRegexp("[0-9]*Slaves")));

      // this should be the number of slaves
      ns_holder=treename.Atoi();

      //skip run with the number of slaves of no interest
      if (ns_holder<ns_min || ns_holder>ns_max){
         continue;
      }
      if ((ns_holder-start)%step){
         continue;
      }

      treename.Remove(0, treename.Index(TRegexp("Run[0-9]*")));
      treename.Remove(0, treename.Index(TRegexp("[0-9]")));

      //this should be run (try) number
      run_holder=treename.Atoi();

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

         Printf("k:%lld fTimeStamp=%lf fEvtNode=%s pe.fType=%d fSlaveName=%s fNodeName=%s fFileName=%s fFileClass=%s fSlave=%s fEventsProcessed=%lld fBytesRead=%lld fLen=%lld fLatency=%lf fProcTime=%lf fCpuTime=%lf fIsStart=%d fIsOk=%d",k, pe.fTimeStamp.GetSec() + 1e-9*pe.fTimeStamp.GetNanoSec(), pe.fEvtNode.Data(), pe.fType, pe.fSlaveName.Data(), pe.fNodeName.Data(), pe.fFileName.Data(), pe.fFileClass.Data(), pe.fSlave.Data(), pe.fEventsProcessed, pe.fBytesRead, pe.fLen, pe.fLatency, pe.fProcTime, pe.fCpuTime, pe.fIsStart, pe.fIsOk);

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
   //TProfile* prof_event=0;
   //TProfile* prof_io=0;

   Double_t event_rate, IO_rate;
   Int_t entries=timing_tree->GetEntries();

   for (int i=0; i<entries; i++){
      timing_tree->GetEntry(i); 
      //printf("ns_holder=%d, run_holder=%d time_holder=%f\n", ns_holder, run_holder, time_holder);
      event_rate=nevents_holder/time_holder/1000.; 
      fProfEvent->Fill(Double_t(ns_holder), event_rate);

      IO_rate=bytes_holder/time_holder/(1024.*1024.); 
      fProfIO->Fill(Double_t(ns_holder), IO_rate);
   }
   fProfEvent->GetXaxis()->SetTitle("Number of Slaves");
   fProfEvent->GetYaxis()->SetTitle("#times10^{3} Events/sec");
   fProfEvent->SetMarkerStyle(21);
   if (fWritable){
      fProfEvent->Write();
   }

   fProfIO->GetXaxis()->SetTitle("Number of Slaves");
   fProfIO->GetYaxis()->SetTitle("MB/sec");
   fProfIO->SetMarkerStyle(22);
   if (fWritable){
      fProfIO->Write();
   }

   delete timing_tree;
   return;
}

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
   if (fProfEvent) fProfEvent->Print(option);
   if (fProfIO) fProfIO->Print(option);

   if (fCPerfProfiles){
      Printf("Performance Profiles Canvas: Name=%s Title=%s", 
              fCPerfProfiles->GetName(), fCPerfProfiles->GetTitle());
   }
}

/*
void TProofBenchRunDataRead::DrawPerfProfiles(TDirectory* dir,
                                              TPad* pad,
                                              TString nameprof_event, TString nameprof_io)
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
   //create canvas
   if (!pad){
      pad=new TCanvas("CPerfProfiles", "Performance Profiles");
      pad->Divide(2, 1);
   }
   
   dir->cd();

   //event rate profile
   TProfile* prof_event=dynamic_cast<TProfile*>(dir->FindObject(nameprof_event.Data())); //list of object
   if (!prof_event){
      //try reading key
      TKey* key=dir->FindKey(nameprof_event.Data()); 
      if (key){
         prof_event=dynamic_cast<TProfile*>(key->ReadObj());
      }
   }
   if (prof_event){
      pad->cd(1);
      prof_event->Draw();
   }
   else{
      Error("DrawPerfProfiles", "Profile %s not found", nameprof_event.Data());
   }

   //IO rate profile
   TProfile* prof_io=dynamic_cast<TProfile*>(dir->FindObject(nameprof_io.Data())); //list of object
   if (!prof_io){
      //try reading key
      TKey* key=dir->FindKey(nameprof_io.Data()); 
      if (key){
         prof_io=dynamic_cast<TProfile*>(key->ReadObj());
      }
   }
   if (prof_io){
      pad->cd(2);
      prof_io->Draw();
   }
   else{
      Error("DrawPerfProfiles", "Profile %s not found", nameprof_io.Data());
   }

   return;
}
*/

void TProofBenchRunDataRead::DrawPerfProfiles()
{
   //TPad* canvas=proofbench->GetCPerfProfiles();
   
   if (!fCPerfProfiles) fCPerfProfiles=new TCanvas("cPerfPrifiles", "Performance Profiles");

   fCPerfProfiles->cd();
   fCPerfProfiles->Clear();
   fCPerfProfiles->Divide(2,1);

   //TProfile* prof_event=proofbench->GetProfEvent();
   if (fProfEvent){
      fCPerfProfiles->cd(1);
      fProfEvent->Draw(); 
   }

   //TProfile* prof_io=proofbench->GetProfIO();
   if (fProfIO){
      fCPerfProfiles->cd(2);
      fProfIO->Draw(); 
   }
   fCPerfProfiles->cd(0);
   return; 
}

void TProofBenchRunDataRead::SetMode(TProofBenchMode* mode)
{
   fMode=mode;
}

void TProofBenchRunDataRead::SetRunCleanup(TProofBenchRunCleanup* runcleanup)
{
   fRunCleanup=runcleanup;
}

void TProofBenchRunDataRead::SetReadType(TProofBenchRun::EReadType readtype)
{
   fReadType=readtype;
}

void TProofBenchRunDataRead::SetNEvents(Long64_t nevents)
{
   fNEvents=nevents;
}

void TProofBenchRunDataRead::SetNTries(Int_t ntries)
{
   fNTries=ntries;
}

void TProofBenchRunDataRead::SetMaxNWorkers(Int_t maxnworkers)
{
  fMaxNWorkers=maxnworkers;
}

void TProofBenchRunDataRead::SetMaxNWorkers(TString sworkers)
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

void TProofBenchRunDataRead::SetStart(Int_t start)
{
   fStart=start;
}

void TProofBenchRunDataRead::SetStop(Int_t stop)
{
   fStop=stop;
}

void TProofBenchRunDataRead::SetStep(Int_t step)
{
   fStep=step;
}

void TProofBenchRunDataRead::SetDraw(Int_t draw)
{
   fDraw=draw;
}

void TProofBenchRunDataRead::SetDebug(Int_t debug)
{
   fDebug=debug;
}

TFile* TProofBenchRunDataRead::OpenFile(const char* filename,
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

void TProofBenchRunDataRead::SetDirProofBench(TDirectory* dir)
{
   fDirProofBench=dir;
}

TProofBenchMode* TProofBenchRunDataRead::GetMode()const
{
   return fMode;
}

TProofBenchRunCleanup* TProofBenchRunDataRead::GetRunCleanup()const
{
   return fRunCleanup;
}

TProofBenchRun::EReadType TProofBenchRunDataRead::GetReadType()const
{
   return fReadType;
}

Long64_t TProofBenchRunDataRead::GetNEvents()const
{
   return fNEvents;
}

Int_t TProofBenchRunDataRead::GetNTries()const
{
   return fNTries;
}

Int_t TProofBenchRunDataRead::GetMaxNWorkers()const
{
   return fMaxNWorkers;
}

Int_t TProofBenchRunDataRead::GetStart()const
{
   return fStart;
}

Int_t TProofBenchRunDataRead::GetStop()const
{
   return fStop;
}

Int_t TProofBenchRunDataRead::GetStep()const
{
   return fStep;
}

Int_t TProofBenchRunDataRead::GetDraw()const
{
   return fDraw;
}

Int_t TProofBenchRunDataRead::GetDebug()const
{
   return fDebug;
}

TFile* TProofBenchRunDataRead::GetFile()const
{
   return fFile;
}

TDirectory* TProofBenchRunDataRead::GetDirProofBench()const
{
   return fDirProofBench;
}

TList* TProofBenchRunDataRead::GetListOfPerfStats()const
{
   return fPerfStats;
}

TList* TProofBenchRunDataRead::GetListPerfProfiles()const
{
   return fListPerfProfiles;
}

TCanvas* TProofBenchRunDataRead::GetCPerfProfiles()const
{
   return fCPerfProfiles;
}

const char* TProofBenchRunDataRead::GetName()const
{
   return fName.Data();
}

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

Int_t TProofBenchRunDataRead::FillNodeInfo()
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

//const char* TProofBenchRunDataRead::BuildPatternName(const char* objname, const char* delimiter){
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

//const char* TProofBenchRunDataRead::BuildNewPatternName(const char* objname, Int_t nactive, Int_t tries, const char* delimiter){
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

//const char* TProofBenchRunDataRead::BuildProfileName(const char* objname, const char* type, const char* delimiter){
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

//const char* TProofBenchRunDataRead::BuildProfileTitle(const char* objname, const char* type, const char* delimiter){
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
