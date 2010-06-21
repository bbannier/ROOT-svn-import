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
   fData(data),
   fForcePriorNuisance(NULL)
{
}

void HybridCalculator2::SetModel(ModelConfig& model) const {
   fTestStatSampler.SetSamplingDistName(model.GetName());
   fTestStatSampler.SetPdf(*model.GetPdf());
   fTestStatSampler.SetObservables(*model.GetObservables());
   fTestStatSampler.SetGlobalObservables(*model.GetGlobalObservables());
   fTestStatSampler.SetNuisanceParameters(*model.GetNuisanceParameters());
   fTestStatSampler.SetParameters(*model.GetParametersOfInterest());

   if(model.GetNuisanceParameters()  &&  model.GetNuisanceParameters()->getSize() > 0) {
      if(fForcePriorNuisance) {
         coutI(InputArguments) << "Using the forced prior for the nuisance parameters." << endl;
         fTestStatSampler.SetPriorNuisance(fForcePriorNuisance);
      }
      else {
         coutE(InputArguments)
            << "You have to specify a prior for the nuisance parameters "
            << "using ForcePriorNuisance(..)." << endl;
      }
   }
}

HypoTestResult* HybridCalculator2::GetHypoTest() const {
   fNullModel.GuessObsAndNuisance(fData);
   fAltModel.GuessObsAndNuisance(fData);

   RooArgSet *allVar = fNullModel.GetPdf()->getVariables();
   allVar->add(*fAltModel.GetPdf()->getVariables());
   RooArgSet *saveAll = (RooArgSet*)allVar->snapshot();

   SetModel(fNullModel);
   fNullModel.LoadSnapshot();
   RooArgSet* nullParams = (RooArgSet*)fNullModel.GetParametersOfInterest()->snapshot();
   SamplingDistribution* samp_null = fTestStatSampler.GetSamplingDistribution(*nullParams);

   *allVar = *saveAll;

   SetModel(fAltModel);
   fAltModel.LoadSnapshot();
   SamplingDistribution* samp_alt = fTestStatSampler.GetSamplingDistribution(*nullParams);

   *allVar = *saveAll;
   delete saveAll;

   string resultname = string(GetName()) + string("_result");
   HypoTestResult* res = new HypoTestResult(resultname.c_str());
   res->SetPValueIsRightTail(fTestStatSampler.GetTestStatistic()->PValueIsRightTail());
   res->SetTestStatisticData(fTestStatSampler.EvaluateTestStatistic(fData, *nullParams));
   res->SetAltDistribution(samp_alt);
   res->SetNullDistribution(samp_null);

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




