// ------------------------------------------------------------------------
//
// TKDE tutorial macro
// 
// Using semi-parametric 1-dimensional kernel density estimation 
//
//
//
// 6/2010 - Bartolomeu Rabacal
// 
// ------------------------------------------------------------------------

#include "TCanvas.h"
#include "TH1D.h"
#include "TKDE.h"
#include "TRandom3.h"
#include "TTree.h"


void kde_tutorial() {

   // C r e a t e   l o w   s t a t s   1D  s a m p l e 
   // -------------------------------------------------------
   
   // 1D sample point
   Double_t x; 
   
   // Create a ROOT Tree for holding data
   TTree *tree = new TTree("Data Sample Tree", "A ROOT tree with 1D data sample branch");
   tree->Branch("x", &x, "x/D");
   
   // Sample 100 gaussian distributed events and fill the Tree
   UInt_t nEvents = 100;
   for (UInt_t i = 0; i < nEvents; ++i) {
      x = gRandom->Gaus(300.0, 50.0);
      tree->Fill();
   }
   
   // Create a histogram for the sample
   TH1D* hx = new TH1D("hx", "Sample's distribution", 50, 0, 500);

   // Get the data from the Tree but do not draw the histogram
   tree->Draw("x>>hx","","goff");
    
   // Normalize histogram
   hx->Scale(1. / nEvents, "width");
   
   
   // C r e a t e   1D   k e r n e l   d e n s i t y   e s t i m a t i o n   p. d. f.
   // ------------------------------------------------------------------------------------
   
   // Create default kernel density estimation (KDE) p.d.f. whose options are:
   // adaptive iteration, no data mirroring and relaxed binning 
   Double_t* sample = tree->GetV1();
   
   TKDE* kde = new TKDE(nEvents, sample);
   
   // Plot the KDE p.d.f. and the sample histogram in the first pad, plot the approximate bias in the second
   TCanvas* c1 = new TCanvas("c1","c1");
   c1->Divide(2,1);
   c1->cd(1);
   hx->Draw();
   kde->GetFunction()->SetLineColor(kBlue);
   kde->GetFunction()->Draw("SAME");
   c1->  cd(2);
   kde->GetApproximateBias()->SetLineColor(kRed);
   kde->GetApproximateBias()->Draw();
}
