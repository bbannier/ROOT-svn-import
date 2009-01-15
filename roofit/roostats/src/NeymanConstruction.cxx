// @(#)root/roostats:$Id: NeymanConstruction.h 26805 2009-01-13 17:45:57Z cranmer $
// Author: Kyle Cranmer   January 2009

/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

//_________________________________________________
/*
BEGIN_HTML
<p>
NeymanConstruction is a concrete implementation of teh NeymanConstruction interface that, as the name suggests,
performs a NeymanConstruction.  
It produces a RooStats::SetInterval, which is a concrete implementation of the ConfInterval interface.  
</p>
<p>
The Neyman Construction is not a uniquely defined statistical technique, it requires that one specify an ordering rule 
or ordering principle, which is usually incoded by choosing a specific test statistic and limits of integration 
(corresponding to upper/lower/central limits).  As a result, this class must be configured with the corresponding
information before it can produce an interval.  Common configurations, such as the Feldman-Cousins approach, can be 
enforced by other light weight classes.
</p>
<p>
The Neyman Construction considers every point in the parameter space independently, no assumptions are 
made that the interval is connected or of a particular shape.  As a result, the SetInterval class is used to 
represent the result.  The user indicate which points in the parameter space to perform the constrution by providing
a SetInterval instance with the desired points.
</p>
<p>
This class is fairly light weight, because the choice of parameter points to be considered is factorized and so is the 
creation of the sampling distribution of the test statistic (which is done by a concrete class implementing the DistributionCreator interface).  As a result, this class basically just drives the construction by:
<ul>
<li> using a DistributionCreator to create the SamplingDistribution of a user-defined test statistic for each parameter point of interest,</li>
<li>defining the acceptance region in the data by finding the thresholds on the test statistic such that the integral of the sampling distribution is of the appropriate size and consistent with the limits of integration (eg. upper/lower/central limits), </li>
<li> and finally updating the SetInterval based on whether the value of the test statistic evaluated on the data are in the acceptance region.</li>
</p>
END_HTML
*/
//

#ifndef RooStats_NeymanConstruction
#include "RooStats/NeymanConstruction.h"
#endif

#ifndef RooStats_RooStatsUtils
#include "RooStats/RooStatsUtils.h"
#endif

#ifndef RooStats_SetInterval
#include "RooStats/SetInterval.h"
#endif

#include "RooDataSet.h"
#include "RooGlobalFunc.h"
#include "RooCmdArg.h"


#include "RooStats/SamplingDistribution.h"

ClassImp(RooStats::NeymanConstruction) ;



using namespace RooFit;
using namespace RooStats;


//_______________________________________________________
NeymanConstruction::NeymanConstruction() {
   // default constructor
  fWS = new RooWorkspace();
  fOwnsWorkspace = true;
  fDataName = "";
  fPdfName = "";
}


//_______________________________________________________
ConfInterval* NeymanConstruction::GetInterval() const {
  // Main interface to get a RooStats::ConfInterval.  
  // It constructs a RooStats::SetInterval.

  // local variables
  RooAbsData* data = fWS->data(fDataName);
  Int_t npass = 0;
  RooArgSet* point; 

  // loop over points to test
  for(Int_t i=0; i<fPointsToTest->numEntries(); ++i){
     // get a parameter point from the list of points to test.
    point = (RooArgSet*) fPointsToTest->get(i)->clone("temp");

    // the next line is where most of the time will be spent generating the sampling dist of the test statistic.
    SamplingDistribution* samplingDist = fDistCreator->GetSamplingDistribution(*point); 

    // find the lower & upper thresholds on the test statistic that define the acceptance region in the data
    Double_t lowerEdgeOfAcceptance = samplingDist->InverseCDF( fLeftSideFraction * fSize );
    Double_t upperEdgeOfAcceptance = samplingDist->InverseCDF( 1. - ((1.-fLeftSideFraction) * fSize) );

     // get the value of the test statistic for this data set
    Double_t thisTestStatistic = fDistCreator->EvaluateTestStatistic( *( data) );

    //    std::cout << "dbg= " << lowerEdgeOfAcceptance << ", " 
    //      << upperEdgeOfAcceptance << std::endl;

    // Check if this data is in the acceptance region
    if(thisTestStatistic > lowerEdgeOfAcceptance && thisTestStatistic < upperEdgeOfAcceptance) {
      // if so, set this point to true
      fPointsToTest->add(*point, 1.); 
      ++npass;
    }
    delete samplingDist;
  }
  std::cout << npass << " points in interval" << std::endl;

  // create an interval based fPointsToTest
  SetInterval* interval 
    = new SetInterval("ClassicalConfidenceInterval", "ClassicalConfidenceInterval", *fPointsToTest);

   return interval;
}

