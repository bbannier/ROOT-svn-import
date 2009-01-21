// @(#)root/roostats:$Id: DebuggingTestStatFunctor.h 26805 2009-01-13 17:45:57Z cranmer $
// Author: Kyle Cranmer, Lorenzo Moneta, Gregory Schott, Wouter Verkerke
/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOSTATS_DebuggingTestStatFunctor
#define ROOSTATS_DebuggingTestStatFunctor

//_________________________________________________
/*
BEGIN_HTML
<p>
DebuggingTestStatFunctor is a simple implementation of the DistributionCreator interface used for debugging.
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
#include "RooDataSet.h"
#include "SamplingDistribution.h"
#include "TRandom.h"
#include <vector>
#include "RooStats/TemplatedDistributionCreator.h"


namespace RooStats {

 class DebuggingTestStatFunctor {

   public:
     DebuggingTestStatFunctor() {
       fTestStatistic = new RooRealVar("UniformTestStatistic","UniformTestStatistic",0,0,1);
       fRand = new TRandom();
     }
     virtual ~DebuggingTestStatFunctor() {
       //       delete fRand;
       //       delete fTestStatistic;
     }
    
     // Main interface to evaluate the test statistic on a dataset
     virtual Double_t Evaluate(RooAbsData& data, RooArgSet& paramsOfInterest)  {
       //data = data; // avoid warning
       //paramsOfInterest = paramsOfInterest; //avoid warning
       return fRand->Uniform();
     }

      // Get the TestStatistic
      virtual const RooAbsArg* GetTestStatistic()  const {return fTestStatistic;}  
    
      
   private:

      RooRealVar* fTestStatistic;
      TRandom* fRand;

   protected:
      ClassDef(DebuggingTestStatFunctor,1)   
   };

 typedef TemplatedDistributionCreator<DebuggingTestStatFunctor> TestDistributionCreator;


}


#endif
