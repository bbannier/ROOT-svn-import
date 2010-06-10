// @(#)root/roostats:$Id$
// Author: Kyle Cranmer, Lorenzo Moneta, Gregory Schott, Wouter Verkerke
/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOSTATS_BayesianCalculator
#define ROOSTATS_BayesianCalculator

#include "TNamed.h"

#include "Math/IFunctionfwd.h"

#ifndef ROO_ARG_SET
#include "RooArgSet.h"
#endif

#ifndef ROOSTATS_IntervalCalculator
#include "RooStats/IntervalCalculator.h"
#endif

#ifndef ROOSTATS_SimpleInterval
#include "RooStats/SimpleInterval.h"
#endif

class RooAbsData; 
class RooAbsPdf; 
class RooPlot; 
class RooAbsReal;

namespace RooStats {

   class ModelConfig; 
   class SimpleInterval; 

   class BayesianCalculator : public IntervalCalculator, public TNamed {

   public:

      // constructor
      BayesianCalculator( );

      BayesianCalculator( RooAbsData& data,
                          RooAbsPdf& pdf,
                          const RooArgSet& POI,
                          RooAbsPdf& priorPOI,
                          const RooArgSet* nuisanceParameters = 0 );

      BayesianCalculator( RooAbsData& data,
                          ModelConfig& model );

      // destructor
      virtual ~BayesianCalculator();

      // get the plot with option to get it normalized 
      RooPlot* GetPosteriorPlot(bool norm = false, double precision = 0.01) const; 

      // return posterior pdf (object is managed by the BayesianCalculator class)
      RooAbsPdf* GetPosteriorPdf() const; 
      // return posterior function (object is managed by the BayesianCalculator class)
      RooAbsReal* GetPosteriorFunction() const; 

      virtual SimpleInterval* GetInterval() const ; 

      virtual void SetData( RooAbsData & data ) {
         fData = &data;
         ClearAll();
      }

      virtual void SetModel( const ModelConfig& model ); 

      // set the size of the test (rate of Type I error) ( Eg. 0.05 for a 95% Confidence Interval)
      virtual void SetTestSize( Double_t size ) {
         fSize = size;
         fValidInterval = false; 
      }
      // set the confidence level for the interval (eg. 0.95 for a 95% Confidence Interval)
      virtual void SetConfidenceLevel( Double_t cl ) { SetTestSize(1.-cl); }
      // Get the size of the test (eg. rate of Type I error)
      virtual Double_t Size() const { return fSize; }
      // Get the Confidence level for the test
      virtual Double_t ConfidenceLevel() const { return 1.-fSize; }

      void SetBrfPrecision( double precision ) { fBrfPrecision = precision; }

      // set the integration type (possible type are) 
      // 1D: adaptive, gauss, nonadaptive
      // multidim: adaptive, vegas, miser, plain. These last 3 are based on MC integration
      void SetIntegrationType(const char * type); 

   protected:

      void ClearAll() const; 
   
   private:

      // compute the most probable value: move to public once implemented
      // returns a RooArgSet
      RooArgSet* GetMode( RooArgSet* parameters ) const;
      // plan to replace the above: return a SimpleInterval integrating 
      // over all other parameters except the one specified as argument
      //virtual SimpleInterval* GetInterval( RooRealVar* parameter  ) const { return 0; }
    
      RooAbsData* fData;
      RooAbsPdf* fPdf;
      RooArgSet fPOI;
      RooAbsPdf* fPriorPOI;
      RooArgSet fNuisanceParameters;

      mutable RooAbsPdf* fProductPdf; 
      mutable RooAbsReal* fLogLike; 
      mutable RooAbsReal* fLikelihood; 
      mutable RooAbsReal* fIntegratedLikelihood;  // integrated likelihood function, i.e - unnormalized posterior function  
      mutable RooAbsPdf* fPosteriorPdf;          // normalized posterior on the poi
      mutable ROOT::Math::IGenFunction * fPosteriorFunction;  // function representing the posterior
      mutable Double_t  fLower; 
      mutable Double_t  fUpper; 
      double fBrfPrecision;
      mutable Bool_t    fValidInterval;

      double fSize;  // size used for getting the interval

      TString fIntegrationType; 

   protected:

      ClassDef(BayesianCalculator,1)  // BayesianCalculator class

   };
}

#endif
