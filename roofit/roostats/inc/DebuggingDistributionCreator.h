// @(#)root/roostats:$Id: DebuggingDistributionCreator.h 26805 2009-01-13 17:45:57Z cranmer $
// Author: Kyle Cranmer, Lorenzo Moneta, Gregory Schott, Wouter Verkerke
/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOSTATS_DebuggingDistributionCreator
#define ROOSTATS_DebuggingDistributionCreator

//_________________________________________________
/*
BEGIN_HTML
<p>
DebuggingDistributionCreator is a simple implementation of the DistributionCreator interface used for debugging.
The sampling distribution is uniformly random between [0,1] and is INDEPENDENT of the data.  So it is not useful
for true statistical tests, but it is useful for debugging.
</p>
END_HTML
*/
//

#ifndef ROOT_Rtypes
#include "Rtypes.h"
#endif

#include "RooStats/DistributionCreator.h"
#include "RooAbsPdf.h"
#include "RooArgSet.h"
#include "RooRealVar.h"
#include "SamplingDistribution.h"
#include "TRandom.h"
#include <vector>

namespace RooStats {

 class DebuggingDistributionCreator: public DistributionCreator {

   public:
     DebuggingDistributionCreator() {
       fTestStatistic = new RooRealVar("UniformTestStatistic","UniformTestStatistic",0,0,1);
       fRand = new TRandom();
     }
     virtual ~DebuggingDistributionCreator() {
       delete fRand;
       delete fTestStatistic;
     }
    
      // Main interface to get a ConfInterval, pure virtual
     virtual SamplingDistribution* GetSamplingDistribution(RooArgSet& paramsOfInterest) const {
       // normally this method would be complex, but here it is simple for debugging
       std::vector<Double_t> testStatVec;
       for(Int_t i=0; i<1000; ++i){
	 testStatVec.push_back( fRand->Uniform() );
       }
       return new SamplingDistribution("UniformSamplingDist", "for debugging", testStatVec );
     } 

      // Main interface to evaluate the test statistic on a dataset
      virtual Double_t EvaluateTestStatistic(RooAbsData& data) const {return fRand->Uniform();}

      // Get the TestStatistic
      virtual const RooAbsArg* GetTestStatistic()  const {return fTestStatistic;}  
    
      // Get the Confidence level for the test
      virtual Double_t ConfidenceLevel()  const {return 1.-fSize;}  

      // Common Initialization
      virtual void Initialize(RooAbsArg& testStatistic, RooArgSet& paramsOfInterest, RooArgSet& nuisanceParameters) {}

      // Set the Pdf, add to the the workspace if not already there
      virtual void SetPdf(RooAbsPdf&) {}

      // specify the parameters of interest in the interval
      virtual void SetParameters(RooArgSet&) {}
      // specify the nuisance parameters (eg. the rest of the parameters)
      virtual void SetNuisanceParameters(RooArgSet&) {}

      // set the size of the test (rate of Type I error) ( Eg. 0.05 for a 95% Confidence Interval)
      virtual void SetSize(Double_t size) {fSize = size;}
      // set the confidence level for the interval (eg. 0.95 for a 95% Confidence Interval)
      virtual void SetConfidenceLevel(Double_t cl) {fSize = 1.-cl;}

      // Set the TestStatistic (want the argument to be a function of the data & parameter points
      virtual void SetTestStatistic(RooAbsArg&)  const {}  
      
   private:
      Double_t fSize;
      RooRealVar* fTestStatistic;
      TRandom* fRand;

   protected:
      ClassDef(DebuggingDistributionCreator,1)   // A simple implementation of the DistributionCreator interface
   };
}


#endif
