// @(#)root/roostats:$Id$
// Authors: Kevin Belasco        17/06/2009
// Authors: Kyle Cranmer         17/06/2009
/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOSTATS_MCMCCalculator
#define ROOSTATS_MCMCCalculator

#ifndef ROOT_Rtypes
#include "Rtypes.h"
#endif

#ifndef ROOT_TObject
#include "TObject.h"
#endif
#ifndef ROO_ABS_PDF
#include "RooAbsPdf.h"
#endif
#ifndef ROO_ABS_DATA
#include "RooAbsData.h"
#endif
#ifndef ROO_ARG_SET
#include "RooArgSet.h"
#endif
#ifndef ROO_ARG_LIST
#include "RooArgList.h"
#endif
#ifndef ROOSTATS_ProposalFunction
#include "RooStats/ProposalFunction.h"
#endif
#ifndef ROOSTATS_IntervalCalculator
#include "RooStats/IntervalCalculator.h"
#endif
#ifndef RooStats_MCMCInterval
#include "RooStats/MCMCInterval.h"
#endif

namespace RooStats {

   class ModelConfig;

   class MCMCCalculator : public IntervalCalculator, public TNamed {

   public:
      // default constructor
      MCMCCalculator();

      // Constructor for automatic configuration with basic settings and a
      // ModelConfig.  Uses a UniformProposal, 10,000 iterations, 40 burn in
      // steps, 50 bins for each RooRealVar, determines interval by histogram,
      // and finds a 95% confidence interval.  Any of these basic settings can
      // be overridden by calling one of the Set...() methods.
      MCMCCalculator(RooAbsData& data, const ModelConfig& model);

      virtual ~MCMCCalculator() {}

      // Main interface to get a ConfInterval
      virtual MCMCInterval* GetInterval() const;

      // Get the size of the test (eg. rate of Type I error)
      virtual Double_t Size() const {return fSize;}
      // Get the Confidence level for the test
      virtual Double_t ConfidenceLevel() const {return 1.-fSize;}

      virtual void SetModel(const ModelConfig & model); 

      // Set the DataSet if not already there
      virtual void SetData(RooAbsData& data) { fData = &data; }

      // Set the Pdf if not already there
      virtual void SetPdf(RooAbsPdf& pdf) { fPdf = &pdf; }

      // Set the Prior Pdf if not already there
      virtual void SetPriorPdf(RooAbsPdf& pdf) { fPriorPdf = &pdf; }

      // specify the parameters of interest in the interval
      virtual void SetParameters(const RooArgSet& set) { fPOI.removeAll(); fPOI.add(set); }

      // specify the nuisance parameters (eg. the rest of the parameters)
      virtual void SetNuisanceParameters(const RooArgSet& set) {fNuisParams.removeAll(); fNuisParams.add(set);}

      // set the size of the test (rate of Type I error) ( Eg. 0.05 for a 95% Confidence Interval)
      virtual void SetTestSize(Double_t size) {fSize = size;}

      // set the confidence level for the interval (eg. 0.95 for a 95% Confidence Interval)
      virtual void SetConfidenceLevel(Double_t cl) {fSize = 1.-cl;}

      // set the proposal function for suggesting new points for the MCMC
      virtual void SetProposalFunction(ProposalFunction& proposalFunction)
      { fPropFunc = &proposalFunction; }

      // set the number of iterations to run the metropolis algorithm
      virtual void SetNumIters(Int_t numIters)
      { fNumIters = numIters; }

      // set the number of steps in the chain to discard as burn-in,
      // starting from the first
      virtual void SetNumBurnInSteps(Int_t numBurnInSteps)
      { fNumBurnInSteps = numBurnInSteps; }

      // set the number of bins to create for each axis when constructing the interval
      virtual void SetNumBins(Int_t numBins) { fNumBins = numBins; }
      // set which variables to put on each axis
      virtual void SetAxes(RooArgList& axes)
      { fAxes = &axes; }
      // set whether to use kernel estimation to determine the interval
      virtual void SetUseKeys(Bool_t useKeys) { fUseKeys = useKeys; }
      // set whether to use sparse histogram (if using histogram at all)
      virtual void SetUseSparseHist(Bool_t useSparseHist)
      { fUseSparseHist = useSparseHist; }

      // set what type of interval to have the MCMCInterval represent
      virtual void SetIntervalType(enum MCMCInterval::IntervalType intervalType)
      { fIntervalType = intervalType; }

      // Set the left side tail fraction. This will automatically configure the
      // MCMCInterval to find a tail-fraction interval.
      // Note: that `a' must be in the range 0 <= a <= 1
      // or the user will be notified of the error
      virtual void SetLeftSideTailFraction(Double_t a);

      // set the acceptable level or error for Keys interval determination
      virtual void SetEpsilon(Double_t epsilon)
      {
         if (epsilon < 0)
            coutE(InputArguments) << "MCMCInterval::SetEpsilon will not allow "
                                  << "negative epsilon value" << endl;
         else
            fEpsilon = epsilon;
      }

      // kbelasco: The inner-workings of the class really should not be exposed
      // like this in a comment, but it seems to be the only way to give
      // the user any control over this process, if he desires it
      //
      // Set the fraction delta such that
      // topCutoff (a) is considered == bottomCutoff (b) iff
      // (TMath::Abs(a - b) < TMath::Abs(fDelta * (a + b)/2))
      // when determining the confidence interval by Keys
      virtual void SetDelta(Double_t delta)
      {
         if (delta < 0.)
            coutE(InputArguments) << "MCMCInterval::SetDelta will not allow "
                                  << "negative delta value" << endl;
         else
            fDelta = delta;
      }

   protected:

      Double_t fSize;   // size of the test (eg. specified rate of Type I error)
      RooArgSet   fPOI;        // parameters of interest for interval
      RooArgSet   fNuisParams; // nuisance parameters for interval (not really used)
      mutable ProposalFunction* fPropFunc; // Proposal function for MCMC integration
      RooAbsPdf * fPdf;        // pointer to common PDF (owned by the workspace)
      RooAbsPdf * fPriorPdf;   // pointer to prior  PDF (owned by the workspace)
      RooAbsData * fData;     // pointer to the data (owned by the workspace)
      Int_t fNumIters; // number of iterations to run metropolis algorithm
      Int_t fNumBurnInSteps; // number of iterations to discard as burn-in, starting from the first
      Int_t fNumBins; // set the number of bins to create for each
                      // axis when constructing the interval
      RooArgList * fAxes; // which variables to put on each axis
      Bool_t fUseKeys; // whether to use kernel estimation to determine interval
      Bool_t fUseSparseHist; // whether to use sparse histogram (if using hist at all)
      Double_t fLeftSideTF; // left side tail-fraction for interval
      Double_t fEpsilon; // acceptable error for Keys interval determination

      Double_t fDelta; // acceptable error for Keys cutoffs being equal
                       // topCutoff (a) considered == bottomCutoff (b) iff
                       // (TMath::Abs(a - b) < TMath::Abs(fDelta * (a + b)/2));
                       // Theoretically, the Abs is not needed here, but
                       // floating-point arithmetic does not always work
                       // perfectly, and the Abs doesn't hurt
      enum MCMCInterval::IntervalType fIntervalType; // type of interval to find

      void SetupBasicUsage();
      void SetBins(const RooAbsCollection& coll, Int_t numBins) const
      {
         TIterator* it = coll.createIterator();
         RooAbsArg* r;
         while ((r = (RooAbsArg*)it->Next()) != NULL)
            if (dynamic_cast<RooRealVar*>(r))
               ((RooRealVar*)r)->setBins(numBins);
         delete it;
      }

      ClassDef(MCMCCalculator,1) // Markov Chain Monte Carlo calculator for Bayesian credible intervals
   };
}


#endif
