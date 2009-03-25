/////////////////////////////////////////////////////////////////////////
//
// 'Debugging Sampling Distribution' RooStats tutorial macro #401
// author: Kyle Cranmer
// date Jan. 2009
//
// This tutorial shows usage of a distribution creator, sampling distribution,
// and the Neyman Construction.
/////////////////////////////////////////////////////////////////////////

#ifndef __CINT__
#include "RooGlobalFunc.h"
#endif
#include "RooStats/ConfInterval.h"
#include "RooStats/FeldmanCousins.h"

#include "RooDataSet.h"
#include "RooRealVar.h"
#include "RooConstVar.h"
#include "RooAddition.h"
#include "RooProduct.h"
#include "RooProdPdf.h"
#include "RooAddPdf.h"

//#include "RooGenericPdf.h"
#include "NuMuToNuE_Oscillation.h"
#include "RooPolynomial.h"

#include "RooPlot.h"

#include "TCanvas.h"
#include "TH1F.h"
#include "TTree.h"
#include "TMarker.h"
#include "TStopwatch.h"

#include <iostream>

// use this order for safety on library loading
using namespace RooFit ;
using namespace RooStats ;


void rs401d_FeldmanCousins()
{
  
  // to time the macro
  TStopwatch t;
  t.Start();

  /*
Taken from Feldman & Cousins paper, Phys.Rev.D57:3873-3889,1998. 
e-Print: physics/9711021 (see page 13.)
To demonstrate how this works in practice, and how it compares to alternative approaches 
that have been used, we consider a toy model of a typical neutrino oscillation experiment. 
The toy model is deﬁned by the following parameters: Mesons are assumed to decay to 
neutrinos uniformly in a region 600 m to 1000 m from the detector. The expected background 
from conventional νe interactions and misidentiﬁed νµ interactions is assumed to be 100 
events in each of 5 energy bins which span the region from 10 to 60 GeV. We assume that 
the νµ ﬂux is such that if P (νµ → ν e ) = 0.01 averaged over any bin, then that bin would 
have an expected additional contribution of 100 events due to ν→ ν 
oscillations. 
   */

  // Make signal model model
  RooRealVar E("E","", 15,10,60,"GeV");
  RooRealVar L("L","", .800,.600, 1.0,"km"); // need these units in formula
  //  RooRealVar deltaMSq("deltaMSq","#Delta m^{2}",40,1,1000,"eV/c^{2}");
  //  RooRealVar sinSq2theta("sinSq2theta","sin^{2}(2#theta)", .006,5E-4,1);
  RooRealVar deltaMSq("deltaMSq","#Delta m^{2}",40,20,50,"eV/c^{2}");
  RooRealVar sinSq2theta("sinSq2theta","sin^{2}(2#theta)", .006,.004,.02);

  NuMuToNuE_Oscillation PnmuTone("PnmuTone","P(#nu_{#mu} #rightarrow #nu_{e}",L,E,sinSq2theta,deltaMSq);
  //  RooGenericPdf PnmuTone("PnmuTone","","sinSq2theta*pow(sin(1.27*deltaMSq*L/E),2)",
  //		 RooArgSet(sinSq2theta,factor,deltaMSq,L,E));

  // need to create a signal model by integrating out L 
  RooAbsPdf* sigModel = PnmuTone.createProjection(L);


  //\int dE dL P(E,L | \Delta m^2).  Guess for deltaMsq=0.006 from results of initial fits
  // want to calculate by using a large range for E,L and then calculating integral in smaller range.
  // Given RooFit will renormalize the PDF in the range of the observables, the PDF will be too large by 1/acceptance.
  Double_t acceptance =.8;

  // flux * 1% chance per bin =  100 events / bin
  RooConstVar flux("flux","#nu_{#mu} flux",10000*acceptance);
  // bkg = 5 bins * 100 events / bin
  RooConstVar bkgNorm("bkgNorm","normalization for background",500);
  RooProduct newFlux("newFlux", "", RooArgSet(flux, sinSq2theta));

  // flat background
  RooPolynomial bkgEShape("bkgEShape","flat bkg shape", E);
  //  RooPolynomial bkgLShape("bkgLShape","flat bkg shape", L);
  //  RooProdPdf bkgShape("bkgShape","flat bkg shape",
  //		      RooArgList(bkgLShape,bkgEShape));

  RooAddPdf model("model","",RooArgList(*sigModel,bkgEShape),
		  RooArgList(newFlux,bkgNorm));

  model.printCompactTree();

  // nevents data = 100*5 (bkg) + flux*sin^2(2\theta)*\int dE dL P(E,L | \Delta m^2)
  Int_t nEventsData = bkgNorm.getVal()+flux.getVal()*sinSq2theta.getVal()*acceptance; // assume int = 1 for now.

  // create a toy dataset
  RooDataSet* data = model.generate(RooArgSet(E), nEventsData);
  

  TCanvas* dataCanvas = new TCanvas("dataCanvas");
  dataCanvas->Divide(2,2);
  dataCanvas->cd(1);
  TH1* hh = PnmuTone.createHistogram("hh",E,Binning(40),YVar(L,Binning(40))) ;
  hh->SetLineColor(kBlue) ;
  hh->Draw("surf");

  dataCanvas->cd(2);
  RooPlot* Eframe = E.frame();
  data->plotOn(Eframe);
  model.fitTo(*data);
  model.plotOn(Eframe);
  model.plotOn(Eframe,Components(*sigModel),LineColor(kRed));
  model.plotOn(Eframe,Components(bkgEShape),LineColor(kGreen));
  model.plotOn(Eframe);
  Eframe->Draw();
  dataCanvas->Update();



  //////// show use of Feldman-Cousins
  RooStats::FeldmanCousins fc;
  // set the distribution creator, which encodes the test statistic
  RooArgSet parameters(deltaMSq, sinSq2theta);
  fc.SetPdf(model);
  fc.SetParameters(parameters);
  fc.SetTestSize(.1); // set size of test
  fc.SetData(*data);
  fc.UseAdaptiveSampling(true);

  // use the Feldman-Cousins tool
  ConfInterval* interval = fc.GetInterval();

  // make a canvas for plots
  //  TCanvas* intervalCanvas =  new TCanvas("intervalCanvas");
  dataCanvas->cd(3);
  
  std::cout << "is this point in the interval? " << 
    interval->IsInInterval(parameters) << std::endl;
  
  // make a plot
  //  RooPlot plot(param1, param2);
  //  parameterScan.plotOn(&plot);
  //  plot.Draw();

  //  TTree* tree = const_cast<TTree*> (&parameterScan.tree());
  //  tree->Print();
  //  tree->Draw("param1:param2 >> hist");
  //  TH2F* hist = (TH2F*) gROOT->Get("hist");
  //  hist->Draw();

  RooDataHist* parameterScan = (RooDataHist*) fc.GetPointsToScan();
  parameterScan->Draw("deltaMSq:sinSq2theta");

 
  RooArgSet* tmpPoint;
  // loop over points to test
  for(Int_t i=0; i<parameterScan->numEntries(); ++i){
    //    cout << "on parameter point " << i << " out of " << parameterScan->numEntries() << endl;
     // get a parameter point from the list of points to test.
    tmpPoint = (RooArgSet*) parameterScan->get(i)->clone("temp");
    TMarker* mark = new TMarker(tmpPoint->getRealValue("sinSq2theta"), tmpPoint->getRealValue("deltaMSq"), 25);
    if (interval->IsInInterval( *tmpPoint ) ) 
      mark->SetMarkerColor(kBlue);
    else
      mark->SetMarkerColor(kRed);

    mark->Draw("s");
    //delete tmpPoint;
    //    delete mark;
  }
  t.Stop();
  t.Print();
    

}
