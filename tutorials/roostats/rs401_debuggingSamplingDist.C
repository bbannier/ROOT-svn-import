/////////////////////////////////////////////////////////////////////////
//
// 'Number Counting Example' RooStats tutorial macro #100
// author: Kyle Cranmer
// date Nov. 2008 
//
// This tutorial shows an example of a combination of 
// two searches using number counting with background uncertainty.
//
// The macro uses a RooStats "factory" to construct a PDF
// that represents the two number counting analyses with background 
// uncertainties.  The uncertainties are taken into account by 
// considering a sideband measurement of a size that corresponds to the
// background uncertainty.  The problem has been studied in these references:
//   http://arxiv.org/abs/physics/0511028
//   http://arxiv.org/abs/physics/0702156
//   http://cdsweb.cern.ch/record/1099969?ln=en
//
// After using the factory to make the model, we use a RooStats 
// ProfileLikelihoodCalculator for a Hypothesis test and a confidence interval.
// The calculator takes into account systematics by eliminating nuisance parameters
// with the profile likelihood.  This is equivalent to the method of MINOS.
//
/////////////////////////////////////////////////////////////////////////

#ifndef __CINT__
#include "RooGlobalFunc.h"
#endif
#include "RooStats/SamplingDistribution.h"
#include "RooStats/DebuggingDistributionCreator.h"

#include "RooStats/NeymanConstruction.h"

#include "RooRealVar.h"

// use this order for safety on library loading
using namespace RooFit ;
using namespace RooStats ;


void rs100_debuggingSamplingDist()
{
  
  //// create a distribution creator.  The debugging one creates a uniform distribution on [0,1] independent of data.
  DebuggingDistributionCreator ddc ;
  // normally you would set a test statistic, but this debugging one doesn't need it.

  //// show use of a distribution creator
  RooArgSet* point;
  ddc.GetSamplingDistribution(*point);
  SamplingDistribution* samp = ddc.GetSamplingDistribution(*point);
  samp->InverseCDF(.1); // should give a number close to 0.1 b/c the distribution is uniform on [0,1]
  samp->InverseCDF(.9); // should give a number close to 0.9 b/c the distribution is uniform on [0,1]
  // it knows the test statistic, so you can ask it to evaluate the test statistic on data
  RooAbsData* data;
  ddc.EvaluateTestStatistic(*data);

  //////// show use of NeymanConstruction
  // Create points to test
  RooRealVar param1("param1", "", 0, 0, 1);
  param1.setBins(10);
  RooRealVar param2("param2", "", 0, 0, 1);
  param1.setBins(100);
  RooArgSet parameters(param1, param2);
  RooDataHist parameterScan("parameterScan", "", parameters);

  // Create a Neyman Construction
  RooStats::NeymanConstruction nc;
  // set the distribution creator, which encodes the test statistic
  nc.SetDistributionCreator(ddc);
  nc.SetSize(.1); // set size of test
  nc.SetParameterPointsToTest( parameterScan );

  // use the Neyman Construction
  ConfInterval* interval = nc.GetInterval();
  
  
}
