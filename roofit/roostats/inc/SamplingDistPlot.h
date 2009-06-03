// @(#)root/roostats:$Id: SamplingDistPlot.h 26427 2009-05-20 15:45:36Z pellicci $

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

#ifndef ROOSTATS_SamplingDistPlot
#define ROOSTATS_SamplingDistPlot

#include "RooList.h"
#include "RooPrintable.h"
#include "TNamed.h"
#include "TIterator.h"
#include "TH1F.h"

#include "RooStats/SamplingDistribution.h"

namespace RooStats {

 class SamplingDistPlot : public TNamed, public RooPrintable {

   public:
    SamplingDistPlot();

    // Constructors for SamplingDistribution
    SamplingDistPlot(const Int_t nbins);
    SamplingDistPlot(const char* name, const char* title, Int_t nbins, Double_t xmin, Double_t xmax);

    // Destructor of SamplingDistribution
    virtual ~SamplingDistPlot();

    void AddSamplingDistribution(const SamplingDistribution *samplingDist, Option_t *drawOptions);

    void Draw(Option_t *options);

    void SetLineColor(const Color_t color, const SamplingDistribution *samplDist = 0);
    void SetLineWidth(const Width_t lwidth, const SamplingDistribution *samplDist = 0);
    void SetLineStyle(const Style_t style, const SamplingDistribution *samplDist = 0);

    void SetMarkerColor(const Color_t color, const SamplingDistribution *samplDist = 0);
    void SetMarkerStyle(const Style_t style, const SamplingDistribution *samplDist = 0);
    void SetMarkerSize(const Size_t size, const SamplingDistribution *samplDist = 0);

    void SetAxisTitle(char *varName) {_fVarName = TString(varName);}

  private:

    std::vector<Double_t> fSamplingDistr;
    std::vector<Double_t> fSampleWeights;

    Bool_t isWeighted;

    Int_t _fbins;
    Int_t _fMarkerType;
    Int_t _fColor;

    TString _fVarName;

  protected:

    TH1F* _hist;

    RooList _items;
    TIterator* _iterator;

    void SetSampleWeights(const SamplingDistribution *samplingDist);

    void addObject(TObject *obj, Option_t *drawOptions);
    void GetAbsoluteInterval(Float_t &theMin, Float_t &theMax, Float_t &theYMax) const;

    ClassDef(SamplingDistPlot,1)  // Class containing the results of the HybridCalculator
  };
}

#endif
