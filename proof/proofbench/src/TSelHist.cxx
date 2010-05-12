#define TSelHist_cxx
// The class definition in TSelHist.h has been generated automatically
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
// Root > T->Process("TSelHist.C")
// Root > T->Process("TSelHist.C","some options")
// Root > T->Process("TSelHist.C+")
//

#include "TSelHist.h"
#include <TCanvas.h>
#include <TFrame.h>
#include <TPaveText.h>
#include <TFormula.h>
#include <TF1.h>
#include <TH1F.h>
#include <TH2.h>
#include <TH3.h>
#include <TMath.h>
#include <TRandom3.h>
#include <TString.h>
#include <TStyle.h>
#include <TSystem.h>
#include <TParameter.h>
#include <TROOT.h>

ClassImp(TSelHist)

//_____________________________________________________________________________
TSelHist::TSelHist()
   // Constructor
   :fHistType(TProofBenchRun::kHist1D),
   fNHists(16),
   fDraw(0),
   fHist1D(0),
   fHist2D(0),
   fHist3D(0),
   fRandom(0),
   fCHist1D(0),
   fCHist2D(0),
   fCHist3D(0)
{ }

//_____________________________________________________________________________
TSelHist::~TSelHist()
{
   // Destructor

   //if (fRandom) delete fRandom;
   SafeDelete(fRandom);

   // Info("TSelHist","destroying ...");
   
   if (!fDraw){
   for (Int_t i=0; i < fNHists; i++) {
      if (fHist1D && fHist1D[i] && !fOutput->FindObject(fHist1D[i])) {
         SafeDelete(fHist1D[i]);
      }
      if (fHist2D && fHist2D[i] && !fOutput->FindObject(fHist2D[i])) {
         SafeDelete(fHist2D[i]);
      }
      if (fHist3D && fHist3D[i] && !fOutput->FindObject(fHist3D[i])) {
         SafeDelete(fHist3D[i]);
      }
   }
   }
   SafeDelete(fHist1D);
   SafeDelete(fHist2D);
   SafeDelete(fHist3D);

//   SafeDelete(fCHist1D);
//   SafeDelete(fCHist2D);
//   SafeDelete(fCHist3D);
}

//_____________________________________________________________________________
void TSelHist::Begin(TTree * /*tree*/)
{
   // The Begin() function is called at the start of the query.
   // When running with PROOF Begin() is only called on the client.
   // The tree argument is deprecated (on PROOF 0 is passed).

   TString option = GetOption();

   Bool_t found_histtype=kFALSE;
   Bool_t found_nhists=kFALSE;
   Bool_t found_draw=kFALSE;

   TIter nxt(fInput);
   TString sinput;
   TObject *obj;

   while ((obj = nxt())){
      sinput=obj->GetName();
      Info("Begin", "object name=%s", sinput.Data());
      if (sinput.Contains("PROOF_BenchmarkHistType")){
         TParameter<Int_t>* a=dynamic_cast<TParameter<Int_t>*>(obj);
         if (a){
            fHistType=TProofBenchRun::EHistType(a->GetVal());
            found_histtype=kTRUE;
            Info("Begin", "PROOF_BenchmarkRunType=%d", fHistType);
         }
         else{
            Error("Begin", "PROOF_BenchmarkHistType not type TParameter<Int_t>*");
         } 
         continue;
      }
      if (sinput.Contains("PROOF_BenchmarkNHists")){
         TParameter<Int_t>* a=dynamic_cast<TParameter<Int_t>*>(obj);
         if (a){
            fNHists= a->GetVal();
            found_nhists=kTRUE;
            Info("Begin", "PROOF_BenchmarkNHists=%d", fNHists);
         }
         else{
            Error("Begin", "PROOF_BenchmarkNHists not type TParameter<Int_t>*");
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
   }

   if (!found_histtype){
      Warning("Begin", "PROOF_BenchmarkHistType not found; using default: %d", fHistType);
   }
   if (!found_nhists){
      Warning("Begin", "PROOF_BenchmarkNHists not found; using default: %d", fNHists);
   }
   if (!found_draw){
      Warning("Begin", "PROOF_BenchmarkDraw not found; using default: %d", fDraw);
   }

   if (fDraw) {
      if (fHistType & TProofBenchRun::kHist1D) fHist1D = new TH1F*[fNHists];
      if (fHistType & TProofBenchRun::kHist2D) fHist2D = new TH2F*[fNHists];
      if (fHistType & TProofBenchRun::kHist3D) fHist3D = new TH3F*[fNHists];
   }
}

//_____________________________________________________________________________
void TSelHist::SlaveBegin(TTree * /*tree*/)
{
   // The SlaveBegin() function is called after the Begin() function.
   // When running with PROOF SlaveBegin() is called on each slave server.
   // The tree argument is deprecated (on PROOF 0 is passed).

   TString option = GetOption();

   Bool_t found_histtype=kFALSE;
   Bool_t found_nhists=kFALSE;
   Bool_t found_draw=kFALSE;

   TIter nxt(fInput);
   TString sinput;
   TObject *obj;

   while ((obj = nxt())){
      sinput=obj->GetName();
      Info("SlaveBegin", "object name=%s", sinput.Data());
      if (sinput.Contains("PROOF_BenchmarkHistType")){
         TParameter<Int_t>* a=dynamic_cast<TParameter<Int_t>*>(obj);
         if (a){
            fHistType=TProofBenchRun::EHistType(a->GetVal());
            found_histtype=kTRUE;
            Info("SlaveBegin", "PROOF_BenchmarkHistType=%d", fHistType);
         }
         else{
            Error("SlaveBegin", "PROOF_BenchmarkHistType not type TParameter<Int_t>*");
         } 
         continue;
      }
      if (sinput.Contains("PROOF_BenchmarkNHists")){
         TParameter<Int_t>* a=dynamic_cast<TParameter<Int_t>*>(obj);
         if (a){
            fNHists=a->GetVal();
            found_nhists=kTRUE;
            Info("SlaveBegin", "PROOF_BenchmarkNHists=%d", fNHists);
         }
         else{
            Error("SlaveBegin", "PROOF_BenchmarkNHists not type TParameter<Int_t>*");
         } 
         continue;
      }
      if (sinput.Contains("PROOF_BenchmarkDraw")){
         TParameter<Int_t>* a=dynamic_cast<TParameter<Int_t>*>(obj);
         if (a){
            fDraw=a->GetVal();
            found_draw=kTRUE;
            Info("SlaveBegin", "PROOF_BenchmarkDraw=%d", fDraw);
         }
         else{
            Error("SlaveBegin", "PROOF_BenchmarkDraw not type TParameter<Int_t>*");
         } 
         continue;
      }
   }

   if (!found_histtype){
      Warning("Begin", "PROOF_BenchmarkHistType not found; using default: %d", fHistType);
   }
   if (!found_nhists){
      Warning("Begin", "PROOF_BenchmarkNHists not found; using default: %d", fNHists);
   }
   if (!found_draw){
      Warning("SlaveBegin", "PROOF_BenchmarkDraw not found; using default: %d", fDraw);
   }

   // Create the histogram
   if (fHistType & TProofBenchRun::kHist1D){
      fHist1D = new TH1F*[fNHists];
      for (Int_t i=0; i < fNHists; i++) {
         fHist1D[i] = new TH1F(Form("h1d_%d",i), Form("h1d_%d",i), 100, -3., 3.);
         fHist1D[i]->SetFillColor(kRed);
         if (fDraw) fOutput->Add(fHist1D[i]);
      }
   }
   if (fHistType & TProofBenchRun::kHist2D){
      fHist2D = new TH2F*[fNHists];
      for (Int_t i=0; i < fNHists; i++) {
         fHist2D[i] = new TH2F(Form("h2d_%d",i), Form("h2d_%d",i), 100, -3., 3., 100, -3., 3.);
         fHist2D[i]->SetFillColor(kRed);
         if (fDraw) fOutput->Add(fHist2D[i]);
      }
   }
   if (fHistType & TProofBenchRun::kHist3D){
      fHist3D = new TH3F*[fNHists];
      for (Int_t i=0; i < fNHists; i++) {
         fHist3D[i] = new TH3F(Form("h3d_%d",i), Form("h3d_%d",i), 100, -3., 3., 100, -3., 3., 100, -3., 3.);
         fHist3D[i]->SetFillColor(kRed);
         if (fDraw) fOutput->Add(fHist3D[i]);
      }
   }
   // Set random seed
   fRandom = new TRandom3(0);
}

//_____________________________________________________________________________
Bool_t TSelHist::Process(Long64_t entry)
{
   // The Process() function is called for each entry in the tree (or possibly
   // keyed object in the case of PROOF) to be processed. The entry argument
   // specifies which entry in the currently loaded tree is to be processed.
   // It can be passed to either TSelHist::GetEntry() or TBranch::GetEntry()
   // to read either all or the required parts of the data. When processing
   // keyed objects with PROOF, the object is already loaded and is available
   // via the fObject pointer.
   //
   // This function should contain the "body" of the analysis. It can contain
   // simple or elaborate selection criteria, run algorithms on the data
   // of the event and typically fill histograms.
   //
   // The processing can be stopped by calling Abort().
   //
   // Use fStatus to set the return value of TTree::Process().
   //
   // The return value is currently not used.

   Double_t x, y, z;
   if (fHistType & TProofBenchRun::kHist1D){
      for (Int_t i=0; i < fNHists; i++) {
         if (fRandom && fHist1D[i]) {
            x = fRandom->Gaus(0.,1.);
            fHist1D[i]->Fill(x);
         }
      }
   }
   if (fHistType & TProofBenchRun::kHist2D){
      for (Int_t i=0; i < fNHists; i++) {
         if (fRandom && fHist2D[i]) {
            x = fRandom->Gaus(0.,1.);
            y = fRandom->Gaus(0.,1.);
            fHist2D[i]->Fill(x, y);
         }
      }
   }
   if (fHistType & TProofBenchRun::kHist3D){
      for (Int_t i=0; i < fNHists; i++) {
         if (fRandom && fHist3D[i]) {
            x = fRandom->Gaus(0.,1.);
            y = fRandom->Gaus(0.,1.);
            z = fRandom->Gaus(0.,1.);
            fHist3D[i]->Fill(x, y, z);
         }
      }
   }

   return kTRUE;
}

//_____________________________________________________________________________
void TSelHist::SlaveTerminate()
{
   // The SlaveTerminate() function is called after all entries or objects
   // have been processed. When running with PROOF SlaveTerminate() is called
   // on each slave server.

}

//_____________________________________________________________________________
void TSelHist::Terminate()
{
   // The Terminate() function is the last function to be called during
   // a query. It always runs on the client, it can be used to present
   // the results graphically or save the results to file.

   //
   // Create a canvas, with 100 pads
   //

   if (!fDraw || gROOT->IsBatch()){
      return;
   }

   if (fHistType & TProofBenchRun::kHist1D){
      fCHist1D=dynamic_cast<TCanvas*>(gROOT->FindObject("CHist1D"));
      if (!fCHist1D){
         fCHist1D = new TCanvas("CHist1D","Proof TSelHist Canvas (1D)",200,10,700,700);
         Int_t nside = (Int_t)TMath::Sqrt((Float_t)fNHists);
         nside = (nside*nside < fNHists) ? nside+1 : nside;
         fCHist1D->Divide(nside,nside,0,0);
      }

      for (Int_t i=0; i < fNHists; i++) {
         fHist1D[i] = dynamic_cast<TH1F *>(fOutput->FindObject(Form("h1d_%d",i)));
         fCHist1D->cd(i+1);
         if (fHist1D[i]) fHist1D[i]->Draw();
      }
      // Final update
      fCHist1D->cd();
      fCHist1D->Update();
   }
   if (fHistType & TProofBenchRun::kHist2D){
      fCHist2D=dynamic_cast<TCanvas*>(gROOT->FindObject("CHist2D"));
      if (!fCHist2D){
         fCHist2D = new TCanvas("CHist2D","Proof TSelHist Canvas (2D)",200,10,700,700);
         Int_t nside = (Int_t)TMath::Sqrt((Float_t)fNHists);
         nside = (nside*nside < fNHists) ? nside+1 : nside;
         fCHist2D->Divide(nside,nside,0,0);
      }
      for (Int_t i=0; i < fNHists; i++) {
         fHist2D[i] = dynamic_cast<TH2F *>(fOutput->FindObject(Form("h2d_%d",i)));
         fCHist2D->cd(i+1);
         if (fHist2D[i]) fHist2D[i]->Draw("SURF");
      }
      // Final update
      fCHist2D->cd();
      fCHist2D->Update();
   }

   if (fHistType & TProofBenchRun::kHist3D){
      fCHist3D=dynamic_cast<TCanvas*>(gROOT->FindObject("CHist3D"));
      if (!fCHist3D){
         fCHist3D = new TCanvas("CHist3D","Proof TSelHist Canvas (3D)",200,10,700,700);
         Int_t nside = (Int_t)TMath::Sqrt((Float_t)fNHists);
         nside = (nside*nside < fNHists) ? nside+1 : nside;
         fCHist3D->Divide(nside,nside,0,0);
      }

     fOutput->Print("a");
      for (Int_t i=0; i < fNHists; i++) {
         fHist3D[i] = dynamic_cast<TH3F *>(fOutput->FindObject(Form("h3d_%d",i)));
         fCHist3D->cd(i+1);
      if (fHist3D[i]) printf("fHist3D[%d] found\n", i);
         if (fHist3D[i]) fHist3D[i]->Draw();
      }
      // Final update
      fCHist3D->cd();
      fCHist3D->Update();
   }

}

void TSelHist::Print(Option_t* option) const{
}
