// @(#)root/roostats:$Id: ProfileLikelihoodFunctor.h 26805 2009-01-13 17:45:57Z cranmer $
// Author: Kyle Cranmer, Lorenzo Moneta, Gregory Schott, Wouter Verkerke
/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOSTATS_ProfileLikelihoodFunctor
#define ROOSTATS_ProfileLikelihoodFunctor

//_________________________________________________
/*
BEGIN_HTML
<p>
ProfileLikelihoodFunctor is a simple implementation of the DistributionCreator interface used for debugging.
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
#include "RooStats/ToyMCDistCreator.h"
#include "RooProfileLL.h"
#include "RooNLLVar.h"

namespace RooStats {

 class ProfileLikelihoodFunctor {

   public:
     ProfileLikelihoodFunctor(RooAbsPdf& pdf) {
       fPdf = &pdf;
       fProfile = 0;
       fNll = 0;
     }
     virtual ~ProfileLikelihoodFunctor() {
       //       delete fRand;
       //       delete fTestStatistic;
     }
    
     // Main interface to evaluate the test statistic on a dataset
     virtual Double_t Evaluate(RooAbsData& data, RooArgSet& paramsOfInterest)  {       
       if(!&data){ cout << "problem with data" << endl;}
       if(fProfile) delete fProfile; 
       if (fNll)    delete fNll;
       
       RooMsgService::instance().setGlobalKillBelow(RooMsgService::FATAL) ;
       bool needToRebuild = true; // try to avoid rebuilding if possible
       if(needToRebuild){
	 RooNLLVar* nll = new RooNLLVar("nll","",*fPdf,data);
	 fNll = nll;
	 fProfile = new RooProfileLL("pll","",*nll, paramsOfInterest);
	 //	 fProfile->addOwnedComponents(*nll) ;  // to avoid memory leak       
       }
       if(!fProfile){ cout << "problem making profile" << endl;}

       RooArgSet* paramsToChange = fProfile->getParameters(data);

       TIter it = paramsOfInterest.createIterator();
       RooRealVar *myarg; 
       RooRealVar *mytarget; 
       while ((myarg = (RooRealVar *)it.Next())) { 
	 if(!myarg) continue;
	 mytarget = (RooRealVar*) paramsToChange->find(myarg->GetName());
	 if(!mytarget) continue;
	 mytarget->setVal( myarg->getVal() );
       }
       delete paramsToChange;

       /*       TIter      itr = fProfile->getParameters(data)->createIterator();
       while ((myarg = (RooRealVar *)itr.Next())) { 
	 cout << myarg->GetName() << myarg->getVal();
       }
       cout << " = " << fProfile->evaluate()  << endl;
       */

       Double_t value = fProfile->evaluate();
       RooMsgService::instance().setGlobalKillBelow(RooMsgService::DEBUG) ;
       return value;
     }

      // Get the TestStatistic
      virtual const RooAbsArg* GetTestStatistic()  const {return fProfile;}  
    
      
   private:
      RooProfileLL* fProfile;
      RooAbsPdf* fPdf;
      RooNLLVar* fNll;

   protected:
      ClassDef(ProfileLikelihoodFunctor,1)   
   };

 typedef ToyMCDistCreator<ProfileLikelihoodFunctor> ProfileDistributionCreator;


}


#endif
