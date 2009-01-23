// @(#)root/roostats:$Id: TemplatedDistributionCreator.h 26805 2009-01-13 17:45:57Z cranmer $
// Author: Kyle Cranmer, Lorenzo Moneta, Gregory Schott, Wouter Verkerke
/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOSTATS_TemplatedDistributionCreator
#define ROOSTATS_TemplatedDistributionCreator

//_________________________________________________
/*
BEGIN_HTML
<p>
TemplatedDistributionCreator is a simple implementation of the DistributionCreator interface used for debugging.
The sampling distribution is uniformly random between [0,1] and is INDEPENDENT of the data.  So it is not useful
for true statistical tests, but it is useful for debugging.
</p>
END_HTML
*/
//

#ifndef ROOT_Rtypes
#include "Rtypes.h"
#endif

#include "RooStats/DistributionCreator.h"
#include "RooStats/RooStatsUtils.h"

#include "RooAbsPdf.h"
#include "RooArgSet.h"
#include "RooRealVar.h"
#include "RooAbsData.h"
#include "RooDataSet.h"
#include "RooWorkspace.h"
#include "SamplingDistribution.h"
#include "TRandom.h"
#include <vector>

namespace RooStats {

  template <class TestStatFunctor>
    class TemplatedDistributionCreator : public DistributionCreator {


  public:
    TemplatedDistributionCreator(TestStatFunctor &ts) {
      fTestStat = &ts;
      fWS = new RooWorkspace();
      fOwnsWorkspace = true;
      fDataName = "";
      fPdfName = "";
      fPOI = 0;
      fNuisParams=0;
    }

    virtual ~TemplatedDistributionCreator() {
      if(fOwnsWorkspace) delete fWS;
    }
    
     // Main interface to get a ConfInterval, pure virtual
     virtual SamplingDistribution* GetSamplingDistribution(RooArgSet& paramsOfInterest) {
       // normally this method would be complex, but here it is simple for debugging
       std::vector<Double_t> testStatVec;
       //       cout << " about to generate sampling dist " << endl;
       for(Int_t i=0; i<100; ++i){
	 //cout << " on toy number " << i << endl;
	 RooAbsData* toydata = GenerateToyData(paramsOfInterest);
	 testStatVec.push_back( fTestStat->Evaluate(*toydata, paramsOfInterest) );
	 delete toydata;
       }
       //       cout << " generated sampling dist " << endl;
       return new SamplingDistribution("TemplatedSamplingDist", "Samplint Distribution of Test Statistic", testStatVec );
     } 
     
     virtual RooAbsData* GenerateToyData(RooArgSet& paramsOfInterest) const{
       RooAbsPdf* pdf = fWS->pdf(fPdfName);
       // need a nicer way to specify observables in the dataset
       RooArgSet* observables = pdf->getVariables();

       // Set the parameters to desired values for generating toys
       RooStats::SetParameters(&paramsOfInterest, observables);
       /*
       TIter      itr = observables->createIterator();
       RooRealVar* myarg;
       while ((myarg = (RooRealVar *)itr.Next())) { 
	 cout << myarg->GetName() << " = " << myarg->getVal() << "  " ;
       }
       cout << endl;
       */

       if(fPOI) observables->remove(*fPOI, kFALSE, kTRUE);
       if(fNuisParams) observables->remove(*fNuisParams, kFALSE, kTRUE);
       // Need a nice way to determine how many events in a toy experiment
       Int_t nEvents = 100;
       RooAbsData* data = pdf->generate(*observables, nEvents);
       delete observables;
       //       delete pdf;
       return data;
     }

      // Main interface to evaluate the test statistic on a dataset
     virtual Double_t EvaluateTestStatistic(RooAbsData& data, RooArgSet& paramsOfInterest) {
       return fTestStat->Evaluate(data, paramsOfInterest);
     }

      // Get the TestStatistic
      virtual const RooAbsArg* GetTestStatistic()  const {
	 return fTestStat->GetTestStatistic();}  
    
      // Get the Confidence level for the test
      virtual Double_t ConfidenceLevel()  const {return 1.-fSize;}  

      // Common Initialization
      virtual void Initialize(RooAbsArg& testStatistic, 
			      RooArgSet& paramsOfInterest, 
			      RooArgSet& nuisanceParameters) {}

      // Set the DataSet, add to the the workspace if not already there
      virtual void SetData(RooAbsData& data) {
	if(&data){
	  fWS->import(data);
	  fDataName = data.GetName();
	  fWS->Print();
	}
      }
      // Set the Pdf, add to the the workspace if not already there
      virtual void SetPdf(RooAbsPdf& pdf) { 
	if(&pdf){
	  fWS->import(pdf);
	  fPdfName = pdf.GetName();
	}
      }

      // specify the name of the dataset in the workspace to be used
      virtual void SetData(const char* name) {fDataName = name;}
      // specify the name of the PDF in the workspace to be used
      virtual void SetPdf(const char* name) {fPdfName = name;}

      // specify the parameters of interest in the interval
      virtual void SetParameters(RooArgSet& set) {fPOI = &set;}
      // specify the nuisance parameters (eg. the rest of the parameters)
      virtual void SetNuisanceParameters(RooArgSet& set) {fNuisParams = &set;}

      // set the size of the test (rate of Type I error) ( Eg. 0.05 for a 95% Confidence Interval)
      virtual void SetSize(Double_t size) {fSize = size;}
      // set the confidence level for the interval (eg. 0.95 for a 95% Confidence Interval)
      virtual void SetConfidenceLevel(Double_t cl) {fSize = 1.-cl;}

      // Set the TestStatistic (want the argument to be a function of the data & parameter points
      virtual void SetTestStatistic(RooAbsArg&)  const {}  


      
   private:
      Double_t fSize;
      RooWorkspace* fWS; // a workspace that owns all the components to be used by the calculator
      Bool_t fOwnsWorkspace; // flag if this object owns its workspace
      const char* fPdfName; // name of  common PDF in workspace
      const char* fDataName; // name of data set in workspace
      RooArgSet* fPOI; // RooArgSet specifying  parameters of interest for interval
      RooArgSet* fNuisParams;// RooArgSet specifying  nuisance parameters for interval
      TestStatFunctor* fTestStat;

   protected:
      ClassDef(TemplatedDistributionCreator,1)   // A simple implementation of the DistributionCreator interface
	};
}


#endif
