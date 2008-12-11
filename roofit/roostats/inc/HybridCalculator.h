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


#ifndef ROOSTATS_HypoTestResult
#include "RooStats/HybridResult.h"
#endif

class TH1; 

namespace RooStats {

   class HybridCalculator : public HypoTestCalculator , public TNamed {

   public:

      /// Default constructor  
      HybridCalculator(); 

      /// Constructor with only name and title 
      HybridCalculator(const char *name,
                       const char *title); 
      
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

      /// inherited methods from HypoTestCalculanterface
      virtual HybridResult* GetHypoTest() const;

      // set a workspace that owns all the necessary components for the analysis
      virtual void SetWorkspace(RooWorkspace& ws);
      // set the PDF for the null hypothesis (only B)
      virtual void SetNullPdf(const char* name) { fBModelName = name; }
      // set the PDF for the alternate hypothesis  (S+B)
      virtual void SetAlternatePdf(const char* name ) { fSbModelName = name;} 
      // set a common PDF for both the null and alternate hypotheses
      virtual void SetCommonPdf(const char* name) {fSbModelName = name; }
      // Set a common PDF for both the null and alternate
      virtual void SetCommonPdf(RooAbsPdf & pdf) { fSbModel = &pdf; }
      // Set the PDF for the null (only B)
      virtual void SetNullPdf(RooAbsPdf& pdf) { fBModel = &pdf; }
      // Set the PDF for the alternate hypothesis ( i.e. S+B)
      virtual void SetAlternatePdf(RooAbsPdf& pdf) { fSbModel = &pdf;  }

      // specify the name of the dataset in the workspace to be used
      virtual void SetData(const char* name) { fDataName = name; } 
      // Set the DataSet, add to the the workspace if not already there
      virtual void SetData(RooAbsData& data) { fData = &data; }

      // set parameter values for the null if using a common PDF
      virtual void SetNullParameters(RooArgSet& params) { fParameters = &params; }
      // set parameter values for the alternate if using a common PDF
      virtual void SetAlternateParameters(RooArgSet&) {}  // not needed

      // set number of toy MC 
      void SetNumberOfToys(unsigned int ntoys) { fNToys = ntoys; }

      // set a  prior pdf for the nuisance parameters 
      void SetNuisancePriorPdf(RooAbsPdf & prior_pdf) { fPriorPdf = &prior_pdf; } 
      // set name of a  prior pdf for the nuisance parameters in the previously given workspace
      void SetNuisancePriorPdf(const char * name) { fPriorPdfName = name; } 
      // control use of the prior pdf for the nuisance parameter
      void UseNuisancePriorPdf(bool on = true) { fUsePriorPdf = on; }
      
      void SetTestStatistics(int index);
      HybridResult* Calculate(TH1& data, unsigned int nToys, bool usePriors) const;
      HybridResult* Calculate(RooTreeData& data, unsigned int nToys, bool usePriors) const;
      HybridResult* Calculate(unsigned int nToys, bool usePriors) const;
      void PrintMore(const char* options) const;


   private:
      void RunToys(std::vector<double>& bVals, std::vector<double>& sbVals, unsigned int nToys, bool usePriors) const;

      // check input parameters before performing the calculation
      bool DoCheckInputs() const; 
      // initialize all the data and pdf by using a workspace as input 
      bool DoInitializeFromWS();  

      

      unsigned int fTestStatisticsIdx; // Index of the test statistics to use
      unsigned int fNToys;            // number of Toys MC
      bool  fUsePriorPdf;               // use a prior for nuisance parameters  

      RooAbsPdf* fSbModel; // The pdf of the signal+background model
      RooAbsPdf* fBModel; // The pdf of the background model
      mutable RooArgList* fObservables; // Collection of the observables of the model
      RooArgSet* fParameters; // Collection of the nuisance parameters in the model
      RooAbsPdf* fPriorPdf; // Prior PDF of the nuisance parameters
      RooAbsData * fData;     // pointer to the data sets 
      //bool fOwnsWorkspace;    // flag indicating if calculator manages the workspace 
      RooWorkspace * fWS;     // a workspace that owns all the components to be used by the calculator
      TString fSbModelName;   // name of pdf of the signal+background model
      TString fBModelName;   // name of pdf of the background model
      TString fPriorPdfName;   // name of pdf of the background model
      TString fDataName;      // name of the dataset in the workspace

   protected:
      ClassDef(HybridCalculator,1)  // Hypothesis test calculator using a Bayesian-frequentist hybrid method
   };
}

#endif
