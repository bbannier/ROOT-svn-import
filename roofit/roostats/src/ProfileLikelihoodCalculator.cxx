// @(#)root/roostats:$Id: ProfileLikelihoodCalculator.cxx  $
// Author: Kyle Cranmer   28/07/2008

/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

/////////////////////////////////////////
// ProfileLikelihoodCalculator
//

#ifndef RooStats_ProfileLikelihoodCalculator
#include "RooStats/ProfileLikelihoodCalculator.h"
#endif

#ifndef RooStats_RooStatsUtils
#include "RooStats/RooStatsUtils.h"
#endif

#include "RooStats/SimpleInterval.h"

#include "RooRealVar.h"
#include "RooDataSet.h"
#include "RooGlobalFunc.h"
#include "RooCmdArg.h"


// Without this macro the THtml doc  can not be generated
#if !defined(R__ALPHA) && !defined(R__SOLARIS) && !defined(R__ACC) && !defined(R__FBSD)
NamespaceImp(RooStats)
#endif


//ClassImp(RooStats::ProfileLikelihoodCalculator) ;


// A combined calculator tool that uses the profile likelihood ratio to eliminate nuisance parameters.
// Essentially this is the method of MINUIT/MINOS.


using namespace RooStats;


//_______________________________________________________
ProfileLikelihoodCalculator::ProfileLikelihoodCalculator() : 
CombinedCalculator() {
  // default constructor

}

//_______________________________________________________
ProfileLikelihoodCalculator::~ProfileLikelihoodCalculator(){
  // destructor
}


//_______________________________________________________
ConfInterval* ProfileLikelihoodCalculator::GetInterval() const {

  // dummy to get started
  
  // result possibilities:
  //   - If 1-d use a simple interval
  //   - If 2-d could return a contour interval
  //   - could return an elliptical interval (eg. HESSE, general but not really profile LR)
  //   - could make an interval class that caches the profile LR internally (totally general)

  SimpleInterval* interval = new SimpleInterval();


  return interval;
}

//_______________________________________________________
HypoTestResult* ProfileLikelihoodCalculator::GetHypoTest() const {

  // dummy to get started


  // set POI to null values, set constant, calculate conditional MLE

  // set POI to alternate values, set constant, calculate MLE

  // Use Wilks' theorem to translate -2 log lambda into p-values
  HypoTestResult* htr = new HypoTestResult();

  return htr;

}

