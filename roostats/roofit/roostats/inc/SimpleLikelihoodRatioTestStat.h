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
SimpleLikelihoodRatioTestStat: TestStatistic that returns -log(L[null] / L[alt]) where
L is the likelihood.
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
         // The parameter values for the alternative are taken at the time
         // this constructor is called. The parameter values for the null
         // are given in each call to Evaluate(...).

         fAltVars = (RooArgSet*)fAltPdf.getVariables()->snapshot();
      }
      virtual ~SimpleLikelihoodRatioTestStat() {
      }

      virtual Double_t Evaluate(RooAbsData& data, RooArgSet& nullPOI) {
         RooFit::MsgLevel msglevel = RooMsgService::instance().globalKillBelow();
         RooMsgService::instance().setGlobalKillBelow(RooFit::FATAL);

         RooArgSet *altVars = fAltPdf.getVariables();
         RooArgSet *nullVars = fNullPdf.getVariables();


         RooArgSet *allVars = fNullPdf.getVariables();
         allVars->add(*altVars);
         RooArgSet *saveAll = (RooArgSet*)allVars->snapshot();

         RooAbsReal *nll;
         *nullVars = nullPOI;
         nll = fNullPdf.createNLL(data, RooFit::CloneData(kFALSE), RooFit::Extended());
         double nullNLL = nll->getVal();
         delete nll;
         *altVars = *fAltVars;
         nll = fAltPdf.createNLL(data, RooFit::CloneData(kFALSE), RooFit::Extended());
         double altNLL = nll->getVal();
         delete nll;

         *allVars = *saveAll;
         delete saveAll;
         delete allVars;



         delete nullVars;
         delete altVars;

         RooMsgService::instance().setGlobalKillBelow(msglevel);
         return -((-nullNLL) - (-altNLL));
      }

      // TODO Has to be implemented?
      virtual const RooAbsArg* GetTestStatistic() const { return NULL; }

      virtual const TString GetVarName() const { return "log(L(#mu_{1}) / L(#mu_{0}))"; }

      virtual const bool PValueIsRightTail(void) { return false; } // overwrite the default


   private:
      RooAbsPdf& fNullPdf;
      RooAbsPdf& fAltPdf;
      RooArgSet* fAltVars;

   protected:
   ClassDef(SimpleLikelihoodRatioTestStat,1)
};

}


#endif
