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
// TSelEvent                                                            //
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#define TSelEvent_cxx

#include "TSelEvent.h"
#include <TH1.h>
#include <TStyle.h>
#include "TParameter.h"
#include "TProofBenchRun.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TFileInfo.h"
#include "THashList.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "TSystem.h"

ClassImp(TSelEvent)

//______________________________________________________________________________
TSelEvent::TSelEvent(TTree *):
   fReadType(TProofBenchRun::kReadNotSpecified),
   fCleanupType(TProofBenchRun::kCleanupNotSpecified),
   fFilesToCleanupCacheFor(0),
   fDraw(kFALSE),
   fDebug(kFALSE),
   fCHist(0), 
   fPtHist(0),
   fNTracksHist(0)
{}

//______________________________________________________________________________
TSelEvent::TSelEvent():
   fReadType(TProofBenchRun::kReadNotSpecified),
   fCleanupType(TProofBenchRun::kCleanupNotSpecified),
   fFilesToCleanupCacheFor(0),
   fDraw(kFALSE),
   fDebug(kFALSE),
   fCHist(0),
   fPtHist(0),
   fNTracksHist(0)
{}

//______________________________________________________________________________
void TSelEvent::Begin(TTree *)
{
   // The Begin() function is called at the start of the query.
   // When running with PROOF Begin() is only called on the client.
   // The tree argument is deprecated (on PROOF 0 is passed).

   TString option = GetOption();

   //get parameters

   Bool_t found_readtype=kFALSE;
   Bool_t found_cleanuptype=kFALSE;
   Bool_t found_filestocleanupcachefor=kFALSE;
   Bool_t found_draw=kFALSE;
   Bool_t found_debug=kFALSE;

   TIter nxt(fInput);
   TString sinput;
   TObject *obj;
   while ((obj = nxt())){
      sinput=obj->GetName();
      Info("Begin", "name=%s", sinput.Data());
      if (sinput.Contains("PROOF_BenchmarkReadType")){
         TParameter<Int_t>* a=dynamic_cast<TParameter<Int_t>*>(obj);
         if (a){
            fReadType=TProofBenchRun::EReadType(a->GetVal());
            found_readtype=kTRUE;
            Info("Begin", "PROOF_BenchmarkReadType=%d", fReadType);
         }
         else{
            Error("Begin", "PROOF_BenchmarkReadType not type TParameter<Int_t>*"); 
         } 
         continue;
      }
      if (sinput.Contains("PROOF_BenchmarkCleanupType")){
         TParameter<Int_t>* a=dynamic_cast<TParameter<Int_t>*>(obj);
         if (a){
            fCleanupType=TProofBenchRun::ECleanupType(a->GetVal());
            found_cleanuptype=kTRUE;
            Info("Begin", "PROOF_BenchmarkCleanupType=%d", fCleanupType);
         }
         else{
            Error("Begin", "PROOF_BenchmarkCleanupType not type TParameter<Int_t>*"); 
         } 
         continue;
      }
      if (sinput.Contains("PROOF_BenchmarkDraw")){
         TParameter<Int_t>* a=dynamic_cast<TParameter<Int_t>*>(obj);
         if (a){
            fDraw= a->GetVal();
            found_draw=kTRUE;
            Info("Begin", "PROOF_BenchmarkDraw=%d", fDraw);
         }
         else{
            Error("Begin", "PROOF_BenchmarkDraw not type TParameter<Int_t>*"); 
         } 
         continue;
      }
      if (sinput.Contains("PROOF_BenchmarkDebug")){
         TParameter<Int_t>* a=dynamic_cast<TParameter<Int_t>*>(obj);
         if (a){
            fDebug= a->GetVal();
            found_debug=kTRUE;
            Info("Begin", "PROOF_BenchmarkDebug=%d", fDebug);
         }
         else{
            Error("Begin", "PROOF_BenchmarkDebug not type TParameter<Int_t>*"); 
         } 
         continue;
      }
      if (sinput.Contains("PROOF_BenchmarkFilesToCleanupCacheFor")){
         THashList* l=dynamic_cast<THashList*>(obj);
         if (l){
            fFilesToCleanupCacheFor=l;
            found_filestocleanupcachefor=kTRUE;
            Info("Begin", "PROOF_BenchmarkFilesToCleanupCacheFor:");
            fFilesToCleanupCacheFor->Print("A");
         }
         else{
            Error("Begin", "PROOF_BenchmarkFilesToCleanupCacheFor not type THashList*"); 
         } 
         continue;
      }
   }

   if (!found_readtype && !found_cleanuptype){
      Error("Begin", "Neither of PROOF_BenchmarkReadType nor PROOF_BenchmarkCleanupType found");
      return;
   }
   if (!found_draw){
      Warning("Begin", "PROOF_BenchmarkDraw not found; using default: %d", fDraw);
   }
   if (!found_debug){
      Warning("Begin", "PROOF_BenchmarkDebug not found; using default: %d", fDebug);
   }
}

//______________________________________________________________________________
void TSelEvent::SlaveBegin(TTree *tree)
{
   // The SlaveBegin() function is called after the Begin() function.
   // When running with PROOF SlaveBegin() is called on each slave server.
   // The tree argument is deprecated (on PROOF 0 is passed).

   Init(tree);

   TString option = GetOption();

   Bool_t found_readtype=kFALSE;
   Bool_t found_cleanuptype=kFALSE;
   Bool_t found_filestocleanupcachefor=kFALSE;
   Bool_t found_draw=kFALSE;
   Bool_t found_debug=kFALSE;

   //fInput->Print("A");
   TIter nxt(fInput);
   TString sinput;
   TObject *obj;
   while ((obj = nxt())){
      sinput=obj->GetName();
      Info("SlaveBegin", "name=%s", sinput.Data());
      if (sinput.Contains("PROOF_BenchmarkReadType")){
         TParameter<Int_t>* a=dynamic_cast<TParameter<Int_t>*>(obj);
         if (a){
            fReadType=TProofBenchRun::EReadType(a->GetVal());
            found_readtype=kTRUE;
            Info("SlaveBegin", "PROOF_BenchmarkReadType=%d", fReadType);
         }
         else{
            Error("SlaveBegin", "PROOF_BenchmarkReadType not type TParameter<Int_t>*"); 
         } 
         continue;
      }
      if (sinput.Contains("PROOF_BenchmarkCleanupType")){
         TParameter<Int_t>* a=dynamic_cast<TParameter<Int_t>*>(obj);
         if (a){
            fCleanupType=TProofBenchRun::ECleanupType(a->GetVal());
            found_cleanuptype=kTRUE;
            Info("SlaveBegin", "PROOF_BenchmarkCleanupType=%d", fCleanupType);
         }
         else{
            Error("SlaveBegin", "PROOF_BenchmarkCleanupType not type TParameter<Int_t>*"); 
         } 
         continue;
      }
      if (sinput.Contains("PROOF_BenchmarkDraw")){
         TParameter<Int_t>* a=dynamic_cast<TParameter<Int_t>*>(obj);
         if (a){
            fDraw= a->GetVal();
            found_draw=kTRUE;
            Info("SlaveBegin", "PROOF_BenchmarkDraw=%d", fDraw);
         }
         else{
            Error("SlaveBegin", "PROOF_BenchmarkDraw not type TParameter<Int_t>*"); 
         } 
         continue;
      }
      if (sinput.Contains("PROOF_BenchmarkDebug")){
         TParameter<Int_t>* a=dynamic_cast<TParameter<Int_t>*>(obj);
         if (a){
            fDebug= a->GetVal();
            found_debug=kTRUE;
            Info("SlaveBegin", "PROOF_BenchmarkDebug=%d", fDebug);
         }
         else{
            Error("SlaveBegin", "PROOF_BenchmarkDebug not type TParameter<Int_t>*"); 
         } 
         continue;
      }
      if (sinput.Contains("PROOF_BenchmarkFilesToCleanupCacheFor")){
         THashList* l=dynamic_cast<THashList*>(obj);
         if (l){
            fFilesToCleanupCacheFor=l;
            found_filestocleanupcachefor=kTRUE;
            Info("SlaveBegin", "PROOF_BenchmarkFilesToCleanupCacheFor:");
            fFilesToCleanupCacheFor->Print("A");
         }
         else{
            Error("SlaveBegin", "PROOF_BenchmarkFilesToCleanupCacheFor not type THashList*"); 
         } 
         continue;
      }
   }

   if (!found_readtype && !found_cleanuptype){
      Error("SlaveBegin", "Neither of PROOF_BenchmarkReadType  PROOF_BenchmarkCleanupType found");
      return;
   }
   if (!found_draw){
      Warning("SlaveBegin", "PROOF_BenchmarkDraw not found; using default: %d", fDraw);
   }
   if (!found_debug){
      Warning("SlaveBegin", "PROOF_BenchmarkDebug not found; using default: %d", fDebug);
   }

   if (fDraw || fDebug){
      fPtHist = new TH1F("pt_dist","p_{T} Distribution", 100, 0, 5);
      fPtHist->SetDirectory(0);
      fPtHist->GetXaxis()->SetTitle("p_{T}");
      fPtHist->GetYaxis()->SetTitle("dN/p_{T}dp_{T}");
   
      fOutput->Add(fPtHist);
   
/*      if (fRun==TProofBenchRun::kRunCleanup){
         fNTracksHist = new TH1I("ntracks_dist","N_{Tracks} per Event Distribution", 100, 50, 150);
      }
      else{
         fNTracksHist = new TH1I("ntracks_dist","N_{Tracks} per Event Distribution", 100, 5, 15);
      }
*/
      //enable rebinning
      fNTracksHist->SetBit(TH1::kCanRebin);
      fNTracksHist->SetDirectory(0);
      fNTracksHist->GetXaxis()->SetTitle("N_{Tracks}");
      fNTracksHist->GetYaxis()->SetTitle("N_{Events}");
   
      fOutput->Add(fNTracksHist);
   }

   //clear file cache
   if (fCleanupType==TProofBenchRun::kCleanupKernel){
      TIter nxt(fFilesToCleanupCacheFor);
      TFileInfo* fi=0;
      while ((fi=dynamic_cast<TFileInfo*>(nxt()))){
         TUrl* url=0;
//         do {
            url=fi->GetCurrentUrl();
//            if (!url) break;
            TString hostname=url->GetHostFQDN(); 
            TString localhostname=TUrl(gSystem->HostName()).GetHostFQDN();
            if (hostname==localhostname){
               TString filename=url->GetFile();
               Info("SlaveBegin", "Cleaning up cache for file: %s", filename.Data());
               Int_t fd;
               fd = open(filename.Data(), O_RDONLY);
               if (fd) {
                  fdatasync(fd);
                  posix_fadvise(fd, 0,0,POSIX_FADV_DONTNEED);
                  close(fd);
               } else {
                  Error("SlaveBegin", "Cannot open file for clean up: %s", filename.Data());
               }
            }
//         } while ((url=fi->NextUrl()));
      }
   }
}

//______________________________________________________________________________
Bool_t TSelEvent::Process(Long64_t entry)
{
   // The Process() function is called for each entry in the tree (or possibly
   // keyed object in the case of PROOF) to be processed. The entry argument
   // specifies which entry in the currently loaded tree is to be processed.
   // It can be passed to either TTree::GetEntry() or TBranch::GetEntry()
   // to read either all or the required parts of the data. When processing
   // keyed objects with PROOF, the object is already loaded and is available
   // via the fObject pointer.
   //
   // This function should contain the "body" of the analysis. It can contain
   // simple or elaborate selection criteria, run algorithms on the data
   // of the event and typically fill histograms.

   // WARNING when a selector is used with a TChain, you must use
   //  the pointer to the current TTree to call GetEntry(entry).
   //  The entry is always the local entry number in the current tree.
   //  Assuming that fChain is the pointer to the TChain being processed,
   //  use fChain->GetTree()->GetEntry(entry).
 
   if (fReadType!=TProofBenchRun::kReadNotSpecified){
      switch (fReadType){
      case TProofBenchRun::kReadNotSpecified:
         Info("Process", "Run type not specified, doing nothing");
         //return kTRUE; 
         return kFALSE; 
         break;
      case TProofBenchRun::kReadFull://full read
         fChain->GetTree()->GetEntry(entry);
         if (fDraw || fDebug){
            //printf("fNtrack=%d\n", fNtrack);
            fNTracksHist->Fill(fNtrack);
   
            for(Int_t j=0;j<fTracks->GetEntries();j++){
               Track* curtrack = dynamic_cast<Track*>(fTracks->At(j));
               fPtHist->Fill(curtrack->GetPt(),1./curtrack->GetPt());
            }
         }
         fTracks->Clear("C");
         break;
      case TProofBenchRun::kReadOpt: //partial read
         b_event_fNtrack->GetEntry(entry);
   
         if (fDraw || fDebug){
            //printf("fNtrack=%d\n", fNtrack);
            fNTracksHist->Fill(fNtrack);
         }
      
         if (fNtrack>0) {
            b_fTracks->GetEntry(entry);
            if (fDraw || fDebug){
               for(Int_t j=0;j<fTracks->GetEntries();j++){
                 Track* curtrack = dynamic_cast<Track*>(fTracks->At(j));
                 fPtHist->Fill(curtrack->GetPt(),1./curtrack->GetPt());
               }
            }
            fTracks->Clear("C");
         }
         break;
      case TProofBenchRun::kReadNo: //no read
         break;
      default:
         Error("Process", "Read type not supported; %d", fReadType);
         return kFALSE;
         break;
      }
   }
   if (fCleanupType!=TProofBenchRun::kCleanupNotSpecified){
      switch (fCleanupType){
      case TProofBenchRun::kCleanupFile:
         fChain->GetTree()->GetEntry(entry);
         if (fDraw || fDebug){
            //printf("fNtrack=%d\n", fNtrack);
            fNTracksHist->Fill(fNtrack);

            for(Int_t j=0;j<fTracks->GetEntries();j++){
               Track* curtrack = dynamic_cast<Track*>(fTracks->At(j));
               fPtHist->Fill(curtrack->GetPt(),1./curtrack->GetPt());
            }
         }
         fTracks->Clear("C");
         break;

      case TProofBenchRun::kCleanupKernel:
         break;
      default:
         Error("Process", "Cleanup type not supported; %d", fCleanupType);
         break;
      }
   }
   return kTRUE;
}

//______________________________________________________________________________
void TSelEvent::SlaveTerminate()
{
   // The SlaveTerminate() function is called after all entries or objects
   // have been processed. When running with PROOF SlaveTerminate() is called
   // on each slave server.

}

//______________________________________________________________________________
void TSelEvent::Terminate()
{
   // The Terminate() function is the last function to be called during
   // a query. It always runs on the client, it can be used to present
   // the results graphically or save the results to file.
}
