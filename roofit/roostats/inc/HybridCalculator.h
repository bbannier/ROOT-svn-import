// @(#)root/roostats:$Id$

/*************************************************************************
 * Project: RooStats                                                     *
 * Package: RooFit/RooStats                                              *
 * Authors:                                                              *
 *   Kyle Cranmer, Lorenzo Moneta, Gregory Schott, Wouter Verkerke       *
 *************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOSTATS_HybridCalculator
#define ROOSTATS_HybridCalculator

#ifndef ROOSTATS_HypoTestCalculator
#include "RooStats/HypoTestCalculator.h"
#endif

#include <vector>

#include "TH1.h"

#include "RooStats/HybridResult.h"

namespace RooStats {

   class HybridCalculator : /*public HypoTestCalculator ,*/ public TNamed {

   public:
      /// Constructor for HybridCalculator
      HybridCalculator(const char *name,
                       const char *title,
                       RooAbsPdf& sb_model,
                       RooAbsPdf& b_model,
                       RooArgList& observables,
                       RooArgSet& nuisance_parameters,
                       RooAbsPdf& prior_pdf);

      /// Destructor of HybridCalculator
      virtual ~HybridCalculator();

      void SetTestStatistics(int index);
      HybridResult* Calculate(TH1& data, unsigned int nToys, bool usePriors);
      HybridResult* Calculate(RooTreeData& data, unsigned int nToys, bool usePriors);
      HybridResult* Calculate(unsigned int nToys, bool usePriors);
      void PrintMore(const char* options);

   private:
      void RunToys(std::vector<double>& bVals, std::vector<double>& sbVals, unsigned int nToys, bool usePriors);

      RooAbsPdf& fSbModel;
      RooAbsPdf& fBModel;
      RooArgList& fObservables;
      RooArgSet& fParameters;
      RooAbsPdf& fPriorPdf;
      unsigned int fTestStatisticsIdx;

   protected:
      ClassDef(HybridCalculator,1)  // Hypothesis test calculator using a Bayesian-frequentist hybrid method
   };
}

#endif
