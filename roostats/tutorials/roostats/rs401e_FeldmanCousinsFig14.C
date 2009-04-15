#include "TFile.h"
#include "TKey.h"
#include "RooStats/SamplingDistribution.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TMath.h"
#include <iostream>

using namespace std;

void Inspect(TH1F* cutoff, TH2F* cutoffMap){ 
   TFile f("SamplingDistributions.root"); 
   TIter next(f.GetListOfKeys()); 
   TKey *key; 

   Double_t chi2Nom = 4.61;
   //   chi2Nom=3.;
   Double_t prob = 1.-TMath::Prob(chi2Nom, 2);

   if(!cutoff) cutoff = new TH1F("cutoff","",60,0.,6.);
   if(!cutoffMap) cutoffMap = new TH2F("cutoffMap","",30, 0, .02, 30,1.,300.);

   int index = 0;
   int overFlowBin = 1;

   while ((key=(TKey*)next())) { 
     cout << key->GetName() << endl;
     RooStats::SamplingDistribution* samp = (RooStats::SamplingDistribution*) f.Get(key->GetName());

     if(samp->GetSize()>0){

       cout << samp->GetName() << " has " << samp->GetSize() << " samples" << " and cut off = " 
       	    << samp->InverseCDF(prob) << samp->InverseCDFInterpolate(prob) << endl;
       cutoff->Fill(2*samp->InverseCDFInterpolate(prob) );

       if ( (index%30) == 0) {overFlowBin+=2;}
       cutoffMap->SetBinContent(30+index+overFlowBin, 2*samp->InverseCDFInterpolate(prob) - chi2Nom);
       ++index;
     } 

   }
   cout << index << endl;

}

void rs401e_FeldmanCousinsFig14(){
  TCanvas* canv = new TCanvas("myCanvas","",500,800);
   canv->Divide(1,2);
   TH1F* cutoff = new TH1F("cutoff","histogram of -2 log LR_{critical}",60,0.,10.);
   TH2F* cutoffMap = new TH2F("cutoffMap","-2 log LR_{critical} - (-2 log LR_{Wilks})",30, 0, .02, 30,1.,300.);
   Inspect(cutoff,cutoffMap);
   canv->cd(1);
   cutoff->GetXaxis()->SetTitle("critical -2 log LR");
   cutoff->Draw();
   canv->cd(2);
   cutoffMap->Smooth();
   gStyle->SetPalette(1);
   gStyle->SetOptStat(0);
   cutoffMap->GetXaxis()->SetTitle("sin^{2}(2#theta)");
   cutoffMap->GetYaxis()->SetTitle("#Delta m^{2}");
   cutoffMap->Draw("contz");
   canv->SaveAs("fc_cutoff_map.pdf");
}
