// @(#)root/roostats:$Id$
// Author: Sven Kreiss    June 2010
// Author: Kyle Cranmer, Lorenzo Moneta, Gregory Schott, Wouter Verkerke
// Additions and modifications by Mario Pelliccioni
/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include "RooStats/ToyMCSampler2.h"


namespace RooStats {

SamplingDistribution* ToyMCSampler2::GetSamplingDistribution(RooArgSet& allParameters) {
   std::vector<Double_t> testStatVec;
   std::vector<Double_t> testStatWeights;

   RooArgSet *saveAll = (RooArgSet*) fModel->GetPdf()->getVariables()->snapshot();

   RooAbsData *nuisanceParPoints = NULL;
   if (fModel->GetPriorPdf()  &&  fModel->GetNuisanceParameters()) {
      if (fExpectedNuisancePar) {
         // ad hoc method
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
         // ad hoc method
         nuisanceParPoints = fModel->GetPriorPdf()->generate(*fModel->GetNuisanceParameters(), fNToys);
      }
   }

   for (Int_t i = 0; i < fNToys; ++i) {
      if (nuisanceParPoints) {
         *fModel->GetPdf()->getVariables() = *nuisanceParPoints->get(i);
         RooAbsData* toydata = GenerateToyData(allParameters);
         testStatVec.push_back(fTestStat->Evaluate(*toydata, allParameters));
         testStatWeights.push_back(nuisanceParPoints->weight());
         delete toydata;
      }else{
         RooAbsData* toydata = GenerateToyData(allParameters);
         testStatVec.push_back(fTestStat->Evaluate(*toydata, allParameters));
         delete toydata;
      }
   }
   delete nuisanceParPoints;

   *fModel->GetPdf()->getVariables() = *saveAll;
   delete saveAll;

#ifdef EXPECTED_NUISANCE_PAR
   if (testStatWeights.size()) {
      return new SamplingDistribution(
         "temp",
         "Sampling Distribution of Test Statistic (Expected NuisPar)",
         testStatVec,
         testStatWeights,
         fTestStat->GetVarName()
      );
   }
#endif
   return new SamplingDistribution(
      "temp",
      "Sampling Distribution of Test Statistic",
      testStatVec,
      fTestStat->GetVarName()
   );
}


RooAbsData* ToyMCSampler2::GenerateToyData(RooArgSet& /*allParameters*/) const {
   // This method generates a toy dataset for the given parameter point.

   RooFit::MsgLevel level = RooMsgService::instance().globalKillBelow();
   RooMsgService::instance().setGlobalKillBelow(RooFit::ERROR);

   RooDataSet* data = NULL;
   if(fNEvents == 0  &&  fModel->GetPdf()->expectedEvents(fModel->GetObservables()) > 0) {
      data = fModel->GetPdf()->generate(*fModel->GetObservables(), RooFit::Extended());
   } else {
      data = fModel->GetPdf()->generate(*fModel->GetObservables(), fNEvents);
   }

   RooMsgService::instance().setGlobalKillBelow(level);

   return data;
}



} // end namespace RooStats
