/////////////////////////////////////////////////////////////////////////
//
// 'Neutrino Oscillation Example from Feldman & Cousins' RooStats tutorial macro #401
// author: Kyle Cranmer
// date March 2009
//
// This tutorial shows a more complex example using the FeldmanCousins utility
// to create a confidence interval for a toy neutrino oscillation experiment.
// The example attempts to faithfully reproduce the toy example described in Feldman & Cousins' 
// original paper, Phys.Rev.D57:3873-3889,1998. 
/////////////////////////////////////////////////////////////////////////

#ifndef __CINT__
#include "RooGlobalFunc.h"
#endif
#include "RooStats/ConfInterval.h"
#include "RooStats/FeldmanCousins.h"
#include "RooStats/ProfileLikelihoodCalculator.h"

#include "RooDataSet.h"
#include "RooRealVar.h"
#include "RooConstVar.h"
#include "RooAddition.h"
#include "RooProduct.h"
#include "RooProdPdf.h"
#include "RooAddPdf.h"

#include "NuMuToNuE_Oscillation.h"
#include "RooPolynomial.h"

#include "RooNLLVar.h"
#include "RooProfileLL.h"

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
  RooRealVar deltaMSq("deltaMSq","#Delta m^{2}",40,20,70,"eV/c^{2}");
  RooRealVar sinSq2theta("sinSq2theta","sin^{2}(2#theta)", .006,.0001,.1);
  // PDF for oscillation only describes deltaMSq dependence, sinSq2theta goes into sigNorm
  NuMuToNuE_Oscillation PnmuTone("PnmuTone","P(#nu_{#mu} #rightarrow #nu_{e}",L,E,deltaMSq);

  // only E is observable, so create the signal model by integrating out L 
  RooAbsPdf* sigModel = PnmuTone.createProjection(L);

  //\int dE' dL' P(E',L' | \Delta m^2). 
  // Given RooFit will renormalize the PDF in the range of the observables, 
  // the total probability to oscillate in the experiment's acceptance
  // needs to be incorporated into the extended term in the likelihood.
  // Do this by creating a RooAbsReal representing the integral.
  // The integral should be over "primed" observables, so we need
  // an independent copy of PnmuTone

  // Independent copy for Integral
  RooRealVar EPrime("EPrime","", 15,10,60,"GeV");
  RooRealVar LPrime("LPrime","", .800,.600, 1.0,"km"); // need these units in formula
  NuMuToNuE_Oscillation PnmuTonePrime("PnmuTonePrime","P(#nu_{#mu} #rightarrow #nu_{e}",
				      LPrime,EPrime,deltaMSq);
  RooAbsReal* probToOscInExp = PnmuTonePrime.createIntegral(RooArgSet(EPrime,LPrime));


  // flux * 1% chance per bin =  100 events / bin
  // therefore flux = 10000.
  RooConstVar flux("flux","#nu_{#mu} flux",10000
		   /(EPrime.getMax()-EPrime.getMin())
		   /(LPrime.getMax()-LPrime.getMin()));
  // sigNorm = flux * prob to oscillate in experiment * sin^2(2\theta)
  RooProduct sigNorm("sigNorm", "", RooArgSet(flux, sinSq2theta, *probToOscInExp));
  // bkg = 5 bins * 100 events / bin
  RooConstVar bkgNorm("bkgNorm","normalization for background",500);

  // flat background
  RooPolynomial bkgEShape("bkgEShape","flat bkg shape", E);

  // total model
  RooAddPdf model("model","",RooArgList(*sigModel,bkgEShape),
		  RooArgList(sigNorm,bkgNorm));

  // for debugging, check model tree
  model.printCompactTree();
  model.graphVizTree("model.dot");


  //////////////////////////////////////////////
  // n events in data to data, simply sum of sig+bkg
  Int_t nEventsData = bkgNorm.getVal()+sigNorm.getVal(); 
  cout << "generate toy data with nEvents = " << nEventsData << endl;
  // create a toy dataset
  RooDataSet* data = model.generate(RooArgSet(E), nEventsData);
  

  /////////////////////////////////////////////
  // make some plots
  TCanvas* dataCanvas = new TCanvas("dataCanvas");
  dataCanvas->Divide(2,2);
  dataCanvas->cd(1);
  TH1* hh = PnmuTone.createHistogram("hh",E,Binning(40),YVar(L,Binning(40))) ;
  hh->SetLineColor(kBlue) ;
  hh->SetTitle("True Signal Model");
  hh->Draw("surf");

  dataCanvas->cd(2);
  RooPlot* Eframe = E.frame();
  data->plotOn(Eframe);
  model.fitTo(*data);
  model.plotOn(Eframe);
  model.plotOn(Eframe,Components(*sigModel),LineColor(kRed));
  model.plotOn(Eframe,Components(bkgEShape),LineColor(kGreen));
  model.plotOn(Eframe);
  Eframe->SetTitle("toy data with best fit model (and sig+bkg components)");
  Eframe->Draw();

  dataCanvas->cd(3);
  //  TH1* hhh = PnmuTone.createHistogram("hhh",E,Binning(40),YVar(L,Binning(40))) ;
  RooNLLVar nll("nll", "nll", model, *data);
  RooProfileLL pll("pll", "", nll, RooArgSet(deltaMSq, sinSq2theta));
  //  TH1* hhh = nll.createHistogram("hhh",sinSq2theta,Binning(40),YVar(deltaMSq,Binning(40))) ;
  TH1* hhh = pll.createHistogram("hhh",sinSq2theta,Binning(40),YVar(deltaMSq,Binning(40))) ;
  hhh->SetLineColor(kBlue) ;
  hhh->SetTitle("Best Fit Signal Model");
  hhh->Draw("surf");

  

  dataCanvas->Update();

  model.fitTo(*data);


  ///////////////////////////////////////
  //////// show use of Feldman-Cousins utility in RooStats
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
  //ConfInterval* interval = 0;


  /////////////////////////////////////////
  ///////// show use of ProfileLikeihoodCalculator utility in RooStats
  RooStats::ProfileLikelihoodCalculator plc;
  plc.SetPdf(model);
  plc.SetParameters(parameters);
  plc.SetTestSize(.1);
  plc.SetData(*data);
  
  ConfInterval* plcInterval = plc.GetInterval();
  //ConfInterval* plcInterval = 0;

  ////////////////////////////////////////////
  // make plot of resulting interval
  dataCanvas->cd(4);
  
  if(interval){
    std::cout << "is this point in the interval? " << 
      interval->IsInInterval(parameters) << std::endl;
  }

  RooDataHist* parameterScan = (RooDataHist*) fc.GetPointsToScan();
  parameterScan->Draw("deltaMSq:sinSq2theta");

 
  RooArgSet* tmpPoint;
  // loop over points to test
  for(Int_t i=0; i<parameterScan->numEntries(); ++i){
     // get a parameter point from the list of points to test.
    tmpPoint = (RooArgSet*) parameterScan->get(i)->clone("temp");

    TMarker* mark = new TMarker(tmpPoint->getRealValue("sinSq2theta"), tmpPoint->getRealValue("deltaMSq"), 25);
    if (interval){
      if (interval->IsInInterval( *tmpPoint ) ) 
	mark->SetMarkerColor(kBlue);
      else
	mark->SetMarkerColor(kRed);
      mark->Draw("s");
    }
    

    // mark for ProfileLikelihood
    TMarker* plcMark = new TMarker(tmpPoint->getRealValue("sinSq2theta"), tmpPoint->getRealValue("deltaMSq"), 22);
    parameters=*tmpPoint;
    cout << "pll = " << pll.getVal() << endl;;
    if (plcInterval){
      if(plcInterval->IsInInterval( *tmpPoint ) ) 
	plcMark->SetMarkerColor(kGreen);
      else
	plcMark->SetMarkerColor(kMagenta);

      plcMark->Draw("s");
    }
    //delete tmpPoint;
    //    delete mark;
  }


  /// print timing info
  t.Stop();
  t.Print();
    

}
