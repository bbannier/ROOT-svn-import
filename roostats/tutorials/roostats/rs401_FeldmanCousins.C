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
#include "RooGaussian.h"
#include "RooPlot.h"

#include "TCanvas.h"
#include "TTree.h"
#include "TH2F.h"
#include "RooDataHist.h"
#include "TMarker.h"
#include "TStopwatch.h"

#include <iostream>

// use this order for safety on library loading
using namespace RooFit ;
using namespace RooStats ;


void rs401_FeldmanCousins()
{
  
  // to time the macro
  TStopwatch t;
  t.Start();

  // make a simple model
  RooRealVar x("x","", 1,-5,5);
  RooRealVar mu("mu","", 0,-.5, .5);
  RooRealVar sigma("sigma","", 1, 0.5 ,1.5);
  RooGaussian gaus("gaus", "", x, mu, sigma);
  RooArgSet parameters(mu, sigma);
  //  sigma.setConstant();

  Int_t nEventsData = 100;

  // create a toy dataset
  RooDataSet* data = gaus.generate(RooArgSet(x), nEventsData);
  
  std::cout << "This data has mean, stdev = " << data->moment(x,1,0.) << ", " << data->moment(x,2,data->moment(x,1,0.) ) << endl; 

  TCanvas* dataCanvas = new TCanvas("dataCanvas");
  RooPlot* frame = x.frame();
  data->plotOn(frame);
  frame->Draw();
  dataCanvas->Update();

  RooWorkspace w;
  ModelConfig m("example", &w);
  m.SetParametersOfInterest(parameters);
  m.SetPdf(gaus);

  //////// show use of Feldman-Cousins
  RooStats::FeldmanCousins fc(*data, m);
  fc.SetTestSize(.2); // set size of test
  fc.UseAdaptiveSampling(true);

  // use the Feldman-Cousins tool
  ConfInterval* interval = fc.GetInterval();

  // make a canvas for plots
  TCanvas* intervalCanvas =  new TCanvas("intervalCanvas");
  
  std::cout << "is this point in the interval? " << 
    interval->IsInInterval(parameters) << std::endl;
  
  RooDataHist* parameterScan = (RooDataHist*) fc.GetPointsToScan();
  TH2F* hist = (TH2F*) parameterScan->createHistogram("sigma:mu",30,30);
  hist->Draw();

 
  RooArgSet* tmpPoint;
  // loop over points to test
  for(Int_t i=0; i<parameterScan->numEntries(); ++i){
    //    cout << "on parameter point " << i << " out of " << parameterScan->numEntries() << endl;
     // get a parameter point from the list of points to test.
    tmpPoint = (RooArgSet*) parameterScan->get(i)->clone("temp");
      TMarker* mark = new TMarker(tmpPoint->getRealValue("sigma"), 
				  tmpPoint->getRealValue("mu"), 25);
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
