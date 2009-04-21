#include "TFile.h"
#include "TKey.h"
#include "RooStats/SamplingDistribution.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TMath.h"
#include <iostream>

#include "RooChiSquare.h"
#include "RooRealVar.h"
#include "RooFitResult.h"
#ifndef __CINT__
#include "RooGlobalFunc.h"
#endif

using namespace std;
using namespace RooFit;

void Inspect(TH1F* cutoff, TH2F* cutoffMap, TH2F* effChi2Map, TH2F* scatter){ 
   TFile f("fc_big_samplingDist.root"); 
   TIter next(f.GetListOfKeys()); 
   TKey *key; 

   Double_t chi2Nom = 4.61;
   chi2Nom=2;
   Double_t prob = 1.-TMath::Prob(chi2Nom, 2);

   int index = 0;
   int overFlowBin = 1;

   Double_t tmpX, tmpY;

   RooRealVar x("x","",1,0,20);
   RooRealVar ndof("ndof","",2.,0,10);
   RooChiSquare pdf("pdf","",x,ndof);

   while ((key=(TKey*)next())) { 
     cout << key->GetName() << endl;
     RooStats::SamplingDistribution* samp = (RooStats::SamplingDistribution*) f.Get(key->GetName());
     TH1F* sampHist = (TH1F*) f.Get(key->GetName());

     if(samp->GetSize()>0){

       cout << samp->GetName() << " has " << samp->GetSize() << " samples" << " and cut off = " 
       	    << samp->InverseCDF(prob) << " interp as " << samp->InverseCDFInterpolate(prob) << endl;
       cutoff->Fill(2*samp->InverseCDFInterpolate(prob) );


       if ( (index%30) == 0) {overFlowBin+=2;}
       cutoffMap->SetBinContent(30+index+overFlowBin, 2*samp->InverseCDFInterpolate(prob) - chi2Nom);
       tmpY = 2*samp->InverseCDFInterpolate(prob);
     } else{

       RooMsgService::instance().setGlobalKillBelow(RooMsgService::ERROR) ;
       RooDataHist data("data","",x,sampHist);
       RooFitResult* result = pdf.fitTo(data,Save(kTRUE));
       RooMsgService::instance().setGlobalKillBelow(RooMsgService::DEBUG) ;
       cout << "size, result = " << result->floatParsFinal().getSize() << " " << 
	 ((RooRealVar*)(result->floatParsFinal().at(0)))->getVal() << endl;

       effChi2Map->SetBinContent(30+index+overFlowBin,
				 ((RooRealVar*)(result->floatParsFinal().at(0)))->getVal());	  
       tmpX = ((RooRealVar*)(result->floatParsFinal().at(0)))->getVal();	  
     }

     scatter->Fill(tmpX, tmpY);
     
     ++index;
     

   }
   cout << index << endl;

}

void rs401e_FeldmanCousinsFig14(){
  TCanvas* canv = new TCanvas("myCanvas","",900,800);
  canv->Divide(2,2);

   TH1F* cutoff = new TH1F("cutoff","histogram of -2 log LR_{critical}",60,0.,10.);
   TH2F* cutoffMap = new TH2F("cutoffMap","-2 log LR_{critical} - (-2 log LR_{Wilks})",30, 0, .02, 30,1.,300.);
   TH2F* effChi2Map = new TH2F("effChi2Map","effective #chi^{2}",30, 0, .02, 30,1.,300.);
   TH2F* scatter = new TH2F("scatter","",30, 0, 3, 30,0, 10.);

   Inspect(cutoff,cutoffMap, effChi2Map, scatter);

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

   canv->cd(3);
   //   effChi2Map->Smooth();
   gStyle->SetPalette(1);
   gStyle->SetOptStat(0);
   effChi2Map->GetXaxis()->SetTitle("sin^{2}(2#theta)");
   effChi2Map->GetYaxis()->SetTitle("#Delta m^{2}");
   effChi2Map->Draw("lego2");

   canv->cd(4);
   scatter->Draw("");
   scatter->GetXaxis()->SetTitle("effective #chi^{2}");
   scatter->GetYaxis()->SetTitle("-2 log LR_{critical}");

   canv->SaveAs("fc_cutoff_map.pdf");
}
