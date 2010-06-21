// @(#)root/roostats:$Id$
// Authors: Sven Kreiss    June 2010
// Authors: Kyle Cranmer, Lorenzo Moneta, Gregory Schott, Wouter Verkerke
/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOSTATS_SamplingDistPlot
#define ROOSTATS_SamplingDistPlot

#include "RooList.h"
#include "RooPrintable.h"
#include "TNamed.h"
#include "TIterator.h"
#include "TH1F.h"
#include "TLegend.h"

#include "RooStats/SamplingDistribution.h"

namespace RooStats {

 class SamplingDistPlot : public TNamed, public RooPrintable {

   public:
    // Constructors for SamplingDistribution
    SamplingDistPlot(const Int_t nbins = 100);
    SamplingDistPlot(const char* name, const char* title, Int_t nbins, Double_t xmin, Double_t xmax);

    // Destructor of SamplingDistribution
    virtual ~SamplingDistPlot();

    // adds the sampling distribution and returns the scale factor
    Double_t AddSamplingDistribution(const SamplingDistribution *samplingDist, Option_t *drawOptions=0);
    // Like AddSamplingDistribution, but also sets a shaded area in the
    // minShaded and maxShaded boundaries.
    Double_t AddSamplingDistributionShaded(const SamplingDistribution *samplingDist, Double_t minShaded, Double_t maxShaded, Option_t *drawOptions);

    // add a line
    void AddLine(Double_t x1, Double_t y1, Double_t x2, Double_t y2, const char* title = NULL);

    void Draw(const Option_t *options=0);

    void SetLineColor(const Color_t color, const SamplingDistribution *samplDist = 0);
    void SetLineWidth(const Width_t lwidth, const SamplingDistribution *samplDist = 0);
    void SetLineStyle(const Style_t style, const SamplingDistribution *samplDist = 0);

    void SetMarkerColor(const Color_t color, const SamplingDistribution *samplDist = 0);
    void SetMarkerStyle(const Style_t style, const SamplingDistribution *samplDist = 0);
    void SetMarkerSize(const Size_t size, const SamplingDistribution *samplDist = 0);

    void RebinDistribution(const Int_t rebinFactor, const SamplingDistribution *samplDist = 0);

    void SetAxisTitle(char *varName) { fVarName = TString(varName); }

    // If you do not want SamplingDistPlot to interfere with your style settings, call this
    // function with "false" before Draw().
    void SetApplyStyle(const Bool_t s) { fApplyStyle = s; }

  private:
    std::vector<Double_t> fSamplingDistr;
    std::vector<Double_t> fSampleWeights;

    Bool_t fIsWeighted;

    Int_t fBins;
    Int_t fMarkerType;
    Int_t fColor;

    TString fVarName;

  protected:

    TH1F* fHist;
    TLegend *fLegend;

    RooList fItems; // holds TH1Fs only
    RooList fOtherItems; // other objects to be drawn like TLine etc.
    TIterator* fIterator; // TODO remove class variable and instanciate locally as necessary

    Bool_t fApplyStyle;
    Style_t fFillStyle;

    void SetSampleWeights(const SamplingDistribution *samplingDist);

    void addObject(TObject *obj, Option_t *drawOptions=0); // for TH1Fs only
    void addOtherObject(TObject *obj, Option_t *drawOptions=0);
    void GetAbsoluteInterval(Float_t &theMin, Float_t &theMax, Float_t &theYMax) const;

    ClassDef(SamplingDistPlot,1)  // Class providing utilities to plot SamplingDistribution objects
  };
}

#endif
