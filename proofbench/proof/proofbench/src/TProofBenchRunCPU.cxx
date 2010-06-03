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
// TProofBenchRunCPU                                                    //
//                                                                      //
// TProofBenchRunCPU is ...                                             //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "TProofBenchRunCPU.h"
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
#include "TEnv.h"
#include "TTree.h"
#include "TLeaf.h"
#include "TCanvas.h"
#include "TROOT.h"
#include "TProfile.h"
#include "TLegend.h"
#include "TKey.h"
#include "TRegexp.h"
#include "TPerfStats.h"
#include "TQueryResult.h"
#include "TMath.h"

ClassImp(TProofBenchRunCPU)

TProofBenchRunCPU::TProofBenchRunCPU(TProofBenchRun::EHistType histtype,
                                     Int_t nhists,
                                     TString filename, //user has to provide one
                                     Option_t* foption, //option to TFile() 
                                     TProof* proof,
                                     Int_t maxnworkers,//maximum number of workers to be tested. 
                                                       //If not set (default), 1 times the number of total workers in the cluster available
                                     Long64_t nevents,
                                     Int_t ntries,
                                     Int_t start,
                                     Int_t stop,
                                     Int_t step,
                                     Int_t draw,
                                     Int_t debug):
fProof(0),
fHistType(histtype),
fNHists(nhists),
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
fListPerfProfiles(0),
fCPerfProfiles(0),
fName(0)
{
//Default constructor
   fProof=proof?proof:gProof;

   if (filename.Length()){
      OpenFile(filename.Data(), foption);
   }

   fName="CPU"+GetNameStem();

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

}

TProofBenchRunCPU::~TProofBenchRunCPU()
{
//destructor
   fProof=0;
   if (fProfEvent) delete fProfEvent;
   fDirProofBench=0;
   if (fFile){
      fFile->Close();
      delete fFile;
   }
   if (fNodes) delete fNodes;  //fNodes is the owner of its members
   if (fCPerfProfiles) delete fCPerfProfiles;
} 

void TProofBenchRunCPU::Run(Long64_t nevents,
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

   //find perfstat profile
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
   }
   else{
      profile_perfstat_event->Reset();
   }

   //file queryresult profile
   TString profile_queryresult_event_name=BuildProfileName("hProf", "QueryResult_Event");
   TProfile* profile_queryresult_event=(TProfile*)(fListPerfProfiles->FindObject(profile_queryresult_event_name.Data()));
 
   if (!profile_queryresult_event){
      TString profile_queryresult_event_title=BuildProfileTitle("Profile", "QueryResult Event");
      TProfile* profile_queryresult_event=new TProfile(profile_queryresult_event_name, profile_queryresult_event_title, ndiv, ns_min-0.5, ns_max+0.5);

      profile_queryresult_event->GetXaxis()->SetTitle("Number of Slaves");
      profile_queryresult_event->GetYaxis()->SetTitle("#times10^{3} Events/sec");
      profile_queryresult_event->SetMarkerStyle(22);

      fListPerfProfiles->Add(profile_queryresult_event);
   }
   else{
       profile_queryresult_event->Reset();
   }

   //get pad
   if (!fCPerfProfiles){
      fCPerfProfiles=new TCanvas("CPerfProfiles");
   }
   //divide the canvas as many as the number of profiles in the list
   Int_t nprofiles=fListPerfProfiles->GetSize();
   if (nprofiles<=2){
      fCPerfProfiles->Divide(nprofiles);
   }
   else{
      Int_t nside = (Int_t)TMath::Sqrt((Float_t)nprofiles);
      nside = (nside*nside<nprofiles)?nside+1:nside;
      fCPerfProfiles->Divide(nside,nside);
   }

   TString perfstats_name = "PROOF_PerfStats";

   SetParameters();

   //Delete the list of performance statistics trees
   fPerfStats->Delete();

   for (Int_t nactive=start; nactive<=stop; nactive+=step) {
      fProof->SetParallel(nactive);
      for (Int_t j=0; j<ntries; j++) {

         Int_t npad=1; //pad number

         TTime starttime = gSystem->Now();
         TTime endtime = gSystem->Now();

         fProof->Process("TSelHist", nevents);

         TList* l = fProof->GetOutputList();

         //save perfstats
         TTree* t = dynamic_cast<TTree*>(l->FindObject(perfstats_name.Data()));
         if (t) {

            FillPerfStatProfiles(t, profile_perfstat_event, nactive);
            fCPerfProfiles->cd(npad++);
            profile_perfstat_event->Draw();
            gPad->Update();

            t->SetDirectory(fDirProofBench);

            //build up new name
            TString newname=BuildNewPatternName(perfstats_name, nactive, j);
            t->SetName(newname);

            fPerfStats->Add(t);

            if (debug && fWritable){
               fDirProofBench->cd();
               t->Write();
            }
         } else {
            Error("RunBenchmark", "tree %s not found", perfstats_name.Data());
         }
         
         //performance measures from TQueryResult

         TQueryResult* queryresult=fProof->GetQueryResult();  
         TDatime qr_start=queryresult->GetStartTime(); 
         TDatime qr_end=queryresult->GetEndTime(); 
         Float_t qr_init=queryresult->GetInitTime(); 
         Float_t qr_proc=queryresult->GetProcTime(); 
         Float_t qr_usedcpu=queryresult->GetUsedCPU(); 
    
         Long64_t qr_entries=queryresult->GetEntries();

         //Info("Run", "start="); qr_start.Print();
         //Info("Run", "end="); qr_end.Print();
         //Info("Run", "init=%f proc=%f used cpu=%f", qr_init, qr_proc, qr_usedcpu);

         //calculate event rate
         Double_t qr_eventrate=qr_entries/Double_t(qr_init+qr_proc);

         //build profile name
         TString profile_queryresult_event_name=BuildProfileName("hProf", "QueryResult_Event");
         //get profile
         TProfile* profile_queryresult_event=(TProfile*)(fListPerfProfiles->FindObject(profile_queryresult_event_name.Data()));

         //fill and draw
         if (profile_queryresult_event){
            profile_queryresult_event->Fill(nactive, qr_eventrate);
            fCPerfProfiles->cd(npad++);
            profile_queryresult_event->Draw();
            gPad->Update();
         }
         else{
            Error("Run", "Profile not found: %s", profile_queryresult_event_name.Data());
         }
      }//for iterations
   }//for number of workers
}

void TProofBenchRunCPU::FillPerfStatProfiles(TTree* t, TProfile* profile, Int_t nactive)
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
   time_holder = end-start;

   Double_t event_rate;

   event_rate=nevents_holder/time_holder/1000.; 
   profile->Fill(Double_t(nactive), event_rate);

   //if (fWritable){
   //   fProfEvent->Write();
   //}
   return;
}

void TProofBenchRunCPU::BuildPerfProfiles(Int_t start,
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

   TString namestem=GetNameStem();
   TString perfstats_name = "PROOF_PerfStats";

   TString pattern;
   pattern=perfstats_name+"_"+GetName();

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
//   TIter NextKey(fDirProofBench->GetListOfKeys());
//   TKey* key = 0;
//   while ((key = dynamic_cast<TKey*>(NextKey()))) {
//       printf("key in the list: %s\n", key->GetName());
//      if(!TString(key->GetName()).Contains(TRegexp(pattern)))
//         continue;
//
//      TObject* obj = key->ReadObj();
//

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

   Double_t event_rate;
   Int_t entries=timing_tree->GetEntries();

   //BookProfile();
   //prof_event= new TProfile(hname_event, htitle_event, max_slaves, 0.5, max_slaves+0.5);
   //prof_io= new TProfile(hname_io, htitle_io, max_slaves, 0.5, max_slaves+0.5);

   for (int i=0; i<entries; i++){
      timing_tree->GetEntry(i); 
      //printf("ns_holder=%d, run_holder=%d time_holder=%f\n", ns_holder, run_holder, time_holder);
      event_rate=nevents_holder/time_holder/1000.; 
      fProfEvent->Fill(Double_t(ns_holder), event_rate);

   }
   fProfEvent->GetXaxis()->SetTitle("Number of Slaves");
   fProfEvent->GetYaxis()->SetTitle("#times10^{3} Events/sec");
   fProfEvent->SetMarkerStyle(21);
   if (fWritable){
      fProfEvent->Write();
   }

   delete timing_tree;
   return;
}

void TProofBenchRunCPU::Print(Option_t* option)const{

   Printf("Name=%s", fName.Data());
   if (fProof) fProof->Print(option);
   Printf("fHistType=%s%s", "k", GetNameStem().Data());
   Printf("fNHists=%d", fNHists);
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

   if (fCPerfProfiles){
      Printf("Performance Profiles Canvas: Name=%s Title=%s",
              fCPerfProfiles->GetName(), fCPerfProfiles->GetTitle());
   }
}

/*
void TProofBenchRunCPU::DrawPerfProfiles(TDirectory* dir,
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

void TProofBenchRunCPU::DrawPerfProfiles()
{
   //TPad* canvas=proofbench->GetCPerfProfiles();
   fCPerfProfiles->cd();
   fCPerfProfiles->Clear();
   fCPerfProfiles->Divide(2,1);

   if (fProfEvent){
      fCPerfProfiles->cd(1);
      fProfEvent->Draw(); 
   }

   fCPerfProfiles->cd(0);
   return; 
}

void TProofBenchRunCPU::SetHistType(TProofBenchRun::EHistType histtype)
{
   fHistType=histtype;
   fName=GetNameStem()+"CPU";
}

void TProofBenchRunCPU::SetNHists(Int_t nhists)
{
   fNHists=nhists;
}

void TProofBenchRunCPU::SetNEvents(Long64_t nevents)
{
   fNEvents=nevents;
}

void TProofBenchRunCPU::SetNTries(Int_t ntries)
{
   fNTries=ntries;
}
   
void TProofBenchRunCPU::SetMaxNWorkers(Int_t maxnworkers)
{
  fMaxNWorkers=maxnworkers;
}

void TProofBenchRunCPU::SetMaxNWorkers(TString sworkers)
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

void TProofBenchRunCPU::SetStart(Int_t start){
   fStart=start;
}

void TProofBenchRunCPU::SetStop(Int_t stop){
   fStop=stop;
}

void TProofBenchRunCPU::SetStep(Int_t step){
   fStep=step;
}

void TProofBenchRunCPU::SetDraw(Int_t draw)
{
   fDraw=draw;
}

void TProofBenchRunCPU::SetDebug(Int_t debug)
{
   fDebug=debug;
}

TFile* TProofBenchRunCPU::OpenFile(const char* filename,
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

void TProofBenchRunCPU::SetDirProofBench(TDirectory* dir)
{
   fDirProofBench=dir;
}

TProofBenchRun::EHistType TProofBenchRunCPU::GetHistType()const
{
   return fHistType;
}

Int_t TProofBenchRunCPU::GetNHists()const
{
   return fNHists;
}

Int_t TProofBenchRunCPU::GetNTries()const
{
   return fNTries;
}

Long64_t TProofBenchRunCPU::GetNEvents()const
{
   return fNEvents;
}

Int_t TProofBenchRunCPU::GetMaxNWorkers()const
{
   return fMaxNWorkers;
}

Int_t TProofBenchRunCPU::GetStart()const
{
   return fStart;
}

Int_t TProofBenchRunCPU::GetStop()const
{
   return fStop;
}

Int_t TProofBenchRunCPU::GetStep()const
{
   return fStep;
}

Int_t TProofBenchRunCPU::GetDraw()const
{
   return fDraw;
}

Int_t TProofBenchRunCPU::GetDebug()const
{
   return fDebug;
}

TFile* TProofBenchRunCPU::GetFile()const
{
   return fFile;
}

TDirectory* TProofBenchRunCPU::GetDirProofBench() const
{
   return fDirProofBench;
}

TList* TProofBenchRunCPU::GetListOfPerfStats()const
{
   return fPerfStats;
}  

TCanvas* TProofBenchRunCPU::GetCPerfProfiles() const
{
   return fCPerfProfiles;
}

const char* TProofBenchRunCPU::GetName()const
{
   return fName.Data();
}

TString TProofBenchRunCPU::GetNameStem()const
{
   TString namestem;
   switch (fHistType){
   case TProofBenchRun::kHist1D:
      namestem="Hist1D";
      break;
   case TProofBenchRun::kHist2D:
      namestem="Hist2D";
      break;
   case TProofBenchRun::kHist3D:
      namestem="Hist3D";
      break;
   case TProofBenchRun::kHistAll:
      namestem="HistAll";
      break;
   default:
      break;
   }
    return namestem;
}

Int_t TProofBenchRunCPU::FillNodeInfo()
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
   fNodes->SetOwner();  //fNodes is owner of members
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

Int_t TProofBenchRunCPU::SetParameters(){
   if (!fProof){
      Error("SetParameters", "proof not set; Doing nothing");
      return 1;
   }

   fProof->SetParameter("PROOF_BenchmarkHistType", Int_t(fHistType));
   fProof->SetParameter("PROOF_BenchmarkNHists", fNHists);
   fProof->SetParameter("PROOF_BenchmarkDraw", Int_t(fDraw));
   return 0;
}

Int_t TProofBenchRunCPU::DeleteParameters(){
   if (!fProof){
      Error("DeleteParameters", "proof not set; Doing nothing");
      return 1;
   }
   fProof->DeleteParameters("PROOF_BenchmarkHistType");
   fProof->DeleteParameters("PROOF_BenchmarkNHists");
   fProof->DeleteParameters("PROOF_BenchmarkDraw");
   return 0;
}

const char* TProofBenchRunCPU::BuildPatternName(const char* objname, const char* delimiter)
{
   TString newname(objname);
   newname+=delimiter;
   newname+=GetName();
   return newname.Data();
}

const char* TProofBenchRunCPU::BuildNewPatternName(const char* objname, Int_t nactive, Int_t tries, const char* delimiter)
{
   TString newname(BuildPatternName(objname, delimiter));
   newname+=delimiter;
   newname+=nactive;
   newname+="Slaves";
   newname+=delimiter;
   newname+="Run";
   newname+=tries;
   return newname.Data();
}

const char* TProofBenchRunCPU::BuildProfileName(const char* objname, const char* type, const char* delimiter)
{
   TString newname(BuildPatternName(objname, delimiter));
   newname+=delimiter;
   newname+=type;
   return newname.Data();
}

const char* TProofBenchRunCPU::BuildProfileTitle(const char* objname, const char* type, const char* delimiter)
{
   TString newname(BuildPatternName(objname, delimiter));
   newname+=delimiter;
   newname+=type;
   return newname.Data();
}

