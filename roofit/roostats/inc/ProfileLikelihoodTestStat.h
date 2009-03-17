// @(#)root/roostats:$Id: ProfileLikelihoodTestStat.h 26805 2009-01-13 17:45:57Z cranmer $
// Author: Kyle Cranmer, Lorenzo Moneta, Gregory Schott, Wouter Verkerke
/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOSTATS_ProfileLikelihoodTestStat
#define ROOSTATS_ProfileLikelihoodTestStat

//_________________________________________________
/*
BEGIN_HTML
<p>
ProfileLikelihoodTestStat is an implementation of the TestStatistic interface that calculates the profile
likelihood ratio at a particular parameter point given a dataset.  It does not constitute a statistical test, for that one may either use:
<ul>
 <li> the ProfileLikelihoodCalculator that relies on asymptotic properties of the Profile Likelihood Ratio</li>
 <li> the Neyman Construction classes with this class as a test statistic</li>
 <li> the Hybrid Calculator class with this class as a test statistic</li>
</ul>

</p>
END_HTML
*/
//

#ifndef ROOT_Rtypes
#include "Rtypes.h"
#endif

#include <vector>

#include "RooStats/DistributionCreator.h"
#include "RooStats/SamplingDistribution.h"
#include "RooStats/TestStatistic.h"

#include "RooRealVar.h"
#include "RooProfileLL.h"
#include "RooNLLVar.h"

namespace RooStats {

  class ProfileLikelihoodTestStat : public TestStatistic{

   public:
     ProfileLikelihoodTestStat(RooAbsPdf& pdf) {
       fPdf = &pdf;
       fProfile = 0;
       fNll = 0;
     }
     virtual ~ProfileLikelihoodTestStat() {
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

       //       Double_t value = fProfile->evaluate();
       Double_t value = fProfile->getVal();
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
      ClassDef(ProfileLikelihoodTestStat,1)   
   };

}


#endif
