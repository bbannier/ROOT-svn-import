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
#include "RooStats/SamplingDistribution.h"
#include "RooStats/DebuggingDistributionCreator.h"

#include "RooStats/NeymanConstruction.h"

#include "RooRealVar.h"
#include "RooPlot.h"

#include "TTree.h"
#include "TMarker.h"

#include <iostream>

// use this order for safety on library loading
using namespace RooFit ;
using namespace RooStats ;


void rs401_debuggingSamplingDist()
{
  
  //// create a distribution creator.  The debugging one creates a uniform distribution on [0,1] independent of data.
  DebuggingDistributionCreator ddc ;
  // normally you would set a test statistic, but this debugging one doesn't need it.

  //// show use of a distribution creator
  RooArgSet* point;
  SamplingDistribution* samp = ddc.GetSamplingDistribution(*point);

  // should give a number close to 0.1 b/c the distribution is uniform on [0,1]
  std::cout << "test stat with p=0.1 is " << samp->InverseCDF(.1) << std::endl;

  // should give a number close to 0.1 b/c the distribution is uniform on [0,1]
  std::cout << "test stat with p=0.9 is " << samp->InverseCDF(.9) << std::endl;

  // it knows the test statistic, so you can ask it to evaluate the test statistic on data
  RooAbsData* data;
  std::cout << "test stat for this data is " 
	    << ddc.EvaluateTestStatistic(*data, *point) << std::endl;

  //////// show use of NeymanConstruction
  // Create points to test
  RooRealVar param1("param1", "", 0, 0, 1);
  param1.setBins(10);
  RooRealVar param2("param2", "", 0, 0, 1);
  param2.setBins(100);
  RooArgSet parameters(param1, param2);
  RooDataHist parameterScan("parameterScan", "", parameters);

 
  // Create a Neyman Construction
  RooStats::NeymanConstruction nc;
  // set the distribution creator, which encodes the test statistic
  nc.SetDistributionCreator(ddc);
  nc.SetSize(.2); // set size of test
  nc.SetParameterPointsToTest( parameterScan );

  // use the Neyman Construction
  ConfInterval* interval = nc.GetInterval();

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

  parameterScan.Draw("param1:param2");

  RooArgSet* tmpPoint;
  // loop over points to test
  for(Int_t i=0; i<parameterScan.numEntries(); ++i){
     // get a parameter point from the list of points to test.
    tmpPoint = (RooArgSet*) parameterScan.get(i)->clone("temp");
      TMarker* mark = new TMarker(tmpPoint->getRealValue("param2"), 
				  tmpPoint->getRealValue("param1"), 25);
    if (interval->IsInInterval( *tmpPoint ) ) 
      mark->SetMarkerColor(kBlue);
    else
      mark->SetMarkerColor(kRed);

    mark->Draw("s");
  }

}
