// @(#)root/roostats:$Id$
// Author: Sven Kreiss    June 2010
/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOSTATS_RatioOfProfiledLikelihoodsTestStat
#define ROOSTATS_RatioOfProfiledLikelihoodsTestStat

//_________________________________________________
/*
BEGIN_HTML
<p>
TestStatistic that returns the ratio of profiled likelihoods.
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

#ifndef ROOSTATS_TestStatistic
#include "RooStats/TestStatistic.h"
#endif

namespace RooStats {

class RatioOfProfiledLikelihoodsTestStat: public TestStatistic {

   public:
      RatioOfProfiledLikelihoodsTestStat(RooAbsPdf& nullPdf, RooAbsPdf& altPdf) :
         fNullPdf(nullPdf), fAltPdf(altPdf)
      {
         // Calculates the ratio of profiled likelihoods. The variable values
         // for the alternative hypothesis are taken at the time of the construction
         // of this test statistics. The null hypotheses values are passed into
         // each call of Evaluate(...).

         fAltVars = (RooArgSet*)fAltPdf.getVariables()->snapshot();
      }
      ~RatioOfProfiledLikelihoodsTestStat(void) {
         delete fAltVars;
      }

      Double_t ProfiledLikelihood(RooAbsData& data, RooArgSet& poi, RooAbsPdf& pdf) {
         RooArgSet* constrainedParams = pdf.getParameters(data);
         RemoveConstantParameters(constrainedParams);
         RooNLLVar* nll = (RooNLLVar*) pdf.createNLL(data, RooFit::CloneData(kFALSE), RooFit::Constrain(*constrainedParams));
         RooProfileLL* profile = (RooProfileLL*) nll->createProfile(poi);
         double nllVal = profile->getVal();
         delete profile;
         delete nll;
         delete constrainedParams;

         return nllVal;
      }

      virtual Double_t Evaluate(RooAbsData& data, RooArgSet& nullParamsOfInterest) {
         RooFit::MsgLevel msglevel = RooMsgService::instance().globalKillBelow();
         RooMsgService::instance().setGlobalKillBelow(RooFit::FATAL);

         // construct allVars
         RooArgSet *allVars = fNullPdf.getVariables();
         RooArgSet *altVars = fAltPdf.getVariables();
         allVars->add(*altVars);
         delete altVars;

         RooArgSet *saveNullPOI = (RooArgSet*)nullParamsOfInterest.snapshot();
         RooArgSet *saveAll = (RooArgSet*)allVars->snapshot();


         // null
         *allVars = nullParamsOfInterest;
         double nullNLL = ProfiledLikelihood(data, nullParamsOfInterest, fNullPdf);

         // alt
         RooArgSet altPOI(nullParamsOfInterest);
         altPOI = *fAltVars;
         *allVars = altPOI;
         double altNLL = ProfiledLikelihood(data, altPOI, fAltPdf);


         *allVars = *saveAll;
         delete saveAll;
         delete allVars;

         nullParamsOfInterest = *saveNullPOI;
         delete saveNullPOI;

         RooMsgService::instance().setGlobalKillBelow(msglevel);
         return -((-nullNLL) - (-altNLL));
      }

      // TODO It has to be implemented?
      virtual const RooAbsArg* GetTestStatistic() const { return NULL; }

      virtual const TString GetVarName() const { return "log(L(#mu_{1},#hat{#nu}_{1}) / L(#mu_{0},#hat{#nu}_{0}))"; }

      const bool PValueIsRightTail(void) { return false; } // overwrites default


   private:
      RooAbsPdf& fNullPdf;
      RooAbsPdf& fAltPdf;
      RooArgSet* fAltVars;

   protected:
   ClassDef(RatioOfProfiledLikelihoodsTestStat,1)
};

}


#endif
