/**********************************************************************************
 * Project   : TMVA - a Root-integrated toolkit for multivariate data analysis    *
 * Package   : TMVA                                                               *
 * Exectuable: PlotDecisionBoundary                                               *
 *                                                                                *
 * Plots decision boundary for simple cases in 2 (3?) dimensions                  *
 **********************************************************************************/

#include <cstdlib>
#include <vector>
#include <iostream>
#include <map>
#include <string>

#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "TSystem.h"
#include "TROOT.h"
#include "TStopwatch.h"
#include "TGraph.h"
#include "TH2F.h"
#include "TH3F.h"
#include "TMVAGui.C"

#if not defined(__CINT__) || defined(__MAKECINT__)
#include "TMVA/Tools.h"
#include "TMVA/Reader.h"
#include "TMVA/MethodCuts.h"
#endif

using namespace TMVA;


void plot(TH2D *sig, TH2D *bkg, TH2F *MVA, TString v0="var0", TString v1="var1"){

   TCanvas *c = new TCanvas(Form("DecisionBoundary%s",MVA->GetTitle()),MVA->GetTitle(),800,800);

   gStyle->SetPalette(1);
   MVA->SetXTitle(v0);
   MVA->SetYTitle(v1);
   MVA->SetStats(0);
   MVA->Draw("cont1");
   sig->SetMarkerColor(2);
   bkg->SetMarkerColor(4);
   sig->SetMarkerStyle(20);
   bkg->SetMarkerStyle(20);
   sig->SetMarkerSize(.2);
   bkg->SetMarkerSize(.2);
   sig->Draw("same");
   bkg->Draw("same");
}


void PlotDecisionBoundary( TString weightFile = "weights/TMVAClassification_Fisher_B0000.weights.xml",TString v0="var0", TString v1="var1", TString dataFileName = "circledata.root",) 
{   
   //---------------------------------------------------------------
   // default MVA methods to be trained + tested

   // this loads the library
   TMVA::Tools::Instance();

   std::cout << std::endl;
   std::cout << "==> Start TMVAClassificationApplication" << std::endl;


   //
   // create the Reader object
   //
   TMVA::Reader *reader = new TMVA::Reader( "!Color:!Silent" );    

   // create a set of variables and declare them to the reader
   // - the variable names must corresponds in name and type to 
   // those given in the weight file(s) that you use
   Float_t var0, var1;
   reader->AddVariable( v0,                &var0 );
   reader->AddVariable( v1,                &var1 );

   //
   // book the MVA method
   //
   reader->BookMVA( "M1", weightFile ); 
   
   TFile *f = new TFile(dataFileName);
   TTree *signal     = (TTree*)f->Get("TreeS");
   TTree *background = (TTree*)f->Get("TreeB");


//Declaration of leaves types
   Float_t         svar0;
   Float_t         svar1;
   Float_t         bvar0;
   Float_t         bvar1;

   // Set branch addresses.
   signal->SetBranchAddress(v0,&svar0);
   signal->SetBranchAddress(v1,&svar1);
   background->SetBranchAddress(v0,&bvar0);
   background->SetBranchAddress(v1,&bvar1);


   UInt_t nbin = 50;
   Float_t xmax = signal->GetMaximum(v0.Data());
   Float_t xmin = signal->GetMinimum(v0.Data());
   Float_t ymax = signal->GetMaximum(v1.Data());
   Float_t ymin = signal->GetMinimum(v1.Data());
 
   xmax = TMath::Max(xmax,background->GetMaximum(v0.Data()));  
   xmin = TMath::Min(xmin,background->GetMinimum(v0.Data()));
   ymax = TMath::Max(ymax,background->GetMaximum(v1.Data()));
   ymin = TMath::Min(ymin,background->GetMinimum(v1.Data()));


   TH2D *hs=new TH2D("hs","",nbin,xmin,xmax,nbin,ymin,ymax);   
   TH2D *hb=new TH2D("hb","",nbin,xmin,xmax,nbin,ymin,ymax);   


   Long64_t nentries;
   nentries = TreeS->GetEntries();
   for (Long64_t is=0; is<nentries;is++) {
      signal->GetEntry(is);
      hs->Fill(svar0,svar1);
   }
   nentries = TreeB->GetEntries();
   for (Long64_t ib=0; ib<nentries;ib++) {
      background->GetEntry(ib);
      hb->Fill(bvar0,bvar1);
   }


   hb->SetMarkerColor(4);
   hs->SetMarkerColor(2);


   TH2F * hist = new TH2F( "MVA",    "MVA",    nbin,xmin,xmax,nbin,ymin,ymax);

   // Prepare input tree (this must be replaced by your data source)
   // in this example, there is a toy tree with signal and one with background events
   // we'll later on use only the "signal" events for the test in this example.

   for (Int_t ibin=1; ibin<nbin+1; ibin++){
      for (Int_t jbin=1; jbin<nbin+1; jbin++){
         var0 = hs->GetXaxis()->GetBinCenter(ibin);
         var1 = hs->GetYaxis()->GetBinCenter(jbin);
         hist->SetBinContent(ibin,jbin, reader->EvaluateMVA( "M1"    ) );
      }
   }


   delete reader;

   gStyle->SetPalette(1);

   plot(hs,hb,hist     ,v0,v1);

   
   //
   // write histograms
   //
   TFile *target  = new TFile( "TMVApp.root","RECREATE" );

   hs->Write();
   hb->Write();

   hist->Write();

   target->Close();

} 

