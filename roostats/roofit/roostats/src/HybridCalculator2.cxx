// @(#)root/roostats:$Id$
// Author: Sven Kreiss   23/05/10
/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

/**
Same purpose as HybridCalculator, but different implementation.
*/


#include "RooStats/HybridCalculator2.h"
#include "RooStats/HybridPlot.h"

ClassImp(RooStats::HybridCalculator2)

using namespace RooStats;




HybridCalculator2::HybridCalculator2(
   const char* name,
   TestStatSampler &sampler,
   ModelConfig &altModel,
   ModelConfig &nullModel,
   RooAbsData &data
) :
   TNamed(name, name),
   fTestStatSampler(sampler),
   fAltModel(altModel),
   fNullModel(nullModel),
   fData(data)
{
}

HybridResult* HybridCalculator2::GetHypoTest() const {
   fNullModel.GuessObsAndNuisance(fData);
   fAltModel.GuessObsAndNuisance(fData);

   RooArgSet *allVar = fNullModel.GetPdf()->getVariables();
   allVar->add(*fAltModel.GetPdf()->getVariables());
   RooArgSet *saveAll = (RooArgSet*)allVar->snapshot();

   fTestStatSampler.SetModel(fNullModel);
   fNullModel.LoadSnapshot();
   RooArgSet* nullParams = (RooArgSet*)fNullModel.GetParametersOfInterest()->snapshot();
   SamplingDistribution* samp_null = fTestStatSampler.GetSamplingDistribution(*nullParams);

   *allVar = *saveAll;

   fTestStatSampler.SetModel(fAltModel);
   fAltModel.LoadSnapshot();
   SamplingDistribution* samp_alt = fTestStatSampler.GetSamplingDistribution(*nullParams);

   *allVar = *saveAll;
   delete saveAll;

   string resultname = string(GetName()) + string("_result");
   HybridResult* res = new HybridResult(
      resultname.c_str(),
      samp_alt->GetSamplingDistribution(),
      samp_null->GetSamplingDistribution(),
      fTestStatSampler.GetTestStatistic()->PValueIsRightTail()
   );
   res->SetDataTestStatistics(fTestStatSampler.EvaluateTestStatistic(fData, *nullParams));

#ifdef EXPECTED_NUISANCE_PAR
   HybridPlot* p = new HybridPlot(
      resultname.c_str(),
      resultname.c_str(),
      samp_alt->GetSamplingDistribution(),
      samp_alt->GetSampleWeights(),
      samp_null->GetSamplingDistribution(),
      samp_null->GetSampleWeights(),
      fTestStatSampler.EvaluateTestStatistic(fData, *allParams),
      100,
      kFALSE
   );
   p->Draw();
#endif

   return res;
}




