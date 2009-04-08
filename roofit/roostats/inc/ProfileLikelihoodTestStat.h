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

//#include "RooStats/DistributionCreator.h"
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
       fLastBestFitParams = 0;
       fLastData = 0;
     }
     virtual ~ProfileLikelihoodTestStat() {
       //       delete fRand;
       //       delete fTestStatistic;
     }
    
     // Main interface to evaluate the test statistic on a dataset
     virtual Double_t Evaluate(RooAbsData& data, RooArgSet& paramsOfInterest)  {       
       if(!&data){ cout << "problem with data" << endl;}
       
       RooMsgService::instance().setGlobalKillBelow(RooMsgService::FATAL) ;
       bool needToRebuild = true; // try to avoid rebuilding if possible

       if(fLastData == &data) // simple pointer comparison for now (note NLL makes COPY of data)
	 needToRebuild=false;
       else
	 fLastData = &data; // keep a copy of pointer to original data

       if(needToRebuild){
	 if(fProfile) delete fProfile; 
	 if (fNll)    delete fNll;

	 RooNLLVar* nll = new RooNLLVar("nll","",*fPdf,data, RooFit::Extended());
	 fNll = nll;
	 fProfile = new RooProfileLL("pll","",*nll, paramsOfInterest);


	 // set parameters to previous best fit params, to speed convergence
	 // and to avoid local minima
	 if(fLastBestFitParams){
	   // these parameters are not guaranteed to be the best for this data	   
	   SetParameters(fLastBestFitParams, fProfile->getParameters(data) );
	   // now evaluate to force this profile to evaluate and store
	   // best fit parameters for this data
	   fProfile->getVal();
	   // possibly store last MLE for reference
	   //	 Double mle = fNll->getVal();

	 } else {
	   // evaluate to force this profile to evaluate and store
	   // best fit parameters for this data
	   fProfile->getVal();
	   
	   // store best fit parameters
	   fLastBestFitParams = (RooArgSet*) (fProfile->bestFitParams().clone("lastBestFit"));

	 }


       }
       if(!fProfile){ cout << "problem making profile" << endl;}



       // set parameters
       SetParameters(&paramsOfInterest, fProfile->getParameters(data) );


       /*
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
       */

       /*       TIter      itr = fProfile->getParameters(data)->createIterator();
       while ((myarg = (RooRealVar *)itr.Next())) { 
	 cout << myarg->GetName() << myarg->getVal();
       }
       cout << " = " << fProfile->evaluate()  << endl;
       */

       Double_t value = fProfile->getVal();
       RooMsgService::instance().setGlobalKillBelow(RooMsgService::DEBUG) ;

       // warning message
       if(value<0)
	 cout << "ProfileLikelihoodTestStat: problem that profileLL<0, indicates false minimum used earlier"<<endl;

       return value;
     }

      // Get the TestStatistic
      virtual const RooAbsArg* GetTestStatistic()  const {return fProfile;}  
    
      
   private:
      RooProfileLL* fProfile;
      RooAbsPdf* fPdf;
      RooNLLVar* fNll;
      const RooArgSet* fLastBestFitParams;
      RooAbsData* fLastData;
      //      Double_t fLastMLE;

   protected:
      ClassDef(ProfileLikelihoodTestStat,1)   
   };

}


#endif
