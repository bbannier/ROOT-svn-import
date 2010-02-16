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
   fNTries(10),
   fNEvents(10000),
   fDraw(kFALSE),
   fCHist(0), 
   fPtHist(0),
   fNTracksHist(0)
{}

TSelEvent::TSelEvent()
   :fRunType(TProofBench::kRunNotSpecified),
   fNTries(10),
   fNEvents(10000),
   fDraw(kFALSE),
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

   TIter nxt(fInput);
   TString sinput;
   TObject *obj;
   while ((obj = nxt())){
      sinput=obj->GetName();
      if (sinput.Contains("fRunType")){
         TParameter<Int_t>* a=dynamic_cast<TParameter<Int_t>*>(obj);
         if (a){
            fRunType= a->GetVal();
            Info("Begin", "fRunType=%d", fRunType);
         }
         else{
            Error("Begin", "fRunType not type TParameter<Int_t>*"); 
         } 
         continue;
      }
      /* if (sinput.Contains("fBaseDir")){
         TNamed *a=dynamic_cast<TNamed*>(obj);
         if (a){
            TString basedir= a->GetTitle();
            basedir.Remove(TString::kBoth, ' '); //remove leading and trailing white space(s)
            basedir.Remove(TString::kBoth, '\t');//remove leading and trailing tab character(s)
            basedir.Remove(TString::kTrailing, '/'); //remove trailing /

            if (basedir.Length()>0 && gSystem->AccessPathName(basedir)) {
               Error("SlaveBegin", "No such file or directory: %s; using default directory: %s",
                     basedir.Data(), fBaseDir.Data());
               continue; 
            }
            fBaseDir=basedir;
         }
         else{
            Error("SlaveBegin", "fBaseDir not type TNamed *"); 
         } 
         continue;
      }*/
      if (sinput.Contains("fNTries")){
         TParameter<Int_t>* a=dynamic_cast<TParameter<Int_t>*>(obj);
         if (a){
            fNTries= a->GetVal();
            Info("Begin", "fNTries=%d", fNTries);
         }
         else{
            Error("Begin", "fNTries not type TParameter<Int_t>*"); 
         } 
         continue;
      }
      if (sinput.Contains("fNEvents")){
         TParameter<Long64_t>* a=dynamic_cast<TParameter<Long64_t>*>(obj);
         if (a){
            fNEvents= a->GetVal();
            Info("Begin", "fNEvents=%d", fNEvents);
         }
         else{
            Error("Begin", "fNEvents not type TParameter<Long64_t>*"); 
         } 
         continue;
      }
      if (sinput.Contains("fDraw")){
         TParameter<Int_t>* a=dynamic_cast<TParameter<Int_t>*>(obj);
         if (a){
            fDraw= a->GetVal();
            Info("Begin", "fDraw=%d", fDraw);
         }
         else{
            Error("Begin", "fDraw not type TParameter<Int_t>*"); 
         } 
         continue;
      }
   }
}

void TSelEvent::SlaveBegin(TTree *tree)
{
   // The SlaveBegin() function is called after the Begin() function.
   // When running with PROOF SlaveBegin() is called on each slave server.
   // The tree argument is deprecated (on PROOF 0 is passed).

   Init(tree);

   TString option = GetOption();

   fPtHist = new TH1F("pt_dist","p_{T} Distribution",100,0,5);
   fPtHist->SetDirectory(0);
   fPtHist->GetXaxis()->SetTitle("p_{T}");
   fPtHist->GetYaxis()->SetTitle("dN/p_{T}dp_{T}");

   fOutput->Add(fPtHist);

   fNTracksHist = new TH1I("ntracks_dist","N_{Tracks} per Event Distribution",5,0,5);
   fNTracksHist->SetDirectory(0);
   fNTracksHist->GetXaxis()->SetTitle("N_{Tracks}");
   fNTracksHist->GetYaxis()->SetTitle("N_{Events}");

   fOutput->Add(fNTracksHist);

   fRunType=0;

   //get parameters

   TIter nxt(fInput);
   TString sinput;
   TObject *obj;
   while ((obj = nxt())){
      sinput=obj->GetName();
      if (sinput.Contains("fRunType")){
         TParameter<Int_t>* a=dynamic_cast<TParameter<Int_t>*>(obj);
         if (a){
            fRunType= a->GetVal();
            Info("SlaveBegin", "fRunType=%d", fRunType);
         }
         else{
            Error("SlaveBegin", "fRunType not type TParameter<Int_t>*"); 
         } 
         continue;
      }
      /* if (sinput.Contains("fBaseDir")){
         TNamed *a=dynamic_cast<TNamed*>(obj);
         if (a){
            TString basedir= a->GetTitle();
            basedir.Remove(TString::kBoth, ' '); //remove leading and trailing white space(s)
            basedir.Remove(TString::kBoth, '\t');//remove leading and trailing tab character(s)
            basedir.Remove(TString::kTrailing, '/'); //remove trailing /

            if (basedir.Length()>0 && gSystem->AccessPathName(basedir)) {
               Error("SlaveBegin", "No such file or directory: %s; using default directory: %s",
                     basedir.Data(), fBaseDir.Data());
               continue; 
            }
            fBaseDir=basedir;
         }
         else{
            Error("SlaveBegin", "fBaseDir not type TNamed *"); 
         } 
         continue;
      }*/
      if (sinput.Contains("fNTries")){
         TParameter<Int_t>* a=dynamic_cast<TParameter<Int_t>*>(obj);
         if (a){
            fNTries= a->GetVal();
            Info("SlaveBegin", "fNTries=%d", fNTries);
         }
         else{
            Error("SlaveBegin", "fNTries not type TParameter<Int_t>*"); 
         } 
         continue;
      }
      if (sinput.Contains("fNEvents")){
         TParameter<Long64_t>* a=dynamic_cast<TParameter<Long64_t>*>(obj);
         if (a){
            fNEvents= a->GetVal();
            Info("SlaveBegin", "fNEvents=%d", fNEvents);
         }
         else{
            Error("SlaveBegin", "fNEvents not type TParameter<Long64_t>*"); 
         } 
         continue;
      }
      if (sinput.Contains("fDraw")){
         TParameter<Int_t>* a=dynamic_cast<TParameter<Int_t>*>(obj);
         if (a){
            fDraw= a->GetVal();
            Info("SlaveBegin", "fDraw=%d", fDraw);
         }
         else{
            Error("SlaveBegin", "fDraw not type TParameter<Int_t>*"); 
         } 
         continue;
      }
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
   case TProofBench::kRunCleanup:

      fChain->GetTree()->GetEntry(entry);
#if 0
      fNTracksHist->Fill(fNtrack);

      for(Int_t j=0;j<fTracks->GetEntries();j++){
         Track* curtrack = dynamic_cast<Track*>(fTracks->At(j));
         fPtHist->Fill(curtrack->GetPt(),1./curtrack->GetPt());
      }
      fTracks->Clear("C");
#endif
      break;

   case TProofBench::kRunOptDataRead: //partial read
      b_event_fNtrack->GetEntry(entry);

      fNTracksHist->Fill(fNtrack);
   
      if (fNtrack>0) {
         b_fTracks->GetEntry(entry);
         for(Int_t j=0;j<fTracks->GetEntries();j++){
           Track* curtrack = dynamic_cast<Track*>(fTracks->At(j));
           fPtHist->Fill(curtrack->GetPt(),1./curtrack->GetPt());
         }
         fTracks->Clear("C");
      }
 
      break;
   case TProofBench::kRunNoDataRead: //no read
      break;
   default:
      Error("Process", "Runtype (%d) not supported for this selector (TSelEvent) is requested", fRunType);
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
