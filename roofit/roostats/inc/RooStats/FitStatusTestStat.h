// @(#)root/roostats:$Id: FitStatusTestStat.h 40018 2011-06-27 16:35:13Z moneta $
// Author: Sven Kreiss, Kyle Cranmer, Lorenzo Moneta, Gregory Schott, Wouter Verkerke
/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOSTATS_FitStatusTestStat
#define ROOSTATS_FitStatusTestStat

//_________________________________________________
/*
BEGIN_HTML
<p>
</p>
END_HTML
*/
//

#ifndef ROOT_Rtypes
#include "Rtypes.h"
#endif

#include <vector>

#include "RooStats/RooStatsUtils.h"

//#include "RooStats/DistributionCreator.h"
#include "RooStats/SamplingDistribution.h"
#include "RooStats/TestStatistic.h"

#include "RooStats/RooStatsUtils.h"

#include "RooRealVar.h"
#include "RooProfileLL.h"
#include "RooNLLVar.h"

#include "RooMinuit.h"
#include "RooFitResult.h"


#define ROOSTATS_FITSTATUS_TSVALUE 0
#define ROOSTATS_FITSTATUS_STATUS 1
#define ROOSTATS_FITSTATUS_COVQUAL 2
#define ROOSTATS_FITSTATUS_INVALIDNLL 3


namespace RooStats {

   class FitStatusTestStat: public TestStatistic {

      public:
         FitStatusTestStat() {
            // Proof constructor. Do not use.
            fPdf = 0;
            fProfile = 0;
            fNll = 0;
            fCachedBestFitParams = 0;
            fLastData = 0;
            fOneSided = false;
            fReuseNll = false;
            fReturnFlag = 1;
         }
         FitStatusTestStat(RooAbsPdf& pdf) {
            fPdf = &pdf;
            fProfile = 0;
            fNll = 0;
            fCachedBestFitParams = 0;
            fLastData = 0;
            fOneSided = false;
            fReuseNll = false;
            fReturnFlag = 1;
         }
         virtual ~FitStatusTestStat() {
            //       delete fRand;
            //       delete fTestStatistic;
            if(fProfile) delete fProfile;
            if(fNll) delete fNll;
            if(fCachedBestFitParams) delete fCachedBestFitParams;
         }
         void SetOneSided(Bool_t flag = true) {
            fOneSided = flag;
         }
         void SetReturnFlag(int r) { fReturnFlag = r; }

         static void SetAlwaysReuseNLL(Bool_t flag) {
            fgAlwaysReuseNll = flag;
         }
         void SetReuseNLL(Bool_t flag) {
            fReuseNll = flag;
         }

         // Main interface to evaluate the test statistic on a dataset
         virtual Double_t Evaluate(RooAbsData& data, RooArgSet& paramsOfInterest) {
            if (!&data) {
               cout << "problem with data" << endl;
               return 0;
            }

            RooRealVar* firstPOI = (RooRealVar*) paramsOfInterest.first();
            double initial_mu_value = firstPOI->getVal();
            //paramsOfInterest.getRealValue(firstPOI->GetName());

            RooFit::MsgLevel msglevel = RooMsgService::instance().globalKillBelow();
            RooMsgService::instance().setGlobalKillBelow(RooFit::FATAL);

            // simple
            Bool_t reuse = (fReuseNll || fgAlwaysReuseNll);

            Bool_t created(kFALSE);
            if (!reuse || fNll == 0) {
               fNll = (RooNLLVar*) fPdf->createNLL(data, RooFit::CloneData(kFALSE));
               fProfile = (RooProfileLL*) fNll->createProfile(paramsOfInterest);
               created = kTRUE;
               //cout << "creating profile LL " << fNll << " " << fProfile << " data = " << &data << endl ;
            }
            if (reuse && !created) {
               //cout << "reusing profile LL " << fNll << " new data = " << &data << endl ;
               fNll->setData(data, kFALSE);
               if (fProfile) delete fProfile;
               fProfile = (RooProfileLL*) fNll->createProfile(paramsOfInterest);
               //fProfile->clearAbsMin() ;
            }

            // make sure we set the variables attached to this nll
            RooArgSet* attachedSet = fNll->getVariables();

            *attachedSet = paramsOfInterest;

            //       fPdf->setEvalErrorLoggingMode(RooAbsReal::CountErrors);
            //       profile->setEvalErrorLoggingMode(RooAbsReal::CountErrors);
            //       ((RooProfileLL*)profile)->nll().setEvalErrorLoggingMode(RooAbsReal::CountErrors);
            //       nll->setEvalErrorLoggingMode(RooAbsReal::CountErrors);
            //cout << "evaluating profile LL" << endl ;
            double ret = fProfile->getVal();
            //       cout << "profile value = " << ret << endl ;
            //       cout <<"eval errors pdf = "<<fPdf->numEvalErrors() << endl;
            //       cout <<"eval errors profile = "<<profile->numEvalErrors() << endl;
            //       cout <<"eval errors profile->nll = "<<((RooProfileLL*)profile)->nll().numEvalErrors() << endl;
            //       cout <<"eval errors nll = "<<nll->numEvalErrors() << endl;
            //       if(profile->numEvalErrors()>0)
            //       	 cout <<"eval errors = "<<profile->numEvalErrors() << endl;
            //       paramsOfInterest.Print("v");
            //       cout << "ret = " << ret << endl;

            if (fOneSided) {
               double fit_favored_mu = ((RooProfileLL*) fProfile)->bestFitObs().getRealValue(firstPOI->GetName());

               if (fit_favored_mu > initial_mu_value)
               // cout <<"fit-favored_mu, initial value" << fit_favored_mu << " " << initial_mu_value<<endl;
               ret = 0;
            }
            delete attachedSet;


            // ==============================================
            // ========= to return fit status ===============
            // ==============================================
            RooMinuit *minuit = fProfile->minuit();
            RooFitResult *fitResult = minuit->save();

            if(fReturnFlag == ROOSTATS_FITSTATUS_STATUS)
               ret = fitResult->status(); // minuit status code
            if(fReturnFlag == ROOSTATS_FITSTATUS_COVQUAL)
               ret = fitResult->covQual(); // minuit quality code of covariance matrix
            if(fReturnFlag == ROOSTATS_FITSTATUS_INVALIDNLL)
               ret = fitResult->numInvalidNLL(); // minuit quality code of covariance matrix

            delete fitResult;
            //delete minuit; // dont own this one
            // ==============================================


            if (!reuse) {
               //cout << "deleting ProfileLL " << fNll << " " << fProfile << endl ;
               delete fNll;
               fNll = 0;
               delete fProfile;
               fProfile = 0;
            }

            RooMsgService::instance().setGlobalKillBelow(msglevel);

            return ret;
         }

         virtual const TString GetVarName() const {
            return "Fit Status";
         }

         //      const bool PValueIsRightTail(void) { return false; } // overwrites default


      private:
         RooProfileLL* fProfile; //!
         RooAbsPdf* fPdf;
         RooNLLVar* fNll; //!
         const RooArgSet* fCachedBestFitParams;
         RooAbsData* fLastData;
         //      Double_t fLastMLE;
         Bool_t fOneSided;

         static Bool_t fgAlwaysReuseNll;
         Bool_t fReuseNll;

         int fReturnFlag;

      protected:
         ClassDef(FitStatusTestStat,1) // implements the profile likelihood ratio as a test statistic to be used with several tools
   };
}


Bool_t RooStats::FitStatusTestStat::fgAlwaysReuseNll = kFALSE ;


#endif
