// @(#)root/roostats:$Id$
// Authors: Sven Kreiss          23/05/2012
// Authors: Kevin Belasco        17/06/2009
// Authors: Kyle Cranmer         17/06/2009
/*************************************************************************
 * Project: RooStats                                                     *
 * Package: RooFit/RooStats                                              *
 *************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOSTATS_MCMCIntervalPlot
#define ROOSTATS_MCMCIntervalPlot

#ifndef ROO_PRINTABLE
#include "RooPrintable.h"
#endif
#ifndef ROO_ARG_SET
#include "RooArgSet.h"
#endif
#ifndef ROOT_TNamed
#include "TNamed.h"
#endif
#ifndef ROOT_TH1
#include "TH1.h"
#endif
#ifndef ROOT_TH2
#include "TH2.h"
#endif
#ifndef RooStats_MCMCInterval
#include "RooStats/MCMCInterval.h"
#endif
#ifndef ROO_KEYS_PDF
#include "RooNDKeysPdf.h"
#endif
#ifndef ROO_PRODUCT
#include "RooProduct.h"
#endif

namespace RooStats {

   class MCMCIntervalPlot : public TNamed, public RooPrintable {

   public:
      MCMCIntervalPlot();
      MCMCIntervalPlot(MCMCInterval& interval);

      // Destructor of SamplingDistribution
      virtual ~MCMCIntervalPlot();

      void SetMCMCInterval(MCMCInterval& interval);
      void SetLineColor(Color_t color) {fLineColor = color;}
      void SetLineWidth(Int_t width) {fLineWidth = width;}
      void SetShadeColor(Color_t color) {fShadeColor = color;}
      void SetShowBurnIn(Bool_t showBurnIn) { fShowBurnIn = showBurnIn; }

      void Draw(const Option_t* options = NULL);

      void DrawChainScatter(RooRealVar& xVar, RooRealVar& yVar);
      void DrawParameterVsTime(RooRealVar& param);
      void DrawNLLVsTime();
      void DrawNLLHist(const Option_t* options = NULL);
      void DrawWeightHist(const Option_t* options = NULL);

      TH1* GetMaxNLLHist1D(RooRealVar& xVar);
      TH1* GetMaxLikelihoodHist1D(RooRealVar& xVar);
      TH2* GetMaxNLLHist2D(RooRealVar& xVar, RooRealVar& yVar);
      TH2* GetMaxLikelihoodHist2D(RooRealVar& xVar, RooRealVar& yVar);


      TH1* GetHist1D(RooRealVar& var);
      TH1* GetHist1DSlice(RooRealVar& var, RooRealVar& sliceVar, double sliceMin, double sliceMax);
      TH1* GetHist1DSliceNormalized(RooRealVar& var, RooRealVar& sliceVar, double sliceMin, double sliceMax);
      TH2* GetHist2D(RooRealVar& xVar, RooRealVar& yVar, int xbins=100, int ybins=100);

   private:

      MCMCInterval *fInterval;
      RooArgSet *fParameters;
      TH1* fPosteriorHist;
      RooNDKeysPdf* fPosteriorKeysPdf;
      RooProduct* fPosteriorKeysProduct;
      TH1* fNLLHist;
      TH1* fWeightHist;
      TH1* fPosteriorHistHistCopy;
      TH1* fPosteriorHistTFCopy;
      Int_t fDimension;
      Color_t fLineColor;
      Color_t fShadeColor;
      Int_t fLineWidth;
      Bool_t fShowBurnIn;
      TGraph* fWalk;
      TGraph* fBurnIn;
      TGraph* fFirst;
      TGraph* fParamGraph;
      TGraph* fNLLGraph;

   protected:
      void DrawPosterior(const Option_t* options = NULL);
      void* DrawPosteriorHist(const Option_t* options = NULL,
            const char* title = NULL, Bool_t scale = kTRUE);
      void* DrawPosteriorKeysPdf(const Option_t* options = NULL);
      void* DrawPosteriorKeysProduct(const Option_t* options = NULL);

      void DrawInterval(const Option_t* options = NULL);
      void DrawShortestInterval(const Option_t* options = NULL);
      void DrawHistInterval(const Option_t* options = NULL);
      void DrawKeysPdfInterval(const Option_t* options = NULL);
      void DrawTailFractionInterval(const Option_t* options = NULL);

      ClassDef(MCMCIntervalPlot,1)  // Class containing the results of the MCMCCalculator
   };
}

#endif
