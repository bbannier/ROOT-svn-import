/////////////////////////////////////////////////////////////////////////
//
// 'Number Counting Utils' RooStats tutorial 
// author: Kyle Cranmer
// date June. 2009 
//
// This tutorial shows an example of the RooStats standalone 
// utilities that calculate the p-value or Z value (eg. significance in
// 1-sided Gaussian standard deviations) for a number counting experiment.
// This is a hypothesis test between background only and signal-plus-background.
// The background estimate has uncertainty.
//
// The problem is treated in a fully frequentist fashioon by 
// interpreting the relative background uncertainty as
// being due to an auxiliary or sideband observation 
// that is also Poisson distributed with only background.
// Finally, one considers the test as a ratio of Poisson means
// where an interval is well known based on the conditioning on the total
// number of events and the binomial distribution.
// For more on this, see
//  arXiv:0905.3831
//  arXiv:physics/0702156
//
/////////////////////////////////////////////////////////////////////////


#ifndef __CINT__
#include "RooStats/NumberCountingUtils.h"
#endif

#include <iostream>

using namespace RooFit ;
using namespace RooStats ;
using namespace std ;

void rs_numbercountingutils()
{

    //root [1] NumberCountingUtils::BinomialExpZ( <tab>
    //Double_t BinomialExpZ(Double_t sExp, Double_t bExp, Double_t fractionalBUncertainty)

  /////////////////////////////////////////////////////
  // Expected p-values and significance with background uncertainty
  ////////////////////////////////////////////////////
  double sExpected = 50;
  double bExpected = 100;
  double relativeBkgUncert = 0.1;

  double pExp = NumberCountingUtils::BinomialExpP(sExpected, bExpected, relativeBkgUncert);
  double zExp = NumberCountingUtils::BinomialExpZ(sExpected, bExpected, relativeBkgUncert);
  cout << "expected p-value ="<< pExp << "  Z value (Gaussian sigma) = "<< zExp << endl;

  /////////////////////////////////////////////////////
  // Expected p-values and significance with background uncertainty
  ////////////////////////////////////////////////////
  double observed = 150;
  double pObs = NumberCountingUtils::BinomialObsP(observed, bExpected, relativeBkgUncert);
  double zObs = NumberCountingUtils::BinomialObsZ(observed, bExpected, relativeBkgUncert);
  cout << "observed p-value ="<< pObs << "  Z value (Gaussian sigma) = "<< zObs << endl;

}
