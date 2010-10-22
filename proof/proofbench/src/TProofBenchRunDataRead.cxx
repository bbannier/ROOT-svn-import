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
#include "TProofNodes.h"
#include "TFileCollection.h"
#include "TFileInfo.h"
#include "TProof.h"
#include "TString.h"
#include "Riostream.h"
#include "TMap.h"
#include "TTree.h"
#include "TH1.h"
#include "TH2.h"
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
#include "TROOT.h"

ClassImp(TProofBenchRunDataRead)

//______________________________________________________________________________
TProofBenchRunDataRead::TProofBenchRunDataRead(TProofBenchMode* mode,
                            TProofBenchRunCleanup* runcleanup,
                            TProofBenchRun::EReadType readtype,
                            TDirectory* dirproofbench, TProof* proof,
                            TProofNodes* nodes, Long64_t nevents, Int_t ntries,
                            Int_t start, Int_t stop, Int_t step, Int_t nx,
                            Int_t debug):
fProof(proof), fReadType(readtype), fMode(mode), fRunCleanup(runcleanup),
fNEvents(nevents), fNTries(ntries), fStart(start), fStop(stop), fStep(step),
fNx(nx), fDebug(debug), fDirProofBench(dirproofbench), fNodes(nodes),
fPerfStats(0), fListPerfProfiles(0), fCPerfProfiles(0), fName(0)
{

   //Default constructor

   if (!fProof){
      fProof=gProof;
   }

   //set name
   fName="DataRead"+GetNameStem();

   if (stop==-1){
      if (fNodes){
         fNodes->GetNWorkersCluster();
      }
      else{
         if (fProof){
            fStop=fProof->GetListOfSlaveInfos()->GetSize();
         }
      }
   }

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
void TProofBenchRunDataRead::Run(Long64_t nevents, Int_t start, Int_t stop,
                                 Int_t step, Int_t ntries, Int_t nx,
                                 Int_t debug, Int_t)
{
   // Run benchmark
   // Input parameters
   //    nevents: Number of events to run per file. When it is -1, data member
   //             fNEvents is used. 
   //    start: Start scan with 'start' workers.
   //    stop: Stop scan at 'stop workers.
   //    step: Scan every 'step' workers.
   //    ntries: Number of tries. When it is -1, data member fNTries is used.
   //    nx:
   //    debug: debug switch.
   //    Int_t: Ignored
   // Returns
   //    Nothing

   if (!fProof){
      Error("Run", "Proof not set");
      return;
   }

   nevents=(nevents==-1)?fNEvents:nevents;
   start=(start==-1)?fStart:start;
   stop=(stop==-1)?fStop:stop;
   step=(step==-1)?fStep:step;
   ntries=(ntries==-1)?fNTries:ntries;
   nx=(nx==-1)?fNx:nx;
   debug=(debug==-1)?fDebug:debug;

   if (nx==0){
   }
   else if (nx==1){
      const Int_t minnworkersanode=fNodes->GetMinNWorkersANode();
      if (stop>minnworkersanode){
         stop=minnworkersanode;
      }
   }
   
   if (!fListPerfProfiles){
      fListPerfProfiles=new TList();
      fListPerfProfiles->SetOwner(kFALSE); //do not delete outputs
   }
   else{
      fListPerfProfiles->Clear();
   }

   Int_t quotient=(stop-start)/step;
   Int_t ndiv=quotient+1;
   Double_t ns_min=start-step/2.;
   Double_t ns_max=quotient*step+start+step/2.;

   //find perfstat event profile
   TString profile_perfstat_event_name=BuildProfileName("hProf",
                                                        "PerfStat_Event");
   TProfile* profile_perfstat_event=(TProfile*)
            (fListPerfProfiles->FindObject(profile_perfstat_event_name.Data()));

   fDirProofBench->cd();
   //book one if one does not exists yet or reset the existing one
   if (!profile_perfstat_event){
      TString profile_perfstat_event_title=BuildProfileTitle("Profile",
                                                             "PerfStat Event");
      profile_perfstat_event= new TProfile(profile_perfstat_event_name,
                           profile_perfstat_event_title, ndiv, ns_min, ns_max);

      profile_perfstat_event->SetDirectory(fDirProofBench);
      if (nx==0){
         profile_perfstat_event->GetXaxis()->SetTitle("Active Slaves");
      }
      else if (nx==1){
         profile_perfstat_event->GetXaxis()->SetTitle("Active Slaves/Node");
      }
      profile_perfstat_event->GetYaxis()->SetTitle("Events/sec");
      profile_perfstat_event->SetMarkerStyle(21);

      fListPerfProfiles->Add(profile_perfstat_event);
   }
   else{
      profile_perfstat_event->Reset();
   }
   //find perfstat IO profile
   TString profile_perfstat_IO_name=BuildProfileName("hProf", "PerfStat_IO");
   TProfile* profile_perfstat_IO=(TProfile*)
               (fListPerfProfiles->FindObject(profile_perfstat_IO_name.Data()));

   //book one if one does not exists yet or reset the existing one
   if (!profile_perfstat_IO){
      TString profile_perfstat_IO_title=BuildProfileTitle("Profile",
                                                          "PerfStat IO");
      profile_perfstat_IO= new TProfile(profile_perfstat_IO_name,
                               profile_perfstat_IO_title, ndiv, ns_min, ns_max);
      profile_perfstat_IO->SetDirectory(fDirProofBench);
      if (nx==0){
         profile_perfstat_IO->GetXaxis()->SetTitle("Active Slaves");
      }
      else if (nx==1){
         profile_perfstat_IO->GetXaxis()->SetTitle("Active Slaves/Node");
      }

      profile_perfstat_IO->GetYaxis()->SetTitle("MBs/sec");
      profile_perfstat_IO->SetMarkerStyle(21);

      fListPerfProfiles->Add(profile_perfstat_IO);
   }
   else{
      profile_perfstat_IO->Reset();
   }

   //find perfstat event histogram
   TString hist_perfstat_event_name=BuildProfileName("hHist",
                                                        "PerfStat_Event");
   TH2* hist_perfstat_event=(TH2*)
            (fListPerfProfiles->FindObject(hist_perfstat_event_name.Data()));

   //book one if one does not exists yet or reset the existing one
   if (!hist_perfstat_event){
      TString hist_perfstat_event_title=BuildProfileTitle("Hist",
                                                             "PerfStat Event");
      hist_perfstat_event= new TH2C(hist_perfstat_event_name,
             hist_perfstat_event_title, ndiv, ns_min, ns_max, 10000, 0, 10000);

      hist_perfstat_event->SetDirectory(fDirProofBench);
      if (nx==0){
         hist_perfstat_event->GetXaxis()->SetTitle("Active Slaves");
      }
      else if (nx==1){
         hist_perfstat_event->GetXaxis()->SetTitle("Active Slaves/Node");
      }
      hist_perfstat_event->GetYaxis()->SetTitle("Events/sec");
      hist_perfstat_event->SetMarkerStyle(7);

      hist_perfstat_event->SetBit(TH1::kCanRebin);

      fListPerfProfiles->Add(hist_perfstat_event);
   }
   else{
      hist_perfstat_event->Reset();
   }

   //find perfstat io histogram
   TString hist_perfstat_io_name=BuildProfileName("hHist",
                                                        "PerfStat_IO");
   TH2* hist_perfstat_IO=(TH2*)
            (fListPerfProfiles->FindObject(hist_perfstat_io_name.Data()));

   //book one if one does not exists yet or reset the existing one
   if (!hist_perfstat_IO){
      TString hist_perfstat_io_title=BuildProfileTitle("Hist",
                                                             "PerfStat IO");
      hist_perfstat_IO= new TH2C(hist_perfstat_io_name,
                           hist_perfstat_io_title, ndiv, ns_min, ns_max, 1000, 0, 100);

      hist_perfstat_IO->SetDirectory(fDirProofBench);
      if (nx==0){
         hist_perfstat_IO->GetXaxis()->SetTitle("Active Slaves");
      }
      else if (nx==1){
         hist_perfstat_IO->GetXaxis()->SetTitle("Active Slaves/Node");
      }
      hist_perfstat_IO->GetYaxis()->SetTitle("MBs/sec");
      hist_perfstat_IO->SetMarkerStyle(7);

      hist_perfstat_IO->SetBit(TH1::kCanRebin);

      fListPerfProfiles->Add(hist_perfstat_IO);
   }
   else{
      hist_perfstat_IO->Reset();
   }

   //find queryresult profile
   TString profile_queryresult_event_name=BuildProfileName("hProf",
                                                           "QueryResult_Event");

   TProfile* profile_queryresult_event=(TProfile*)
         (fListPerfProfiles->FindObject(profile_queryresult_event_name.Data()));

   if (!profile_queryresult_event){
      TString profile_queryresult_event_title=BuildProfileTitle("Profile",
                                                           "QueryResult Event");
      profile_queryresult_event=new TProfile(profile_queryresult_event_name,
                         profile_queryresult_event_title, ndiv, ns_min, ns_max);
      profile_queryresult_event->SetDirectory(fDirProofBench);
      if (nx==0){
         profile_queryresult_event->GetXaxis()->SetTitle("Active Slaves");
      }
      else if (nx==1){
         profile_queryresult_event->GetXaxis()->SetTitle("Active Slaves/Node");
      }
      profile_queryresult_event->GetYaxis()->SetTitle("Events/sec");
      profile_queryresult_event->SetMarkerStyle(22);

      fListPerfProfiles->Add(profile_queryresult_event);
   }
   else{
       profile_queryresult_event->Reset();
   }

   //find queryresult profile
   TString profile_queryresult_IO_name=BuildProfileName("hProf",
                                                        "QueryResult_IO");
   TProfile* profile_queryresult_IO=(TProfile*)
            (fListPerfProfiles->FindObject(profile_queryresult_IO_name.Data()));

   if (!profile_queryresult_IO){
      TString profile_queryresult_IO_title=BuildProfileTitle("Profile",
                                                             "QueryResult IO");
      profile_queryresult_IO=new TProfile(profile_queryresult_IO_name,
                            profile_queryresult_IO_title, ndiv, ns_min, ns_max);
      profile_queryresult_IO->SetDirectory(fDirProofBench);
      if (nx==0){
         profile_queryresult_IO->GetXaxis()->SetTitle("Active Slaves");
      }
      else if (nx==1){
         profile_queryresult_IO->GetXaxis()->SetTitle("Active Slaves/Node");
      }
      profile_queryresult_IO->GetYaxis()->SetTitle("MBs/sec");
      profile_queryresult_IO->SetMarkerStyle(22);

      fListPerfProfiles->Add(profile_queryresult_IO);
   }
   else{
       profile_queryresult_IO->Reset();
   }

   //get pad
   if (!fCPerfProfiles){
      TString canvasname=TString("Performance Profiles ")+GetName();
      fCPerfProfiles=new TCanvas(canvasname.Data(), canvasname.Data());
   }

   //cleanup up the canvas
   fCPerfProfiles->Clear();

   //divide the canvas as many as the number of profiles in the list
   Int_t nprofiles=fListPerfProfiles->GetSize();

   if (nprofiles<=2){
      fCPerfProfiles->Divide(nprofiles);
   }
   else{
      Int_t nside = (Int_t)TMath::Sqrt((Float_t)nprofiles);
      nside = (nside*nside<nprofiles)?nside+1:nside;
      if ((nside-1)*nside>=nprofiles){
         fCPerfProfiles->Divide(nside, nside-1);
      }
      else{
         fCPerfProfiles->Divide(nside, nside);
      }
   }

   //do not delete performance statistics trees
   fPerfStats->Clear();

   Int_t nf=fMode->GetNFiles();
   TString smode=fMode->GetName();
   if (nx==0){
      Info("Run", "Running IO-bound tests for mode %s; %d ~ %d active"
           " worker(s), every %d worker(s).", smode.Data(), start, stop, step);
   }
   else if (nx==1){
      Info("Run", "Running IO-bound tests for mode %s; %d ~ %d active"
           " worker(s)/node, every %d worker(s)/node.", smode.Data(), start,
           stop, step);
   }

   for (Int_t nactive=start; nactive<=stop; nactive+=step) {
      for (Int_t j=0; j<ntries; j++) {

         Int_t npad=1; //pad number

         TString dsname; 

         //cleanup run
         if (nx==0){
            dsname.Form("DataSetEvent%s_%d_%d", smode.Data(), nactive, nf);
            Info("Run", "Running IO-bound tests with %d active worker(s)."
                        " %dth trial.", nactive, j);
         }
         else if (nx==1){
            dsname.Form("DataSetEvent%s_%dX_%d", smode.Data(), nactive, nf);
            Info("Run", "Running IO-bound tests with %d active worker(s)/node."
                        " %dth trial.", nactive, j);
         }

         if (fRunCleanup->GetCleanupType()==TProofBenchRun::kCleanupFileAdvise){
            fRunCleanup->SetDataSetCleanup(dsname);
         }
         fRunCleanup->Run(nevents, 0, 0, 0, 0, 0, debug, 0);

         //TString namestem=GetNameStem();
         DeleteParameters();
         SetParameters();
         if (nx==0){
            fProof->SetParallel(nactive);
         }
         else if (nx==1){
            TString workers;
            workers.Form("%dx", nactive);
            if (fNodes->ActivateWorkers(workers)<0){
               Error("Run", "Could not activate the requestednumber of"
                     " workers/node on the cluster; Skipping the test point"
                     " (%d workers/node, %dth trial).", nactive, j);
               continue;
            }
         }

         TFileCollection* fc=0;
         Long64_t nfiles=0;
         Long64_t nevents_total=-1; //-1 for all events in the group of files
         if ((fc=fProof->GetDataSet(dsname.Data()))){
            //fc->Print();
            nfiles=fc->GetNFiles();
            if (nevents>0){
               nevents_total=nfiles*nevents;
            }
         }
         else{
            Error("Run", "no such data set found; %s", dsname.Data());
            continue;
         }

         if (nx==0){
            Info("Run", "Processing data set %s (%lld files, %lld events to"
                 " process) with %d active worker(s).", dsname.Data(),
                 nfiles, nevents_total, nactive); 
         }
         else if (nx==1){
            Info("Run", "Processing data set %s (%lld files, %lld events to"
                 " process) with %d active worker(s)/node.", dsname.Data(),
                 nfiles, nevents_total, nactive); 
         }

         TTime starttime = gSystem->Now();
         fProof->Process(dsname.Data(), "TSelEvent", "", nevents_total);
         DeleteParameters();

         TTime endtime = gSystem->Now();

         TList* l = fProof->GetOutputList();

         //save perfstats
         TString perfstats_name = "PROOF_PerfStats";
         TTree* t = dynamic_cast<TTree*>(l->FindObject(perfstats_name.Data()));
         if (t) {
            TTree* tnew=(TTree*)t->Clone("tnew");

            FillPerfStatProfiles(tnew, profile_perfstat_event,
                                 profile_perfstat_IO, 
                                 hist_perfstat_event, hist_perfstat_IO,
                                 nactive);

            fCPerfProfiles->cd(npad++);
            profile_perfstat_event->Draw();
            gPad->Update();
            fCPerfProfiles->cd(npad++);
            profile_perfstat_IO->Draw();
            gPad->Update();

            fCPerfProfiles->cd(npad++);
            hist_perfstat_event->Draw();
            gPad->Update();

            fCPerfProfiles->cd(npad++);
            hist_perfstat_IO->Draw();
            gPad->Update();

            tnew->SetDirectory(fDirProofBench);

            //change the name
            TString newname=BuildNewPatternName(perfstats_name, nactive, j, nx);
            tnew->SetName(newname);
            fPerfStats->Add(tnew);

            if (debug && fDirProofBench->IsWritable()){
               fDirProofBench->cd();
               tnew->Write();
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
         //Float_t qr_usedcpu=queryresult->GetUsedCPU();
         Long64_t qr_bytes=queryresult->GetBytes();

         Long64_t qr_entries=queryresult->GetEntries();

         //calculate event rate, fill and draw
         Double_t qr_eventrate=0;

         qr_eventrate=qr_entries/Double_t(qr_init+qr_proc);

         profile_queryresult_event->Fill(nactive, qr_eventrate);
         fCPerfProfiles->cd(npad++);
         profile_queryresult_event->Draw();
         gPad->Update();

         //calculate IO rate, fill and draw
         Double_t qr_IOrate=0;

         const Double_t Dmegabytes=1024*1024;

         qr_IOrate=qr_bytes/Dmegabytes/Double_t(qr_init+qr_proc);

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
               TString newname=BuildNewPatternName(ptdist_name, nactive, j, nx);
               hnew->SetName(newname);

               if (fDirProofBench->IsWritable()){
                  fDirProofBench->cd();
                  hnew->Write();
                  delete hnew;
               }
            } else {
               Error("Run", "histogram %s not found", ptdist_name.Data());
            }

            TString tracksdist_name = "ntracks_dist";
            TH1* h2 = dynamic_cast<TH1*>(l->FindObject(tracksdist_name.Data()));
            if (h2) {
               TH1 *hnew = (TH1*)h2->Clone("hnew");
               hnew->SetDirectory(fDirProofBench);
               TString newname=BuildNewPatternName(tracksdist_name, nactive, j,
                                                   nx);
               hnew->SetName(newname);

               if (fDirProofBench->IsWritable()){
                  fDirProofBench->cd();
                  hnew->Write();
                  delete hnew;
               }
            } 
            else {
               Error("Run", "histogram %s not found", tracksdist_name.Data());
            }
         }

         fCPerfProfiles->cd(0); 
      }//for ntries
   }//for number of workers
   
   //save performance profiles to file
   if (fDirProofBench->IsWritable()){
      fDirProofBench->cd();
      profile_perfstat_event->Write();
      profile_perfstat_IO->Write();
      hist_perfstat_event->Write();
      hist_perfstat_IO->Write();
      profile_queryresult_event->Write();
      profile_queryresult_IO->Write();
   }
}

//______________________________________________________________________________
void TProofBenchRunDataRead::FillPerfStatProfiles(TTree* t,
                                 TProfile* profile_event, TProfile* profile_IO,
                                 TH2* hist_event, TH2* hist_IO, Int_t nactive)
{

   // Fill performance profiles using tree 't'(PROOF_PerfStats).
   // Input parameters
   //    t: Proof output tree (PROOF_PerfStat) containing performance
   //       statistics.
   //    profile_event: Event-rate profile to be filled up with information
   //                   from tree 't'.
   //    profile_IO: IO-rate profile to be filled up with information from
   //                tree 't'.
   //    nactive: Number of active workers processed the query.
   // Return
   //    Nothing

   const Double_t Dmegabytes=1024.*1024.;

   // extract timing information
   TPerfEvent pe;
   TPerfEvent* pep = &pe;
   t->SetBranchAddress("PerfEvents",&pep);
   Long64_t entries = t->GetEntries();
   Double_t start(0), end(0);
   //Bool_t started=kFALSE;
      
   Long64_t nevents_kPacket=0;
   Long64_t bytesread_kPacket=0;
   Double_t proctime_kPacket=0;

   Long64_t nevents_kRate=0;
   Long64_t bytesread_kRate=0;
   Double_t proctime_kRate=0;

   Double_t event_rate_packet=0, io_rate_packet=0;

   Double_t proctime_kPacket_worker[nactive];
   Long64_t nevents_kPacket_worker[nactive];
   Long64_t bytesread_kPacket_worker[nactive];
   for (Int_t i=0; i<nactive; i++){
      proctime_kPacket_worker[i]=0;
      nevents_kPacket_worker[i]=0;
      bytesread_kPacket_worker[i]=0;
   }

   for (Long64_t k=0; k<entries; k++) {
      t->GetEntry(k);

      //Printf("k:%lld fTimeStamp=%lf fEvtNode=%s pe.fType=%d fSlaveName=%s"
      //       " fNodeName=%s fFileName=%s fFileClass=%s fSlave=%s"
      //       " fEventsProcessed=%lld fBytesRead=%lld fLen=%lld fLatency=%lf"
      //       " fProcTime=%lf fCpuTime=%lf fIsStart=%d fIsOk=%d", k,
      //       pe.fTimeStamp.GetSec()+ 1e-9*pe.fTimeStamp.GetNanoSec(),
      //       pe.fEvtNode.Data(), pe.fType, pe.fSlaveName.Data(),
      //       pe.fNodeName.Data(), pe.fFileName.Data(), pe.fFileClass.Data(),
      //       pe.fSlave.Data(), pe.fEventsProcessed, pe.fBytesRead, pe.fLen,
      //       pe.fLatency, pe.fProcTime, pe.fCpuTime, pe.fIsStart, pe.fIsOk);

      //skip information from workers
      if (pe.fEvtNode.Contains(".")) continue;

      if (pe.fType==TVirtualPerfStats::kPacket){
         event_rate_packet=pe.fEventsProcessed/pe.fProcTime;
         io_rate_packet=pe.fBytesRead/Dmegabytes/pe.fProcTime;

         hist_event->Fill(Double_t(nactive), event_rate_packet);
         hist_IO->Fill(Double_t(nactive), io_rate_packet);

         TString sslave=pe.fSlave; 
         sslave.Replace(0, sslave.Last('.')+1, "");
         Int_t slave=sslave.Atoi();

         proctime_kPacket_worker[slave]+=pe.fProcTime;
         nevents_kPacket_worker[slave]+=pe.fEventsProcessed;
         bytesread_kPacket_worker[slave]+=pe.fBytesRead;

         nevents_kPacket+=pe.fEventsProcessed;
         bytesread_kPacket+=pe.fBytesRead;
         proctime_kPacket+=pe.fProcTime;
      }

      if (pe.fType==TVirtualPerfStats::kRate){
         //printf("adding pe.fEventsProcessed=%lld\n", pe.fEventsProcessed);
         nevents_kRate+=pe.fEventsProcessed;
         bytesread_kRate+=pe.fBytesRead;
         proctime_kRate+=pe.fProcTime;
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
      if (pe.fType==TVirtualPerfStats::kStart) start= pe.fTimeStamp.GetSec()
                                              +1e-9*pe.fTimeStamp.GetNanoSec();
      if (pe.fType==TVirtualPerfStats::kStop) end= pe.fTimeStamp.GetSec()
                                              +1e-9*pe.fTimeStamp.GetNanoSec();
   }
     
   //if (nevents_kPacket!=fNEvents){
    //  Error("BuildTimingTree", "Number of events processed is different from the number of events in the file");
     // return 0;
   //}

   Double_t event_rate=0, io_rate=0;
   Double_t event_rate_worker[nactive];
   Double_t io_rate_worker[nactive];

   for (Int_t i=0; i<nactive; i++){
      //Info("FillPerfStatProfiles", "i=%d nevents_kPacket_worker=%lld,"
      //     " proctime_kPacket_worker=%f", i, nevents_kPacket_worker[i],
      //     proctime_kPacket_worker[i]);
      event_rate_worker[i]=nevents_kPacket_worker[i]/proctime_kPacket_worker[i];
      io_rate_worker[i]=bytesread_kPacket_worker[i]/Dmegabytes
                        /proctime_kPacket_worker[i];
      event_rate+=event_rate_worker[i];
      io_rate+=io_rate_worker[i];
   }

   profile_event->Fill(Double_t(nactive), event_rate);
   profile_IO->Fill(Double_t(nactive), io_rate);

   return;
}

//______________________________________________________________________________
void TProofBenchRunDataRead::Print(Option_t* option) const
{

   Printf("Name=%s", fName.Data());
   if (fProof) fProof->Print(option);
   if (fMode) fMode->Print();
   Printf("fReadType=%s%s", "k", GetNameStem().Data());
   if (fRunCleanup) fRunCleanup->Print(option);
   Printf("fNEvents=%lld", fNEvents);
   Printf("fNTries=%d", fNTries);
   Printf("fStart=%d", fStart);
   Printf("fStop=%d", fStop);
   Printf("fStep=%d", fStep);
   Printf("fDebug=%d", fDebug);
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
void TProofBenchRunDataRead::SetNx(Int_t nx)
{
   fNx=nx;
}

//______________________________________________________________________________
void TProofBenchRunDataRead::SetDebug(Int_t debug)
{
   fDebug=debug;
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
Int_t TProofBenchRunDataRead::GetNx()const
{
   return fNx;
}

//______________________________________________________________________________
Int_t TProofBenchRunDataRead::GetDebug()const
{
   return fDebug;
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
Int_t TProofBenchRunDataRead::SetParameters()
{
   if (!fProof){
      Error("SetParameters", "Proof not set; Doing nothing");
      return 1;
   }
   
   fProof->SetParameter("PROOF_BenchmarkReadType", Int_t(fReadType));
   fProof->SetParameter("PROOF_BenchmarkDebug", Int_t(fDebug));
   return 0;
}

//______________________________________________________________________________
Int_t TProofBenchRunDataRead::DeleteParameters()
{
   if (!fProof){
      Error("DeleteParameters", "Proof not set; Doing nothing");
      return 1;
   }
   fProof->DeleteParameters("PROOF_BenchmarkReadType");
   fProof->DeleteParameters("PROOF_BenchmarkDebug");
   return 0;
}

//______________________________________________________________________________
TString TProofBenchRunDataRead::BuildPatternName(const TString& objname,
                                                 const TString& delimiter)
{
  
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
TString TProofBenchRunDataRead::BuildNewPatternName(const TString& objname,
                                       Int_t nactive, Int_t tries, Int_t nx,
                                       const TString& delimiter)
{
   if (!fMode){
      Error("BuildNewPatternName", "Mode is not set");
      //return 0;
      return "";
   }
   TString newname(BuildPatternName(objname, delimiter));
   newname+=delimiter;
   newname+=nactive;
   if (nx==0){
      newname+="Slaves_Run";
   }
   else if (nx==1){
      newname+="XSlaves_Run";
   }
   newname+=tries;
   //return newname.Data();
   return newname;
}

//______________________________________________________________________________
TString TProofBenchRunDataRead::BuildProfileName(const TString& objname,
                                 const TString& type, const TString& delimiter)
{
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
TString TProofBenchRunDataRead::BuildProfileTitle(const TString& objname,
                                 const TString& type, const TString& delimiter)
{
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
