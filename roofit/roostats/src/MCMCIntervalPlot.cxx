// @(#)root/roostats:$Id$
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

//_________________________________________________
/*
BEGIN_HTML
<p>
This class provides simple and straightforward utilities to plot a MCMCInterval
object.  Basic use only requires a few lines once you have an MCMCInterval*:
</p>
<p>
MCMCIntervalPlot plot(*interval);
plot.Draw();
</p>
<p>
The standard Draw() function will currently draw the confidence interval
range with bars if 1-D and a contour if 2-D.  The MCMC posterior will also be
plotted for the 1-D case.
</p>
END_HTML
*/
//_________________________________________________

#ifndef ROOSTATS_MCMCIntervalPlot
#include "RooStats/MCMCIntervalPlot.h"
#endif
#include <iostream>
#ifndef ROOT_TROOT
#include "TROOT.h"
#endif
#ifndef ROOT_TMath
#include "TMath.h"
#endif
#ifndef ROOT_TLine
#include "TLine.h"
#endif
#ifndef ROOT_TObjArray
#include "TObjArray.h"
#endif
#ifndef ROOT_TList
#include "TList.h"
#endif
#ifndef ROOT_TGraph
#include "TGraph.h"
#endif
#ifndef ROOT_TPad
#include "TPad.h"
#endif
#ifndef ROO_REAL_VAR
#include "RooRealVar.h"
#endif
#ifndef ROO_PLOT
#include "RooPlot.h"
#endif
#ifndef ROOT_TH2
#include "TH2.h"
#endif
#ifndef ROO_ARG_LIST
#include "RooArgList.h"
#endif
#ifndef ROOT_TAxis
#include "TAxis.h"
#endif
#ifndef ROO_GLOBAL_FUNC
#include "RooGlobalFunc.h"
#endif

// Extra draw commands
static const char* POSTERIOR_HIST = "posterior_hist";
static const char* POSTERIOR_KEYS_PDF = "posterior_keys_pdf";
static const char* POSTERIOR_KEYS_PRODUCT = "posterior_keys_product";
static const char* HIST_INTERVAL = "hist_interval";
static const char* KEYS_PDF_INTERVAL = "keys_pdf_interval";
static const char* TAIL_FRACTION_INTERVAL = "tail_fraction_interval";
//static const char* OPTION_SEP = ":";

ClassImp(RooStats::MCMCIntervalPlot);

using namespace std;
using namespace RooStats;

MCMCIntervalPlot::MCMCIntervalPlot()
{
   fInterval = NULL;
   fParameters = NULL;
   fPosteriorHist = NULL;
   fPosteriorKeysPdf = NULL;
   fPosteriorKeysProduct = NULL;
   fDimension = 0;
   fLineColor = kBlack;
   fLineWidth = 1;
   //fContourColor = kBlack;
   fShowBurnIn = kTRUE;
   fWalk = NULL;
   fBurnIn = NULL;
   fFirst = NULL;
   fParamGraph = NULL;
   fNLLGraph = NULL;
   fNLLHist = NULL;
   fWeightHist = NULL;
}

MCMCIntervalPlot::MCMCIntervalPlot(MCMCInterval& interval)
{
   SetMCMCInterval(interval);
   fPosteriorHist = NULL;
   fPosteriorKeysPdf = NULL;
   fPosteriorKeysProduct = NULL;
   fLineColor = kBlack;
   fLineWidth = 1;
   //fContourColor = kBlack;
   fShowBurnIn = kTRUE;
   fWalk = NULL;
   fBurnIn = NULL;
   fFirst = NULL;
   fParamGraph = NULL;
   fNLLGraph = NULL;
   fNLLHist = NULL;
   fWeightHist = NULL;
}

MCMCIntervalPlot::~MCMCIntervalPlot()
{
   delete fParameters;
   // kbelasco: why does deleting fPosteriorHist remove the graphics
   // but deleting TGraphs doesn't?
   //delete fPosteriorHist;
   // can we delete fNLLHist and fWeightHist?
   //delete fNLLHist;
   //delete fWeightHist;

   // kbelasco: should we delete fPosteriorKeysPdf and fPosteriorKeysProduct?
   delete fPosteriorKeysPdf;
   delete fPosteriorKeysProduct;

   delete fWalk;
   delete fBurnIn;
   delete fFirst;
   delete fParamGraph;
   delete fNLLGraph;
}

void MCMCIntervalPlot::SetMCMCInterval(MCMCInterval& interval)
{
   fInterval = &interval;
   fDimension = fInterval->GetDimension();
   fParameters = fInterval->GetParameters();
}

void MCMCIntervalPlot::Draw(const Option_t* options)
{
   TString tmpOpt(options);

   if (tmpOpt.Contains(POSTERIOR_HIST))
      DrawPosteriorHist(options);
   else if (tmpOpt.Contains(POSTERIOR_KEYS_PDF))
      DrawPosteriorKeysPdf(options);
   else if (tmpOpt.Contains(POSTERIOR_KEYS_PRODUCT))
      DrawPosteriorKeysProduct(options);
   else if (tmpOpt.Contains(HIST_INTERVAL))
      DrawHistInterval(options);
   else if (tmpOpt.Contains(KEYS_PDF_INTERVAL))
      DrawKeysPdfInterval(options);
   else if (tmpOpt.Contains(TAIL_FRACTION_INTERVAL))
      DrawTailFractionInterval(options);
   else 
      DrawInterval(options);
}

void MCMCIntervalPlot::DrawPosterior(const Option_t* options)
{
   if (fInterval->GetUseKeys())
      DrawPosteriorKeysPdf(options);
   else
      DrawPosteriorHist(options);
}

void MCMCIntervalPlot::DrawPosteriorHist(const Option_t* options,
      const char* title, Bool_t scale)
{
   if (fPosteriorHist == NULL)
      fPosteriorHist = fInterval->GetPosteriorHist();

   if (fPosteriorHist == NULL) {
      coutE(InputArguments) << "MCMCIntervalPlot::DrawPosteriorHist: "
         << "Couldn't get posterior histogram." << endl;
      return;
   }

   // kbelasco: annoying hack because histogram drawing fails when it sees
   // an unrecognized option like POSTERIOR_HIST, etc.
   const Option_t* myOpt = NULL;

   TString tmpOpt(options);
   if (tmpOpt.Contains("same"))
      myOpt = "same";

   // scale so highest bin has height 1
   if (scale)
      fPosteriorHist->Scale(1/fPosteriorHist->GetBinContent(fPosteriorHist->GetMaximumBin()));
   if (title == NULL)
      fPosteriorHist->SetTitle("MCMC Posterior Histogram");
   else
      fPosteriorHist->SetTitle(title);
   fPosteriorHist->Draw(myOpt);
}

void MCMCIntervalPlot::DrawPosteriorKeysPdf(const Option_t* options)
{
   if (fPosteriorKeysPdf == NULL)
      fPosteriorKeysPdf = fInterval->GetPosteriorKeysPdf();

   if (fPosteriorKeysPdf == NULL) {
      coutE(InputArguments) << "MCMCIntervalPlot::DrawPosteriorKeysPdf: "
         << "Couldn't get posterior Keys PDF." << endl;
      return;
   }

   if (fDimension == 1) {
      RooRealVar* v = (RooRealVar*)fParameters->first();
      RooPlot* frame = v->frame();
      frame->SetTitle(Form("Posterior Keys PDF for %s", v->GetName()));
      //fPosteriorKeysPdf->plotOn(frame);
      fPosteriorKeysPdf->plotOn(frame,
            RooFit::Normalization(1, RooAbsReal::Raw));
      frame->Draw(options);
   } else if (fDimension == 2) {
      RooArgList* axes = fInterval->GetAxes();
      RooRealVar* xVar = (RooRealVar*)axes->at(0);
      RooRealVar* yVar = (RooRealVar*)axes->at(1);
      TH2F* keysHist = (TH2F*)fPosteriorKeysPdf->createHistogram(
            "keysPlot2D", *xVar, RooFit::YVar(*yVar), RooFit::Scaling(kFALSE));
      keysHist->SetTitle(Form("MCMC histogram of posterior Keys PDF for %s, %s",
               axes->at(0)->GetName(), axes->at(1)->GetName()));

      keysHist->Draw(options);
      delete axes;
   }
}

void MCMCIntervalPlot::DrawInterval(const Option_t* options)
{
   switch (fInterval->GetIntervalType()) {
      case MCMCInterval::kShortest:
         DrawShortestInterval(options);
         break;
      case MCMCInterval::kTailFraction:
         DrawTailFractionInterval(options);
         break;
      default:
         coutE(InputArguments) << "MCMCIntervalPlot::DrawInterval(): " <<
            "Interval type not supported" << endl;
         break;
   }
}

void MCMCIntervalPlot::DrawShortestInterval(const Option_t* options)
{
   if (fInterval->GetUseKeys())
      DrawKeysPdfInterval(options);
   else
      DrawHistInterval(options);
}

void MCMCIntervalPlot::DrawKeysPdfInterval(const Option_t* options)
{
   if (fDimension == 1) {
      // Draw the posterior keys PDF as well so the user can see where the
      // limit bars line up
      DrawPosteriorKeysPdf(options);

      //Double_t height = 1;
      //Double_t height = 2.0 * fInterval->GetKeysPdfCutoff();
      Double_t height = fInterval->GetKeysMax();

      RooRealVar* p = (RooRealVar*)fParameters->first();
      Double_t ul = fInterval->UpperLimitByKeys(*p);
      Double_t ll = fInterval->LowerLimitByKeys(*p);
      TLine* llLine = new TLine(ll, 0, ll, height);
      TLine* ulLine = new TLine(ul, 0, ul, height);
      llLine->SetLineColor(fLineColor);
      ulLine->SetLineColor(fLineColor);
      llLine->SetLineWidth(fLineWidth);
      ulLine->SetLineWidth(fLineWidth);
      llLine->Draw(options);
      ulLine->Draw(options);
   } else if (fDimension == 2) {
      if (fPosteriorKeysPdf == NULL)
         fPosteriorKeysPdf = fInterval->GetPosteriorKeysPdf();

      if (fPosteriorKeysPdf == NULL) {
         coutE(InputArguments) << "MCMCIntervalPlot::DrawKeysPdfInterval: "
            << "Couldn't get posterior Keys PDF." << endl;
         return;
      }

      RooArgList* axes = fInterval->GetAxes();
      RooRealVar* xVar = (RooRealVar*)axes->at(0);
      RooRealVar* yVar = (RooRealVar*)axes->at(1);
      TH2F* contHist = (TH2F*)fPosteriorKeysPdf->createHistogram(
          "keysContour2D", *xVar, RooFit::YVar(*yVar), RooFit::Scaling(kFALSE));
      contHist->SetTitle(Form("MCMC Keys conf. interval for %s, %s",
               axes->at(0)->GetName(), axes->at(1)->GetName()));

      contHist->SetStats(kFALSE);

      TString tmpOpt(options);
      if (!tmpOpt.Contains("CONT2")) tmpOpt.Append("CONT2");

      Double_t cutoff = fInterval->GetKeysPdfCutoff();
      contHist->SetContour(1, &cutoff);
      contHist->SetLineColor(fLineColor);
      contHist->SetLineWidth(fLineWidth);
      contHist->Draw(tmpOpt.Data());
      delete axes;
   } else {
      coutE(InputArguments) << "MCMCIntervalPlot::DrawKeysPdfInterval: "
         << " Sorry: " << fDimension << "-D plots not currently supported" << endl;
   }
}

void MCMCIntervalPlot::DrawHistInterval(const Option_t* options)
{
   if (fDimension == 1) {
      // Draw the posterior histogram as well so the user can see where the
      // limit bars line up
      DrawPosteriorHist(options);

      // draw lower and upper limits
      RooRealVar* p = (RooRealVar*)fParameters->first();
      Double_t ul = fInterval->UpperLimitByHist(*p);
      Double_t ll = fInterval->LowerLimitByHist(*p);
      TLine* llLine = new TLine(ll, 0, ll, 1);
      TLine* ulLine = new TLine(ul, 0, ul, 1);
      llLine->SetLineColor(fLineColor);
      ulLine->SetLineColor(fLineColor);
      llLine->SetLineWidth(fLineWidth);
      ulLine->SetLineWidth(fLineWidth);
      llLine->Draw(options);
      ulLine->Draw(options);
   } else if (fDimension == 2) {
      if (fPosteriorHist == NULL)
         fPosteriorHist = fInterval->GetPosteriorHist();

      if (fPosteriorHist == NULL) {
         coutE(InputArguments) << "MCMCIntervalPlot::DrawHistInterval: "
            << "Couldn't get posterior histogram." << endl;
         return;
      }

      RooArgList* axes = fInterval->GetAxes();
      fPosteriorHist->SetTitle(Form("MCMC histogram conf. interval for %s, %s",
               axes->at(0)->GetName(), axes->at(1)->GetName()));
      delete axes;

      fPosteriorHist->SetStats(kFALSE);

      TString tmpOpt(options);
      if (!tmpOpt.Contains("CONT2")) tmpOpt.Append("CONT2");

      Double_t cutoff = fInterval->GetHistCutoff();
      fPosteriorHist->SetContour(1, &cutoff);
      fPosteriorHist->SetLineColor(fLineColor);
      fPosteriorHist->SetLineWidth(fLineWidth);
      fPosteriorHist->Draw(tmpOpt.Data());
   } else {
      coutE(InputArguments) << "MCMCIntervalPlot::DrawHistInterval: "
         << " Sorry: " << fDimension << "-D plots not currently supported" << endl;
   }
}

void MCMCIntervalPlot::DrawTailFractionInterval(const Option_t* options)
{
   if (fDimension == 1) {
      // Draw the posterior histogram as well so the user can see where the
      // limit bars line up
      DrawPosteriorHist(options,
            "MCMC Tail-Fraction Interval w/ Posterior Histogram");

      // draw lower and upper limits
      RooRealVar* p = (RooRealVar*)fParameters->first();
      Double_t ul = fInterval->UpperLimitTailFraction(*p);
      Double_t ll = fInterval->LowerLimitTailFraction(*p);
      TLine* llLine = new TLine(ll, 0, ll, 1);
      TLine* ulLine = new TLine(ul, 0, ul, 1);
      llLine->SetLineColor(fLineColor);
      ulLine->SetLineColor(fLineColor);
      llLine->SetLineWidth(fLineWidth);
      ulLine->SetLineWidth(fLineWidth);
      llLine->Draw(options);
      ulLine->Draw(options);
   } else {
      coutE(InputArguments) << "MCMCIntervalPlot::DrawTailFractionInterval: "
         << " Sorry: " << fDimension << "-D plots not currently supported"
         << endl;
   }
}

void MCMCIntervalPlot::DrawPosteriorKeysProduct(const Option_t* options)
{
   if (fPosteriorKeysProduct == NULL)
      fPosteriorKeysProduct = fInterval->GetPosteriorKeysProduct();

   if (fPosteriorKeysProduct == NULL) {
      coutE(InputArguments) << "MCMCIntervalPlot::DrawPosteriorKeysProduct: "
         << "Couldn't get posterior Keys product." << endl;
      return;
   }

   RooArgList* axes = fInterval->GetAxes();

   if (fDimension == 1) {
      RooPlot* frame = ((RooRealVar*)fParameters->first())->frame();
      fPosteriorKeysProduct->plotOn(frame);
      frame->SetTitle(Form("Posterior Keys PDF * Heaviside product for %s",
               axes->at(0)->GetName()));
      frame->Draw(options);
   } else if (fDimension == 2) {
      RooRealVar* xVar = (RooRealVar*)axes->at(0);
      RooRealVar* yVar = (RooRealVar*)axes->at(1);
      TH2F* productHist = (TH2F*)fPosteriorKeysProduct->createHistogram(
            "prodPlot2D", *xVar, RooFit::YVar(*yVar), RooFit::Scaling(kFALSE));
      productHist->SetTitle(Form("MCMC Posterior Keys Product Hist. for %s, %s",
               axes->at(0)->GetName(), axes->at(1)->GetName()));
      productHist->Draw(options);
   }
   delete axes;
}

void MCMCIntervalPlot::DrawChainScatter(RooRealVar& xVar, RooRealVar& yVar)
{
   const MarkovChain* markovChain = fInterval->GetChain();

   Int_t size = markovChain->Size();
   Int_t burnInSteps;
   if (fShowBurnIn)
      burnInSteps = fInterval->GetNumBurnInSteps();
   else
      burnInSteps = 0;

   Double_t* x = new Double_t[size - burnInSteps];
   Double_t* y = new Double_t[size - burnInSteps];
   Double_t* burnInX = NULL;
   Double_t* burnInY = NULL;
   if (burnInSteps > 0) {
      burnInX = new Double_t[burnInSteps];
      burnInY = new Double_t[burnInSteps];
   }
   Double_t firstX;
   Double_t firstY;

   for (Int_t i = burnInSteps; i < size; i++) {
      x[i - burnInSteps] = markovChain->Get(i)->getRealValue(xVar.GetName());
      y[i - burnInSteps] = markovChain->Get(i)->getRealValue(yVar.GetName());
   }

   for (Int_t i = 0; i < burnInSteps; i++) {
      burnInX[i] = markovChain->Get(i)->getRealValue(xVar.GetName());
      burnInY[i] = markovChain->Get(i)->getRealValue(yVar.GetName());
   }

   firstX = markovChain->Get(0)->getRealValue(xVar.GetName());
   firstY = markovChain->Get(0)->getRealValue(yVar.GetName());

   TGraph* walk = new TGraph(size - burnInSteps, x, y);
   walk->SetTitle(Form("2-D Scatter Plot of Markov chain for %s, %s",
            xVar.GetName(), yVar.GetName()));
   // kbelasco: figure out how to set TGraph variable ranges
   walk->GetXaxis()->Set(xVar.numBins(), xVar.getMin(), xVar.getMax());
   walk->GetXaxis()->SetTitle(xVar.GetName());
   walk->GetYaxis()->Set(yVar.numBins(), yVar.getMin(), yVar.getMax());
   walk->GetYaxis()->SetTitle(yVar.GetName());
   walk->SetLineColor(kGray);
   walk->SetMarkerStyle(6);
   walk->SetMarkerColor(kViolet);
   walk->Draw("A,L,P,same");

   TGraph* burnIn = NULL;
   if (burnInX != NULL && burnInY != NULL) {
      burnIn = new TGraph(burnInSteps - 1, burnInX, burnInY);
      burnIn->SetLineColor(kPink);
      burnIn->SetMarkerStyle(6);
      burnIn->SetMarkerColor(kPink);
      burnIn->Draw("L,P,same");
   }

   TGraph* first = new TGraph(1, &firstX, &firstY);
   first->SetLineColor(kGreen);
   first->SetMarkerStyle(3);
   first->SetMarkerSize(2);
   first->SetMarkerColor(kGreen);
   first->Draw("L,P,same");

   //walkCanvas->Update();
   //delete x;
   //delete y;
   //delete burnInX;
   //delete burnInY;
   //delete walk;
   //delete burnIn;
   //delete first;
}

void MCMCIntervalPlot::DrawParameterVsTime(RooRealVar& param)
{
   const MarkovChain* markovChain = fInterval->GetChain();
   Int_t size = markovChain->Size();
   Int_t numEntries = 2 * size;
   Double_t* value = new Double_t[numEntries];
   Double_t* time = new Double_t[numEntries];
   Double_t val;
   Int_t weight;
   Int_t t = 0;
   for (Int_t i = 0; i < size; i++) {
      val = markovChain->Get(i)->getRealValue(param.GetName());
      weight = (Int_t)markovChain->Weight();
      value[2*i] = val;
      value[2*i + 1] = val;
      time[2*i] = t;
      t += weight;
      time[2*i + 1] = t;
   }

   TGraph* paramGraph = new TGraph(numEntries, time, value);
   paramGraph->SetTitle(Form("%s vs. time in Markov chain", param.GetName()));
   paramGraph->GetXaxis()->SetTitle("Time (in discrete steps of Markov chain)");
   paramGraph->GetYaxis()->SetTitle(param.GetName());
   //paramGraph->SetLineColor(fLineColor);
   paramGraph->Draw("A,L,same");
   //gPad->Update();
}

void MCMCIntervalPlot::DrawNLLVsTime()
{
   const MarkovChain* markovChain = fInterval->GetChain();
   Int_t size = markovChain->Size();
   Int_t numEntries = 2 * size;
   Double_t* nllValue = new Double_t[numEntries];
   Double_t* time = new Double_t[numEntries];
   Double_t nll;
   Int_t weight;
   Int_t t = 0;
   for (Int_t i = 0; i < size; i++) {
      nll = markovChain->NLL(i);
      weight = (Int_t)markovChain->Weight();
      nllValue[2*i] = nll;
      nllValue[2*i + 1] = nll;
      time[2*i] = t;
      t += weight;
      time[2*i + 1] = t;
   }

   TGraph* nllGraph = new TGraph(numEntries, time, nllValue);
   nllGraph->SetTitle("NLL value vs. time in Markov chain");
   nllGraph->GetXaxis()->SetTitle("Time (in discrete steps of Markov chain)");
   nllGraph->GetYaxis()->SetTitle("NLL (-log(likelihood))");
   //nllGraph->SetLineColor(fLineColor);
   nllGraph->Draw("A,L,same");
   //gPad->Update();
}

void MCMCIntervalPlot::DrawNLLHist(const Option_t* options)
{
   if (fNLLHist == NULL) {
      const MarkovChain* markovChain = fInterval->GetChain();
      // find the max NLL value
      Double_t maxNLL = 0;
      Int_t size = markovChain->Size();
      for (Int_t i = 0; i < size; i++)
         if (markovChain->NLL(i) > maxNLL)
            maxNLL = markovChain->NLL(i);
      RooRealVar* nllVar = fInterval->GetNLLVar();
      fNLLHist = new TH1F("mcmc_nll_hist", "MCMC NLL Histogram", nllVar->getBins(),
            0, maxNLL);
      for (Int_t i = 0; i < size; i++)
         fNLLHist->Fill(markovChain->NLL(i), markovChain->Weight());
   }
   fNLLHist->Draw(options);
}

void MCMCIntervalPlot::DrawWeightHist(const Option_t* options)
{
   if (fWeightHist == NULL) {
      const MarkovChain* markovChain = fInterval->GetChain();
      // find the max weight value
      Double_t maxWeight = 0;
      Int_t size = markovChain->Size();
      for (Int_t i = 0; i < size; i++)
         if (markovChain->Weight(i) > maxWeight)
            maxWeight = markovChain->Weight(i);
      fWeightHist = new TH1F("mcmc_weight_hist", "MCMC Weight Histogram",
            (Int_t)(maxWeight + 1), 0, maxWeight * 1.02);
      for (Int_t i = 0; i < size; i++)
         fWeightHist->Fill(markovChain->Weight(i));
   }
   fWeightHist->Draw(options);
}

/*
/////////////////////////////////////////////////////////////////////
  // 3-d plot of the parameter points
  dataCanvas->cd(2);
  // also plot the points in the markov chain
  RooDataSet* markovChainData = ((MCMCInterval*)mcmcint)->GetChainAsDataSet();

  TTree& chain =  ((RooTreeDataStore*) markovChainData->store())->tree();
  chain.SetMarkerStyle(6);
  chain.SetMarkerColor(kRed);
  chain.Draw("s:ratioSigEff:ratioBkgEff","","box"); // 3-d box proporional to posterior

  // the points used in the profile construction
  TTree& parameterScan =  ((RooTreeDataStore*) fc.GetPointsToScan()->store())->tree();
  parameterScan.SetMarkerStyle(24);
  parameterScan.Draw("s:ratioSigEff:ratioBkgEff","","same");

  chain.SetMarkerStyle(6);
  chain.SetMarkerColor(kRed);
  //chain.Draw("s:ratioSigEff:ratioBkgEff", "_MarkovChain_local_nll","box");
  //chain.Draw("_MarkovChain_local_nll");
////////////////////////////////////////////////////////////////////
*/
