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

#ifndef ROOSTATS_ToyMCSampler2
#define ROOSTATS_ToyMCSampler2

//_________________________________________________
/*
BEGIN_HTML
<p>
ToyMCSampler is an implementation of the TestStatSampler interface.
It generates Toy Monte Carlo for a given parameter point and evaluates a
TestStatistic.
</p>
END_HTML
*/
//

#ifndef ROOT_Rtypes
#include "Rtypes.h"
#endif

#include <vector>
#include <sstream>

#include "RooStats/TestStatSampler.h"
#include "RooStats/SamplingDistribution.h"
#include "RooStats/TestStatistic.h"
#include "RooStats/ModelConfig.h"

#include "RooWorkspace.h"
#include "RooMsgService.h"
#include "RooAbsPdf.h"

#include "RooDataSet.h"

namespace RooStats {

class ToyMCSampler2: public TestStatSampler {

   public:
      ToyMCSampler2(TestStatistic &ts, Int_t ntoys = 1000) :
         fTestStat(&ts), fSamplingDistName("temp"), fNToys(ntoys)
      {
         fPdf = NULL;
         fPriorNuisance = NULL;
         fPOI = NULL;
         fNuisancePars = NULL;
         fObservables = NULL;
         fGlobalObservables = NULL;

         fSize = 0.05;
         fNEvents = 0;
         fExpectedNuisancePar = kFALSE;
      }

      virtual ~ToyMCSampler2() {
      }

      virtual SamplingDistribution* GetSamplingDistribution(RooArgSet& nullPOI);

      virtual RooAbsData* GenerateToyData(RooArgSet& /*nullPOI*/) const;



      // append more toys to sampling distribution
      virtual SamplingDistribution* AppendSamplingDistribution(
         RooArgSet& nullPOI,
         SamplingDistribution* last,
         Int_t nToys
      ) {
        Int_t tmp = fNToys;
        fNToys = nToys;
        SamplingDistribution* newSamples = GetSamplingDistribution(nullPOI);
        fNToys = tmp;

        if(last) {
           last->Add(newSamples);
           delete newSamples;
           return last;
        }

        return newSamples;
      }



      // Main interface to evaluate the test statistic on a dataset
      virtual Double_t EvaluateTestStatistic(RooAbsData& data, RooArgSet& allParameters) {
         return fTestStat->Evaluate(data, allParameters);
      }

      virtual TestStatistic* GetTestStatistic() const { return fTestStat; }
      virtual Double_t ConfidenceLevel() const { return 1. - fSize; }
      virtual void Initialize(
         RooAbsArg& /*testStatistic*/,
         RooArgSet& /*paramsOfInterest*/,
         RooArgSet& /*nuisanceParameters*/
      ) {}

      virtual void SetNToys(const Int_t ntoy) { fNToys = ntoy; }
      virtual void SetNEventsPerToy(const Int_t nevents) {
         // Forces n events even for extended PDFs. Set NEvents=0 to
         // use the Poisson distributed events from the extended PDF.
         fNEvents = nevents;
      }


      // Set the Pdf, add to the the workspace if not already there
      virtual void SetPdf(RooAbsPdf& pdf) { fPdf = &pdf; }
      // How to randomize the prior. Set to NULL to deactivate randomization.
      virtual void SetPriorNuisance(RooAbsPdf* pdf) { fPriorNuisance = pdf; }
      // specify the parameters of interest in the interval
      virtual void SetParameters(const RooArgSet& poi) { fPOI = &poi; }
      // specify the nuisance parameters (eg. the rest of the parameters)
      virtual void SetNuisanceParameters(const RooArgSet& np) { fNuisancePars = &np; }
      // specify the observables in the dataset (needed to evaluate the test statistic)
      virtual void SetObservables(const RooArgSet& o) { fObservables = &o; }
      // specify the conditional observables
      virtual void SetGlobalObservables(const RooArgSet& o) { fGlobalObservables = &o; }


      // set the size of the test (rate of Type I error) ( Eg. 0.05 for a 95% Confidence Interval)
      virtual void SetTestSize(Double_t size) { fSize = size; }
      // set the confidence level for the interval (eg. 0.95 for a 95% Confidence Interval)
      virtual void SetConfidenceLevel(Double_t cl) { fSize = 1. - cl; }

      // Set the TestStatistic (want the argument to be a function of the data & parameter points
      virtual void SetTestStatistic(TestStatistic *testStatistic) { fTestStat = testStatistic; }

      virtual void SetExpectedNuisancePar(Bool_t i) { fExpectedNuisancePar = i; cout << "WILL NOT WORK YET" << endl; } // TODO
      virtual void SetAsimovNuisancePar(Bool_t i) { fExpectedNuisancePar = i; cout << "WILL NOT WORK YET" << endl; } // TODO

      // Checks for sufficient information to do a GetSamplingDistribution(...).
      Bool_t CheckConfig(void);

      // control to use bin data generation
      void SetGenerateBinned(bool binned = true) { fGenerateBinned = binned; }

      void SetSamplingDistName(const char* name) { if(name) fSamplingDistName = name; }

   private:
      TestStatistic *fTestStat; // test statistic that is being sampled
      RooAbsPdf *fPdf; // model
      string fSamplingDistName; // name of the model
      RooAbsPdf *fPriorNuisance; // prior pdf for nuisance parameters
      const RooArgSet *fPOI; // parameters of interest
      const RooArgSet *fNuisancePars;
      const RooArgSet *fObservables;
      const RooArgSet *fGlobalObservables;
      Int_t fNToys; // number of toys to generate
      Int_t fNEvents; // number of events per toy (may be ignored depending on settings)
      Double_t fSize;
      Bool_t fExpectedNuisancePar; // whether to use expectation values for nuisance parameters (ie Asimov data set)
      Bool_t fGenerateBinned;


   protected:
   ClassDef(ToyMCSampler2,1) // A simple implementation of the TestStatSampler interface
};
}


#endif
