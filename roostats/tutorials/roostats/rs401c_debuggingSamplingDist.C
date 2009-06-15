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
#include "RooStats/ToyMCSampler.h"
#include "RooStats/ProfileLikelihoodTestStat.h"
#include "RooStats/NeymanConstruction.h"
#include "RooStats/SamplingDistPlot.h"

#include "RooDataSet.h"
#include "RooRealVar.h"
#include "RooGaussian.h"
#include "RooPlot.h"

#include "TCanvas.h"
#include "TTree.h"
#include "TMarker.h"
#include "TStopwatch.h"

#include <iostream>

// use this order for safety on library loading
using namespace RooFit ;
using namespace RooStats ;


void rs401c_debuggingSamplingDist()
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

  Int_t nEventsData = 100;

  //// create a distribution creator.  
  // The Templated Distribution Creator is templated on a class which implements:
  // Double_t EvaluateTestStatistic(RooAbsData&, RooArgSet& parameterPoint)
  // and RooAbsArg* GetTestStatisitc()
  // 
  // As an example, we use the ProfileLikelihoodFunctor.
  ProfileLikelihoodTestStat testStatFunct(gaus);
  ToyMCSampler samplingDistCreator(testStatFunct) ;
  samplingDistCreator.SetPdf(gaus);
  samplingDistCreator.SetParameters(parameters);
  samplingDistCreator.SetNToys(500);
  samplingDistCreator.SetNEventsPerToy(nEventsData); // this needs to match what was used for this dataset

  //// show use of a distribution creator
  RooArgSet* point = new RooArgSet(mu, sigma);
  SamplingDistribution* samp = samplingDistCreator.GetSamplingDistribution(*point);

  mu.setVal(0.5);
  sigma.setVal(1.2);
  SamplingDistribution* samp1 = samplingDistCreator.GetSamplingDistribution(*point);

  //  samp = samplingDistCreator.GetSamplingDistribution(*point);

  // should give a number close to 0.1 b/c the distribution is uniform on [0,1]
  std::cout << "test stat with p=0.1 is " << samp->InverseCDF(.1) << std::endl;

  // should give a number close to 0.1 b/c the distribution is uniform on [0,1]
  std::cout << "test stat with p=0.9 is " << samp->InverseCDF(.9) << std::endl;

  // it knows the test statistic, so you can ask it 
  // to evaluate the test statistic on data at a point
  RooDataSet* data = gaus.generate(RooArgSet(x), nEventsData);
  
  std::cout << "This data has mean, stdev = " << data->moment(x,1,0.) << ", " << data->moment(x,2,data->moment(x,1,0.) ) << endl; 
  std::cout << "test stat for this data is " 
	    << samplingDistCreator.EvaluateTestStatistic(*data, *point) << std::endl;

  TCanvas* dataCanvas = new TCanvas("dataCanvas");
  RooPlot* frame = x.frame();
  data->plotOn(frame);
  frame->Draw();
  dataCanvas->Update();

  //plot the sampling distribution
  SamplingDistPlot plotSampl(10);
  plotSampl.AddSamplingDistribution(samp,"");
  plotSampl.AddSamplingDistribution(samp1,"");

  TCanvas *samplCanvas = new TCanvas("samplCanvas");
  samplCanvas->cd();
  plotSampl.Draw();
  samplCanvas->SaveAs("samplingPlot.ps");

  return;

  //////// show use of NeymanConstruction
  // Create points to test
  mu.setBins(30);
  sigma.setBins(30);
  RooDataHist parameterScan("parameterScan", "", parameters);
  //  parameterScan.Scan("mu:sigma");
 
  // Create a Neyman Construction
  RooStats::NeymanConstruction nc;
  // set the distribution creator, which encodes the test statistic
  nc.SetTestStatSampler(samplingDistCreator);
  nc.SetTestSize(.2); // set size of test
  nc.SetParameterPointsToTest( parameterScan );
  nc.SetData(*data);


  TCanvas* intervalCanvas =  new TCanvas("intervalCanvas");
  
  // use the Neyman Construction
  ConfInterval* interval = nc.GetInterval();

  //These two methods produce the same result as GetInterval() but
  //the sampling distribution is saved in a root file by the first
  //method for further use. The second method uses the distribution
  //in the file instead of generating it again (time consuming)
  //nc.GenSamplingDistribution("debug.root");
  //ConfInterval* interval = nc.GetInterval("debug.root");

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

  parameterScan.Draw("mu:sigma");

 
  RooArgSet* tmpPoint;
  // loop over points to test
  for(Int_t i=0; i<parameterScan.numEntries(); ++i){
    //    cout << "on parameter point " << i << " out of " << parameterScan.numEntries() << endl;
     // get a parameter point from the list of points to test.
    tmpPoint = (RooArgSet*) parameterScan.get(i)->clone("temp");
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
