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
// TSelEvent                                                            //
//                                                                      //
// PROOF selector for I/O benchmark test.                               //
// For I/O benchmark, event files are read in and histograms are filled.//
// For memory clean-up, dedicated files large enough to clean up memory //
// cache on the machine are read in. Or memory clean-up can be          //
// accompolished by system call on Linux machine inside SlaveBegin(..)  //
// which should be much faster the reading in large files.              //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#define TSelEvent_cxx

#include "TSelEvent.h"
#include <TH1F.h>
#include <TStyle.h>
#include "TParameter.h"
#include "TProofBenchTypes.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TFileInfo.h"
#include "THashList.h"
#include "TClonesArray.h"
#include "TRefArray.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "TSystem.h"
#include "TROOT.h"

ClassImp(TSelEvent)

//______________________________________________________________________________
TSelEvent::TSelEvent(TTree *):
fReadType(kPBReadNotSpecified),
fCleanupType(kPBCleanupNotSpecified),
fFilesToCleanupCacheFor(0), fDebug(kFALSE), fCHist(0), fPtHist(0),
fNTracksHist(0), fEventName(0), fTracks(0), fHighPt(0), fMuons(0),
fH(0), b_event_fType(0), b_fEventName(0), b_event_fNtrack(0), b_event_fNseg(0),
b_event_fNvertex(0), b_event_fFlag(0), b_event_fTemperature(0),
b_event_fMeasures(0), b_event_fMatrix(0), b_fClosestDistance(0),
b_event_fEvtHdr(0), b_fTracks(0), b_fHighPt(0), b_fMuons(0),
b_event_fLastTrack(0), b_event_fWebHistogram(0), b_fH(0),
b_event_fTriggerBits(0), b_event_fIsValid(0)
{}

//______________________________________________________________________________
TSelEvent::TSelEvent():
fReadType(kPBReadNotSpecified),
fCleanupType(kPBCleanupNotSpecified),
fFilesToCleanupCacheFor(0), fDebug(kFALSE), fCHist(0), fPtHist(0),
fNTracksHist(0), fEventName(0), fTracks(0), fHighPt(0), fMuons(0),
fH(0), b_event_fType(0), b_fEventName(0), b_event_fNtrack(0), b_event_fNseg(0),
b_event_fNvertex(0), b_event_fFlag(0), b_event_fTemperature(0),
b_event_fMeasures(0), b_event_fMatrix(0), b_fClosestDistance(0),
b_event_fEvtHdr(0), b_fTracks(0), b_fHighPt(0), b_fMuons(0),
b_event_fLastTrack(0), b_event_fWebHistogram(0), b_fH(0),
b_event_fTriggerBits(0), b_event_fIsValid(0)
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
   Bool_t found_debug=kFALSE;

   TIter nxt(fInput);
   TString sinput;
   TObject *obj;
   while ((obj = nxt())){
      sinput=obj->GetName();
      //Info("Begin", "name=%s", sinput.Data());
      if (sinput.Contains("PROOF_BenchmarkReadType")){
         TParameter<Int_t>* a=dynamic_cast<TParameter<Int_t>*>(obj);
         if (a){
            fReadType=EPBReadType(a->GetVal());
            found_readtype=kTRUE;
            //Info("Begin", "PROOF_BenchmarkReadType=%d", fReadType);
         }
         else{
            Error("Begin", "PROOF_BenchmarkReadType not type TParameter"
                           "<Int_t>*"); 
         } 
         continue;
      }
      if (sinput.Contains("PROOF_BenchmarkCleanupType")){
         TParameter<Int_t>* a=dynamic_cast<TParameter<Int_t>*>(obj);
         if (a){
            fCleanupType=EPBCleanupType(a->GetVal());
            found_cleanuptype=kTRUE;
            //Info("Begin", "PROOF_BenchmarkCleanupType=%d", fCleanupType);
         }
         else{
            Error("Begin", "PROOF_BenchmarkCleanupType not type TParameter"
                           "<Int_t>*"); 
         } 
         continue;
      }
      if (sinput.Contains("PROOF_BenchmarkDebug")){
         TParameter<Int_t>* a=dynamic_cast<TParameter<Int_t>*>(obj);
         if (a){
            fDebug= a->GetVal();
            found_debug=kTRUE;
            //Info("Begin", "PROOF_BenchmarkDebug=%d", fDebug);
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
            //Info("Begin", "PROOF_BenchmarkFilesToCleanupCacheFor:");
            fFilesToCleanupCacheFor->Print("A");
         }
         else{
            Error("Begin", "PROOF_BenchmarkFilesToCleanupCacheFor not type"
                           " THashList*"); 
         } 
         continue;
      }
   }

   if (!found_readtype && !found_cleanuptype){
      Error("Begin", "Neither of PROOF_BenchmarkReadType nor"
                     " PROOF_BenchmarkCleanupType found");
      return;
   }
   if (!found_debug){
      Warning("Begin", "PROOF_BenchmarkDebug not found; using default: %d",
                       fDebug);
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
   Bool_t found_debug=kFALSE;

   //fInput->Print("A");
   TIter nxt(fInput);
   TString sinput;
   TObject *obj;
   while ((obj = nxt())){
      sinput=obj->GetName();
      //Info("SlaveBegin", "name=%s", sinput.Data());
      if (sinput.Contains("PROOF_BenchmarkReadType")){
         TParameter<Int_t>* a=dynamic_cast<TParameter<Int_t>*>(obj);
         if (a){
            fReadType=EPBReadType(a->GetVal());
            found_readtype=kTRUE;
            //Info("SlaveBegin", "PROOF_BenchmarkReadType=%d", fReadType);
         }
         else{
            Error("SlaveBegin", "PROOF_BenchmarkReadType not type TParameter"
                                "<Int_t>*"); 
         } 
         continue;
      }
      if (sinput.Contains("PROOF_BenchmarkCleanupType")){
         TParameter<Int_t>* a=dynamic_cast<TParameter<Int_t>*>(obj);
         if (a){
            fCleanupType=EPBCleanupType(a->GetVal());
            found_cleanuptype=kTRUE;
            //Info("SlaveBegin", "PROOF_BenchmarkCleanupType=%d", fCleanupType);
         }
         else{
            Error("SlaveBegin", "PROOF_BenchmarkCleanupType not type TParameter"
                                "<Int_t>*"); 
         } 
         continue;
      }
      if (sinput.Contains("PROOF_BenchmarkDebug")){
         TParameter<Int_t>* a=dynamic_cast<TParameter<Int_t>*>(obj);
         if (a){
            fDebug= a->GetVal();
            found_debug=kTRUE;
            //Info("SlaveBegin", "PROOF_BenchmarkDebug=%d", fDebug);
         }
         else{
            Error("SlaveBegin", "PROOF_BenchmarkDebug not type TParameter"
                                "<Int_t>*"); 
         } 
         continue;
      }
      if (sinput.Contains("PROOF_BenchmarkFilesToCleanupCacheFor")){
         THashList* l=dynamic_cast<THashList*>(obj);
         if (l){
            fFilesToCleanupCacheFor=l;
            found_filestocleanupcachefor=kTRUE;
            //Info("SlaveBegin", "PROOF_BenchmarkFilesToCleanupCacheFor:");
            //fFilesToCleanupCacheFor->Print("A");
         }
         else{
            Error("SlaveBegin", "PROOF_BenchmarkFilesToCleanupCacheFor not type"
                                " THashList*"); 
         } 
         continue;
      }
   }

   if (!found_readtype && !found_cleanuptype){
      Error("SlaveBegin", "Neither of PROOF_BenchmarkReadType"
                          "  PROOF_BenchmarkCleanupType found");
      return;
   }
   if (!found_debug){
      Warning("SlaveBegin", "PROOF_BenchmarkDebug not found; using default: %d",
                            fDebug);
   }

   if (fDebug){
      fPtHist = new TH1F("pt_dist","p_{T} Distribution", 100, 0, 5);
      fPtHist->SetDirectory(0);
      fPtHist->GetXaxis()->SetTitle("p_{T}");
      fPtHist->GetYaxis()->SetTitle("dN/p_{T}dp_{T}");
   
      fOutput->Add(fPtHist);
   
      fNTracksHist = new TH1F("ntracks_dist","N_{Tracks} per Event"
                              " Distribution", 100, 50, 150);
      //enable rebinning
      fNTracksHist->SetBit(TH1::kCanRebin);
      fNTracksHist->SetDirectory(0);
      fNTracksHist->GetXaxis()->SetTitle("N_{Tracks}");
      fNTracksHist->GetYaxis()->SetTitle("N_{Events}");
   
      fOutput->Add(fNTracksHist);
   }

   //clear file cache
   if (fCleanupType==kPBCleanupFileAdvise){
      TIter nxtf(fFilesToCleanupCacheFor);
      TFileInfo* fi=0;
      while ((fi=dynamic_cast<TFileInfo*>(nxtf()))){
         TUrl* url=0;
//         do {
            url=fi->GetCurrentUrl();
//            if (!url) break;
            TString hostname=url->GetHostFQDN(); 
            TString localhostname=TUrl(gSystem->HostName()).GetHostFQDN();
            if (hostname.IsNull() || hostname == localhostname){
               TString filename=url->GetFile();
               Info("SlaveBegin", "Cleaning up file cache of file %s.",
                                   filename.Data());
               Int_t fd;
               fd = open(filename.Data(), O_RDONLY);
               if (fd) {
                  fdatasync(fd);
                  posix_fadvise(fd, 0,0,POSIX_FADV_DONTNEED);
                  close(fd);
               } else {
                  Error("SlaveBegin", "Cannot open file for clean up: %s",
                                       filename.Data());
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
 
   if (fReadType!=kPBReadNotSpecified){
      switch (fReadType){
      case kPBReadNotSpecified:
         Info("Process", "Run type not specified, doing nothing");
         //return kTRUE; 
         return kFALSE; 
         break;
      case kPBReadFull://full read
         fChain->GetTree()->GetEntry(entry);
         if (fDebug){
            //printf("fNtrack=%d\n", fNtrack);
            fNTracksHist->Fill(fNtrack);
   
            for(Int_t j=0;j<fTracks->GetEntries();j++){
               Track* curtrack = dynamic_cast<Track*>(fTracks->At(j));
               fPtHist->Fill(curtrack->GetPt(),1./curtrack->GetPt());
            }
         }
         fTracks->Clear("C");
         break;
      case kPBReadOpt: //partial read
         b_event_fNtrack->GetEntry(entry);
   
         if (fDebug){
            //printf("fNtrack=%d\n", fNtrack);
            fNTracksHist->Fill(fNtrack);
         }
      
         if (fNtrack>0) {
            b_fTracks->GetEntry(entry);
            if (fDebug){
               for(Int_t j=0;j<fTracks->GetEntries();j++){
                 Track* curtrack = dynamic_cast<Track*>(fTracks->At(j));
                 fPtHist->Fill(curtrack->GetPt(),1./curtrack->GetPt());
               }
            }
            fTracks->Clear("C");
         }
         break;
      case kPBReadNo: //no read
         break;
      default:
         Error("Process", "Read type not supported; %d", fReadType);
         return kFALSE;
         break;
      }
   }
   if (fCleanupType!=kPBCleanupNotSpecified){
      switch (fCleanupType){
      case kPBCleanupReadInFiles:
         fChain->GetTree()->GetEntry(entry);
         if (fDebug){
            //printf("fNtrack=%d\n", fNtrack);
            fNTracksHist->Fill(fNtrack);

            for(Int_t j=0;j<fTracks->GetEntries();j++){
               Track* curtrack = dynamic_cast<Track*>(fTracks->At(j));
               fPtHist->Fill(curtrack->GetPt(),1./curtrack->GetPt());
            }
         }
         fTracks->Clear("C");
         break;

      case kPBCleanupFileAdvise:
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
