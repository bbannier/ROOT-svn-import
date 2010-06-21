// @(#)root/roostats:$Id$
// Author: Sven Kreiss   23/05/10
/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOSTATS_HybridCalculator2
#define ROOSTATS_HybridCalculator2


#ifndef ROOT_Rtypes
#include "Rtypes.h" // necessary for TNamed
#endif

#ifndef ROOSTATS_HypoTestCalculator
#include "RooStats/HypoTestCalculator.h"
#endif

#ifndef ROOSTATS_HybridResult
#include "RooStats/HybridResult.h"
#endif

#ifndef ROOSTATS_ModelConfig
#include "RooStats/ModelConfig.h"
#endif

#ifndef ROOSTATS_TestStatistic
#include "RooStats/TestStatistic.h"
#endif

#ifndef ROOSTATS_TestStatSampler
#include "RooStats/TestStatSampler.h"
#endif

#ifndef ROOSTATS_SamplingDistribution
#include "RooStats/SamplingDistribution.h"
#endif

#ifndef ROOSTATS_HypoTestResult
#include "RooStats/HypoTestResult.h"
#endif

namespace RooStats {

   class HybridCalculator2: public HypoTestCalculator, public TNamed {

   public:
      HybridCalculator2(
         const char *name,
         TestStatSampler &sampler,
         ModelConfig &altModel,
         ModelConfig &nullModel,
         RooAbsData &data
      );

   public:

      /// inherited methods from HypoTestCalculator interface
      virtual HypoTestResult* GetHypoTest() const;

      // set the model for the null hypothesis (only B)
      virtual void SetNullModel(const ModelConfig &nullModel) { fNullModel = nullModel; }
      // set the model for the alternate hypothesis  (S+B)
      virtual void SetAlternateModel(const ModelConfig &altModel) { fAltModel = altModel; }
      // Set the DataSet
      virtual void SetData(RooAbsData &data) { fData = data; }

      // Returns instance of TestStatSampler. Use to change properties of
      // TestStatSampler, e.g. GetTestStatSampler.SetTestSize(Double_t size);
      TestStatSampler& GetTestStatSampler(void) { return fTestStatSampler; }

      virtual void ForcePriorNuisance(RooAbsPdf& p) { fForcePriorNuisance = &p; }

   private:
      TestStatSampler &fTestStatSampler;
      ModelConfig &fAltModel;
      ModelConfig &fNullModel;
      RooAbsData &fData;
      RooAbsPdf *fForcePriorNuisance;

      void SetModel(ModelConfig& model) const;

   protected:
   ClassDef(HybridCalculator2,1)
};
}

#endif
