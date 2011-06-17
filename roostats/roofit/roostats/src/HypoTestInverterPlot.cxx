// @(#)root/roostats:$Id$
// Author: Kyle Cranmer, Lorenzo Moneta, Gregory Schott, Wouter Verkerke
/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

/**
   HypoTestInverterPlot class
**/

#include <cmath>

// include other header files
#include "RooStats/HybridResult.h"

// include header file of this class 
#include "RooStats/HypoTestInverterPlot.h"
#include "RooStats/HypoTestInverterResult.h"

#include "TGraphErrors.h"
#include "TGraphAsymmErrors.h"
#include "TMultiGraph.h"
#include "TDirectory.h"
#include "TLine.h"
#include "TAxis.h"
#include "Math/DistFuncMathCore.h"

ClassImp(RooStats::HypoTestInverterPlot)

using namespace RooStats;


HypoTestInverterPlot::HypoTestInverterPlot(HypoTestInverterResult* results ) :
   TNamed( results->GetName(), results->GetTitle() ),
   fResults(results)
{
   // constructor from a HypoTestInverterResult class 
   // name and title are taken from the result class 
}


HypoTestInverterPlot::HypoTestInverterPlot( const char* name,
					    const char* title,
					    HypoTestInverterResult* results ) :
   TNamed( TString(name), TString(title) ),
   fResults(results)
{
  // constructor with name and title from a HypoTestInverterResult class 
}


TGraphErrors* HypoTestInverterPlot::MakePlot()
{
   // Make the plot of the result of the scan 
   // using the observed data
   
   const int nEntries = fResults->ArraySize();

   // sort the arrays based on the x values
   std::vector<unsigned int> index(nEntries);
   TMath::SortItr(fResults->fXValues.begin(), fResults->fXValues.end(), index.begin(), false);

   // copy result in sorted arrays
   std::vector<Double_t> xArray(nEntries);
   std::vector<Double_t> yArray(nEntries);
   std::vector<Double_t> yErrArray(nEntries);
   for (int i=0; i<nEntries; i++) {
      xArray[i] = fResults->GetXValue(index[i]);
      yArray[i] = fResults->GetYValue(index[i]);
      yErrArray[i] = fResults->GetYError(index[i]);
   }

   TGraphErrors* graph = new TGraphErrors(nEntries,&xArray.front(),&yArray.front(),0,&yErrArray.front());
   TString name = GetName() + TString("_observed");
   TString title = TString("Observed ") + GetTitle(); 
   graph->SetName(name);
   graph->SetTitle(title);
   graph->SetMarkerStyle(kFullDotMedium);
   graph->SetLineWidth(2);
   return graph;
}

TMultiGraph* HypoTestInverterPlot::MakeExpectedPlot(double nsig1, double nsig2 )
{
   // Make the expected plot and the bands 
   // nsig1 and nsig2 indicates the n-sigma value for the bands
   // if nsig1 = 0 no band is drawn (only expected value)
   // if nsig2 > nsig1 (default is nsig1=1 and nsig2=2) the second band is also drawn
   // The first band is drawn in green while the second in yellow 
   // THe return result is a TMultiGraph object



   const int nEntries = fResults->ArraySize();
   bool doFirstBand = (nsig1 > 0);
   bool doSecondBand = (nsig2 > nsig1);

   nsig1 = std::abs(nsig1);
   nsig2 = std::abs(nsig2);

   // sort the arrays based on the x values
   std::vector<unsigned int> index(nEntries);
   TMath::SortItr(fResults->fXValues.begin(), fResults->fXValues.end(), index.begin(), false);

   // create the graphs 
   TGraph * g0 = new TGraph(nEntries);
   TGraphAsymmErrors * g1 = 0;
   TGraphAsymmErrors * g2l = 0; 
   TGraphAsymmErrors * g2u = 0; 
   if (doFirstBand) 
      g1 = new TGraphAsymmErrors(nEntries);
   if (doSecondBand) { 
      g2l = new TGraphAsymmErrors(nEntries);
      g2u = new TGraphAsymmErrors(nEntries);
   }
   double p[7]; 
   double q[7];
   p[0] = ROOT::Math::normal_cdf(-nsig2);
   p[1] = ROOT::Math::normal_cdf(-0.5*(nsig1+nsig2) );
   p[2] = ROOT::Math::normal_cdf(-nsig1);
   p[3] = 0.5;
   p[4] = ROOT::Math::normal_cdf(nsig1);
   p[5] = ROOT::Math::normal_cdf(0.5*(nsig1+nsig2));
   p[6] = ROOT::Math::normal_cdf(nsig2);
   for (int j=0; j<nEntries; ++j) {
      int i = index[j]; // i is the order index 
      SamplingDistribution * s = fResults->GetExpectedDistribution(i);
      const std::vector<double> & values = s->GetSamplingDistribution();
      double * x = const_cast<double *>(&values[0]); // need to change TMath::Quantiles
      TMath::Quantiles(values.size(), 7, x,q,p,false);
      g0->SetPoint(j, fResults->GetXValue(i),  q[3]);
      if (g1) { 
         g1->SetPoint(j, fResults->GetXValue(i),  q[3]);
         g1->SetPointEYlow(j, q[3] - q[2]); // -1 sigma errorr   
         g1->SetPointEYhigh(j, q[4] - q[3]);//+1 sigma error
      }
      if (g2l && g2u) {
         g2l->SetPoint(j, fResults->GetXValue(i), q[1]);
         g2u->SetPoint(j, fResults->GetXValue(i), q[5]);

         g2l->SetPointEYlow(j, q[1]-q[0]);   // -2 -- -1 sigma error
         g2l->SetPointEYhigh(j, q[2]-q[1]);

         g2u->SetPointEYlow(j, q[5]-q[4]);
         g2u->SetPointEYhigh(j, q[6]-q[5]);
      }
   }



   TString name = GetName() + TString("_expected");
   TString title = TString("Expected ") + GetTitle(); 
   TMultiGraph* graph = new TMultiGraph(name,title);
  
   // set the graphics options and add in multi graph
   if (g1) { 
      g1->SetFillColor(kGreen);
      graph->Add(g1,"3");
   }
   if (g2l && g2u) { 
      g2l->SetFillColor(kYellow);
      g2u->SetFillColor(kYellow);
      graph->Add(g2l,"3");
      graph->Add(g2u,"3");
   }
   g0->SetLineStyle(2);
   g0->SetLineWidth(2);
   graph->Add(g0,"L");

   return graph;
}

HypoTestInverterPlot::~HypoTestInverterPlot()
{
   // destructor
}

void HypoTestInverterPlot::Draw(Option_t * opt) { 
   // Draw the result in the current canvas 
   // Possible options: 
   //   SAME : draw in the current axis 
   //   OBS  :  draw only the observed plot 
   //   EXP  :  draw only the expected plot 
   // 
   // default draw observed + expected with 1 and 2 sigma bands 

   TString option(opt);
   option.ToUpper();
   bool drawAxis = !option.Contains("SAME");
   bool drawObs = option.Contains("OBS") || !option.Contains("EXP");
   bool drawExp = option.Contains("EXP") || !option.Contains("OBS");      
   
   TGraphErrors * gobs = 0;
   TGraph * gplot = 0;
   if (drawObs) { 
      std::cout << "Draw observed plot ..." << std::endl;
      gobs = MakePlot(); 
      // add object to current directory to avoid mem leak
      if (gDirectory) gDirectory->Add(gobs); 
      if (drawAxis) { 
         gobs->Draw("APL");
         gplot = gobs;
      }
      else gobs->Draw("PL");
   }
   if (drawExp) { 
      TMultiGraph * gexp = MakeExpectedPlot(); 
      // add object to current directory to avoid mem leak
      if (gDirectory) gDirectory->Add(gexp); 
      if (drawAxis && !drawObs) gexp->Draw("A");
      else gexp->Draw();
   }

   // draw also an horizontal  line at the desired conf level
   if (gplot) {
      double alpha = 1.-fResults->ConfidenceLevel();
      double x1 = gplot->GetXaxis()->GetXmin();
      double x2 = gplot->GetXaxis()->GetXmax();
      TLine * line = new TLine(x1, alpha, x2,alpha);
      line->SetLineColor(kRed);
      line->Draw();
   }

   // draw again observed values otherwise will be covered byb the bands
   if (gobs) gobs->Draw("PL"); 

}
