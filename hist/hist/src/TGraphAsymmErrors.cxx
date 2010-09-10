// @(#)root/hist:$Id$
// Author: Rene Brun   03/03/99

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include <string.h>

#include "Riostream.h"
#include "TEfficiency.h"
#include "TROOT.h"
#include "TGraphAsymmErrors.h"
#include "TStyle.h"
#include "TMath.h"
#include "TArrow.h"
#include "TBox.h"
#include "TVirtualPad.h"
#include "TF1.h"
#include "TH1.h"
#include "TVector.h"
#include "TVectorD.h"
#include "TClass.h"

ClassImp(TGraphAsymmErrors)

//______________________________________________________________________________
/* Begin_Html
<center><h2>TGraphAsymmErrors class</h2></center>
A TGraphAsymmErrors is a TGraph with assymetric error bars.
<p>
The TGraphAsymmErrors painting is permofed thanks to the
<a href="http://root.cern.ch/root/html/TGraphPainter.html">TGraphPainter</a>
class. All details about the various painting options are given in
<a href="http://root.cern.ch/root/html/TGraphPainter.html">this class</a>.
<p>
The picture below gives an example:
End_Html
Begin_Macro(source)
{
   c1 = new TCanvas("c1","A Simple Graph with assymetric error bars",200,10,700,500);
   c1->SetFillColor(42);
   c1->SetGrid();
   c1->GetFrame()->SetFillColor(21);
   c1->GetFrame()->SetBorderSize(12);
   Int_t n = 10;
   Double_t x[n]   = {-0.22, 0.05, 0.25, 0.35, 0.5, 0.61,0.7,0.85,0.89,0.95};
   Double_t y[n]   = {1,2.9,5.6,7.4,9,9.6,8.7,6.3,4.5,1};
   Double_t exl[n] = {.05,.1,.07,.07,.04,.05,.06,.07,.08,.05};
   Double_t eyl[n] = {.8,.7,.6,.5,.4,.4,.5,.6,.7,.8};
   Double_t exh[n] = {.02,.08,.05,.05,.03,.03,.04,.05,.06,.03};
   Double_t eyh[n] = {.6,.5,.4,.3,.2,.2,.3,.4,.5,.6};
   gr = new TGraphAsymmErrors(n,x,y,exl,exh,eyl,eyh);
   gr->SetTitle("TGraphAsymmErrors Example");
   gr->SetMarkerColor(4);
   gr->SetMarkerStyle(21);
   gr->Draw("ALP");
   return c1;
}
End_Macro */


//______________________________________________________________________________
TGraphAsymmErrors::TGraphAsymmErrors(): TGraph()
{
   // TGraphAsymmErrors default constructor.

   fEXlow       = 0;
   fEYlow       = 0;
   fEXhigh      = 0;
   fEYhigh      = 0;
}


//______________________________________________________________________________
TGraphAsymmErrors::TGraphAsymmErrors(const TGraphAsymmErrors &gr)
       : TGraph(gr)
{
   // TGraphAsymmErrors copy constructor

   if (!CtorAllocate()) return;
   Int_t n = fNpoints*sizeof(Double_t);
   memcpy(fEXlow, gr.fEXlow, n);
   memcpy(fEYlow, gr.fEYlow, n);
   memcpy(fEXhigh, gr.fEXhigh, n);
   memcpy(fEYhigh, gr.fEYhigh, n);
}


//______________________________________________________________________________
TGraphAsymmErrors& TGraphAsymmErrors::operator=(const TGraphAsymmErrors &gr)
{
   // TGraphAsymmErrors assignment operator

   if(this!=&gr) {
      TGraph::operator=(gr);
      if (!CtorAllocate()) return *this;
      Int_t n = fNpoints*sizeof(Double_t);
      memcpy(fEXlow, gr.fEXlow, n);
      memcpy(fEYlow, gr.fEYlow, n);
      memcpy(fEXhigh, gr.fEXhigh, n);
      memcpy(fEYhigh, gr.fEYhigh, n);
   }
   return *this;
}


//______________________________________________________________________________
TGraphAsymmErrors::TGraphAsymmErrors(Int_t n)
       : TGraph(n)
{
   // TGraphAsymmErrors normal constructor.
   //
   // the arrays are preset to zero

   if (!CtorAllocate()) return;
   FillZero(0, fNpoints);
}


//______________________________________________________________________________
TGraphAsymmErrors::TGraphAsymmErrors(Int_t n, const Float_t *x, const Float_t *y, const Float_t *exl, const Float_t *exh, const Float_t *eyl, const Float_t *eyh)
       : TGraph(n,x,y)
{
   // TGraphAsymmErrors normal constructor.
   //
   // if exl,h or eyl,h are null, the corresponding arrays are preset to zero

   if (!CtorAllocate()) return;

   for (Int_t i=0;i<n;i++) {
      if (exl) fEXlow[i]  = exl[i];
      else     fEXlow[i]  = 0;
      if (exh) fEXhigh[i] = exh[i];
      else     fEXhigh[i] = 0;
      if (eyl) fEYlow[i]  = eyl[i];
      else     fEYlow[i]  = 0;
      if (eyh) fEYhigh[i] = eyh[i];
      else     fEYhigh[i] = 0;
   }
}


//______________________________________________________________________________
TGraphAsymmErrors::TGraphAsymmErrors(Int_t n, const Double_t *x, const Double_t *y, const Double_t *exl, const Double_t *exh, const Double_t *eyl, const Double_t *eyh)
       : TGraph(n,x,y)
{
   // TGraphAsymmErrors normal constructor.
   //
   // if exl,h or eyl,h are null, the corresponding arrays are preset to zero

   if (!CtorAllocate()) return;

   n = fNpoints*sizeof(Double_t);
   if(exl) { memcpy(fEXlow, exl, n);
   } else { memset(fEXlow, 0, n); }
   if(exh) { memcpy(fEXhigh, exh, n);
   } else { memset(fEXhigh, 0, n); }
   if(eyl) { memcpy(fEYlow, eyl, n);
   } else { memset(fEYlow, 0, n); }
   if(eyh) { memcpy(fEYhigh, eyh, n);
   } else { memset(fEYhigh, 0, n); }
}


//______________________________________________________________________________
TGraphAsymmErrors::TGraphAsymmErrors(const TVectorF  &vx, const TVectorF  &vy, const TVectorF  &vexl, const TVectorF  &vexh, const TVectorF  &veyl, const TVectorF  &veyh)
                  :TGraph()
{
   // Constructor with six vectors of floats in input
   // A grapherrors is built with the X coordinates taken from vx and Y coord from vy
   // and the errors from vectors vexl/h and veyl/h.
   // The number of points in the graph is the minimum of number of points
   // in vx and vy.

   fNpoints = TMath::Min(vx.GetNrows(), vy.GetNrows());
   if (!TGraph::CtorAllocate()) return;
   if (!CtorAllocate()) return;
   Int_t ivxlow  = vx.GetLwb();
   Int_t ivylow  = vy.GetLwb();
   Int_t ivexllow = vexl.GetLwb();
   Int_t ivexhlow = vexh.GetLwb();
   Int_t iveyllow = veyl.GetLwb();
   Int_t iveyhlow = veyh.GetLwb();
      for (Int_t i=0;i<fNpoints;i++) {
      fX[i]      = vx(i+ivxlow);
      fY[i]      = vy(i+ivylow);
      fEXlow[i]  = vexl(i+ivexllow);
      fEYlow[i]  = veyl(i+iveyllow);
      fEXhigh[i] = vexh(i+ivexhlow);
      fEYhigh[i] = veyh(i+iveyhlow);
   }
}


//______________________________________________________________________________
TGraphAsymmErrors::TGraphAsymmErrors(const TVectorD &vx, const TVectorD &vy, const TVectorD &vexl, const TVectorD &vexh, const TVectorD &veyl, const TVectorD &veyh)
                  :TGraph()
{
   // Constructor with six vectors of doubles in input
   // A grapherrors is built with the X coordinates taken from vx and Y coord from vy
   // and the errors from vectors vexl/h and veyl/h.
   // The number of points in the graph is the minimum of number of points
   // in vx and vy.

   fNpoints = TMath::Min(vx.GetNrows(), vy.GetNrows());
   if (!TGraph::CtorAllocate()) return;
   if (!CtorAllocate()) return;
   Int_t ivxlow  = vx.GetLwb();
   Int_t ivylow  = vy.GetLwb();
   Int_t ivexllow = vexl.GetLwb();
   Int_t ivexhlow = vexh.GetLwb();
   Int_t iveyllow = veyl.GetLwb();
   Int_t iveyhlow = veyh.GetLwb();
      for (Int_t i=0;i<fNpoints;i++) {
      fX[i]      = vx(i+ivxlow);
      fY[i]      = vy(i+ivylow);
      fEXlow[i]  = vexl(i+ivexllow);
      fEYlow[i]  = veyl(i+iveyllow);
      fEXhigh[i] = vexh(i+ivexhlow);
      fEYhigh[i] = veyh(i+iveyhlow);
   }
}


//______________________________________________________________________________
TGraphAsymmErrors::TGraphAsymmErrors(const TH1 *h)
       : TGraph(h)
{
   // TGraphAsymmErrors constructor importing its parameters from the TH1 object passed as argument
   // the low and high errors are set to the bin error of the histogram.

   if (!CtorAllocate()) return;

   for (Int_t i=0;i<fNpoints;i++) {
      fEXlow[i]  = h->GetBinWidth(i+1)*gStyle->GetErrorX();
      fEXhigh[i] = fEXlow[i];
      fEYlow[i]  = h->GetBinError(i+1);
      fEYhigh[i] = fEYlow[i];
   }
}


//______________________________________________________________________________
TGraphAsymmErrors::TGraphAsymmErrors(const TH1* pass, const TH1* total, Option_t *option)
   : TGraph((pass)?pass->GetNbinsX():0)
{
   // Creates a TGraphAsymmErrors by dividing two input TH1 histograms:
   // pass/total. (see TGraphAsymmErrors::Divide)

   if (!CtorAllocate()) return;

   std::string sname = "divide_" + std::string(pass->GetName()) + "_by_" +
      std::string(total->GetName());
   SetName(sname.c_str());
   SetTitle(pass->GetTitle());
   
   //copy style from pass
   pass->TAttLine::Copy(*this);
   pass->TAttFill::Copy(*this);
   pass->TAttMarker::Copy(*this);
   
   Divide(pass, total, option);
}


//______________________________________________________________________________
TGraphAsymmErrors::~TGraphAsymmErrors()
{
   // TGraphAsymmErrors default destructor.

   if(fEXlow) delete [] fEXlow;
   if(fEXhigh) delete [] fEXhigh;
   if(fEYlow) delete [] fEYlow;
   if(fEYhigh) delete [] fEYhigh;
}


//______________________________________________________________________________
void TGraphAsymmErrors::Apply(TF1 *f)
{
   // apply a function to all data points
   // y = f(x,y)
   //
   // Errors are calculated as eyh = f(x,y+eyh)-f(x,y) and
   // eyl = f(x,y)-f(x,y-eyl)
   //
   // Special treatment has to be applied for the functions where the
   // role of "up" and "down" is reversed.
   // function suggested/implemented by Miroslav Helbich <helbich@mail.desy.de>

   Double_t x,y,exl,exh,eyl,eyh,eyl_new,eyh_new,fxy;

   for (Int_t i=0;i<GetN();i++) {
      GetPoint(i,x,y);
      exl=GetErrorXlow(i);
      exh=GetErrorXhigh(i);
      eyl=GetErrorYlow(i);
      eyh=GetErrorYhigh(i);

      fxy = f->Eval(x,y);
      SetPoint(i,x,fxy);

      // in the case of the functions like y-> -1*y the roles of the
      // upper and lower error bars is reversed
      if (f->Eval(x,y-eyl)<f->Eval(x,y+eyh)) {
         eyl_new = TMath::Abs(fxy - f->Eval(x,y-eyl));
         eyh_new = TMath::Abs(f->Eval(x,y+eyh) - fxy);
      }
      else {
         eyh_new = TMath::Abs(fxy - f->Eval(x,y-eyl));
         eyl_new = TMath::Abs(f->Eval(x,y+eyh) - fxy);
      }

      //error on x doesn't change
      SetPointError(i,exl,exh,eyl_new,eyh_new);
   }
}

//______________________________________________________________________________
void TGraphAsymmErrors::BayesDivide(const TH1* pass, const TH1* total, Option_t *)
{
   //This function is only kept for backward compatibility.
   //You should rather use the Divide method.
   //It calls Divide(pass,total,"cl=0.683 b(1,1)") which is equivalent to the
   //former BayesDivide method.

   Divide(pass,total,"cl=0.683 b(1,1)");
}

//______________________________________________________________________________
void TGraphAsymmErrors::Divide(const TH1* pass, const TH1* total, Option_t *opt)
{
   // Fill this TGraphAsymmErrors by dividing two 1-dimensional histograms pass/total
   //
   // The assumption is that the entries in "pass" are a subset of those in
   // "total". That is, we create an "efficiency" graph, where each entry is
   // between 0 and 1, inclusive.
   //
   // If the histograms are not filled with unit weights, the number of effective
   // entries is used which might lead to wrong results.
   // Begin_Latex effective entries = #frac{(#sum w_{i})^{2}}{#sum w_{i}^{2}}End_Latex
   //
   // The points are assigned a x value at the center of each histogram bin.
   // The y values are Begin_Latex eff = #frac{pass}{total} End_Latex for all options except for the 
   // bayesian one where the estimated efficiency is given by
   // Begin_Latex eff = #frac{pass + a}{total + a + b} End_Latex.
   //
   // If the denominator becomes 0 or pass >  total, the corresponding bin is
   // skipped.
   //
   // The x errors span each histogram bin (lowedge ... lowedge+width)
   // The y errors depend on the chosen statistic methode which can be determined
   // by the options given below. For a detailed description of the used statistic
   // calculations please have a look at the corresponding functions! 
   //
   // Options:
   // - v     : verbose mode: prints information about the number of used bins
   //           and calculated efficiencies with their errors
   // - cl=x  : determine the used confidence level (0<x<1) (default is 0.683)
   // - cp    : Clopper-Pearson interval (see TEfficiency::ClopperPearson)
   // - w     : Wilson interval (see TEfficiency::Wilson)
   // - n     : normal approximation propagation (see TEfficiency::Normal)
   // - ac    : Agresti-Coull interval (see TEfficiency::AgrestiCoull)
   // - b(a,b): bayesian interval using a prior probability ~Beta(a,b); a,b > 0
   //           (see TEfficiency::Bayesian)
   //
   // Note:
   // Unfortunately there is no straightforward approach for determining a confidence
   // interval for a given confidence level. The actual coverage probability of the
   // confidence interval oscillates significantly according to the total number of
   // events and the true efficiency. In order to decrease the impact of this
   // oscillation on the actual coverage probability a couple of approximations and
   // methodes has been developped. For a detailed discussion, please have a look at
   // this statistical paper:
   // Begin_Html <a href="http://www-stat.wharton.upenn.edu/~tcai/paper/Binomial-StatSci.pdf"
   // > http://www-stat.wharton.upenn.edu/~tcai/paper/Binomial-StatSci.pdf</a> End_Html

   //check pointers
   if(!pass || !total) {
      Error("Divide","one of the passed pointers is zero");
      return;
   }
   
   //check dimension of histograms; only 1-dimensional ones are accepted
   if((pass->GetDimension() > 1) || (total->GetDimension() > 1)) {
      Error("Divide","passed histograms are not one-dimensional");
      return;
   }
   
   //check consistency of histograms, allowing weights
   if(!TEfficiency::CheckConsistency(*pass,*total,"w")) {
      Error("Divide","passed histograms are not consistent");
      return;
   }

   //check whether histograms are filled with weights -> use number of effective
   //entries
   Bool_t bEffective = false;
   //compare sum of weights with sum of squares of weights
   Double_t stats[10];
   pass->GetStats(stats);
   if (TMath::Abs(stats[0] -stats[1]) > 1e-6)
      bEffective = true;
   total->GetStats(stats);
   if (TMath::Abs(stats[0] -stats[1]) > 1e-6)
      bEffective = true;

   //parse option
   TString option = opt;
   option.ToLower();

   Bool_t bVerbose = false;
   //pointer to function returning the boundaries of the confidence interval
   //(is only used in the frequentist cases.)
   Double_t (*pBound)(Int_t,Int_t,Double_t,Bool_t) = 0;
   //confidence level
   Double_t conf = 0.683;
   //values for bayesian statistics
   Bool_t bIsBayesian = false;
   Double_t alpha = 1;
   Double_t beta = 1;

   //verbose mode
   if(option.Contains("v")) {
      option.ReplaceAll("v","");
      bVerbose = true;
   }

   //confidence level
   if(option.Contains("cl=")) {
      Double_t level = -1;
      sscanf(strstr(option.Data(),"cl="),"cl=%lf",&level);
      if((level > 0) && (level < 1))
	 conf = level;
      else
	 Warning("Divide","given confidence level %.3lf is invalid",level);
      option.ReplaceAll("cl=","");
   }

   //normal approximation
   if(option.Contains("n")) {
      option.ReplaceAll("n","");
      pBound = &TEfficiency::Normal;
   }

   //clopper pearson interval
   if(option.Contains("cp")) {
      option.ReplaceAll("cp","");
      pBound = &TEfficiency::ClopperPearson;
   }

   //wilson interval
   if(option.Contains("w")) {
      option.ReplaceAll("w","");
      pBound = &TEfficiency::Wilson;
   }

   //agresti coull interval
   if(option.Contains("ac")) {
      option.ReplaceAll("ac","");
      pBound = &TEfficiency::AgrestiCoull;
   }

   //bayesian with prior
   if(option.Contains("b(")) {
      Double_t a = 0;
      Double_t b = 0;
      sscanf(strstr(option.Data(),"b("),"b(%lf,%lf)",&a,&b);
      if(a > 0)
	 alpha = a;
      else
	 Warning("Divide","given shape parameter for alpha %.2lf is invalid",a);
      if(b > 0)
	 beta = b;
      else
	 Warning("Divide","given shape parameter for beta %.2lf is invalid",b);
      option.ReplaceAll("b(","");
      bIsBayesian = true;
   }
   
   //Set the graph to have a number of points equal to the number of histogram
   //bins
   Int_t nbins = pass->GetNbinsX();
   Set(nbins);

   // Ok, now set the points for each bin
   // (Note: the TH1 bin content is shifted to the right by one:
   //  bin=0 is underflow, bin=nbins+1 is overflow.)

   //efficiency with lower and upper boundary of confidence interval
   double eff, low, upper;
   //this keeps track of the number of points added to the graph
   int npoint=0;
   //number of total and passed events
   Int_t t,p;
   //loop over all bins and fill the graph
   for (Int_t b=1; b<=nbins; ++b) {

      //shall we use number of effective entries = (sum weights)^2 / sum (weights^2)
      //" + 0.5" ensures correct rounding
      if(bEffective) {
	 t =(Int_t)( total->GetBinContent(b) * total->GetBinContent(b) / total->GetSumw2()->At(b-1) + 0.5);
	 p =(Int_t)(pass->GetBinContent(b) * pass->GetBinContent(b) / pass->GetSumw2()->At(b-1) + 0.5);
	 if (p>t) {
	    Warning("Divide","histogram bin %d in pass has more effective entries than corresponding bin in total! (%d>%d)",b,p,t);
	    continue; //we may as well go on...
	 }
      }
      //use bin contents
      else {
	 t = total->GetBinContent(b);
	 p = pass->GetBinContent(b);
      }

      //using bayesian statistics
      if(bIsBayesian) {
	 if(t + alpha + beta)
	    eff = (p + alpha)/(t + alpha + beta);
	 else
	    continue;
      
	 low = TEfficiency::Bayesian(t,p,conf,alpha,beta,false);
	 upper = TEfficiency::Bayesian(t,p,conf,alpha,beta,true);
      }
      else {
	 if(t)
	    eff = ((Double_t)p)/t;
	 else
	    continue;
	 
	 low = pBound(t,p,conf,false);
	 upper = pBound(t,p,conf,true);
      }
      //Set the point center and its errors
      SetPoint(npoint,pass->GetBinCenter(b),eff);
      SetPointError(npoint,
      pass->GetBinCenter(b)-pass->GetBinLowEdge(b),
      pass->GetBinLowEdge(b)-pass->GetBinCenter(b)+pass->GetBinWidth(b),
      eff-low,upper-eff);
      npoint++;//we have added a point to the graph
   }

   Set(npoint);//tell the graph how many points we've really added

   if (bVerbose) {
      Info("Divide","made a graph with %d points from %d bins",npoint,nbins);
      Info("Divide","used confidence level: %.2lf\n",conf);
      if(bIsBayesian)
	 Info("Divide","used prior probability ~ beta(%.2lf,%.2lf)",alpha,beta);
      Print();
   }
}

//______________________________________________________________________________
void TGraphAsymmErrors::ComputeRange(Double_t &xmin, Double_t &ymin, Double_t &xmax, Double_t &ymax) const
{
   // Compute Range

   TGraph::ComputeRange(xmin,ymin,xmax,ymax);

   for (Int_t i=0;i<fNpoints;i++) {
      if (fX[i] -fEXlow[i] < xmin) {
         if (gPad && gPad->GetLogx()) {
            if (fEXlow[i] < fX[i]) xmin = fX[i]-fEXlow[i];
            else                   xmin = TMath::Min(xmin,fX[i]/3);
         } else {
            xmin = fX[i]-fEXlow[i];
         }
      }
      if (fX[i] +fEXhigh[i] > xmax) xmax = fX[i]+fEXhigh[i];
      if (fY[i] -fEYlow[i] < ymin) {
         if (gPad && gPad->GetLogy()) {
            if (fEYlow[i] < fY[i]) ymin = fY[i]-fEYlow[i];
            else                   ymin = TMath::Min(ymin,fY[i]/3);
         } else {
            ymin = fY[i]-fEYlow[i];
         }
      }
      if (fY[i] +fEYhigh[i] > ymax) ymax = fY[i]+fEYhigh[i];
   }
}


//______________________________________________________________________________
void TGraphAsymmErrors::CopyAndRelease(Double_t **newarrays,
                                       Int_t ibegin, Int_t iend, Int_t obegin)
{
   // Copy and release.

   CopyPoints(newarrays, ibegin, iend, obegin);
   if (newarrays) {
      delete[] fEXlow;
      fEXlow = newarrays[0];
      delete[] fEXhigh;
      fEXhigh = newarrays[1];
      delete[] fEYlow;
      fEYlow = newarrays[2];
      delete[] fEYhigh;
      fEYhigh = newarrays[3];
      delete[] fX;
      fX = newarrays[4];
      delete[] fY;
      fY = newarrays[5];
      delete[] newarrays;
   }
}


//______________________________________________________________________________
Bool_t TGraphAsymmErrors::CopyPoints(Double_t **arrays,
                                     Int_t ibegin, Int_t iend, Int_t obegin)
{
   // Copy errors from fE*** to arrays[***]
   // or to f*** Copy points.

   if (TGraph::CopyPoints(arrays ? arrays+4 : 0, ibegin, iend, obegin)) {
      Int_t n = (iend - ibegin)*sizeof(Double_t);
      if (arrays) {
         memmove(&arrays[0][obegin], &fEXlow[ibegin], n);
         memmove(&arrays[1][obegin], &fEXhigh[ibegin], n);
         memmove(&arrays[2][obegin], &fEYlow[ibegin], n);
         memmove(&arrays[3][obegin], &fEYhigh[ibegin], n);
      } else {
         memmove(&fEXlow[obegin], &fEXlow[ibegin], n);
         memmove(&fEXhigh[obegin], &fEXhigh[ibegin], n);
         memmove(&fEYlow[obegin], &fEYlow[ibegin], n);
         memmove(&fEYhigh[obegin], &fEYhigh[ibegin], n);
      }
      return kTRUE;
   } else {
      return kFALSE;
   }
}


//______________________________________________________________________________
Bool_t TGraphAsymmErrors::CtorAllocate(void)
{
   // Should be called from ctors after fNpoints has been set

   if (!fNpoints) {
      fEXlow = fEYlow = fEXhigh = fEYhigh = 0;
      return kFALSE;
   }
   fEXlow = new Double_t[fMaxSize];
   fEYlow = new Double_t[fMaxSize];
   fEXhigh = new Double_t[fMaxSize];
   fEYhigh = new Double_t[fMaxSize];
   return kTRUE;
}

//______________________________________________________________________________
void TGraphAsymmErrors::FillZero(Int_t begin, Int_t end,
                                 Bool_t from_ctor)
{
   // Set zero values for point arrays in the range [begin, end)

   if (!from_ctor) {
      TGraph::FillZero(begin, end, from_ctor);
   }
   Int_t n = (end - begin)*sizeof(Double_t);
   memset(fEXlow + begin, 0, n);
   memset(fEXhigh + begin, 0, n);
   memset(fEYlow + begin, 0, n);
   memset(fEYhigh + begin, 0, n);
}


//______________________________________________________________________________
Double_t TGraphAsymmErrors::GetErrorX(Int_t i) const
{
   // This function is called by GraphFitChisquare.
   // It returns the error along X at point i.

   if (i < 0 || i >= fNpoints) return -1;
   if (!fEXlow && !fEXhigh) return -1;
   Double_t elow=0, ehigh=0;
   if (fEXlow)  elow  = fEXlow[i];
   if (fEXhigh) ehigh = fEXhigh[i];
   return TMath::Sqrt(0.5*(elow*elow + ehigh*ehigh));
}


//______________________________________________________________________________
Double_t TGraphAsymmErrors::GetErrorY(Int_t i) const
{
   // This function is called by GraphFitChisquare.
   // It returns the error along Y at point i.

   if (i < 0 || i >= fNpoints) return -1;
   if (!fEYlow && !fEYhigh) return -1;
   Double_t elow=0, ehigh=0;
   if (fEYlow)  elow  = fEYlow[i];
   if (fEYhigh) ehigh = fEYhigh[i];
   return TMath::Sqrt(0.5*(elow*elow + ehigh*ehigh));
}


//______________________________________________________________________________
Double_t TGraphAsymmErrors::GetErrorXhigh(Int_t i) const
{
   // Get high error on X.

   if (i<0 || i>fNpoints) return -1;
   if (fEXhigh) return fEXhigh[i];
   return -1;
}


//______________________________________________________________________________
Double_t TGraphAsymmErrors::GetErrorXlow(Int_t i) const
{
   // Get low error on X.

   if (i<0 || i>fNpoints) return -1;
   if (fEXlow) return fEXlow[i];
   return -1;
}


//______________________________________________________________________________
Double_t TGraphAsymmErrors::GetErrorYhigh(Int_t i) const
{
   // Get high error on Y.

   if (i<0 || i>fNpoints) return -1;
   if (fEYhigh) return fEYhigh[i];
   return -1;
}


//______________________________________________________________________________
Double_t TGraphAsymmErrors::GetErrorYlow(Int_t i) const
{
   // Get low error on Y.

   if (i<0 || i>fNpoints) return -1;
   if (fEYlow) return fEYlow[i];
   return -1;
}


//______________________________________________________________________________
void TGraphAsymmErrors::Print(Option_t *) const
{
   // Print graph and errors values.

   for (Int_t i=0;i<fNpoints;i++) {
      printf("x[%d]=%g, y[%d]=%g, exl[%d]=%g, exh[%d]=%g, eyl[%d]=%g, eyh[%d]=%g\n"
         ,i,fX[i],i,fY[i],i,fEXlow[i],i,fEXhigh[i],i,fEYlow[i],i,fEYhigh[i]);
   }
}


//______________________________________________________________________________
void TGraphAsymmErrors::SavePrimitive(ostream &out, Option_t *option /*= ""*/)
{
    // Save primitive as a C++ statement(s) on output stream out

   char quote = '"';
   out<<"   "<<endl;
   if (gROOT->ClassSaved(TGraphAsymmErrors::Class())) {
      out<<"   ";
   } else {
      out<<"   TGraphAsymmErrors *";
   }
   out<<"grae = new TGraphAsymmErrors("<<fNpoints<<");"<<endl;
   out<<"   grae->SetName("<<quote<<GetName()<<quote<<");"<<endl;
   out<<"   grae->SetTitle("<<quote<<GetTitle()<<quote<<");"<<endl;

   SaveFillAttributes(out,"grae",0,1001);
   SaveLineAttributes(out,"grae",1,1,1);
   SaveMarkerAttributes(out,"grae",1,1,1);

   for (Int_t i=0;i<fNpoints;i++) {
      out<<"   grae->SetPoint("<<i<<","<<fX[i]<<","<<fY[i]<<");"<<endl;
      out<<"   grae->SetPointError("<<i<<","<<fEXlow[i]<<","<<fEXhigh[i]<<","<<fEYlow[i]<<","<<fEYhigh[i]<<");"<<endl;
   }

   static Int_t frameNumber = 0;
   if (fHistogram) {
      frameNumber++;
      TString hname = fHistogram->GetName();
      hname += frameNumber;
      fHistogram->SetName(hname.Data());
      fHistogram->SavePrimitive(out,"nodraw");
      out<<"   grae->SetHistogram("<<fHistogram->GetName()<<");"<<endl;
      out<<"   "<<endl;
   }

   // save list of functions
   TIter next(fFunctions);
   TObject *obj;
   while ((obj=next())) {
      obj->SavePrimitive(out,"nodraw");
      if (obj->InheritsFrom("TPaveStats")) {
         out<<"   grae->GetListOfFunctions()->Add(ptstats);"<<endl;
         out<<"   ptstats->SetParent(grae->GetListOfFunctions());"<<endl;
      } else {
         out<<"   grae->GetListOfFunctions()->Add("<<obj->GetName()<<");"<<endl;
      }
   }

   const char *l = strstr(option,"multigraph");
   if (l) {
      out<<"   multigraph->Add(grae,"<<quote<<l+10<<quote<<");"<<endl;
   } else {
      out<<"   grae->Draw("<<quote<<option<<quote<<");"<<endl;
   }
}

//______________________________________________________________________________
void TGraphAsymmErrors::SetPointError(Double_t exl, Double_t exh, Double_t eyl, Double_t eyh)
{
   // Set ex and ey values for point pointed by the mouse.

   Int_t px = gPad->GetEventX();
   Int_t py = gPad->GetEventY();

   //localize point to be deleted
   Int_t ipoint = -2;
   Int_t i;
   // start with a small window (in case the mouse is very close to one point)
   for (i=0;i<fNpoints;i++) {
      Int_t dpx = px - gPad->XtoAbsPixel(gPad->XtoPad(fX[i]));
      Int_t dpy = py - gPad->YtoAbsPixel(gPad->YtoPad(fY[i]));
      if (dpx*dpx+dpy*dpy < 25) {ipoint = i; break;}
   }
   if (ipoint == -2) return;

   fEXlow[ipoint]  = exl;
   fEYlow[ipoint]  = eyl;
   fEXhigh[ipoint] = exh;
   fEYhigh[ipoint] = eyh;
   gPad->Modified();
}


//______________________________________________________________________________
void TGraphAsymmErrors::SetPointError(Int_t i, Double_t exl, Double_t exh, Double_t eyl, Double_t eyh)
{
   // Set ex and ey values for point number i.

   if (i < 0) return;
   if (i >= fNpoints) {
   // re-allocate the object
      TGraphAsymmErrors::SetPoint(i,0,0);
   }
   fEXlow[i]  = exl;
   fEYlow[i]  = eyl;
   fEXhigh[i] = exh;
   fEYhigh[i] = eyh;
}


//______________________________________________________________________________
void TGraphAsymmErrors::SetPointEXlow(Int_t i, Double_t exl)
{
   // Set EXlow for point i

   if (i < 0) return;
   if (i >= fNpoints) {
   // re-allocate the object
      TGraphAsymmErrors::SetPoint(i,0,0);
   }
   fEXlow[i]  = exl;
}


//______________________________________________________________________________
void TGraphAsymmErrors::SetPointEXhigh(Int_t i, Double_t exh)
{
   // Set EXhigh for point i

   if (i < 0) return;
   if (i >= fNpoints) {
   // re-allocate the object
      TGraphAsymmErrors::SetPoint(i,0,0);
   }
   fEXhigh[i]  = exh;
}


//______________________________________________________________________________
void TGraphAsymmErrors::SetPointEYlow(Int_t i, Double_t eyl)
{
   // Set EYlow for point i

   if (i < 0) return;
   if (i >= fNpoints) {
   // re-allocate the object
      TGraphAsymmErrors::SetPoint(i,0,0);
   }
   fEYlow[i]  = eyl;
}


//______________________________________________________________________________
void TGraphAsymmErrors::SetPointEYhigh(Int_t i, Double_t eyh)
{
   // Set EYhigh for point i

   if (i < 0) return;
   if (i >= fNpoints) {
   // re-allocate the object
      TGraphAsymmErrors::SetPoint(i,0,0);
   }
   fEYhigh[i]  = eyh;
}


//______________________________________________________________________________
void TGraphAsymmErrors::Streamer(TBuffer &b)
{
   // Stream an object of class TGraphAsymmErrors.

   if (b.IsReading()) {
      UInt_t R__s, R__c;
      Version_t R__v = b.ReadVersion(&R__s, &R__c);
      if (R__v > 2) {
         b.ReadClassBuffer(TGraphAsymmErrors::Class(), this, R__v, R__s, R__c);
         return;
      }
      //====process old versions before automatic schema evolution
      TGraph::Streamer(b);
      fEXlow  = new Double_t[fNpoints];
      fEYlow  = new Double_t[fNpoints];
      fEXhigh = new Double_t[fNpoints];
      fEYhigh = new Double_t[fNpoints];
      if (R__v < 2) {
         Float_t *exlow  = new Float_t[fNpoints];
         Float_t *eylow  = new Float_t[fNpoints];
         Float_t *exhigh = new Float_t[fNpoints];
         Float_t *eyhigh = new Float_t[fNpoints];
         b.ReadFastArray(exlow,fNpoints);
         b.ReadFastArray(eylow,fNpoints);
         b.ReadFastArray(exhigh,fNpoints);
         b.ReadFastArray(eyhigh,fNpoints);
         for (Int_t i=0;i<fNpoints;i++) {
            fEXlow[i]  = exlow[i];
            fEYlow[i]  = eylow[i];
            fEXhigh[i] = exhigh[i];
            fEYhigh[i] = eyhigh[i];
         }
         delete [] eylow;
         delete [] exlow;
         delete [] eyhigh;
         delete [] exhigh;
      } else {
         b.ReadFastArray(fEXlow,fNpoints);
         b.ReadFastArray(fEYlow,fNpoints);
         b.ReadFastArray(fEXhigh,fNpoints);
         b.ReadFastArray(fEYhigh,fNpoints);
      }
      b.CheckByteCount(R__s, R__c, TGraphAsymmErrors::IsA());
      //====end of old versions

   } else {
      b.WriteClassBuffer(TGraphAsymmErrors::Class(),this);
   }
}


//______________________________________________________________________________
void TGraphAsymmErrors::SwapPoints(Int_t pos1, Int_t pos2)
{
   // Swap points.

   SwapValues(fEXlow,  pos1, pos2);
   SwapValues(fEXhigh, pos1, pos2);
   SwapValues(fEYlow,  pos1, pos2);
   SwapValues(fEYhigh, pos1, pos2);
   TGraph::SwapPoints(pos1, pos2);
}
