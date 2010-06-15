// @(#)root/roostats:$Id$
// Author: Sven Kreiss    June 2010
/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOSTATS_SimpleLikelihoodRatioTestStat
#define ROOSTATS_SimpleLikelihoodRatioTestStat

//_________________________________________________
/*
BEGIN_HTML
<p>
Neg2LogLikeRatioTestStat: TestStatistic that returns -2*log(Lambda) where
Lambda is the likelihood ratio.
</p>
END_HTML
*/
//

#ifndef ROOT_Rtypes
#include "Rtypes.h"
#endif

#ifndef ROO_NLL_VAR
#include "RooNLLVar.h"
#endif

#include "RooStats/TestStatistic.h"

namespace RooStats {

class SimpleLikelihoodRatioTestStat: public TestStatistic {

   public:
      SimpleLikelihoodRatioTestStat(RooAbsPdf& nullPdf, RooAbsPdf& altPdf) :
         fNullPdf(nullPdf), fAltPdf(altPdf)
      {
         fNullVars = (RooArgSet*)fNullPdf.getVariables()->snapshot();
         fAltVars = (RooArgSet*)fAltPdf.getVariables()->snapshot();
      }
      virtual ~SimpleLikelihoodRatioTestStat() {
      }

      virtual Double_t Evaluate(RooAbsData& data, RooArgSet& /*paramsOfInterest*/) {
         RooFit::MsgLevel msglevel = RooMsgService::instance().globalKillBelow();
         RooMsgService::instance().setGlobalKillBelow(RooFit::FATAL);

         RooArgSet *allVars = fNullPdf.getVariables();
         allVars->add(*fAltPdf.getVariables());
         RooArgSet *saveAll = (RooArgSet*)allVars->snapshot();

         *fNullPdf.getVariables() = *fNullVars;
         RooAbsReal *nll;
         nll = fNullPdf.createNLL(data, RooFit::CloneData(kFALSE), RooFit::Extended());
         double nullNLL = nll->getVal();
         delete nll;
         *fAltPdf.getVariables() = *fAltVars;
         nll = fAltPdf.createNLL(data, RooFit::CloneData(kFALSE), RooFit::Extended());
         double altNLL = nll->getVal();
         delete nll;

         *allVars = *saveAll;
         delete saveAll;

         RooMsgService::instance().setGlobalKillBelow(msglevel);
         return -((-nullNLL) - (-altNLL));
      }

      // TODO Has to be implemented?
      virtual const RooAbsArg* GetTestStatistic() const { return NULL; }

      virtual const TString GetVarName() const { return "-ln[L(null) / L(alt)]"; }

      virtual const bool PValueIsRightTail(void) { return false; } // overwrite the default


   private:
      RooAbsPdf& fNullPdf;
      RooAbsPdf& fAltPdf;
      RooArgSet* fNullVars;
      RooArgSet* fAltVars;

   protected:
   ClassDef(SimpleLikelihoodRatioTestStat,1)
};

}


#endif
