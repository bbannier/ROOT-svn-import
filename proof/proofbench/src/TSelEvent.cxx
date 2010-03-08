#define TSelEvent_cxx
// The class definition in TSelEvent.h has been generated automatically
// by the ROOT utility TTree::MakeSelector(). This class is derived
// from the ROOT class TSelector. For more information on the TSelector
// framework see $ROOTSYS/README/README.SELECTOR or the ROOT User Manual.

// The following methods are defined in this file:
//    Begin():        called everytime a loop on the tree starts,
//                    a convenient place to create your histograms.
//    SlaveBegin():   called after Begin(), when on PROOF called only on the
//                    slave servers.
//    Process():      called for each event, in this function you decide what
//                    to read and fill your histograms.
//    SlaveTerminate: called at the end of the loop on the tree, when on PROOF
//                    called only on the slave servers.
//    Terminate():    called at the end of the loop on the tree,
//                    a convenient place to draw/fit your histograms.
//
// To use this file, try the following session on your Tree T:
//
// Root > T->Process("TSelEvent.C")
// Root > T->Process("TSelEvent.C","some options")
// Root > T->Process("TSelEvent.C+")
//

#include "TSelEvent.h"
#include <TH2.h>
#include <TStyle.h>
#include "TParameter.h"
#include "TProofServ.h"
#include "TSystem.h"
#include "TProofNodeInfo.h"
#include "TRandom.h"
#include "TProofBench.h"
#include "TROOT.h"
#include "TChain.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TFile.h"

ClassImp(TSelEvent)

TSelEvent::TSelEvent(TTree *)
   :fRunType(TProofBench::kRunNotSpecified), 
   fDraw(kFALSE),
   fDebug(kFALSE),
   fCHist(0), 
   fPtHist(0),
   fNTracksHist(0)
{}

TSelEvent::TSelEvent()
   :fRunType(TProofBench::kRunNotSpecified),
   fDraw(kFALSE),
   fDebug(kFALSE),
   fCHist(0),
   fPtHist(0),
   fNTracksHist(0)
{}

void TSelEvent::Begin(TTree *)
{
   // The Begin() function is called at the start of the query.
   // When running with PROOF Begin() is only called on the client.
   // The tree argument is deprecated (on PROOF 0 is passed).

   TString option = GetOption();

   //get parameters

   Bool_t found_runtype=kFALSE;
   Bool_t found_draw=kFALSE;
   Bool_t found_debug=kFALSE;

   TIter nxt(fInput);
   TString sinput;
   TObject *obj;
   while ((obj = nxt())){
      sinput=obj->GetName();
      if (sinput.Contains("PROOF_BenchmarkRunType")){
         TParameter<Int_t>* a=dynamic_cast<TParameter<Int_t>*>(obj);
         if (a){
            fRunType= a->GetVal();
            found_runtype=kTRUE;
            //Info("Begin", "PROOF_BenchmarkRunType=%d", fRunType);
         }
         else{
            Error("Begin", "PROOF_BenchmarkRunType not type TParameter<Int_t>*"); 
         } 
         continue;
      }
      if (sinput.Contains("PROOF_BenchmarkDraw")){
         TParameter<Int_t>* a=dynamic_cast<TParameter<Int_t>*>(obj);
         if (a){
            fDraw= a->GetVal();
            found_draw=kTRUE;
            //Info("Begin", "PROOF_BenchmarkDraw=%d", fDraw);
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
            //Info("Begin", "PROOF_BenchmarkDebug=%d", fDebug);
         }
         else{
            Error("Begin", "PROOF_BenchmarkDebug not type TParameter<Int_t>*"); 
         } 
         continue;
      }
   }
   if (!found_runtype){
      Warning("Begin", "PROOF_BenchmarkRunType not found; using default: %d", fRunType);
   }
   if (!found_draw){
      Warning("Begin", "PROOF_BenchmarkDraw not found; using default: %d", fDraw);
   }
   if (!found_debug){
      Warning("Begin", "PROOF_BenchmarkDebug not found; using default: %d", fDebug);
   }
}

void TSelEvent::SlaveBegin(TTree *tree)
{
   // The SlaveBegin() function is called after the Begin() function.
   // When running with PROOF SlaveBegin() is called on each slave server.
   // The tree argument is deprecated (on PROOF 0 is passed).

   Init(tree);

   TString option = GetOption();

   fRunType=0;

   //get parameters

   Bool_t found_runtype=kFALSE;
   Bool_t found_draw=kFALSE;
   Bool_t found_debug=kFALSE;

   fInput->Print("A");
   TIter nxt(fInput);
   TString sinput;
   TObject *obj;
   while ((obj = nxt())){
      sinput=obj->GetName();
      if (sinput.Contains("PROOF_BenchmarkRunType")){
         TParameter<Int_t>* a=dynamic_cast<TParameter<Int_t>*>(obj);
         if (a){
            fRunType= a->GetVal();
            found_runtype=kTRUE;
            Info("SlaveBegin", "PROOF_BenchmarkRunType=%d", fRunType);
         }
         else{
            Error("SlaveBegin", "PROOF_BenchmarkRunType not type TParameter<Int_t>*"); 
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
   }
   if (!found_runtype){
      Warning("Begin", "PROOF_BenchmarkRunType not found; using default: %d", fRunType);
   }
   if (!found_draw){
      Warning("Begin", "PROOF_BenchmarkDraw not found; using default: %d", fDraw);
   }
   if (!found_debug){
      Warning("Begin", "PROOF_BenchmarkDebug not found; using default: %d", fDebug);
   }

   if (fDraw || fDebug){
      fPtHist = new TH1F("pt_dist","p_{T} Distribution", 100, 0, 5);
      fPtHist->SetDirectory(0);
      fPtHist->GetXaxis()->SetTitle("p_{T}");
      fPtHist->GetYaxis()->SetTitle("dN/p_{T}dp_{T}");
   
      fOutput->Add(fPtHist);
   
      if (fRunType==TProofBench::kRunCleanup){
         fNTracksHist = new TH1I("ntracks_dist","N_{Tracks} per Event Distribution", 100, 50, 150);
      }
      else{
         fNTracksHist = new TH1I("ntracks_dist","N_{Tracks} per Event Distribution", 100, 5, 15);
      }
      //enable rebinning
      fNTracksHist->SetBit(TH1::kCanRebin);
      fNTracksHist->SetDirectory(0);
      fNTracksHist->GetXaxis()->SetTitle("N_{Tracks}");
      fNTracksHist->GetYaxis()->SetTitle("N_{Events}");
   
      fOutput->Add(fNTracksHist);
   }
}

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

   switch (fRunType){

   case TProofBench::kRunNotSpecified:
      Info("Process", "Runtype (%d) not specified, doing nothing");
      return kTRUE; 
      break;
   case TProofBench::kRunFullDataRead://full read
   case TProofBench::kRunCleanup://cleanup run

      fChain->GetTree()->GetEntry(entry);
      if (fDraw || fDebug){
         printf("fNtrack=%d\n", fNtrack);
         fNTracksHist->Fill(fNtrack);

         for(Int_t j=0;j<fTracks->GetEntries();j++){
            Track* curtrack = dynamic_cast<Track*>(fTracks->At(j));
            fPtHist->Fill(curtrack->GetPt(),1./curtrack->GetPt());
         }
      }
      fTracks->Clear("C");
      break;
   case TProofBench::kRunOptDataRead: //partial read
      b_event_fNtrack->GetEntry(entry);

      if (fDraw || fDebug){
         printf("fNtrack=%d\n", fNtrack);
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
   case TProofBench::kRunNoDataRead: //no read
      break;
   default:
      Error("Process", "Runtype not supported; %d", fRunType);
      break;
   }

   return kTRUE;
}

void TSelEvent::SlaveTerminate()
{
   // The SlaveTerminate() function is called after all entries or objects
   // have been processed. When running with PROOF SlaveTerminate() is called
   // on each slave server.

}

void TSelEvent::Terminate()
{
   // The Terminate() function is the last function to be called during
   // a query. It always runs on the client, it can be used to present
   // the results graphically or save the results to file.

   if (!fDraw || gROOT->IsBatch()){
      return;
   }

   if (!(fRunType==TProofBench::kRunFullDataRead || fRunType==TProofBench::kRunOptDataRead)){
      return;
   }

   fCHist=dynamic_cast<TCanvas*>(gROOT->FindObject("cPt"));
   if (!fCHist){
      fCHist = new TCanvas("cPt","P_t Distribution",800,600);
      fCHist->SetBorderMode(0);
   }
   fCHist->Clear();
   fCHist->cd();
   TH1F* h = dynamic_cast<TH1F*>(fOutput->FindObject("pt_dist"));
   if (h){
      if (h->GetEntries()>0) fCHist->SetLogy();
      else fCHist->SetLogy(0);
      h->DrawCopy();
      fCHist->Update();
   }
   else Warning("Terminate", "no pt dist found");
}
