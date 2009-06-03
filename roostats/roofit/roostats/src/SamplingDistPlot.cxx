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

//____________________________________________________________________
/*
SamplingDistPlot : 

This class provides simple and straightforward utilities to plot SamplingDistribution
objects.
*/

#include "RooStats/SamplingDistPlot.h"

#include <algorithm>
#include <iostream>

/// ClassImp for building the THtml documentation of the class 
ClassImp(RooStats::SamplingDistPlot);

using namespace RooStats;

//_______________________________________________________
SamplingDistPlot::SamplingDistPlot() :
  _hist(0) , _items()
{
   // SamplingDistPlot default constructor
  _iterator= _items.MakeIterator();
  _fbins = 0;
  _fMarkerType = 20;
  _fColor = 1;
}

//_______________________________________________________
SamplingDistPlot::SamplingDistPlot(const Int_t nbins) :
  _hist(0) , _items()
{
   // SamplingDistPlot default constructor with bin size
  _iterator= _items.MakeIterator();
  _fbins = nbins;
  _fMarkerType = 20;
  _fColor = 1;
}


//_______________________________________________________
SamplingDistPlot::SamplingDistPlot(const char* name, const char* title, Int_t nbins, Double_t xmin, Double_t xmax) :
  _hist(0) , _items()
{
  // SamplingDistPlot constructor
  _hist = new TH1F(name, title, nbins, xmin, xmax);
  _fbins = nbins;
  _fMarkerType = 20;
  _fColor = 1;
}

//_______________________________________________________
SamplingDistPlot::~SamplingDistPlot()
{
   // SamplingDistPlot destructor

   fSamplingDistr.clear();
   fSampleWeights.clear();

   _items.Clear();
}

//_______________________________________________________
void SamplingDistPlot::AddSamplingDistribution(const SamplingDistribution *samplingDist, Option_t *drawOptions)
{
  fSamplingDistr = samplingDist->GetSamplingDistribution();
  SetSampleWeights(samplingDist);

  // add option "SAME" if necessary
  TString options(drawOptions);
  options.ToUpper();
  if(!options.Contains("SAME")) options.Append("SAME");

  const Double_t xlow = *(std::min_element(fSamplingDistr.begin(),fSamplingDistr.end()));
  const Double_t xup  = *(std::max_element(fSamplingDistr.begin(),fSamplingDistr.end()));

  _hist = new TH1F(samplingDist->GetName(),samplingDist->GetTitle(),_fbins,xlow,xup);

  _hist->GetXaxis()->SetTitle(samplingDist->GetVarName().Data());

  std::vector<Double_t>::iterator valuesIt = fSamplingDistr.begin();

  for(int w_idx = 0; valuesIt != fSamplingDistr.end(); ++valuesIt, ++w_idx)
    {
      if(isWeighted) _hist->Fill(*valuesIt,fSampleWeights[w_idx]);
      else _hist->Fill(*valuesIt);
    }

  //some basic aesthetics
  _hist->SetMarkerStyle(_fMarkerType);
  _fMarkerType++;
  _hist->SetMarkerColor(_fColor);
  _hist->SetLineColor(_fColor);
  _fColor++;

  addObject(_hist,options.Data());

  return;
}

//_______________________________________________________
void SamplingDistPlot::SetSampleWeights(const SamplingDistribution* samplingDist)
{
  //Determine if the sampling distribution has weights and store them

  isWeighted = kFALSE;

  if(samplingDist->GetSampleWeights().size() != 0){
    isWeighted = kTRUE;
    fSampleWeights = samplingDist->GetSampleWeights();
  }  

  return;
}

void SamplingDistPlot::addObject(TObject *obj, Option_t *drawOptions) 
{
  // Add a generic object to this plot. The specified options will be
  // used to Draw() this object later. The caller transfers ownership
  // of the object with this call, and the object will be deleted
  // when its containing plot object is destroyed.

  if(0 == obj) {
    std::cerr << fName << "::addObject: called with a null pointer" << std::endl;
    return;
  }

  _items.Add(obj,drawOptions);

  return;
}

//_____________________________________________________________________________
void SamplingDistPlot::Draw(Option_t *options) 
{
  // Draw this plot and all of the elements it contains. The specified options
  // only apply to the drawing of our frame. The options specified in our add...()
  // methods will be used to draw each object we contain.

  Float_t theMin(0.), theMax(0.), theYMax(0.);

  GetAbsoluteInterval(theMin,theMax,theYMax);

  TH1F *drawHist = new TH1F("name","",_fbins,theMin,theMax);
  drawHist->SetMaximum(theYMax);

  if(!_fVarName.IsNull()) drawHist->GetXaxis()->SetTitle(_fVarName.Data());

  drawHist->Draw(options);
  //_hist->Draw(options);
  _iterator->Reset();
  TObject *obj = 0;
  while((obj= _iterator->Next()))
      obj->Draw(_iterator->GetOption());

  if(!_fVarName.IsNull()) _hist->GetXaxis()->SetTitle(_fVarName.Data());
  _hist->Draw("AXISSAME");

  return;
}

//_____________________________________________________________________________
void SamplingDistPlot::GetAbsoluteInterval(Float_t &theMin, Float_t &theMax, Float_t &theYMax) const
{
  Float_t tmpmin = 999.;
  Float_t tmpmax = -999.;
  Float_t tmpYmax = -999.;


  _iterator->Reset();
  TH1F *obj = 0;
  while((obj = (TH1F*)_iterator->Next())) {
    if(obj->GetXaxis()->GetXmin() < tmpmin) tmpmin = obj->GetXaxis()->GetXmin();
    if(obj->GetXaxis()->GetXmax() > tmpmax) tmpmax = obj->GetXaxis()->GetXmax();
    if(obj->GetMaximum() > tmpYmax) tmpYmax = obj->GetMaximum() + 0.1*obj->GetMaximum();
  }

  theMin = tmpmin;
  theMax = tmpmax;
  theYMax = tmpYmax;

  return;
}

//_____________________________________________________________________________
void SamplingDistPlot::SetLineColor(const Color_t color, const SamplingDistribution *samplDist)
{
  if(samplDist == 0){
    _hist->SetLineColor(color);
  }
  else{
    _iterator->Reset();
    TH1F *obj = 0;
    while((obj = (TH1F*)_iterator->Next())) {
      if(!strcmp(obj->GetName(),samplDist->GetName())){
	obj->SetLineColor(color);
	break;
      }
    }
  }

  return;
}

//_____________________________________________________________________________
void SamplingDistPlot::SetLineWidth(const Width_t lwidth, const SamplingDistribution *samplDist)
{
  if(samplDist == 0){
    _hist->SetLineWidth(lwidth);
  }
  else{
    _iterator->Reset();
    TH1F *obj = 0;
    while((obj = (TH1F*)_iterator->Next())) {
      if(!strcmp(obj->GetName(),samplDist->GetName())){
	obj->SetLineWidth(lwidth);
	break;
      }
    }
  }

  return;
}

//_____________________________________________________________________________
void SamplingDistPlot::SetLineStyle(const Style_t style, const SamplingDistribution *samplDist)
{
  if(samplDist == 0){
    _hist->SetLineStyle(style);
  }
  else{
    _iterator->Reset();
    TH1F *obj = 0;
    while((obj = (TH1F*)_iterator->Next())) {
      if(!strcmp(obj->GetName(),samplDist->GetName())){
	obj->SetLineStyle(style);
	break;
      }
    }
  }

  return;
}

//_____________________________________________________________________________
void SamplingDistPlot::SetMarkerStyle(const Style_t style, const SamplingDistribution *samplDist)
{
  if(samplDist == 0){
    _hist->SetMarkerStyle(style);
  }
  else{
    _iterator->Reset();
    TH1F *obj = 0;
    while((obj = (TH1F*)_iterator->Next())) {
      if(!strcmp(obj->GetName(),samplDist->GetName())){
	obj->SetMarkerStyle(style);
	break;
      }
    }
  }

  return;
}

//_____________________________________________________________________________
void SamplingDistPlot::SetMarkerColor(const Color_t color, const SamplingDistribution *samplDist)
{
  if(samplDist == 0){
    _hist->SetMarkerColor(color);
  }
  else{
    _iterator->Reset();
    TH1F *obj = 0;
    while((obj = (TH1F*)_iterator->Next())) {
      if(!strcmp(obj->GetName(),samplDist->GetName())){
	obj->SetMarkerColor(color);
	break;
      }
    }
  }

  return;
}

//_____________________________________________________________________________
void SamplingDistPlot::SetMarkerSize(const Size_t size, const SamplingDistribution *samplDist)
{
  if(samplDist == 0){
    _hist->SetMarkerSize(size);
  }
  else{
    _iterator->Reset();
    TH1F *obj = 0;
    while((obj = (TH1F*)_iterator->Next())) {
      if(!strcmp(obj->GetName(),samplDist->GetName())){
	obj->SetMarkerSize(size);
	break;
      }
    }
  }

  return;
}
