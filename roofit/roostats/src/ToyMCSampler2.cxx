// @(#)root/roostats:$Id$
// Author: Sven Kreiss    June 2010
// Author: Kyle Cranmer, Lorenzo Moneta, Gregory Schott, Wouter Verkerke
/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include "RooStats/ToyMCSampler2.h"

#ifndef ROO_MSG_SERVICE
#include "RooMsgService.h"
#endif

#ifndef ROO_DATA_HIST
#include "RooDataHist.h"
#endif

namespace RooStats {

Bool_t ToyMCSampler2::CheckConfig(void) {
   // only checks, no guessing/determination (do this in calculators,
   // e.g. using ModelConfig::GuessObsAndNuissance(...))
   bool goodConfig = true;

   // TODO
   //if(!fTestStat) { ooccoutE(NULL,InputArguments) << "Test statistic not set." << endl; goodConfig = false; }
   if(!fObservables) { cout << "Observables not set." << endl; goodConfig = false; }
   if(!fPOI) { cout << "Parameters of interest not set." << endl; goodConfig = false; }
   if(!fPdf) { cout << "Pdf not set." << endl; goodConfig = false; }

   return goodConfig;
}

SamplingDistribution* ToyMCSampler2::GetSamplingDistribution(RooArgSet& nullPOI) {
   CheckConfig();

   std::vector<Double_t> testStatVec;
   std::vector<Double_t> testStatWeights;

   RooArgSet *allVars = fPdf->getVariables();
   RooArgSet *saveAll = (RooArgSet*) allVars->snapshot();

   RooDataSet *nuisanceParPoints = NULL;
   if (fPriorNuisance  &&  fNuisancePars) {
      if (fExpectedNuisancePar) {
#ifdef EXPECTED_NUISANCE_PAR
         nuisanceParPoints = fModel->GetPriorPdf()->generateBinned(
            *fModel->GetNuisanceParameters(),
            RooFit::ExpectedData(),
            RooFit::NumEvents(fNToys*100) // TODO Good value?
         );
         if(fNToys != nuisanceParPoints->numEntries()) {
            cout << "Overwriting fNToys with " << nuisanceParPoints->numEntries() << endl;
            fNToys = nuisanceParPoints->numEntries();
         }
#endif
      }else{
         nuisanceParPoints = fPriorNuisance->generate(*fNuisancePars, fNToys);
      }
   }


   for (Int_t i = 0; i < fNToys; ++i) {
      if (nuisanceParPoints) {
         *allVars = *nuisanceParPoints->get(i);
         RooAbsData* toydata = GenerateToyData(nullPOI);
         testStatVec.push_back(fTestStat->Evaluate(*toydata, nullPOI));
         testStatWeights.push_back(nuisanceParPoints->weight());
         delete toydata;
      }else{
         RooAbsData* toydata = GenerateToyData(nullPOI);
         testStatVec.push_back(fTestStat->Evaluate(*toydata, nullPOI));
         delete toydata;
      }
   }
   delete nuisanceParPoints;

   *allVars = *saveAll;
   delete saveAll;
   delete allVars;

#ifdef EXPECTED_NUISANCE_PAR
   if (testStatWeights.size()) {
      return new SamplingDistribution(
         fSamplingDistName.c_str(),
         "Sampling Distribution of Test Statistic (Expected NuisPar)",
         testStatVec,
         testStatWeights,
         fTestStat->GetVarName()
      );
   }
#endif
   return new SamplingDistribution(
      fSamplingDistName.c_str(),
      fSamplingDistName.c_str(),
      testStatVec,
      fTestStat->GetVarName()
   );
}


RooAbsData* ToyMCSampler2::GenerateToyData(RooArgSet& /*nullPOI*/) const {
   // This method generates a toy data set for the given parameter point taking
   // global observables into account.

   RooArgSet observables(*fObservables);
   if(fGlobalObservables  &&  fGlobalObservables->getSize()) {
      observables.remove(*fGlobalObservables);

      // generate one set of global observables and assign it
      RooDataSet *one = fPdf->generate(*fGlobalObservables, 1);
      const RooArgSet *values = one->get();
      RooArgSet *allVars = fPdf->getVariables();
      *allVars = *values;
      delete allVars;
      delete values;
      delete one;
   }

   RooAbsData* data = NULL;
   if(fNEvents == 0  &&  fPdf->expectedEvents(observables) > 0) {
      if(fGenerateBinned) data = fPdf->generateBinned(observables, RooFit::Extended());
      else                data = fPdf->generate      (observables, RooFit::Extended());
   } else {
      if(fNEvents == 0) {
         cout << "ERROR: Please specify number of events or use extended pdf." << endl;
      }
      if(fGenerateBinned) data = fPdf->generateBinned(observables, fNEvents);
      else                data = fPdf->generate      (observables, fNEvents);
   }

   return data;
}



} // end namespace RooStats
