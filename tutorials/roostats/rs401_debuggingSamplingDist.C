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
  ddc.GetSamplingDistribution(*point);
  SamplingDistribution* samp = ddc.GetSamplingDistribution(*point);

  // should give a number close to 0.1 b/c the distribution is uniform on [0,1]
  std::cout << "test stat with p=0.1 is " << samp->InverseCDF(.1) << std::endl;

  // should give a number close to 0.1 b/c the distribution is uniform on [0,1]
  std::cout << "test stat with p=0.9 is " << samp->InverseCDF(.9) << std::endl;

  // it knows the test statistic, so you can ask it to evaluate the test statistic on data
  RooAbsData* data;
  std::cout << "test stat for this data is " 
	    << ddc.EvaluateTestStatistic(*data) << std::endl;

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
  nc.SetSize(.1); // set size of test
  nc.SetParameterPointsToTest( parameterScan );

  std::cout << "debug " << std::endl;
  // use the Neyman Construction
  ConfInterval* interval = nc.GetInterval();
  std::cout << "interval ptr = " << interval << std::endl;
  parameters.Print();
  std::cout << "is this point in the interval? " << 
    interval->IsInInterval(parameters) << std::endl;
  
}
