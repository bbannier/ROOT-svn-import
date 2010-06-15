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
      ToyMCSampler2(TestStatistic &ts, Int_t ntoys) :
         fTestStat(&ts), fNToys(ntoys)
      {
         fModel = NULL;
         fSize = 0.05;
         fNEvents = 0;
         fExpectedNuisancePar = kFALSE;
      }

      ToyMCSampler2(TestStatistic &ts, Int_t ntoys, ModelConfig &model) :
         fTestStat(&ts), fModel(&model), fNToys(ntoys)
      {
         fSize = 0.05;
         fNEvents = 0;
         fExpectedNuisancePar = kFALSE;
      }

      virtual ~ToyMCSampler2() {
      }

      virtual SamplingDistribution* GetSamplingDistribution(RooArgSet& allParameters);

      virtual RooAbsData* GenerateToyData(RooArgSet& /*allParameters*/) const;


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

      virtual void SetData(RooAbsData& data) {
         fModel->GetWS()->import(data);
         fDataName = data.GetName();
         fModel->GetWS()->Print();
      }
      virtual void SetData(const char* name) {
         fDataName = name;
      }

      // set the size of the test (rate of Type I error) ( Eg. 0.05 for a 95% Confidence Interval)
      virtual void SetTestSize(Double_t size) { fSize = size; }
      // set the confidence level for the interval (eg. 0.95 for a 95% Confidence Interval)
      virtual void SetConfidenceLevel(Double_t cl) { fSize = 1. - cl; }

      // Set the TestStatistic (want the argument to be a function of the data & parameter points
      virtual void SetTestStatistic(TestStatistic *testStatistic) { fTestStat = testStatistic; }

      // SetModel and load Snapshot if it exists
      virtual void SetModel(ModelConfig& model) {
         fModel = &model;
      }
      virtual void SetExpectedNuisancePar(Bool_t i) { fExpectedNuisancePar = i; cout << "WILL NOT WORK YET" << endl; } // TODO
      virtual void SetAsimovNuisancePar(Bool_t i) { fExpectedNuisancePar = i; cout << "WILL NOT WORK YET" << endl; } // TODO

   private:
      TestStatistic *fTestStat; // test statistic that is being sampled
      ModelConfig *fModel;
      std::string fDataName;
      Int_t fNToys; // number of toys to generate
      Int_t fNEvents; // number of events per toy (may be ignored depending on settings)
      Double_t fSize;
      Bool_t fExpectedNuisancePar;

   protected:
   ClassDef(ToyMCSampler2,1) // A simple implementation of the TestStatSampler interface
};
}


#endif
