// @(#)root/graf:$Name:  $:$Id: TGraph.cxx,v 1.217 2007/08/15 07:38:16 brun Exp $
// Author: Rene Brun, Olivier Couet   12/12/94

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include <string.h>

#include "Riostream.h"
#include "TROOT.h"
#include "TEnv.h"
#include "TGraph.h"
#include "TGaxis.h"
#include "TH1.h"
#include "TF1.h"
#include "TStyle.h"
#include "TMath.h"
#include "TFrame.h"
#include "TVector.h"
#include "TVectorD.h"
#include "Foption.h"
#include "TRandom.h"
#include "TSpline.h"
#include "TPaveStats.h"
#include "TVirtualFitter.h"
#include "TVirtualPad.h"
#include "TVirtualHistPainter.h"
#include "TBrowser.h"
#include "TClass.h"
#include "TSystem.h"

Double_t *gxwork, *gywork, *gxworkl, *gyworkl;

extern void H1LeastSquareSeqnd(Int_t n, Double_t *a, Int_t idim, Int_t &ifail, Int_t k, Double_t *b);


ClassImp(TGraph)

//______________________________________________________________________________
/* Begin_Html
<center><h2>Graph class</h2></center>
A Graph is a graphics object made of two arrays X and Y with npoints each.
This class supports essentially two graph categories:
<ul>
<li> General case with non equidistant points </li>
<li> Special case with equidistant points </li>
</ul>
The various format options to draw a Graph are explained in
<tt>TGraph::PaintGraph</tt>  and <tt>TGraph::PaintGrapHist</tt>
These two functions are derived from the HIGZ routines IGRAPH and IGHIST
and many modifications.
<p>
The picture below gives an example:
End_Html
Begin_Macro(source)
{
   TCanvas *c1 = new TCanvas("c1","A Simple Graph Example",200,10,700,500);
   Double_t x[100], y[100];
   Int_t n = 20;
   for (Int_t i=0;i<n;i++) {
     x[i] = i*0.1;
     y[i] = 10*sin(x[i]+0.2);
   }
   gr = new TGraph(n,x,y);
   gr->Draw("AC*");
   return c1;
}
End_Macro */


//______________________________________________________________________________
TGraph::TGraph(): TNamed(), TAttLine(), TAttFill(1,1001), TAttMarker()
{
   // Graph default constructor.

   fNpoints   = -1;  //will be reset to 0 in CtorAllocate
   CtorAllocate();
}


//______________________________________________________________________________
TGraph::TGraph(Int_t n)
       : TNamed("Graph","Graph"), TAttLine(), TAttFill(1,1001), TAttMarker()
{
   // Constructor with only the number of points set
   // the arrsys x and y will be set later

   fNpoints = n;
   if (!CtorAllocate()) return;
   FillZero(0, fNpoints);
}


//______________________________________________________________________________
TGraph::TGraph(Int_t n, const Int_t *x, const Int_t *y)
       : TNamed("Graph","Graph"), TAttLine(), TAttFill(1,1001), TAttMarker()
{
   // Graph normal constructor with ints.

   if (!x || !y) {
      fNpoints = 0;
   } else {
      fNpoints = n;
   }
   if (!CtorAllocate()) return;
   for (Int_t i=0;i<n;i++) {
      fX[i] = (Double_t)x[i];
      fY[i] = (Double_t)y[i];
   }
}


//______________________________________________________________________________
TGraph::TGraph(Int_t n, const Float_t *x, const Float_t *y)
       : TNamed("Graph","Graph"), TAttLine(), TAttFill(1,1001), TAttMarker()
{
   // Graph normal constructor with floats.

   if (!x || !y) {
      fNpoints = 0;
   } else {
      fNpoints = n;
   }
   if (!CtorAllocate()) return;
   for (Int_t i=0;i<n;i++) {
      fX[i] = x[i];
      fY[i] = y[i];
   }
}


//______________________________________________________________________________
TGraph::TGraph(Int_t n, const Double_t *x, const Double_t *y)
       : TNamed("Graph","Graph"), TAttLine(), TAttFill(1,1001), TAttMarker()
{
   // Graph normal constructor with doubles.

   if (!x || !y) {
      fNpoints = 0;
   } else {
      fNpoints = n;
   }
   if (!CtorAllocate()) return;
   n = fNpoints*sizeof(Double_t);
   memcpy(fX, x, n);
   memcpy(fY, y, n);
}


//______________________________________________________________________________
TGraph::TGraph(const TGraph &gr)
       : TNamed(gr), TAttLine(gr), TAttFill(gr), TAttMarker(gr)
{
   // Copy constructor for this graph

   fNpoints = gr.fNpoints;
   fMaxSize = gr.fMaxSize;
   if (gr.fFunctions) fFunctions = (TList*)gr.fFunctions->Clone();
   else fFunctions = new TList;
   fHistogram = 0;
   fMinimum = gr.fMinimum;
   fMaximum = gr.fMaximum;
   if (!fMaxSize) {
      fX = fY = 0;
      return;
   } else {
      fX = new Double_t[fMaxSize];
      fY = new Double_t[fMaxSize];
   }

   Int_t n = gr.GetN()*sizeof(Double_t);
   memcpy(fX, gr.fX, n);
   memcpy(fY, gr.fY, n);
}


//______________________________________________________________________________
TGraph& TGraph::operator=(const TGraph &gr)
{
   // Equal operator for this graph

   if(this!=&gr) {
      TNamed::operator=(gr);
      TAttLine::operator=(gr);
      TAttFill::operator=(gr);
      TAttMarker::operator=(gr);
    
      fNpoints = gr.fNpoints;
      fMaxSize = gr.fMaxSize;
      if (gr.fFunctions) fFunctions = (TList*)gr.fFunctions->Clone();
      else fFunctions = new TList;
      if (gr.fHistogram) fHistogram = new TH1F(*(gr.fHistogram));
      else fHistogram = 0;
      fMinimum = gr.fMinimum;
      fMaximum = gr.fMaximum;
      if (!fMaxSize) {
         fX = fY = 0;
         return *this;
      } else {
         fX = new Double_t[fMaxSize];
         fY = new Double_t[fMaxSize];
      }
    
      Int_t n = gr.GetN()*sizeof(Double_t);
      if (n>0) {
         memcpy(fX, gr.fX, n);
         memcpy(fY, gr.fY, n);
      }
   }
   return *this;
}


//______________________________________________________________________________
TGraph::TGraph(const TVectorF &vx, const TVectorF &vy)
       : TNamed("Graph","Graph"), TAttLine(), TAttFill(1,1001), TAttMarker()
{
   // Graph constructor with two vectors of floats in input
   // A graph is build with the X coordinates taken from vx and Y coord from vy
   // The number of points in the graph is the minimum of number of points
   // in vx and vy.

   fNpoints = TMath::Min(vx.GetNrows(), vy.GetNrows());
   if (!CtorAllocate()) return;
   Int_t ivxlow  = vx.GetLwb();
   Int_t ivylow  = vy.GetLwb();
   for (Int_t i=0;i<fNpoints;i++) {
      fX[i]  = vx(i+ivxlow);
      fY[i]  = vy(i+ivylow);
   }
}


//______________________________________________________________________________
TGraph::TGraph(const TVectorD &vx, const TVectorD &vy)
       : TNamed("Graph","Graph"), TAttLine(), TAttFill(1,1001), TAttMarker()
{
   // Graph constructor with two vectors of doubles in input
   // A graph is build with the X coordinates taken from vx and Y coord from vy
   // The number of points in the graph is the minimum of number of points
   // in vx and vy.

   fNpoints = TMath::Min(vx.GetNrows(), vy.GetNrows());
   if (!CtorAllocate()) return;
   Int_t ivxlow  = vx.GetLwb();
   Int_t ivylow  = vy.GetLwb();
   for (Int_t i=0;i<fNpoints;i++) {
      fX[i]  = vx(i+ivxlow);
      fY[i]  = vy(i+ivylow);
   }
}


//______________________________________________________________________________
TGraph::TGraph(const TH1 *h)
       : TNamed("Graph","Graph"), TAttLine(), TAttFill(1,1001), TAttMarker()
{
   // Graph constructor importing its parameters from the TH1 object passed as argument

   if (!h) {
      Error("TGraph", "Pointer to histogram is null");
      fNpoints = 0;
   }
   if (h->GetDimension() != 1) {
      Error("TGraph", "Histogram must be 1-D; h %s is %d-D",h->GetName(),h->GetDimension());
      fNpoints = 0;
   } else {
      fNpoints = ((TH1*)h)->GetXaxis()->GetNbins();
   }

   if (!CtorAllocate()) return;

   TAxis *xaxis = ((TH1*)h)->GetXaxis();
   for (Int_t i=0;i<fNpoints;i++) {
      fX[i] = xaxis->GetBinCenter(i+1);
      fY[i] = h->GetBinContent(i+1);
   }
   h->TAttLine::Copy(*this);
   h->TAttFill::Copy(*this);
   h->TAttMarker::Copy(*this);

   SetName(h->GetName());
   SetTitle(h->GetTitle());
}


//______________________________________________________________________________
TGraph::TGraph(const TF1 *f, Option_t *option)
       : TNamed("Graph","Graph"), TAttLine(), TAttFill(1,1001), TAttMarker()
{
   // Graph constructor importing its parameters from the TF1 object passed as argument
   // if option =="" (default), a TGraph is created with points computed
   //                at the fNpx points of f.
   // if option =="d", a TGraph is created with points computed with the derivatives
   //                at the fNpx points of f.
   // if option =="i", a TGraph is created with points computed with the integral
   //                at the fNpx points of f.
   // if option =="I", a TGraph is created with points computed with the integral
   //                at the fNpx+1 points of f and the integral is normalized to 1.

   char coption = ' ';
   if (!f) {
      Error("TGraph", "Pointer to function is null");
      fNpoints = 0;
   } else {
      fNpoints   = f->GetNpx();
      if (option) coption = *option;
      if (coption == 'i' || coption == 'I') fNpoints++;
   }
   if (!CtorAllocate()) return;

   Double_t xmin = f->GetXmin();
   Double_t xmax = f->GetXmax();
   Double_t dx   = (xmax-xmin)/fNpoints;
   Double_t integ = 0;
   Int_t i;
   for (i=0;i<fNpoints;i++) {
      if (coption == 'i' || coption == 'I') {
         fX[i] = xmin +i*dx;
         if (i == 0) fY[i] = 0;
         else        fY[i] = integ + ((TF1*)f)->Integral(fX[i]-dx,fX[i]);
         integ = fY[i];
      } else if (coption == 'd' || coption == 'D') {
         fX[i] = xmin + (i+0.5)*dx;
         fY[i] = ((TF1*)f)->Derivative(fX[i]);
      } else {
         fX[i] = xmin + (i+0.5)*dx;
         fY[i] = ((TF1*)f)->Eval(fX[i]);
      }
   }
   if (integ != 0 && coption == 'I') {
      for (i=1;i<fNpoints;i++) fY[i] /= integ;
   }

   f->TAttLine::Copy(*this);
   f->TAttFill::Copy(*this);
   f->TAttMarker::Copy(*this);

   SetName(f->GetName());
   SetTitle(f->GetTitle());
}


//______________________________________________________________________________
TGraph::TGraph(const char *filename, const char *format, Option_t *)
       : TNamed("Graph",filename), TAttLine(), TAttFill(1,1001), TAttMarker()
{
   // Graph constructor reading input from filename
   // filename is assumed to contain at least two columns of numbers
   // the string format is by default "%lg %lg"

   Double_t x,y;
   ifstream infile(filename);
   if(!infile.good()){
      MakeZombie();
      Error("TGraph", "Cannot open file: %s, TGraph is Zombie",filename);
      fNpoints = 0;
   } else {
      fNpoints = 100;  //initial number of points
   }
   if (!CtorAllocate()) return;
   std::string line;
   Int_t np=0;
   while(std::getline(infile,line,'\n')){
      if(2 != sscanf(line.c_str(),format,&x,&y) ) {
         continue; // skip empty and ill-formed lines
      }
      SetPoint(np,x,y);
      np++;
   }
   Set(np);
}


//______________________________________________________________________________
TGraph::~TGraph()
{
   // Graph default destructor.

   delete [] fX;
   delete [] fY;
   if (fFunctions) {
      fFunctions->SetBit(kInvalidObject);
      //special logic to support the case where the same object is
      //added multiple times in fFunctions.
      //This case happens when the same object is added with different
      //drawing modes
      TObject *obj;
      while ((obj  = fFunctions->First())) {
         while(fFunctions->Remove(obj));
         delete obj;
      }
      delete fFunctions;
      fFunctions = 0; //to avoid accessing a deleted object in RecursiveRemove
   }
   delete fHistogram;
}


//______________________________________________________________________________
Double_t** TGraph::AllocateArrays(Int_t Narrays, Int_t arraySize)
{
   // Allocate arrays.

   if (arraySize < 0) { arraySize = 0; }
   Double_t **newarrays = new Double_t*[Narrays];
   if (!arraySize) {
      for (Int_t i = 0; i < Narrays; ++i)
         newarrays[i] = 0;
   } else {
      for (Int_t i = 0; i < Narrays; ++i)
         newarrays[i] = new Double_t[arraySize];
   }
   fMaxSize = arraySize;
   return newarrays;
}


//______________________________________________________________________________
void TGraph::Apply(TF1 *f)
{
   // Apply function f to all the data points
   // f may be a 1-D function TF1 or 2-d function TF2
   // The Y values of the graph are replaced by the new values computed
   // using the function

   for (Int_t i=0;i<fNpoints;i++) {
      fY[i] = f->Eval(fX[i],fY[i]);
   }
}


//______________________________________________________________________________
void TGraph::Browse(TBrowser *b)
{
   // Browse

   TString opt = gEnv->GetValue("TGraph.BrowseOption","");
   if (opt.IsNull()) {
      opt = b ? b->GetDrawOption() : "alp";
   }
   Draw(opt.Data());
   gPad->Update();
}


//______________________________________________________________________________
Double_t TGraph::Chisquare(const TF1 *f1) const
{
   // Return the chisquare of this graph with respect to f1.
   // The chisquare is computed as the sum of the quantity below at each point:
   // Begin_Latex
   // #frac{(y-f1(x))^{2}}{ey^{2}+(#frac{1}{2}(exl+exh)f1'(x))^{2}}
   // End_latex
   // where x and y are the graph point coordinates and f1'(x) is the derivative of function f1(x).
   // This method to approximate the uncertainty in y because of the errors in x, is called
   // "effective variance" method.
   // In case of a pure TGraph, the denominator is 1.
   // In case of a TGraphErrors or TGraphAsymmErrors the errors are taken
   // into account.
   
   if (!f1) return 0;
   Double_t cu,eu,exh,exl,ey,eux,fu,fsum;
   Double_t x[1];
   Double_t chi2 = 0;
   TF1 *func = (TF1*)f1; //EvalPar is not const !
   for (Int_t i=0;i<fNpoints;i++) {
      func->InitArgs(x,0); //must be inside the loop because of TF1::Derivative calling InitArgs
      x[0] = fX[i];
      if (!func->IsInside(x)) continue;
      cu   = fY[i];
      TF1::RejectPoint(kFALSE);
      fu   = func->EvalPar(x);
      if (TF1::RejectedPoint()) continue;
      fsum = (cu-fu);
      //npfits++;
      exh = GetErrorXhigh(i);
      exl = GetErrorXlow(i);
      if (fsum < 0)
         ey = GetErrorYhigh(i);
      else
         ey = GetErrorYlow(i);
      if (exl < 0) exl = 0;
      if (exh < 0) exh = 0;
      if (ey < 0)  ey  = 0;
      if (exh > 0 || exl > 0) {
         //"Effective Variance" method introduced by Anna Kreshuk 
         //a copy of the algorithm in GraphFitChisquare from TFitter
         eux = 0.5*(exl + exh)*func->Derivative(x[0]);
      } else
         eux = 0.;
      eu = ey*ey+eux*eux;
      if (eu <= 0) eu = 1;
      chi2 += fsum*fsum/eu;
   }
   return chi2;
}

//______________________________________________________________________________
Bool_t TGraph::CompareArg(const TGraph* gr, Int_t left, Int_t right)
{
  // Return kTRUE if point number "left"'s argument (angle with respect to positive
  // x-axis) is bigger than that of point number "right". Can be used by Sort.
  Double_t xl,yl,xr,yr;
  gr->GetPoint(left,xl,yl);
  gr->GetPoint(right,xr,yr);
  return (TMath::ATan2(yl, xl) > TMath::ATan2(yr, xr));
}

//______________________________________________________________________________
Bool_t TGraph::CompareX(const TGraph* gr, Int_t left, Int_t right)
{
   // Return kTRUE if fX[left] > fX[right]. Can be used by Sort.

   return gr->fX[left]>gr->fX[right];
}


//______________________________________________________________________________
Bool_t TGraph::CompareY(const TGraph* gr, Int_t left, Int_t right)
{
   // Return kTRUE if fY[left] > fY[right]. Can be used by Sort.

   return gr->fY[left]>gr->fY[right];
}


//______________________________________________________________________________
Bool_t TGraph::CompareRadius(const TGraph* gr, Int_t left, Int_t right)
{
   // Return kTRUE if point number "left"'s distance to origin is bigger than
   // that of point number "right". Can be used by Sort.

   return gr->fX[left]*gr->fX[left]+gr->fY[left]*gr->fY[left]
      >gr->fX[right]*gr->fX[right]+gr->fY[right]*gr->fY[right];
}


//______________________________________________________________________________
void TGraph::ComputeRange(Double_t &, Double_t &, Double_t &, Double_t &) const
{
   // This function is dummy in TGraph, but redefined by TGraphErrors
}


//______________________________________________________________________________
void TGraph::CopyAndRelease(Double_t **newarrays, Int_t ibegin, Int_t iend,
                           Int_t obegin)
{
   // Copy points from fX and fY to arrays[0] and arrays[1]
   // or to fX and fY if arrays == 0 and ibegin != iend.
   // If newarrays is non null, replace fX, fY with pointers from newarrays[0,1].
   // Delete newarrays, old fX and fY

   CopyPoints(newarrays, ibegin, iend, obegin);
   if (newarrays) {
      delete[] fX;
      fX = newarrays[0];
      delete[] fY;
      fY = newarrays[1];
      delete[] newarrays;
   }
}


//______________________________________________________________________________
Bool_t TGraph::CopyPoints(Double_t **arrays, Int_t ibegin, Int_t iend,
                        Int_t obegin)
{
   // Copy points from fX and fY to arrays[0] and arrays[1]
   // or to fX and fY if arrays == 0 and ibegin != iend.

   if (ibegin < 0 || iend <= ibegin || obegin < 0) { // Error;
      return kFALSE;
   }
   if (!arrays && ibegin == obegin) { // No copying is needed
      return kFALSE;
   }
   Int_t n = (iend - ibegin)*sizeof(Double_t);
   if (arrays) {
      memmove(&arrays[0][obegin], &fX[ibegin], n);
      memmove(&arrays[1][obegin], &fY[ibegin], n);
   } else {
      memmove(&fX[obegin], &fX[ibegin], n);
      memmove(&fY[obegin], &fY[ibegin], n);
   }
   return kTRUE;
}


//______________________________________________________________________________
Bool_t TGraph::CtorAllocate()
{
   // In constructors set fNpoints than call this method.
   // Return kFALSE if the graph will contain no points.

   fHistogram = 0;
   fMaximum = -1111;
   fMinimum = -1111;
   SetBit(kClipFrame);
   fFunctions = 0;
   if (fNpoints >= 0) fFunctions = new TList;
   if (fNpoints <= 0) {
      fNpoints = 0;
      fMaxSize   = 0;
      fX         = 0;
      fY         = 0;
      return kFALSE;
   } else {
      fMaxSize   = fNpoints;
      fX = new Double_t[fMaxSize];
      fY = new Double_t[fMaxSize];
   }
   return kTRUE;
}


//______________________________________________________________________________
void TGraph::Draw(Option_t *option)
{
   // Draw this graph with its current attributes.
   //
   //   Options to draw a graph are described in TGraph::PaintGraph

   TString opt = option;
   opt.ToLower();

   if (opt.Contains("same")) {
      opt.ReplaceAll("same","");
   }

   // in case of option *, set marker style to 3 (star) and replace
   // * option by option P.
   Ssiz_t pos;
   if ((pos = opt.Index("*")) != kNPOS) {
      SetMarkerStyle(3);
      opt.Replace(pos, 1, "p");
   }
   if (gPad) {
      if (!gPad->IsEditable()) gROOT->MakeDefCanvas();
      if (opt.Contains("a")) gPad->Clear();
   }
   AppendPad(opt);
}


//______________________________________________________________________________
Int_t TGraph::DistancetoPrimitive(Int_t px, Int_t py)
{
   // Compute distance from point px,py to a graph.
   //
   //  Compute the closest distance of approach from point px,py to this line.
   //  The distance is computed in pixels units.

   // Are we on the axis?
   Int_t distance;
   if (fHistogram) {
      distance = fHistogram->DistancetoPrimitive(px,py);
      if (distance <= 5) return distance;
   }

   // Somewhere on the graph points?
   const Int_t big = 9999;
   const Int_t kMaxDiff = 10;
   Int_t puxmin = gPad->XtoAbsPixel(gPad->GetUxmin());
   Int_t puymin = gPad->YtoAbsPixel(gPad->GetUymin());
   Int_t puxmax = gPad->XtoAbsPixel(gPad->GetUxmax());
   Int_t puymax = gPad->YtoAbsPixel(gPad->GetUymax());

   // return if point is not in the graph area
   if (px <= puxmin) return big;
   if (py >= puymin) return big;
   if (px >= puxmax) return big;
   if (py <= puymax) return big;

   // check if point is near one of the graph points
   Int_t i, pxp, pyp, d;
   distance = big;

   for (i=0;i<fNpoints;i++) {
      pxp = gPad->XtoAbsPixel(gPad->XtoPad(fX[i]));
      pyp = gPad->YtoAbsPixel(gPad->YtoPad(fY[i]));
      d   = TMath::Abs(pxp-px) + TMath::Abs(pyp-py);
      if (d < distance) distance = d;
   }
   if (distance < kMaxDiff) return distance;

   for (i=0;i<fNpoints-1;i++) {
      d = DistancetoLine(px, py, gPad->XtoPad(fX[i]), gPad->YtoPad(fY[i]), gPad->XtoPad(fX[i+1]), gPad->YtoPad(fY[i+1]));
      if (d < distance) distance = d;
   }
   
   // If graph has been drawn with the fill area option, check if we are inside
   TString drawOption = GetDrawOption();
   drawOption.ToLower();
   if (drawOption.Contains("f")) {
      Double_t xp = gPad->AbsPixeltoX(px); xp = gPad->PadtoX(xp);
      Double_t yp = gPad->AbsPixeltoY(py); yp = gPad->PadtoY(yp);
      if (TMath::IsInside(xp,yp,fNpoints,fX,fY) != 0) distance = 1;
   }

   // Loop on the list of associated functions and user objects
   TObject *f;
   TIter   next(fFunctions);
   while ((f = (TObject*) next())) {
      Int_t dist;
      if (f->InheritsFrom(TF1::Class())) dist = f->DistancetoPrimitive(-px,py);
      else                               dist = f->DistancetoPrimitive(px,py);
      if (dist < kMaxDiff) {
         gPad->SetSelected(f);
         return 0; //must be o and not dist in case of TMultiGraph
      }
   }

   return distance;
}


//______________________________________________________________________________
void TGraph::DrawGraph(Int_t n, const Int_t *x, const Int_t *y, Option_t *option)
{
   // Draw this graph with new attributes.

   TGraph *newgraph = new TGraph(n, x, y);
   TAttLine::Copy(*newgraph);
   TAttFill::Copy(*newgraph);
   TAttMarker::Copy(*newgraph);
   newgraph->SetBit(kCanDelete);
   newgraph->AppendPad(option);
}


//______________________________________________________________________________
void TGraph::DrawGraph(Int_t n, const Float_t *x, const Float_t *y, Option_t *option)
{
   // Draw this graph with new attributes.

   TGraph *newgraph = new TGraph(n, x, y);
   TAttLine::Copy(*newgraph);
   TAttFill::Copy(*newgraph);
   TAttMarker::Copy(*newgraph);
   newgraph->SetBit(kCanDelete);
   newgraph->AppendPad(option);
}


//______________________________________________________________________________
void TGraph::DrawGraph(Int_t n, const Double_t *x, const Double_t *y, Option_t *option)
{
   // Draw this graph with new attributes.

   const Double_t *xx = x;
   const Double_t *yy = y;
   if (!xx) xx = fX;
   if (!yy) yy = fY;
   TGraph *newgraph = new TGraph(n, xx, yy);
   TAttLine::Copy(*newgraph);
   TAttFill::Copy(*newgraph);
   TAttMarker::Copy(*newgraph);
   newgraph->SetBit(kCanDelete);
   newgraph->AppendPad(option);
}


//______________________________________________________________________________
void TGraph::DrawPanel()
{
   // Display a panel with all graph drawing options.

   printf("TGraph::DrawPanel: not yet implemented\n");
}


//______________________________________________________________________________
Double_t TGraph::Eval(Double_t x, TSpline *spline, Option_t *option) const
{
   // Interpolate points in this graph at x using a TSpline
   //  -if spline==0 and option="" a linear interpolation between the two points
   //   close to x is computed. If x is outside the graph range, a linear
   //   extrapolation is computed.
   //  -if spline==0 and option="S" a TSpline3 object is created using this graph
   //   and the interpolated value from the spline is returned.
   //   the internally created spline is deleted on return.
   //  -if spline is specified, it is used to return the interpolated value.

   if (!spline) {
      TString opt = option;
      opt.ToLower();
      if (opt.Contains("s")) {
         // spline interpolation creating a new spline
         TSpline3 *s = new TSpline3("",this);
         Double_t result = s->Eval(x);
         delete s;
         return result;
      }
      //linear interpolation
      //find point in graph immediatly below x
      //In case x is < fX[0] or > fX[fNpoints-1] return the extrapolated point
      Int_t low = TMath::BinarySearch(fNpoints,fX,x);
      Int_t up = low+1;
      if (low == fNpoints-1) {up=low; low = up-1;}
      if (low == -1) {low=0; up=1;}
      if (fX[low] == fX[up]) return fY[low];
      Double_t yn = x*(fY[low]-fY[up]) +fX[low]*fY[up] - fX[up]*fY[low];
      return yn/(fX[low]-fX[up]);
   } else {
      //spline interpolation using the input spline
      return spline->Eval(x);
   }
}


//______________________________________________________________________________
void TGraph::ExecuteEvent(Int_t event, Int_t px, Int_t py)
{
   // Execute action corresponding to one event.
   //
   //  This member function is called when a graph is clicked with the locator
   //
   //  If Left button clicked on one of the line end points, this point
   //     follows the cursor until button is released.
   //
   //  if Middle button clicked, the line is moved parallel to itself
   //     until the button is released.

   Int_t i, d;
   Double_t xmin, xmax, ymin, ymax, dx, dy, dxr, dyr;
   const Int_t kMaxDiff = 10;
   static Bool_t middle, badcase;
   static Int_t ipoint, pxp, pyp;
   static Int_t px1,px2,py1,py2;
   static Int_t pxold, pyold, px1old, py1old, px2old, py2old;
   static Int_t dpx, dpy;
   static Int_t *x=0, *y=0;

   if (!IsEditable()) {gPad->SetCursor(kHand); return;}
   if (!gPad->IsEditable()) return;

   switch (event) {

   case kButton1Down:
      badcase = kFALSE;
      gVirtualX->SetLineColor(-1);
      TAttLine::Modify();  //Change line attributes only if necessary
      px1 = gPad->XtoAbsPixel(gPad->GetX1());
      py1 = gPad->YtoAbsPixel(gPad->GetY1());
      px2 = gPad->XtoAbsPixel(gPad->GetX2());
      py2 = gPad->YtoAbsPixel(gPad->GetY2());
      ipoint = -1;


      if (x || y) break;
      x = new Int_t[fNpoints+1];
      y = new Int_t[fNpoints+1];
      for (i=0;i<fNpoints;i++) {
         pxp = gPad->XtoAbsPixel(gPad->XtoPad(fX[i]));
         pyp = gPad->YtoAbsPixel(gPad->YtoPad(fY[i]));
         if (pxp < -kMaxPixel || pxp >= kMaxPixel ||
             pyp < -kMaxPixel || pyp >= kMaxPixel) {
            badcase = kTRUE;
            continue;
         }
         gVirtualX->DrawLine(pxp-4, pyp-4, pxp+4,  pyp-4);
         gVirtualX->DrawLine(pxp+4, pyp-4, pxp+4,  pyp+4);
         gVirtualX->DrawLine(pxp+4, pyp+4, pxp-4,  pyp+4);
         gVirtualX->DrawLine(pxp-4, pyp+4, pxp-4,  pyp-4);
         x[i] = pxp;
         y[i] = pyp;
         d   = TMath::Abs(pxp-px) + TMath::Abs(pyp-py);
         if (d < kMaxDiff) ipoint =i;
      }
      dpx = 0;
      dpy = 0;
      pxold = px;
      pyold = py;
      if (ipoint < 0) return;
      if (ipoint == 0) {
         px1old = 0;
         py1old = 0;
         px2old = gPad->XtoAbsPixel(fX[1]);
         py2old = gPad->YtoAbsPixel(fY[1]);
      } else if (ipoint == fNpoints-1) {
         px1old = gPad->XtoAbsPixel(gPad->XtoPad(fX[fNpoints-2]));
         py1old = gPad->YtoAbsPixel(gPad->YtoPad(fY[fNpoints-2]));
         px2old = 0;
         py2old = 0;
      } else {
         px1old = gPad->XtoAbsPixel(gPad->XtoPad(fX[ipoint-1]));
         py1old = gPad->YtoAbsPixel(gPad->YtoPad(fY[ipoint-1]));
         px2old = gPad->XtoAbsPixel(gPad->XtoPad(fX[ipoint+1]));
         py2old = gPad->YtoAbsPixel(gPad->YtoPad(fY[ipoint+1]));
      }
      pxold = gPad->XtoAbsPixel(gPad->XtoPad(fX[ipoint]));
      pyold = gPad->YtoAbsPixel(gPad->YtoPad(fY[ipoint]));

      break;


   case kMouseMotion:

      middle = kTRUE;
      for (i=0;i<fNpoints;i++) {
         pxp = gPad->XtoAbsPixel(gPad->XtoPad(fX[i]));
         pyp = gPad->YtoAbsPixel(gPad->YtoPad(fY[i]));
         d   = TMath::Abs(pxp-px) + TMath::Abs(pyp-py);
         if (d < kMaxDiff) middle = kFALSE;
      }


   // check if point is close to an axis
      if (middle) gPad->SetCursor(kMove);
      else gPad->SetCursor(kHand);
      break;

   case kButton1Motion:
      if (middle) {
         for(i=0;i<fNpoints-1;i++) {
            gVirtualX->DrawLine(x[i]+dpx, y[i]+dpy, x[i+1]+dpx, y[i+1]+dpy);
            pxp = x[i]+dpx;
            pyp = y[i]+dpy;
            if (pxp < -kMaxPixel || pxp >= kMaxPixel ||
                pyp < -kMaxPixel || pyp >= kMaxPixel) continue;
            gVirtualX->DrawLine(pxp-4, pyp-4, pxp+4,  pyp-4);
            gVirtualX->DrawLine(pxp+4, pyp-4, pxp+4,  pyp+4);
            gVirtualX->DrawLine(pxp+4, pyp+4, pxp-4,  pyp+4);
            gVirtualX->DrawLine(pxp-4, pyp+4, pxp-4,  pyp-4);
         }
         pxp = x[fNpoints-1]+dpx;
         pyp = y[fNpoints-1]+dpy;
         gVirtualX->DrawLine(pxp-4, pyp-4, pxp+4,  pyp-4);
         gVirtualX->DrawLine(pxp+4, pyp-4, pxp+4,  pyp+4);
         gVirtualX->DrawLine(pxp+4, pyp+4, pxp-4,  pyp+4);
         gVirtualX->DrawLine(pxp-4, pyp+4, pxp-4,  pyp-4);
         dpx += px - pxold;
         dpy += py - pyold;
         pxold = px;
         pyold = py;
         for(i=0;i<fNpoints-1;i++) {
            gVirtualX->DrawLine(x[i]+dpx, y[i]+dpy, x[i+1]+dpx, y[i+1]+dpy);
            pxp = x[i]+dpx;
            pyp = y[i]+dpy;
            if (pxp < -kMaxPixel || pxp >= kMaxPixel ||
                pyp < -kMaxPixel || pyp >= kMaxPixel) continue;
            gVirtualX->DrawLine(pxp-4, pyp-4, pxp+4,  pyp-4);
            gVirtualX->DrawLine(pxp+4, pyp-4, pxp+4,  pyp+4);
            gVirtualX->DrawLine(pxp+4, pyp+4, pxp-4,  pyp+4);
            gVirtualX->DrawLine(pxp-4, pyp+4, pxp-4,  pyp-4);
         }
         pxp = x[fNpoints-1]+dpx;
         pyp = y[fNpoints-1]+dpy;
         gVirtualX->DrawLine(pxp-4, pyp-4, pxp+4,  pyp-4);
         gVirtualX->DrawLine(pxp+4, pyp-4, pxp+4,  pyp+4);
         gVirtualX->DrawLine(pxp+4, pyp+4, pxp-4,  pyp+4);
         gVirtualX->DrawLine(pxp-4, pyp+4, pxp-4,  pyp-4);
      } else {
         if (px1old) gVirtualX->DrawLine(px1old, py1old, pxold,  pyold);
         if (px2old) gVirtualX->DrawLine(pxold,  pyold,  px2old, py2old);
         gVirtualX->DrawLine(pxold-4, pyold-4, pxold+4,  pyold-4);
         gVirtualX->DrawLine(pxold+4, pyold-4, pxold+4,  pyold+4);
         gVirtualX->DrawLine(pxold+4, pyold+4, pxold-4,  pyold+4);
         gVirtualX->DrawLine(pxold-4, pyold+4, pxold-4,  pyold-4);
         pxold = px;
         pxold = TMath::Max(pxold, px1);
         pxold = TMath::Min(pxold, px2);
         pyold = py;
         pyold = TMath::Max(pyold, py2);
         pyold = TMath::Min(pyold, py1);
         if (px1old) gVirtualX->DrawLine(px1old, py1old, pxold,  pyold);
         if (px2old) gVirtualX->DrawLine(pxold,  pyold,  px2old, py2old);
         gVirtualX->DrawLine(pxold-4, pyold-4, pxold+4,  pyold-4);
         gVirtualX->DrawLine(pxold+4, pyold-4, pxold+4,  pyold+4);
         gVirtualX->DrawLine(pxold+4, pyold+4, pxold-4,  pyold+4);
         gVirtualX->DrawLine(pxold-4, pyold+4, pxold-4,  pyold-4);
      }
      break;

   case kButton1Up:

      if (gROOT->IsEscaped()) {
         gROOT->SetEscape(kFALSE);
         delete [] x; x = 0;
         delete [] y; y = 0;
         break;
      }

   // Compute x,y range
      xmin = gPad->GetUxmin();
      xmax = gPad->GetUxmax();
      ymin = gPad->GetUymin();
      ymax = gPad->GetUymax();
      dx   = xmax-xmin;
      dy   = ymax-ymin;
      dxr  = dx/(1 - gPad->GetLeftMargin() - gPad->GetRightMargin());
      dyr  = dy/(1 - gPad->GetBottomMargin() - gPad->GetTopMargin());

      if (fHistogram) {
         // Range() could change the size of the pad pixmap and therefore should
         // be called before the other paint routines
         gPad->Range(xmin - dxr*gPad->GetLeftMargin(),
                      ymin - dyr*gPad->GetBottomMargin(),
                      xmax + dxr*gPad->GetRightMargin(),
                      ymax + dyr*gPad->GetTopMargin());
         gPad->RangeAxis(xmin, ymin, xmax, ymax);
      }      
      if (middle) {
         for(i=0;i<fNpoints;i++) {
            if (badcase) continue;  //do not update if big zoom and points moved
            if (x) fX[i] = gPad->PadtoX(gPad->AbsPixeltoX(x[i]+dpx));
            if (y) fY[i] = gPad->PadtoY(gPad->AbsPixeltoY(y[i]+dpy));
         }
      } else {
         fX[ipoint] = gPad->PadtoX(gPad->AbsPixeltoX(pxold));
         fY[ipoint] = gPad->PadtoY(gPad->AbsPixeltoY(pyold));
         if (InheritsFrom("TCutG")) {
            //make sure first and last point are the same
            if (ipoint == 0) {
               fX[fNpoints-1] = fX[0];
               fY[fNpoints-1] = fY[0];
            }
            if (ipoint == fNpoints-1) {
               fX[0] = fX[fNpoints-1];
               fY[0] = fY[fNpoints-1];
            }
         }
      }
      badcase = kFALSE;
      delete [] x; x = 0;
      delete [] y; y = 0;
      gPad->Modified(kTRUE);
      gVirtualX->SetLineColor(-1);

   }

}


//______________________________________________________________________________
void TGraph::Expand(Int_t newsize)
{
   // If array sizes <= newsize, expand storage to 2*newsize.

   Double_t **ps = ExpandAndCopy(newsize, fNpoints);
   CopyAndRelease(ps, 0, 0, 0);
}


//______________________________________________________________________________
void TGraph::Expand(Int_t newsize, Int_t step)
{
   // If graph capacity is less than newsize points then make array sizes
   // equal to least multiple of step to contain newsize points.
   // Returns kTRUE if size was altered

   if (newsize <= fMaxSize) {
      return;
   }
   Double_t **ps = Allocate(step*(newsize/step + (newsize%step?1:0)));
   CopyAndRelease(ps, 0, fNpoints, 0);
}


//______________________________________________________________________________
Double_t **TGraph::ExpandAndCopy(Int_t size, Int_t iend)
{
   // if size > fMaxSize allocate new arrays of 2*size points
   //  and copy oend first points.
   // Return pointer to new arrays.

   if (size <= fMaxSize) { return 0; }
   Double_t **newarrays = Allocate(2*size);
   CopyPoints(newarrays, 0, iend, 0);
   return newarrays;
}


//______________________________________________________________________________
void TGraph::FillZero(Int_t begin, Int_t end, Bool_t)
{
   // Set zero values for point arrays in the range [begin, end)
   // Should be redefined in descendant classes

   memset(fX + begin, 0, (end - begin)*sizeof(Double_t));
   memset(fY + begin, 0, (end - begin)*sizeof(Double_t));
}


//______________________________________________________________________________
TObject *TGraph::FindObject(const char *name) const
{
   // Search object named name in the list of functions

   if (fFunctions) return fFunctions->FindObject(name);
   return 0;
}


//______________________________________________________________________________
TObject *TGraph::FindObject(const TObject *obj) const
{
   // Search object obj in the list of functions

   if (fFunctions) return fFunctions->FindObject(obj);
   return 0;
}


//______________________________________________________________________________
Int_t TGraph::Fit(const char *fname, Option_t *option, Option_t *, Axis_t xmin, Axis_t xmax)
{
   // Fit this graph with function with name fname.
   //
   //  interface to TGraph::Fit(TF1 *f1...
   //
   //      fname is the name of an already predefined function created by TF1 or TF2
   //      Predefined functions such as gaus, expo and poln are automatically
   //      created by ROOT.
   //      fname can also be a formula, accepted by the linear fitter (linear parts divided
   //      by "++" sign), for example "x++sin(x)" for fitting "[0]*x+[1]*sin(x)"

   char *linear;
   linear= (char*) strstr(fname, "++");
   TF1 *f1=0;
   if (linear)
      f1=new TF1(fname, fname, xmin, xmax);
   else {
      f1 = (TF1*)gROOT->GetFunction(fname);
      if (!f1) { Printf("Unknown function: %s",fname); return -1; }
   }
   return Fit(f1,option,"",xmin,xmax);
}


//______________________________________________________________________________
Int_t TGraph::Fit(TF1 *f1, Option_t *option, Option_t *, Axis_t rxmin, Axis_t rxmax)
{
   // Fit this graph with function f1.
   //
   //   f1 is an already predefined function created by TF1.
   //   Predefined functions such as gaus, expo and poln are automatically
   //   created by ROOT.
   //
   //   The list of fit options is given in parameter option.
   //      option = "W" Set all weights to 1; ignore error bars
   //             = "U" Use a User specified fitting algorithm (via SetFCN)
   //             = "Q" Quiet mode (minimum printing)
   //             = "V" Verbose mode (default is between Q and V)
   //             = "B" Use this option when you want to fix one or more parameters
   //                   and the fitting function is like "gaus","expo","poln","landau".
   //             = "R" Use the Range specified in the function range
   //             = "N" Do not store the graphics function, do not draw
   //             = "0" Do not plot the result of the fit. By default the fitted function
   //                   is drawn unless the option"N" above is specified.
   //             = "+" Add this new fitted function to the list of fitted functions
   //                   (by default, any previous function is deleted)
   //             = "C" In case of linear fitting, not calculate the chisquare
   //                    (saves time)
   //             = "F" If fitting a polN, switch to minuit fitter
   //             = "ROB" In case of linear fitting, compute the LTS regression
   //                     coefficients (robust(resistant) regression), using 
   //                     the default fraction of good points
   //               "ROB=0.x" - compute the LTS regression coefficients, using
   //                           0.x as a fraction of good points
   //
   //   When the fit is drawn (by default), the parameter goption may be used
   //   to specify a list of graphics options. See TGraph::Paint for a complete
   //   list of these options.
   //
   //   In order to use the Range option, one must first create a function
   //   with the expression to be fitted. For example, if your graph
   //   has a defined range between -4 and 4 and you want to fit a gaussian
   //   only in the interval 1 to 3, you can do:
   //        TF1 *f1 = new TF1("f1","gaus",1,3);
   //        graph->Fit("f1","R");
   //
   //
   // Who is calling this function:
   //
   //   Note that this function is called when calling TGraphErrors::Fit
   //   or TGraphAsymmErrors::Fit ot TGraphBentErrors::Fit
   //   see the discussion below on the errors calulation.
   //
   // Linear fitting:
   // 
   //   When the fitting function is linear (contains the "++" sign) or the fitting
   //   function is a polynomial, a linear fitter is initialised.
   //   To create a linear function, use the following syntaxis: linear parts
   //   separated by "++" sign.
   //   Example: to fit the parameters of "[0]*x + [1]*sin(x)", create a
   //    TF1 *f1=new TF1("f1", "x++sin(x)", xmin, xmax);
   //   For such a TF1 you don't have to set the initial conditions
   //   Going via the linear fitter for functions, linear in parameters, gives a considerable
   //   advantage in speed.
   //
   // Setting initial conditions:
   // 
   //   Parameters must be initialized before invoking the Fit function.
   //   The setting of the parameter initial values is automatic for the
   //   predefined functions : poln, expo, gaus, landau. One can however disable
   //   this automatic computation by specifying the option "B".
   //   You can specify boundary limits for some or all parameters via
   //        f1->SetParLimits(p_number, parmin, parmax);
   //   if parmin>=parmax, the parameter is fixed
   //   Note that you are not forced to fix the limits for all parameters.
   //   For example, if you fit a function with 6 parameters, you can do:
   //     func->SetParameters(0,3.1,1.e-6,0.1,-8,100);
   //     func->SetParLimits(4,-10,-4);
   //     func->SetParLimits(5, 1,1);
   //   With this setup, parameters 0->3 can vary freely
   //   Parameter 4 has boundaries [-10,-4] with initial value -8
   //   Parameter 5 is fixed to 100.
   //
   // Fit range:
   // 
   //   The fit range can be specified in two ways:
   //     - specify rxmax > rxmin (default is rxmin=rxmax=0)
   //     - specify the option "R". In this case, the function will be taken
   //       instead of the full graph range.
   //
   // Changing the fitting function:
   // 
   //   By default the fitting function GraphFitChisquare is used.
   //   To specify a User defined fitting function, specify option "U" and
   //   call the following functions:
   //     TVirtualFitter::Fitter(mygraph)->SetFCN(MyFittingFunction)
   //   where MyFittingFunction is of type:
   //   extern void MyFittingFunction(Int_t &npar, Double_t *gin, Double_t &f, Double_t *u, Int_t flag);
   //
   // How errors are used in the chisquare function (see TFitter GraphFitChisquare)//   Access to the fit results
   // 
   //   In case of a TGraphErrors object, ex, the error along x,  is projected
   //   along the y-direction by calculating the function at the points x-exlow and
   //   x+exhigh.
   //
   //   The chisquare is computed as the sum of the quantity below at each point:
   //
   // Begin_Latex
   // #frac{(y-f(x))^{2}}{ey^{2}+(#frac{1}{2}(exl+exh)f'(x))^{2}}
   // End_Latex
   //
   //   where x and y are the point coordinates, and f'(x) is the derivative of function f(x).
   //
   //   In case the function lies below (above) the data point, ey is ey_low (ey_high).
   //
   //   thanks to Andy Haas (haas@yahoo.com) for adding the case with TGraphasymmerrors
   //             University of Washington
   //
   //   The approach used to approximate the uncertainty in y because of the
   //   errors in x, is to make it equal the error in x times the slope of the line.
   //   The improvement, compared to the first method (f(x+ exhigh) - f(x-exlow))/2
   //   is of (error of x)**2 order. This approach is called "effective variance method".
   //   This improvement has been made in version 4.00/08 by Anna Kreshuk.
   //
   // NOTE:
   //   1) By using the "effective variance" method a simple linear regression
   //      becomes a non-linear case, which takes several iterations
   //      instead of 0 as in the linear case .
   //
   //   2) The effective variance technique assumes that there is no correlation 
   //      between the x and y coordinate .
   //
   //   Note, that the linear fitter doesn't take into account the errors in x. If errors
   //   in x are important, go through minuit (use option "F" for polynomial fitting).
   //
   //   3) When fitting a TGraph (ie no errors associated to each point),
   //   a correction is applied to the errors on the parameters with the following
   //   formula:
   //      errorp *= sqrt(chisquare/(ndf-1))
   //
   // Associated functions:
   //
   //   One or more object (typically a TF1*) can be added to the list
   //   of functions (fFunctions) associated to each graph.
   //   When TGraph::Fit is invoked, the fitted function is added to this list.
   //   Given a graph gr, one can retrieve an associated function
   //   with:  TF1 *myfunc = gr->GetFunction("myfunc");
   //
   //   If the graph is made persistent, the list of
   //   associated functions is also persistent. Given a pointer (see above)
   //   to an associated function myfunc, one can retrieve the function/fit
   //   parameters with calls such as:
   //     Double_t chi2 = myfunc->GetChisquare();
   //     Double_t par0 = myfunc->GetParameter(0); //value of 1st parameter
   //     Double_t err0 = myfunc->GetParError(0);  //error on first parameter
   //
   // Fit Statistics
   // 
   //   You can change the statistics box to display the fit parameters with
   //   the TStyle::SetOptFit(mode) method. This mode has four digits.
   //   mode = pcev  (default = 0111)
   //     v = 1;  print name/values of parameters
   //     e = 1;  print errors (if e=1, v must be 1)
   //     c = 1;  print Chisquare/Number of degress of freedom
   //     p = 1;  print Probability
   //
   //   For example: gStyle->SetOptFit(1011);
   //   prints the fit probability, parameter names/values, and errors.
   //   You can change the position of the statistics box with these lines
   //   (where g is a pointer to the TGraph):
   //
   //   Root > TPaveStats *st = (TPaveStats*)g->GetListOfFunctions()->FindObject("stats")
   //   Root > st->SetX1NDC(newx1); //new x start position
   //   Root > st->SetX2NDC(newx2); //new x end position
   //
   //      Access to the fit status
   //      ========================
   //   The function return the status of the fit (fitResult) in the following form
   //     fitResult = migradResult + 10*minosResult + 100*hesseResult + 1000*improveResult
   //   The fitResult is 0 is the fit is OK.
   //   The fitResult is negative in case of an error not connected with the fit.
   
   Int_t fitResult = 0;
   Double_t xmin, xmax, ymin, ymax;
   Int_t i, npar,nvpar,nparx, ndf;
   Double_t par, we, al, bl;
   Double_t eplus,eminus,eparab,globcc,amin,edm,errdef,werr;
   TF1 *fnew1;

   // Check validity of function
   if (!f1) {
      Error("Fit", "function may not be null pointer");
      return -1;
   }
   if (f1->IsZombie()) {
      Error("Fit", "function is zombie");
      return -2;
   }

   npar = f1->GetNpar();
   if (npar <= 0) {
      Error("Fit", "function %s has illegal number of parameters = %d", f1->GetName(), npar);
      return -3;
   }

   // Check that function has same dimension as graph
   if (f1->GetNdim() > 1) {
      Error("Fit", "function %s is not 1-D", f1->GetName());
      return -4;
   }
   //}

   Double_t *arglist = new Double_t[100];

   // Decode string choptin and fill fitOption structure
   Foption_t fitOption;
   Double_t h=0;
   TString opt = option;
   opt.ToUpper();
   opt.ReplaceAll("ROB", "H");
   //for robust fitting, see if # of good points is defined
   if (opt.Contains("H=0.")) {
      int start = opt.Index("H=0.");
      int numpos = start + strlen("H=0.");
      int numlen = 0;
      int len = opt.Length();
      while( (numpos+numlen<len) && isdigit(opt[numpos+numlen]) ) numlen++;
      TString num = opt(numpos,numlen);
      opt.Remove(start+strlen("H"),strlen("=0.")+numlen);
      h = atof(num.Data());
      h*=TMath::Power(10, -numlen);
   }

   if (opt.Contains("U")) fitOption.User    = 1;
   if (opt.Contains("Q")) fitOption.Quiet   = 1;
   if (opt.Contains("V")){fitOption.Verbose = 1; fitOption.Quiet   = 0;}
   if (opt.Contains("W")) fitOption.W1      = 1;
   if (opt.Contains("E")) fitOption.Errors  = 1;
   if (opt.Contains("R")) fitOption.Range   = 1;
   if (opt.Contains("N")) fitOption.Nostore = 1;
   if (opt.Contains("0")) fitOption.Nograph = 1;
   if (opt.Contains("+")) fitOption.Plus    = 1;
   if (opt.Contains("B")) fitOption.Bound   = 1;
   if (opt.Contains("C")) fitOption.Nochisq = 1;
   if (opt.Contains("F")) fitOption.Minuit  = 1;
   if (opt.Contains("H")) fitOption.Robust  = 1; 

   xmin    = fX[0];
   xmax    = fX[fNpoints-1];
   ymin    = fY[0];
   ymax    = fY[fNpoints-1];
   Double_t err0 = GetErrorX(0);
   Double_t errn = GetErrorX(fNpoints-1);
   if (err0 > 0) xmin -= 2*err0;
   if (errn > 0) xmax += 2*errn;
   for (i=0;i<fNpoints;i++) {
      if (fX[i] < xmin) xmin = fX[i];
      if (fX[i] > xmax) xmax = fX[i];
      if (fY[i] < ymin) ymin = fY[i];
      if (fY[i] > ymax) ymax = fY[i];
   }
   if (rxmax > rxmin) {
      xmin = rxmin;
      xmax = rxmax;
   }

   // Set the fitter

   Int_t special = f1->GetNumber();
   Bool_t linear = f1->IsLinear();
   if (special == 299 + npar)
      linear = kTRUE;
   if (fitOption.Bound || fitOption.User || fitOption.Errors || fitOption.Minuit)
      linear = kFALSE;

   char l[] = "TLinearFitter";
   Int_t strdiff = 0;
   Bool_t isSet = kFALSE;
   if (TVirtualFitter::GetFitter()){
      //Is a fitter already set? Is it linear?
      isSet = kTRUE;
      strdiff = strcmp(TVirtualFitter::GetFitter()->IsA()->GetName(), l);
   }
   if (linear){
      //
      TClass *cl = TClass::GetClass("TLinearFitter");
      if (isSet && strdiff!=0) {
         delete TVirtualFitter::GetFitter();
         isSet = kFALSE;
      }
      if (!isSet && cl) {
         TVirtualFitter::SetFitter((TVirtualFitter *)cl->New());
      }
   } else {
      if (isSet && strdiff==0){
         delete TVirtualFitter::GetFitter();
         isSet = kFALSE;
      }
      if (!isSet)
         TVirtualFitter::SetFitter(0);
   }
   TVirtualFitter *grFitter = TVirtualFitter::Fitter(this, f1->GetNpar());
   grFitter->Clear();

   // Get pointer to the function by searching in the list of functions in ROOT
   grFitter->SetUserFunc(f1);
   grFitter->SetFitOption(fitOption);

   // Is a Fit range specified?

   if (fitOption.Range) {
      f1->GetRange(xmin, xmax);
   } else {
      f1->SetRange(xmin, xmax);
   }

   //*-*- Compute sum of squares of errors in the bin range
   Bool_t hasErrors = kFALSE;
   Double_t ex, ey, sumw2=0;
   for (i=0;i<fNpoints;i++) {
      if(fX[i]<xmin || fX[i]>xmax) continue;
      ex = GetErrorX(i);
      ey = GetErrorY(i);
      if (ex > 0 || ey > 0) hasErrors = kTRUE;
      sumw2 += ey*ey;
   }

   // If case of a predefined function, then compute initial values of parameters

   if (linear){
      if (fitOption.Robust)
         fitResult = grFitter->ExecuteCommand("FitGraph", &h, 0);
      else
         fitResult = grFitter->ExecuteCommand("FitGraph", 0, 0);
      amin = f1->GetChisquare();
      ndf  = f1->GetNDF();
   } else {

      //Int_t special = f1->GetNumber();
      if (fitOption.Bound) special = 0;
      if      (special == 100)      InitGaus(xmin,xmax);
      else if (special == 400)      InitGaus(xmin,xmax);
      else if (special == 200)      InitExpo(xmin,xmax);
      else if (special == 299+npar) InitPolynom(xmin,xmax);

      //*-*- Some initialisations
      if (!fitOption.Verbose) {
         arglist[0] = -1;
         grFitter->ExecuteCommand("SET PRINT", arglist,1);
         arglist[0] = 0;
         grFitter->ExecuteCommand("SET NOW",   arglist,0);
      }
      /////////////////////////////////////////////////////////
      // Set error criterion for chisquare
      arglist[0] = TVirtualFitter::GetErrorDef();
      if (!fitOption.User) grFitter->SetFitMethod("GraphFitChisquare");
      fitResult = grFitter->ExecuteCommand("SET err",arglist,1);
      if (fitResult != 0) {
         //   Abnormal termination, MIGRAD might not have converged on a
         //   minimum.
         if (!fitOption.Quiet) {
            Warning("Fit","Abnormal termination of minimization.");
         }
         delete [] arglist;
         return fitResult;
      }

      // Transfer names and initial values of parameters to Minuit
      Int_t nfixed = 0;
      for (i=0;i<npar;i++) {
         par = f1->GetParameter(i);
         f1->GetParLimits(i,al,bl);
         if (al*bl != 0 && al >= bl) {
            al = bl = 0;
            arglist[nfixed] = i+1;
            nfixed++;
         }
         we  = 0.3*TMath::Abs(par);
         if (we <= TMath::Abs(par)*1e-6) we = 1;
         grFitter->SetParameter(i,f1->GetParName(i),par,we,al,bl);
      }
      if(nfixed > 0)grFitter->ExecuteCommand("FIX",arglist,nfixed); // Otto

      // Reset Print level
      if (!fitOption.Quiet) {
         if (fitOption.Verbose) { arglist[0] = 2; grFitter->ExecuteCommand("SET PRINT", arglist,1); }
         else                   { arglist[0] = 0; grFitter->ExecuteCommand("SET PRINT", arglist,1); }
      }

      // Perform minimization

      arglist[0] = TVirtualFitter::GetMaxIterations();
      arglist[1] = sumw2*TVirtualFitter::GetPrecision();
      fitResult += grFitter->ExecuteCommand("MIGRAD",arglist,2);
      if (fitOption.Errors) {
         fitResult += 100*grFitter->ExecuteCommand("HESSE",arglist,0);
         fitResult +=  10*grFitter->ExecuteCommand("MINOS",arglist,0);

      }

      grFitter->GetStats(amin,edm,errdef,nvpar,nparx);
      f1->SetChisquare(amin);
      ndf = f1->GetNumberFitPoints()-npar+nfixed;
      f1->SetNDF(ndf);

      //////////////////////////////////////

      //*-*- Get return status
      char parName[50];
      for (i=0;i<npar;i++) {
         grFitter->GetParameter(i,parName, par,we,al,bl);
         if (!fitOption.Errors) werr = we;
         else {
            grFitter->GetErrors(i,eplus,eminus,eparab,globcc);
            if (eplus > 0 && eminus < 0) werr = 0.5*(eplus-eminus);
            else                         werr = we;
         }
         f1->SetParameter(i,par);
         f1->SetParError(i,werr);
      }
   }

   //In case the graph has no errors (eg TGraph) we can compute
   //a better parameter error estimation using the chisquare and ndf
   for (i=0;i<npar;i++) {
      if (!hasErrors && ndf > 1) {
         werr  = f1->GetParError(i);
         werr *= TMath::Sqrt(amin/(ndf-1));
         f1->SetParError(i,werr);
      }  
   }  
    
   // Print final values of parameters.
   if (!fitOption.Quiet) {
      if (fitOption.Errors) grFitter->PrintResults(4,amin);
      else                  grFitter->PrintResults(3,amin);
   }
   delete [] arglist;


   // Store fitted function in histogram functions list and draw
   if (!fitOption.Nostore) {
      if (!fFunctions) fFunctions = new TList;
      if (!fitOption.Plus) {
         TIter next(fFunctions, kIterBackward);
         TObject *obj;
         while ((obj = next())) {
            if (obj->InheritsFrom(TF1::Class())) delete obj;
         }
      }
      fnew1 = new TF1();
      f1->Copy(*fnew1);
      fFunctions->Add(fnew1);
      fnew1->SetParent(this);
      fnew1->Save(xmin,xmax,0,0,0,0);
      if (fitOption.Nograph) fnew1->SetBit(TF1::kNotDraw);
      fnew1->SetBit(TFormula::kNotGlobal);

      if (TestBit(kCanDelete)) return fitResult;
      if (gPad) gPad->Modified();
   }
   return fitResult;
}


//______________________________________________________________________________
void TGraph::FitPanel()
{
   // Display a GUI panel with all graph fit options.
   //
   //   See class TFitEditor for example

   if (!gPad) {
      Error("FitPanel", "need to draw graph first");
      return;
   }

   if (!TClass::GetClass("TFitEditor")) gSystem->Load("libFitPanel");
   gROOT->ProcessLine(Form("TFitEditor::Open((TVirtualPad*)0x%x,(TObject*)0x%x)",gPad,this));
}


//______________________________________________________________________________
Double_t TGraph::GetCorrelationFactor() const
{
   // Return graph correlation factor

   Double_t rms1 = GetRMS(1);
   if (rms1 == 0) return 0;
   Double_t rms2 = GetRMS(2);
   if (rms2 == 0) return 0;
   return GetCovariance()/rms1/rms2;
}


//______________________________________________________________________________
Double_t TGraph::GetCovariance() const
{
   // Return covariance of vectors x,y

   if (fNpoints <= 0) return 0;
   Double_t sum = fNpoints, sumx = 0, sumy = 0, sumxy = 0;

   for (Int_t i=0;i<fNpoints;i++) {
      sumx  += fX[i];
      sumy  += fY[i];
      sumxy += fX[i]*fY[i];
   }
   return sumxy/sum - sumx/sum*sumy/sum;
}


//______________________________________________________________________________
Double_t TGraph::GetMean(Int_t axis) const
{
   // Return mean value of X (axis=1)  or Y (axis=2)

   if (axis < 1 || axis > 2) return 0;
   if (fNpoints <= 0) return 0;
   Double_t sumx = 0;
   for (Int_t i=0;i<fNpoints;i++) {
      if (axis == 1) sumx += fX[i];
      else           sumx += fY[i];
   }
   return sumx/fNpoints;
}


//______________________________________________________________________________
Double_t TGraph::GetRMS(Int_t axis) const
{
   // Return RMS of X (axis=1)  or Y (axis=2)

   if (axis < 1 || axis > 2) return 0;
   if (fNpoints <= 0) return 0;
   Double_t sumx = 0, sumx2 = 0;
   for (Int_t i=0;i<fNpoints;i++) {
      if (axis == 1) {sumx += fX[i]; sumx2 += fX[i]*fX[i];}
      else           {sumx += fY[i]; sumx2 += fY[i]*fY[i];}
   }
   Double_t x = sumx/fNpoints;
   Double_t rms2 = TMath::Abs(sumx2/fNpoints -x*x);
   return TMath::Sqrt(rms2);
}


//______________________________________________________________________________
Double_t TGraph::GetErrorX(Int_t) const
{
   // This function is called by GraphFitChisquare.
   // It always returns a negative value. Real implementation in TGraphErrors

   return -1;
}


//______________________________________________________________________________
Double_t TGraph::GetErrorY(Int_t) const
{
   // This function is called by GraphFitChisquare.
   // It always returns a negative value. Real implementation in TGraphErrors

   return -1;
}


//______________________________________________________________________________
Double_t TGraph::GetErrorXhigh(Int_t) const
{
   // This function is called by GraphFitChisquare.
   // It always returns a negative value. Real implementation in TGraphErrors
   // and TGraphAsymmErrors

   return -1;
}


//______________________________________________________________________________
Double_t TGraph::GetErrorXlow(Int_t) const
{
   // This function is called by GraphFitChisquare.
   // It always returns a negative value. Real implementation in TGraphErrors
   // and TGraphAsymmErrors

   return -1;
}


//______________________________________________________________________________
Double_t TGraph::GetErrorYhigh(Int_t) const
{
   // This function is called by GraphFitChisquare.
   // It always returns a negative value. Real implementation in TGraphErrors
   // and TGraphAsymmErrors

   return -1;
}


//______________________________________________________________________________
Double_t TGraph::GetErrorYlow(Int_t) const
{
   // This function is called by GraphFitChisquare.
   // It always returns a negative value. Real implementation in TGraphErrors
   // and TGraphAsymmErrors

   return -1;
}


//______________________________________________________________________________
TF1 *TGraph::GetFunction(const char *name) const
{
   // Return pointer to function with name.
   //
   // Functions such as TGraph::Fit store the fitted function in the list of
   // functions of this graph.

   if (!fFunctions) return 0;
   return (TF1*)fFunctions->FindObject(name);
}


//______________________________________________________________________________
TH1F *TGraph::GetHistogram() const
{
   // Returns a pointer to the histogram used to draw the axis
   // Takes into account the two following cases.
   //    1- option 'A' was specified in TGraph::Draw. Return fHistogram
   //    2- user had called TPad::DrawFrame. return pointer to hframe histogram

   if (fHistogram) return fHistogram;
   Double_t rwxmin,rwxmax, rwymin, rwymax, maximum, minimum, dx, dy;
   Double_t uxmin, uxmax;
   if (fNpoints <= 0) {
      rwxmin = rwxmax = 0;
      rwymin = rwymax = 0;
   } else {
      rwxmin = rwxmax = fX[0];
      rwymin = rwymax = fY[0];
   }
   for (Int_t i=1;i<fNpoints;i++) {
      if (fX[i] < rwxmin) rwxmin = fX[i];
      if (fX[i] > rwxmax) rwxmax = fX[i];
      if (fY[i] < rwymin) rwymin = fY[i];
      if (fY[i] > rwymax) rwymax = fY[i];
   }

   ComputeRange(rwxmin, rwymin, rwxmax, rwymax);  //this is redefined in TGraphErrors

   if (rwxmin == rwxmax) rwxmax += 1.;
   if (rwymin == rwymax) rwymax += 1.;
   dx = 0.1*(rwxmax-rwxmin);
   dy = 0.1*(rwymax-rwymin);
   uxmin    = rwxmin - dx;
   uxmax    = rwxmax + dx;
   minimum  = rwymin - dy;
   maximum  = rwymax + dy;
   if (fMinimum != -1111) minimum = fMinimum;
   if (fMaximum != -1111) maximum = fMaximum;

   // the graph is created with at least as many channels as there are points
   // to permit zooming on the full range
   if (uxmin < 0 && rwxmin >= 0) {
      if (gPad && gPad->GetLogx()) uxmin = 0.9*rwxmin;
      else                 uxmin = 0;
   }
   if (uxmax > 0 && rwxmax <= 0) {
      if (gPad && gPad->GetLogx()) uxmax = 1.1*rwxmax;
      else                 uxmax = 0;
   }
   if (minimum < 0 && rwymin >= 0) {
      if(gPad && gPad->GetLogy()) minimum = 0.9*rwymin;
      else                minimum = 0;
   }
   if (minimum <= 0 && gPad && gPad->GetLogy()) minimum = 0.001*maximum;
   if (uxmin <= 0 && gPad && gPad->GetLogx()) {
      if (uxmax > 1000) uxmin = 1;
      else              uxmin = 0.001*uxmax;
   }

   rwxmin = uxmin;
   rwxmax = uxmax;
   Int_t npt = 100;
   if (fNpoints > npt) npt = fNpoints;
   const char *gname = GetName();
   if (strlen(gname) == 0) gname = "Graph";
   ((TGraph*)this)->fHistogram = new TH1F(gname,GetTitle(),npt,rwxmin,rwxmax);
   if (!fHistogram) return 0;
   fHistogram->SetMinimum(minimum);
   fHistogram->SetBit(TH1::kNoStats);
   fHistogram->SetMaximum(maximum);
   fHistogram->GetYaxis()->SetLimits(minimum,maximum);
   fHistogram->SetDirectory(0);
   return fHistogram;
}


//______________________________________________________________________________
void TGraph::GetPoint(Int_t i, Double_t &x, Double_t &y) const
{
   // Get x and y values for point number i.

   if (i < 0 || i >= fNpoints) return;
   if (!fX || !fY) return;
   x = fX[i];
   y = fY[i];
}


//______________________________________________________________________________
TAxis *TGraph::GetXaxis() const
{
   // Get x axis of the graph.

   //if (!gPad) return 0;
   TH1 *h = GetHistogram();
   if (!h) return 0;
   return h->GetXaxis();
}


//______________________________________________________________________________
TAxis *TGraph::GetYaxis() const
{
   // Get y axis of the graph.

   //if (!gPad) return 0;
   TH1 *h = GetHistogram();
   if (!h) return 0;
   return h->GetYaxis();
}


//______________________________________________________________________________
void TGraph::InitGaus(Double_t xmin, Double_t xmax)
{
   // Compute Initial values of parameters for a gaussian.

   Double_t allcha, sumx, sumx2, x, val, rms, mean;
   Int_t bin;
   const Double_t sqrtpi = 2.506628;

   // Compute mean value and RMS of the graph in the given range
   if (xmax <= xmin) {xmin = fX[0]; xmax = fX[fNpoints-1];}
   Int_t np = 0;
   allcha = sumx = sumx2 = 0;
   for (bin=0;bin<fNpoints;bin++) {
      x       = fX[bin];
      if (x < xmin || x > xmax) continue;
      np++;
      val     = fY[bin];
      sumx   += val*x;
      sumx2  += val*x*x;
      allcha += val;
   }
   if (np == 0 || allcha == 0) return;
   mean = sumx/allcha;
   rms  = TMath::Sqrt(sumx2/allcha - mean*mean);
   Double_t binwidx = TMath::Abs((xmax-xmin)/np);
   if (rms == 0) rms = 1;
   TVirtualFitter *grFitter = TVirtualFitter::GetFitter();
   TF1 *f1 = (TF1*)grFitter->GetUserFunc();
   f1->SetParameter(0,binwidx*allcha/(sqrtpi*rms));
   f1->SetParameter(1,mean);
   f1->SetParameter(2,rms);
   f1->SetParLimits(2,0,10*rms);
}


//______________________________________________________________________________
void TGraph::InitExpo(Double_t xmin, Double_t xmax)
{
   // Compute Initial values of parameters for an exponential.

   Double_t constant, slope;
   Int_t ifail;
   if (xmax <= xmin) {xmin = fX[0]; xmax = fX[fNpoints-1];}
   Int_t nchanx = fNpoints;

   LeastSquareLinearFit(-nchanx, constant, slope, ifail, xmin, xmax);

   TVirtualFitter *grFitter = TVirtualFitter::GetFitter();
   TF1 *f1 = (TF1*)grFitter->GetUserFunc();
   f1->SetParameter(0,constant);
   f1->SetParameter(1,slope);
}


//______________________________________________________________________________
void TGraph::InitPolynom(Double_t xmin, Double_t xmax)
{
   // Compute Initial values of parameters for a polynom.

   Double_t fitpar[25];

   TVirtualFitter *grFitter = TVirtualFitter::GetFitter();
   TF1 *f1 = (TF1*)grFitter->GetUserFunc();
   Int_t npar   = f1->GetNpar();
   if (xmax <= xmin) {xmin = fX[0]; xmax = fX[fNpoints-1];}

   LeastSquareFit(npar, fitpar, xmin, xmax);

   for (Int_t i=0;i<npar;i++) f1->SetParameter(i, fitpar[i]);
}


//______________________________________________________________________________
Int_t TGraph::InsertPoint()
{
   // Insert a new point at the mouse position

   Int_t px = gPad->GetEventX();
   Int_t py = gPad->GetEventY();

   //localize point where to insert
   Int_t ipoint = -2;
   Int_t i,d=0;
   // start with a small window (in case the mouse is very close to one point)
   for (i=0;i<fNpoints-1;i++) {
      d = DistancetoLine(px, py, gPad->XtoPad(fX[i]), gPad->YtoPad(fY[i]), gPad->XtoPad(fX[i+1]), gPad->YtoPad(fY[i+1]));
      if (d < 5) {ipoint = i+1; break;}
   }
   if (ipoint == -2) {
      //may be we are far from one point, try again with a larger window
      for (i=0;i<fNpoints-1;i++) {
         d = DistancetoLine(px, py, gPad->XtoPad(fX[i]), gPad->YtoPad(fY[i]), gPad->XtoPad(fX[i+1]), gPad->YtoPad(fY[i+1]));
         if (d < 10) {ipoint = i+1; break;}
      }
   }
   if (ipoint == -2) {
      //distinguish between first and last point
      Int_t dpx = px - gPad->XtoAbsPixel(gPad->XtoPad(fX[0]));
      Int_t dpy = py - gPad->YtoAbsPixel(gPad->XtoPad(fY[0]));
      if (dpx*dpx+dpy*dpy < 25) ipoint = 0;
      else                      ipoint = fNpoints;
   }
   Double_t **ps = ExpandAndCopy(fNpoints + 1, ipoint);
   CopyAndRelease(ps, ipoint, fNpoints++, ipoint + 1);

   // To avoid redefenitions in descendant classes
   FillZero(ipoint, ipoint + 1);

   fX[ipoint] = gPad->PadtoX(gPad->AbsPixeltoX(px));
   fY[ipoint] = gPad->PadtoY(gPad->AbsPixeltoY(py));
   gPad->Modified();
   return ipoint;
}


//______________________________________________________________________________
void TGraph::LeastSquareFit(Int_t m, Double_t *a, Double_t xmin, Double_t xmax)
{
   // Least squares lpolynomial fitting without weights.
   //
   //  m     number of parameters
   //  a     array of parameters
   //  first 1st point number to fit (default =0)
   //  last  last point number to fit (default=fNpoints-1)
   //
   //   based on CERNLIB routine LSQ: Translated to C++ by Rene Brun

   const Double_t zero = 0.;
   const Double_t one = 1.;
   const Int_t idim = 20;

   Double_t  b[400]        /* was [20][20] */;
   Int_t i, k, l, ifail;
   Double_t power;
   Double_t da[20], xk, yk;
   Int_t n = fNpoints;
   if (xmax <= xmin) {xmin = fX[0]; xmax = fX[fNpoints-1];}

   if (m <= 2) {
      LeastSquareLinearFit(n, a[0], a[1], ifail, xmin, xmax);
      return;
   }
   if (m > idim || m > n) return;
   da[0] = zero;
   for (l = 2; l <= m; ++l) {
      b[l-1]           = zero;
      b[m + l*20 - 21] = zero;
      da[l-1]          = zero;
   }
   Int_t np = 0;
   for (k = 0; k < fNpoints; ++k) {
      xk     = fX[k];
      if (xk < xmin || xk > xmax) continue;
      np++;
      yk     = fY[k];
      power  = one;
      da[0] += yk;
      for (l = 2; l <= m; ++l) {
         power   *= xk;
         b[l-1]  += power;
         da[l-1] += power*yk;
      }
      for (l = 2; l <= m; ++l) {
         power            *= xk;
         b[m + l*20 - 21] += power;
      }
   }
   b[0]  = Double_t(np);
   for (i = 3; i <= m; ++i) {
      for (k = i; k <= m; ++k) {
         b[k - 1 + (i-1)*20 - 21] = b[k + (i-2)*20 - 21];
      }
   }
   H1LeastSquareSeqnd(m, b, idim, ifail, 1, da);

   if (ifail < 0) {
      a[0] = fY[0];
      for (i=1; i<m; ++i) a[i] = 0;
      return;
   }
   for (i=0; i<m; ++i) a[i] = da[i];
}


//______________________________________________________________________________
void TGraph::LeastSquareLinearFit(Int_t ndata, Double_t &a0, Double_t &a1, Int_t &ifail, Double_t xmin, Double_t xmax)
{
   // Least square linear fit without weights.
   //
   //  Fit a straight line (a0 + a1*x) to the data in this graph.
   //  ndata:  if ndata<0, fits the logarithm of the graph (used in InitExpo() to set
   //          the initial parameter values for a fit with exponential function.
   //  a0:     constant
   //  a1:     slope
   //  ifail:  return parameter indicating the status of the fit (ifail=0, fit is OK)
   //  xmin, xmax: fitting range
   //
   //  extracted from CERNLIB LLSQ: Translated to C++ by Rene Brun

   Double_t xbar, ybar, x2bar;
   Int_t i;
   Double_t xybar;
   Double_t fn, xk, yk;
   Double_t det;
   if (xmax <= xmin) {xmin = fX[0]; xmax = fX[fNpoints-1];}

   ifail = -2;
   xbar  = ybar = x2bar = xybar = 0;
   Int_t np = 0;
   for (i = 0; i < fNpoints; ++i) {
      xk = fX[i];
      if (xk < xmin || xk > xmax) continue;
      np++;
      yk = fY[i];
      if (ndata < 0) {
         if (yk <= 0) yk = 1e-9;
         yk = TMath::Log(yk);
      }
      xbar  += xk;
      ybar  += yk;
      x2bar += xk*xk;
      xybar += xk*yk;
   }
   fn    = Double_t(np);
   det   = fn*x2bar - xbar*xbar;
   ifail = -1;
   if (det <= 0) {
      if (fn > 0) a0 = ybar/fn;
      else        a0 = 0;
      a1 = 0;
      return;
   }
   ifail = 0;
   a0 = (x2bar*ybar - xbar*xybar) / det;
   a1 = (fn*xybar - xbar*ybar) / det;
}


//______________________________________________________________________________
void TGraph::Paint(Option_t *option)
{
   // Draw this graph with its current attributes.

   if (strstr(option,"H") || strstr(option,"h")) {
      PaintGrapHist(fNpoints, fX, fY, option);
   } else {
      PaintGraph(fNpoints, fX, fY, option);
   }

   //paint associated objects in the list of functions
   if (!fFunctions) return;
   TObjOptLink *lnk = (TObjOptLink*)fFunctions->FirstLink();
   TObject *obj;

   while (lnk) {
      obj = lnk->GetObject();
      TVirtualPad *padsave = gPad;
      if (obj->InheritsFrom(TF1::Class())) {
         if (obj->TestBit(TF1::kNotDraw) == 0) obj->Paint("lsame");
      } else  {
         obj->Paint(lnk->GetOption());
      }
      lnk = (TObjOptLink*)lnk->Next();
      padsave->cd();
   }
}


//______________________________________________________________________________
void TGraph::PaintFit(TF1 *fit)
{
   //  Paint "stats" box with the fit info

   Int_t dofit;
   TPaveStats *stats  = 0;
   TIter next(fFunctions);
   TObject *obj;
   while ((obj = next())) {
      if (obj->InheritsFrom(TPaveStats::Class())) {
         stats = (TPaveStats*)obj;
         break;
      }
   }

   if (stats) dofit  = stats->GetOptFit();
   else       dofit  = gStyle->GetOptFit();

   if (!dofit) fit = 0;
   if (!fit) return;
   if (dofit  == 1) dofit  =  111;
   Int_t nlines = 0;
   Int_t print_fval    = dofit%10;
   Int_t print_ferrors = (dofit/10)%10;
   Int_t print_fchi2   = (dofit/100)%10;
   Int_t print_fprob   = (dofit/1000)%10;
   Int_t nlinesf = print_fval + print_fchi2 + print_fprob;
   if (fit) nlinesf += fit->GetNpar();
   Bool_t done = kFALSE;
   Double_t  statw  = 1.8*gStyle->GetStatW();
   Double_t  stath  = 0.25*(nlines+nlinesf)*gStyle->GetStatH();
   if (stats) {
      stats->Clear();
      done = kTRUE;
   } else {
      stats  = new TPaveStats(
               gStyle->GetStatX()-statw,
               gStyle->GetStatY()-stath,
               gStyle->GetStatX(),
               gStyle->GetStatY(),"brNDC");

      stats->SetParent(fFunctions);
      stats->SetOptFit(dofit);
      stats->SetOptStat(0);
      stats->SetFillColor(gStyle->GetStatColor());
      stats->SetFillStyle(gStyle->GetStatStyle());
      stats->SetBorderSize(gStyle->GetStatBorderSize());
      stats->SetTextFont(gStyle->GetStatFont());
      if (gStyle->GetStatFont()%10 > 2)
         stats->SetTextSize(gStyle->GetStatFontSize());
      stats->SetFitFormat(gStyle->GetFitFormat());
      stats->SetStatFormat(gStyle->GetStatFormat());
      stats->SetName("stats");

      stats->SetTextColor(gStyle->GetStatTextColor());
      stats->SetTextAlign(12);
      stats->SetBit(kCanDelete);
      stats->SetBit(kMustCleanup);
   }

   char t[64];
   char textstats[50];
   Int_t ndf = fit->GetNDF();
   sprintf(textstats,"#chi^{2} / ndf = %s%s / %d","%",stats->GetFitFormat(),ndf);
   sprintf(t,textstats,(Float_t)fit->GetChisquare());
   if (print_fchi2) stats->AddText(t);
   if (print_fprob) {
      sprintf(textstats,"Prob  = %s%s","%",stats->GetFitFormat());
      sprintf(t,textstats,(Float_t)TMath::Prob(fit->GetChisquare(),ndf));
      stats->AddText(t);
   }
   if (print_fval || print_ferrors) {
      for (Int_t ipar=0;ipar<fit->GetNpar();ipar++) {
         if (print_ferrors) {
            sprintf(textstats,"%-8s = %s%s #pm %s%s ",fit->GetParName(ipar),"%",stats->GetFitFormat(),"%",stats->GetFitFormat());
            sprintf(t,textstats,(Float_t)fit->GetParameter(ipar)
                            ,(Float_t)fit->GetParError(ipar));
         } else {
            sprintf(textstats,"%-8s = %s%s ",fit->GetParName(ipar),"%",stats->GetFitFormat());
            sprintf(t,textstats,(Float_t)fit->GetParameter(ipar));
         }
         t[63] = 0;
         stats->AddText(t);
      }
   }

   if (!done) fFunctions->Add(stats);
   stats->Paint();
}


//______________________________________________________________________________
void TGraph::PaintGraph(Int_t npoints, const Double_t *x, const Double_t *y, Option_t *chopt)
{
   // Control function to draw a graph.
   //
   //  Draws one dimensional graphs. The aspect of the graph is done
   //  according to the value of the chopt.
   //
   //  Input parameters:
   //
   //  npoints : Number of points in X or in Y.
   //  x[npoints] or x[2] : X coordinates or (XMIN,XMAX) (WC space).
   //  y[npoints] or y[2] : Y coordinates or (YMIN,YMAX) (WC space).
   //  chopt : Option.
   //
   //  chopt='L' :  A simple polyline between every points is drawn
   //
   //  chopt='F' :  A fill area is drawn ('CF' draw a smooth fill area)
   //
   //  chopt='A' :  Axis are drawn around the graph
   //
   //  chopt='C' :  A smooth Curve is drawn
   //
   //  chopt='*' :  A Star is plotted at each point
   //
   //  chopt='P' :  Idem with the current marker
   //
   //  chopt='B' :  A Bar chart is drawn at each point
   //
   //  chopt='1' :  ylow=rwymin
   //
   //  chopt='X+' : The X-axis is drawn on the top side of the plot.
   //
   //  chopt='Y+' : The Y-axis is drawn on the right side of the plot.
   //
   // When a graph is painted with the option "C" or "L" it is possible to draw
   // a filled area on one side of the line. This is useful to show exclusion
   // zones. This drawing mode is activated when the absolute value of the
   // graph line width (set thanks to SetLineWidth) is greater than 99. In that
   // case the line width number is interpreted as 100*ff+ll = ffll . The two
   // digits number "ll" represent the normal line width whereas "ff" is the
   // filled area width. The sign of "ffll" allows to flip the filled area
   // from one side of the line to the other. The current fill area attributes
   // are used to draw the hatched zone.


   Int_t optionLine , optionAxis , optionCurve, optionStar , optionMark;
   Int_t optionBar  , optionR    , optionOne  , optionE;
   Int_t optionFill , optionZ    , optionCurveFill;
   Int_t i, npt, nloop;
   Int_t drawtype=0;
   Double_t xlow, xhigh, ylow, yhigh;
   Double_t barxmin, barxmax, barymin, barymax;
   Double_t uxmin, uxmax;
   Double_t x1, xn, y1, yn;
   Double_t dbar, bdelta;

   if (npoints <= 0) {
      Error("PaintGraph", "illegal number of points (%d)", npoints);
      return;
   }
   TString opt = chopt;
   opt.ToUpper();
   opt.ReplaceAll("SAME","");

   if(opt.Contains("L")) optionLine = 1;  else optionLine = 0;
   if(opt.Contains("A")) optionAxis = 1;  else optionAxis = 0;
   if(opt.Contains("C")) optionCurve= 1;  else optionCurve= 0;
   if(opt.Contains("*")) optionStar = 1;  else optionStar = 0;
   if(opt.Contains("P")) optionMark = 1;  else optionMark = 0;
   if(opt.Contains("B")) optionBar  = 1;  else optionBar  = 0;
   if(opt.Contains("R")) optionR    = 1;  else optionR    = 0;
   if(opt.Contains("1")) optionOne  = 1;  else optionOne  = 0;
   if(opt.Contains("F")) optionFill = 1;  else optionFill = 0;
   if(opt.Contains("2") || opt.Contains("3") ||
      opt.Contains("4")) optionE = 1;  else optionE = 0;
   optionZ    = 0;

   // If no "drawing" option is selected and if chopt<>' ' nothing is done.
   if (optionLine+optionFill+optionCurve+optionStar+optionMark+optionBar+optionE == 0) {
      if (strlen(chopt) == 0)  optionLine=1;
      else   return;
   }

   if (optionStar) SetMarkerStyle(3);

   optionCurveFill = 0;
   if (optionCurve && optionFill) {
      optionCurveFill = 1;
      optionFill      = 0;
   }

   // Draw the Axis.
   Double_t rwxmin,rwxmax, rwymin, rwymax, maximum, minimum, dx, dy;
   if (optionAxis) {
      if (fHistogram) {
         rwxmin    = gPad->GetUxmin();
         rwxmax    = gPad->GetUxmax();
         rwymin    = gPad->GetUymin();
         rwymax    = gPad->GetUymax();
         minimum   = fHistogram->GetMinimumStored();
         maximum   = fHistogram->GetMaximumStored();
         if (minimum == -1111) { //this can happen after unzooming
            minimum = fHistogram->GetYaxis()->GetXmin();
            fHistogram->SetMinimum(minimum);
         }
         if (maximum == -1111) {
            maximum = fHistogram->GetYaxis()->GetXmax();
            fHistogram->SetMaximum(maximum);
         }
         uxmin     = gPad->PadtoX(rwxmin);
         uxmax     = gPad->PadtoX(rwxmax);
      } else {
         rwxmin = rwxmax = x[0];
         rwymin = rwymax = y[0];
         for (i=1;i<npoints;i++) {
            if (x[i] < rwxmin) rwxmin = x[i];
            if (x[i] > rwxmax) rwxmax = x[i];
            if (y[i] < rwymin) rwymin = y[i];
            if (y[i] > rwymax) rwymax = y[i];
         }

         ComputeRange(rwxmin, rwymin, rwxmax, rwymax);  //this is redefined in TGraphErrors

         if (rwxmin == rwxmax) rwxmax += 1.;
         if (rwymin == rwymax) rwymax += 1.;
         dx = 0.1*(rwxmax-rwxmin);
         dy = 0.1*(rwymax-rwymin);
         uxmin    = rwxmin - dx;
         uxmax    = rwxmax + dx;
         minimum  = rwymin - dy;
         maximum  = rwymax + dy;
      }
      if (fMinimum != -1111) rwymin = minimum = fMinimum;
      if (fMaximum != -1111) rwymax = maximum = fMaximum;
      if (uxmin < 0 && rwxmin >= 0) {
         if (gPad->GetLogx()) uxmin = 0.9*rwxmin;
         else                 uxmin = 0;
      }
      if (uxmax > 0 && rwxmax <= 0) {
         if (gPad->GetLogx()) uxmax = 1.1*rwxmax;
         else                 uxmax = 0;
      }
      if (minimum < 0 && rwymin >= 0) {
         if(gPad->GetLogy()) minimum = 0.9*rwymin;
         else                minimum = 0;
      }
      if (maximum > 0 && rwymax <= 0) {
         //if(gPad->GetLogy()) maximum = 1.1*rwymax;
         //else                maximum = 0;
      }
      if (minimum <= 0 && gPad->GetLogy()) minimum = 0.001*maximum;
      if (uxmin <= 0 && gPad->GetLogx()) {
         if (uxmax > 1000) uxmin = 1;
         else              uxmin = 0.001*uxmax;
      }
      rwymin = minimum;
      rwymax = maximum;

      // Create a temporary histogram and fill each channel with the
      // function value.
      char chopth[8] = " ";
      if (strstr(chopt,"x+")) strcat(chopth, "x+");
      if (strstr(chopt,"y+")) strcat(chopth, "y+");
      if (!fHistogram) {
         // the graph is created with at least as many channels as there are
         // points to permit zooming on the full range.
         rwxmin = uxmin;
         rwxmax = uxmax;
         npt = 100;
         if (fNpoints > npt) npt = fNpoints;
         fHistogram = new TH1F(Form("%s_h",GetName()),GetTitle(),npt,rwxmin,rwxmax);
         if (!fHistogram) return;
         fHistogram->SetMinimum(rwymin);
         fHistogram->SetMaximum(rwymax);
         fHistogram->GetYaxis()->SetLimits(rwymin,rwymax);
         fHistogram->SetBit(TH1::kNoStats);
         fHistogram->SetDirectory(0);
         fHistogram->Paint(chopth); // Draw histogram axis, title and grid
      } else {
         if (gPad->GetLogy()) {
            fHistogram->SetMinimum(rwymin);
            fHistogram->SetMaximum(rwymax);
            fHistogram->GetYaxis()->SetLimits(rwymin,rwymax);
         }
         fHistogram->Paint(chopth); // Draw histogram axis, title and grid
      }
   }

   // Set Clipping option
   gPad->SetBit(kClipFrame, TestBit(kClipFrame));

   TF1 *fit = 0;
   if (fFunctions) fit = (TF1*)fFunctions->First();
   TObject *f;
   if (fFunctions) {
      TIter   next(fFunctions);
      while ((f = (TObject*) next())) {
         if (f->InheritsFrom(TF1::Class())) {
            fit = (TF1*)f;
            break;
         }
      }
   }
   if (fit) PaintFit(fit);

   rwxmin   = gPad->GetUxmin();
   rwxmax   = gPad->GetUxmax();
   rwymin   = gPad->GetUymin();
   rwymax   = gPad->GetUymax();
   uxmin    = gPad->PadtoX(rwxmin);
   uxmax    = gPad->PadtoX(rwxmax);
   if (fHistogram) {
      maximum = fHistogram->GetMaximum();
      minimum = fHistogram->GetMinimum();
   } else {
      maximum = gPad->PadtoY(rwymax);
      minimum = gPad->PadtoY(rwymin);
   }

   // Set attributes
   TAttLine::Modify();
   TAttFill::Modify();
   TAttMarker::Modify();

   // Draw the graph with a polyline or a fill area
   gxwork  = new Double_t[2*npoints+10];
   gywork  = new Double_t[2*npoints+10];
   gxworkl = new Double_t[2*npoints+10];
   gyworkl = new Double_t[2*npoints+10];

   if (optionLine || optionFill) {
      x1    = x[0];
      xn    = x[npoints-1];
      y1    = y[0];
      yn    = y[npoints-1];
      nloop = npoints;
      if (optionFill && (xn != x1 || yn != y1)) nloop++;
      npt = 0;
      for (i=1;i<=nloop;i++) {
         if (i > npoints) {
            gxwork[npt] = gxwork[0];  gywork[npt] = gywork[0];
         } else {
            gxwork[npt] = x[i-1];      gywork[npt] = y[i-1];
            npt++;
         }
         if (i == nloop) {
            ComputeLogs(npt, optionZ);
            Int_t bord = gStyle->GetDrawBorder();
            if (optionR) {
               if (optionFill) {
                  gPad->PaintFillArea(npt,gyworkl,gxworkl);
                  if (bord) gPad->PaintPolyLine(npt,gyworkl,gxworkl);
               } else {
                  if (TMath::Abs(fLineWidth)>99) PaintPolyLineHatches(npt, gyworkl, gxworkl);
                  gPad->PaintPolyLine(npt,gyworkl,gxworkl);
               }
            }
            else {
               if (optionFill) {
                  gPad->PaintFillArea(npt,gxworkl,gyworkl);
                  if (bord) gPad->PaintPolyLine(npt,gxworkl,gyworkl);
               } else {
                  if (TMath::Abs(fLineWidth)>99) PaintPolyLineHatches(npt, gxworkl, gyworkl);
                  gPad->PaintPolyLine(npt,gxworkl,gyworkl);
               }
            }
            gxwork[0] = gxwork[npt-1];  gywork[0] = gywork[npt-1];
            npt      = 1;
         }
      }
   }

   // Draw the graph with a smooth Curve. Smoothing via Smooth
   if (optionCurve) {
      x1 = x[0];
      xn = x[npoints-1];
      y1 = y[0];
      yn = y[npoints-1];
      drawtype = 1;
      nloop = npoints;
      if (optionCurveFill) {
         drawtype += 1000;
         if (xn != x1 || yn != y1) nloop++;
      }
      if (!optionR) {
         npt = 0;
         for (i=1;i<=nloop;i++) {
            if (i > npoints) {
               gxwork[npt] = gxwork[0];  gywork[npt] = gywork[0];
            } else {
               gxwork[npt] = x[i-1];      gywork[npt] = y[i-1];
               npt++;
            }
            ComputeLogs(npt, optionZ);
            if (gyworkl[npt-1] < rwymin || gyworkl[npt-1] > rwymax) {
               if (npt > 2) {
                  ComputeLogs(npt, optionZ);
                  Smooth(npt,gxworkl,gyworkl,drawtype);
               }
               gxwork[0] = gxwork[npt-1]; gywork[0] = gywork[npt-1];
               npt=1;
               continue;
            }
         }
         if (npt > 1) {
            ComputeLogs(npt, optionZ);
            Smooth(npt,gxworkl,gyworkl,drawtype);
         }
      }
      else {
         drawtype += 10;
         npt    = 0;
         for (i=1;i<=nloop;i++) {
            if (i > npoints) {
               gxwork[npt] = gxwork[0];  gywork[npt] = gywork[0];
            } else {
               if (y[i-1] < minimum || y[i-1] > maximum) continue;
               if (x[i-1] < uxmin    || x[i-1] > uxmax)  continue;
               gxwork[npt] = x[i-1];      gywork[npt] = y[i-1];
               npt++;
            }
            ComputeLogs(npt, optionZ);
            if (gxworkl[npt-1] < rwxmin || gxworkl[npt-1] > rwxmax) {
               if (npt > 2) {
                  ComputeLogs(npt, optionZ);
                  Smooth(npt,gxworkl,gyworkl,drawtype);
               }
               gxwork[0] = gxwork[npt-1]; gywork[0] = gywork[npt-1];
               npt=1;
               continue;
            }
         }
         if (npt > 1) {
            ComputeLogs(npt, optionZ);
            Smooth(npt,gxworkl,gyworkl,drawtype);
         }
      }
   }

   // Draw the graph with a '*' on every points
   if (optionStar) {
      SetMarkerStyle(3);
      npt = 0;
      for (i=1;i<=npoints;i++) {
         if (y[i-1] >= minimum && y[i-1] <= maximum && x[i-1] >= uxmin  && x[i-1] <= uxmax) {
            gxwork[npt] = x[i-1];      gywork[npt] = y[i-1];
            npt++;
         }
         if (i == npoints) {
            ComputeLogs(npt, optionZ);
            if (optionR)  gPad->PaintPolyMarker(npt,gyworkl,gxworkl);
            else          gPad->PaintPolyMarker(npt,gxworkl,gyworkl);
            npt = 0;
         }
      }
   }

   // Draw the graph with the current polymarker on every points
   if (optionMark) {
      npt = 0;
      for (i=1;i<=npoints;i++) {
         if (y[i-1] >= minimum && y[i-1] <= maximum && x[i-1] >= uxmin  && x[i-1] <= uxmax) {
            gxwork[npt] = x[i-1];      gywork[npt] = y[i-1];
            npt++;
         }
         if (i == npoints) {
            ComputeLogs(npt, optionZ);
            if (optionR) gPad->PaintPolyMarker(npt,gyworkl,gxworkl);
            else         gPad->PaintPolyMarker(npt,gxworkl,gyworkl);
            npt = 0;
         }
      }
   }

   // Draw the graph as a bar chart
   if (optionBar) {
      if (!optionR) {
         barxmin = x[0];
         barxmax = x[0];
         for (i=1;i<npoints;i++) {
            if (x[i] < barxmin) barxmin = x[i];
            if (x[i] > barxmax) barxmax = x[i];
         }
         bdelta = (barxmax-barxmin)/Double_t(npoints);
      }
      else {
         barymin = y[0];
         barymax = y[0];
         for (i=1;i<npoints;i++) {
            if (y[i] < barymin) barymin = y[i];
            if (y[i] > barymax) barymax = y[i];
         }
         bdelta = (barymax-barymin)/Double_t(npoints);
      }
      dbar  = 0.5*bdelta*gStyle->GetBarWidth();
      if (!optionR) {
         for (i=1;i<=npoints;i++) {
            xlow  = x[i-1] - dbar;
            xhigh = x[i-1] + dbar;
            yhigh = y[i-1];
            if (xlow  < uxmin) continue;
            if (xhigh > uxmax) continue;
            if (!optionOne) ylow = TMath::Max((Double_t)0,gPad->GetUymin());
            else            ylow = gPad->GetUymin();
            gxwork[0] = xlow;
            gywork[0] = ylow;
            gxwork[1] = xhigh;
            gywork[1] = yhigh;
            ComputeLogs(2, optionZ);
            if (gyworkl[0] < gPad->GetUymin()) gyworkl[0] = gPad->GetUymin();
            if (gyworkl[1] < gPad->GetUymin()) continue;
            if (gyworkl[1] > gPad->GetUymax()) gyworkl[1] = gPad->GetUymax();
            if (gyworkl[0] > gPad->GetUymax()) continue;

            gPad->PaintBox(gxworkl[0],gyworkl[0],gxworkl[1],gyworkl[1]);
         }
      }
      else {
         for (i=1;i<=npoints;i++) {
            xhigh = x[i-1];
            ylow  = y[i-1] - dbar;
            yhigh = y[i-1] + dbar;
            xlow     = TMath::Max((Double_t)0, gPad->GetUxmin());
            gxwork[0] = xlow;
            gywork[0] = ylow;
            gxwork[1] = xhigh;
            gywork[1] = yhigh;
            ComputeLogs(2, optionZ);
            gPad->PaintBox(gxworkl[0],gyworkl[0],gxworkl[1],gyworkl[1]);
         }
      }
   }
   gPad->ResetBit(kClipFrame);

   delete [] gxwork;
   delete [] gywork;
   delete [] gxworkl;
   delete [] gyworkl;
}


//______________________________________________________________________________
void TGraph::PaintGrapHist(Int_t npoints, const Double_t *x, const Double_t *y, Option_t *chopt)
{
   // Control function to draw a graphistogram.
   //
   //   Draws one dimensional graphs. The aspect of the graph is done
   // according to the value of the chopt.
   //
   // Input parameters:
   //
   //  npoints : Number of points in X or in Y.
   //  X(N) or x[1] : X coordinates or (XMIN,XMAX) (WC space).
   //  Y(N) or y[1] : Y coordinates or (YMIN,YMAX) (WC space).
   //  chopt : Option.
   //
   //  chopt='R' :  Graph is drawn horizontaly, parallel to X axis.
   //               (default is vertically, parallel to Y axis)
   //               If option R is selected the user must give:
   //                 2 values for Y (y[0]=YMIN and y[1]=YMAX)
   //                 N values for X, one for each channel.
   //               Otherwise the user must give:
   //                 N values for Y, one for each channel.
   //                 2 values for X (x[0]=XMIN and x[1]=XMAX)
   //
   //  chopt='L' :  A simple polyline beetwen every points is drawn
   //
   //  chopt='H' :  An Histogram with equidistant bins is drawn
   //               as a polyline.
   //
   //  chopt='F' :  An histogram with equidistant bins is drawn
   //               as a fill area. Contour is not drawn unless
   //               chopt='H' is also selected..
   //
   //  chopt='N' :  Non equidistant bins (default is equidistant)
   //               If N is the number of channels array X and Y
   //               must be dimensionned as follow:
   //               If option R is not selected (default) then
   //               the user must give:
   //                 (N+1) values for X (limits of channels).
   //                  N values for Y, one for each channel.
   //               Otherwise the user must give:
   //                 (N+1) values for Y (limits of channels).
   //                  N values for X, one for each channel.
   //
   //  chopt='F1':  Idem as 'F' except that fill area is no more
   //               reparted arround axis X=0 or Y=0 .
   //
   //  chopt='F2':  Draw a Fill area polyline connecting the center of bins
   //
   //  chopt='C' :  A smooth Curve is drawn.
   //
   //  chopt='*' :  A Star is plotted at the center of each bin.
   //
   //  chopt='P' :  Idem with the current marker
   //  chopt='P0':  Idem with the current marker. Empty bins also drawn
   //
   //  chopt='B' :  A Bar chart with equidistant bins is drawn as fill
   //               areas (Contours are drawn).
   //
   //  chopt='9' :  Force graph to be drawn in high resolution mode.
   //               By default, the graph is drawn in low resolution
   //               in case the number of points is greater than the number of pixels
   //               in the current pad.
   //
   //  chopt='][' : "Cutoff" style. When this option is selected together with
   //               H option, the first and last vertical lines of the histogram
   //               are not drawn.

   const char *where = "PaintGraphHist";

   Int_t optionLine , optionAxis , optionCurve, optionStar , optionMark;
   Int_t optionBar  , optionRot  , optionOne  , optionOff;
   Int_t optionFill , optionZ;
   Int_t optionHist , optionBins , optionMarker;
   Int_t i, j, npt;
   Int_t drawtype=0, drawborder, drawbordersav;
   Double_t xlow, xhigh, ylow, yhigh;
   Double_t wmin, wmax;
   Double_t dbar, offset, wminstep;
   Double_t delta = 0;
   Double_t ylast = 0;
   Double_t xi, xi1, xj, xj1, yi1, yi, yj, yj1, xwmin, ywmin;
   Int_t first, last, nbins;
   Int_t fillarea;

   char choptaxis[10] = " ";

   if (npoints <= 0) {
      Error(where, "illegal number of points (%d)", npoints);
      return;
   }
   TString opt = chopt;
   opt.ToUpper();
   if(opt.Contains("H"))  optionHist = 1;  else optionHist = 0;
   if(opt.Contains("F"))  optionFill = 1;  else optionFill = 0;
   if(opt.Contains("C"))  optionCurve= 1;  else optionCurve= 0;
   if(opt.Contains("*"))  optionStar = 1;  else optionStar = 0;
   if(opt.Contains("R"))  optionRot  = 1;  else optionRot  = 0;
   if(opt.Contains("1"))  optionOne  = 1;  else optionOne  = 0;
   if(opt.Contains("B"))  optionBar  = 1;  else optionBar  = 0;
   if(opt.Contains("N"))  optionBins = 1;  else optionBins = 0;
   if(opt.Contains("L"))  optionLine = 1;  else optionLine = 0;
   if(opt.Contains("P"))  optionMark = 1;  else optionMark = 0;
   if(opt.Contains("A"))  optionAxis = 1;  else optionAxis = 0;
   if(opt.Contains("][")) optionOff  = 1;  else optionOff  = 0;
   if(opt.Contains("P0")) optionMark = 10;

   Int_t optionFill2 = 0;
   if(opt.Contains("F") && opt.Contains("2")) {
      optionFill = 0; optionFill2 = 1;
   }

   // Set Clipping option
   Option_t *noClip;
   if (TestBit(kClipFrame)) noClip = "";
   else                     noClip = "C";
   gPad->SetBit(kClipFrame, TestBit(kClipFrame));

   optionZ = 1;

   if (optionStar) SetMarkerStyle(3);

   first = 1;
   last  = npoints;
   nbins = last - first + 1;

   //           Draw the Axis with a fixed number of division: 510

   Double_t baroffset = gStyle->GetBarOffset();
   Double_t barwidth  = gStyle->GetBarWidth();
   Double_t rwxmin    = gPad->GetUxmin();
   Double_t rwxmax    = gPad->GetUxmax();
   Double_t rwymin    = gPad->GetUymin();
   Double_t rwymax    = gPad->GetUymax();
   Double_t uxmin     = gPad->PadtoX(rwxmin);
   Double_t uxmax     = gPad->PadtoX(rwxmax);
   Double_t rounding  = (uxmax-uxmin)*1.e-5;
   drawborder         = gStyle->GetDrawBorder();
   if (optionAxis) {
      Int_t nx1, nx2, ndivx, ndivy, ndiv;
      choptaxis[0]  = 0;
      Double_t rwmin  = rwxmin;
      Double_t rwmax  = rwxmax;
      ndivx = gStyle->GetNdivisions("X");
      ndivy = gStyle->GetNdivisions("Y");
      if (ndivx > 1000) {
         nx2   = ndivx/100;
         nx1   = TMath::Max(1, ndivx%100);
         ndivx = 100*nx2 + Int_t(Double_t(nx1)*gPad->GetAbsWNDC());
      }
      ndiv  =TMath::Abs(ndivx);
      if (ndivx < 0) strcat(choptaxis, "N");
      if (gPad->GetGridx()) {
         strcat(choptaxis, "W");
      }
      if (gPad->GetLogx()) {
         rwmin = TMath::Power(10,rwxmin);
         rwmax = TMath::Power(10,rwxmax);
         strcat(choptaxis, "G");
      }
      TGaxis *axis = new TGaxis();
      axis->SetLineColor(gStyle->GetAxisColor("X"));
      axis->SetTextColor(gStyle->GetLabelColor("X"));
      axis->SetTextFont(gStyle->GetLabelFont("X"));
      axis->SetLabelSize(gStyle->GetLabelSize("X"));
      axis->SetLabelOffset(gStyle->GetLabelOffset("X"));
      axis->SetTickSize(gStyle->GetTickLength("X"));

      axis->PaintAxis(rwxmin,rwymin,rwxmax,rwymin,rwmin,rwmax,ndiv,choptaxis);

      choptaxis[0]  = 0;
      rwmin  = rwymin;
      rwmax  = rwymax;
      if (ndivy < 0) {
         nx2   = ndivy/100;
         nx1   = TMath::Max(1, ndivy%100);
         ndivy = 100*nx2 + Int_t(Double_t(nx1)*gPad->GetAbsHNDC());
         strcat(choptaxis, "N");
      }
      ndiv  =TMath::Abs(ndivy);
      if (gPad->GetGridy()) {
         strcat(choptaxis, "W");
      }
      if (gPad->GetLogy()) {
         rwmin = TMath::Power(10,rwymin);
         rwmax = TMath::Power(10,rwymax);
         strcat(choptaxis,"G");
      }
      axis->SetLineColor(gStyle->GetAxisColor("Y"));
      axis->SetTextColor(gStyle->GetLabelColor("Y"));
      axis->SetTextFont(gStyle->GetLabelFont("Y"));
      axis->SetLabelSize(gStyle->GetLabelSize("Y"));
      axis->SetLabelOffset(gStyle->GetLabelOffset("Y"));
      axis->SetTickSize(gStyle->GetTickLength("Y"));

      axis->PaintAxis(rwxmin,rwymin,rwxmin,rwymax,rwmin,rwmax,ndiv,choptaxis);
      delete axis;
   }


   //           Set attributes
   TAttLine::Modify();
   TAttFill::Modify();
   TAttMarker::Modify();

   //       Min-Max scope

   if (!optionRot) {wmin = x[0];   wmax = x[1];}
   else            {wmin = y[0];   wmax = y[1];}

   if (!optionBins) delta = (wmax - wmin)/ Double_t(nbins);

   Int_t fwidth = gPad->GetFrameLineWidth();
   TFrame *frame = gPad->GetFrame();
   if (frame) fwidth = frame->GetLineWidth();
   Double_t dxframe = gPad->AbsPixeltoX(fwidth/2) - gPad->AbsPixeltoX(0);
   Double_t vxmin = gPad->PadtoX(gPad->GetUxmin() + dxframe);
   Double_t vxmax = gPad->PadtoX(gPad->GetUxmax() - dxframe);
   Double_t dyframe = -gPad->AbsPixeltoY(fwidth/2) + gPad->AbsPixeltoY(0);
   Double_t vymin = gPad->GetUymin() + dyframe; //y already in log scale
   vxmin = TMath::Max(vxmin,wmin);
   vxmax = TMath::Min(vxmax,wmax);

   //           Draw the histogram with a fill area

   gxwork  = new Double_t[2*npoints+10];
   gywork  = new Double_t[2*npoints+10];
   gxworkl = new Double_t[2*npoints+10];
   gyworkl = new Double_t[2*npoints+10];

   if (optionFill && !optionCurve) {
      fillarea = kTRUE;
      if (!optionRot) {
         gxwork[0] = vxmin;
         if (!optionOne) gywork[0] = TMath::Min(TMath::Max((Double_t)0,gPad->GetUymin())
                                               ,gPad->GetUymax());
         else            gywork[0] = gPad->GetUymin();
         npt = 2;
         for (j=first; j<=last;j++) {
            if (!optionBins) {
               gxwork[npt-1]   = gxwork[npt-2];
               gxwork[npt]     = wmin+((j-first+1)*delta);
               if (gxwork[npt] < gxwork[0]) gxwork[npt] = gxwork[0];

            }
            else {
               xj1 = x[j];      xj  = x[j-1];
               if (xj1 < xj) {
                  if (j != last) Error(where, "X must be in increasing order");
                  else           Error(where, "X must have N+1 values with option N");
                  return;
               }
               gxwork[npt-1] = x[j-1];       gxwork[npt] = x[j];
            }
            gywork[npt-1] = y[j-1];
            gywork[npt]   = y[j-1];
            if (gywork[npt] < vymin) {gywork[npt] = vymin; gywork[npt-1] = vymin;}
            if (gxwork[npt-1] >= uxmin-rounding && gxwork[npt] <= uxmax+rounding) npt += 2;
            else gxwork[npt-2] = TMath::Min(gxwork[npt], uxmax);
            if (j == last) {
               gxwork[npt-1] = gxwork[npt-2];
               gywork[npt-1] = gywork[0];
               //make sure that the fill area does not overwrite the frame
               //take into account the frame linewidth
               if (gxwork[0    ] < vxmin) {gxwork[0    ] = vxmin; gxwork[1    ] = vxmin;}
               if (gywork[0] < vymin) {gywork[0] = vymin; gywork[npt-1] = vymin;}

               //transform to log ?
               ComputeLogs(npt, optionZ);
               gPad->PaintFillArea(npt,gxworkl,gyworkl);
               if (drawborder) {
                  if (!fillarea) gyworkl[0] = ylast;
                  gPad->PaintPolyLine(npt-1,gxworkl,gyworkl,noClip);
               }
               continue;
            }
         }  //endfor (j=first; j<=last;j++) {
      }
      else {
         gywork[0] = wmin;
         if (!optionOne) gxwork[0] = TMath::Max((Double_t)0,gPad->GetUxmin());
         else            gxwork[0] = gPad->GetUxmin();
         npt = 2;
         for (j=first; j<=last;j++) {
            if (!optionBins) {
               gywork[npt-1] = gywork[npt-2];
               gywork[npt]   = wmin+((j-first+1)*delta);
            }
            else {
               yj1 = y[j];      yj  = y[j-1];
               if (yj1 < yj) {
                  if (j != last) Error(where, "Y must be in increasing order");
                  else           Error(where, "Y must have N+1 values with option N");
                  return;
               }
               gywork[npt-1] = y[j-1];       gywork[npt] = y[j];
            }
            gxwork[npt-1] = x[j-1];      gxwork[npt] = x[j-1];
            if (gxwork[npt-1] >= uxmin-rounding && gxwork[npt] <= uxmax+rounding) npt += 2;
            if (j == last) {
               gywork[npt-1] = gywork[npt-2];
               gxwork[npt-1] = gxwork[0];
               ComputeLogs(npt, optionZ);
               gPad->PaintFillArea(npt,gxworkl,gyworkl);
               if (drawborder) {
                  if (!fillarea) gyworkl[0] = ylast;
                  gPad->PaintPolyLine(npt-1,gxworkl,gyworkl,noClip);
               }
               continue;
            }
         }  //endfor (j=first; j<=last;j++)
      }
      TAttLine::Modify();
      TAttFill::Modify();
   }

   //      Draw a standard Histogram (default)

   if ((optionHist) || strlen(chopt) == 0) {
      if (!optionRot) {
         gxwork[0] = wmin;
         gywork[0] = gPad->GetUymin();
         ywmin    = gywork[0];
         npt      = 2;
         for (i=first; i<=last;i++) {
            if (!optionBins) {
               gxwork[npt-1] = gxwork[npt-2];
               gxwork[npt]   = wmin+((i-first+1)*delta);
            }
            else {
               xi1 = x[i];      xi  = x[i-1];
               if (xi1 < xi) {
                  if (i != last) Error(where, "X must be in increasing order");
                  else           Error(where, "X must have N+1 values with option N");
                  return;
               }
               gxwork[npt-1] = x[i-1];      gxwork[npt] = x[i];
            }
            gywork[npt-1] = y[i-1];
            gywork[npt]   = y[i-1];
            if (gywork[npt] < vymin) {gywork[npt] = vymin; gywork[npt-1] = vymin;}
            if (gxwork[npt-1] >= uxmin-rounding && gxwork[npt] <= uxmax+rounding) npt += 2;
            else gxwork[npt-2] = TMath::Min(gxwork[npt], uxmax);
            if (i == last) {
               gxwork[npt-1] = gxwork[npt-2];
               gywork[npt-1] = gywork[0];
               //make sure that the fill area does not overwrite the frame
               //take into account the frame linewidth
               if (gxwork[0    ] < vxmin) {gxwork[0    ] = vxmin; gxwork[1    ] = vxmin;}
               if (gywork[0] < vymin) {gywork[0] = vymin; gywork[npt-1] = vymin;}

               ComputeLogs(npt, optionZ);

               //do not draw the two vertical lines on the edges
               Int_t npoints = npt-2;
               Int_t point1  = 1;
               if (optionOff) {
                  // remove points before the low cutoff
                  Int_t ip;
                  for (ip=point1; ip<=npoints; ip++) {
                     if (gyworkl[ip] != ywmin) {
                        point1 = ip;
                        break;
                     }
                  }
                  // remove points after the high cutoff
                  Int_t point2 = npoints;
                  for (ip=point2; ip>=point1; ip--) {
                     if (gyworkl[ip] != ywmin) {
                        point2 = ip;
                        break;
                     }
                  }
                  npoints = point2-point1+1;
               }
               gPad->PaintPolyLine(npoints,&gxworkl[point1],&gyworkl[point1],noClip);
               continue;
            }
         }  //endfor (i=first; i<=last;i++)
      }
      else {
         gywork[0] = wmin;
         gxwork[0] = TMath::Max((Double_t)0,gPad->GetUxmin());
         xwmin    = gxwork[0];
         npt      = 2;
         for (i=first; i<=last;i++) {
            if (!optionBins) {
               gywork[npt-1]   = gywork[npt-2];
               gywork[npt] = wmin+((i-first+1)*delta);
            }
            else {
               yi1 = y[i];      yi  = y[i-1];
               if (yi1 < yi) {
                  if (i != last) Error(where, "Y must be in increasing order");
                  else           Error(where, "Y must have N+1 values with option N");
                  return;
               }
               gywork[npt-1] = y[i-1];      gywork[npt] = y[i];
            }
            gxwork[npt-1] = x[i-1];      gxwork[npt] = x[i-1];
            if (gxwork[npt-1] >= uxmin-rounding && gxwork[npt] <= uxmax+rounding) npt += 2;
            if (i == last) {
               gywork[npt-1] = gywork[npt-2];
               gxwork[npt-1] = xwmin;
               ComputeLogs(npt, optionZ);
               gPad->PaintPolyLine(npt,gxworkl,gyworkl,noClip);
               continue;
            }
         }  //endfor (i=first; i<=last;i++)
      }
   }

   //              Draw the histogram with a smooth Curve. The computing
   //              of the smoothing is done by the routine IGRAp1

   if (optionCurve) {
      if (!optionFill) drawtype = 1;
      else {
         if (!optionOne) drawtype = 2;
         else            drawtype = 3;
      }
      if (!optionRot) {
         npt = 0;
         for (i=first; i<=last;i++) {
            npt++;
            if (!optionBins) gxwork[npt-1] = wmin+(i-first)*delta+0.5*delta;
            else {
               xi1 = x[i];      xi  = x[i-1];
               if (xi1 < xi) {
                  if (i != last) Error(where, "X must be in increasing order");
                  else           Error(where, "X must have N+1 values with option N");
                  return;
               }
               gxwork[npt-1] = x[i-1] + 0.5*(x[i]-x[i-1]);
            }
            if (gxwork[npt-1] < uxmin || gxwork[npt-1] > uxmax) {
               npt--;
               continue;
            }
            gywork[npt-1] = y[i-1];
            ComputeLogs(npt, optionZ);
            if ((gyworkl[npt-1] < rwymin) || (gyworkl[npt-1] > rwymax)) {
               if (npt > 2) {
                  ComputeLogs(npt, optionZ);
                  Smooth(npt,gxworkl,gyworkl,drawtype);
               }
               gxwork[0] = gxwork[npt-1];
               gywork[0] = gywork[npt-1];
               npt      = 1;
               continue;
            }
            if (npt >= 50) {
               ComputeLogs(50, optionZ);
               Smooth(50,gxworkl,gyworkl,drawtype);
               gxwork[0] = gxwork[npt-1];
               gywork[0] = gywork[npt-1];
               npt      = 1;
            }
         }  //endfor (i=first; i<=last;i++)
         if (npt > 1) {
            ComputeLogs(npt, optionZ);
            Smooth(npt,gxworkl,gyworkl,drawtype);
         }
      }
      else {
         drawtype = drawtype+10;
         npt   = 0;
         for (i=first; i<=last;i++) {
            npt++;
            if (!optionBins) gywork[npt-1] = wmin+(i-first)*delta+0.5*delta;
            else {
               yi1 = y[i];      yi = y[i-1];
               if (yi1 < yi) {
                  if (i != last) Error(where, "Y must be in increasing order");
                  else           Error(where, "Y must have N+1 values with option N");
                  return;
               }
               gywork[npt-1] = y[i-1] + 0.5*(y[i]-y[i-1]);
            }
            gxwork[npt-1] = x[i-1];
            ComputeLogs(npt, optionZ);
            if ((gxworkl[npt] < uxmin) || (gxworkl[npt] > uxmax)) {
               if (npt > 2) {
                  ComputeLogs(npt, optionZ);
                  Smooth(npt,gxworkl,gyworkl,drawtype);
               }
               gxwork[0] = gxwork[npt-1];
               gywork[0] = gywork[npt-1];
               npt      = 1;
               continue;
            }
            if (npt >= 50) {
               ComputeLogs(50, optionZ);
               Smooth(50,gxworkl,gyworkl,drawtype);
               gxwork[0] = gxwork[npt-1];
               gywork[0] = gywork[npt-1];
               npt      = 1;
            }
         }  //endfor (i=first; i<=last;i++)
         if (npt > 1) {
            ComputeLogs(npt, optionZ);
            Smooth(npt,gxworkl,gyworkl,drawtype);
         }
      }
   }

   //    Draw the histogram with a simple line or/and a marker

   optionMarker = 0;
   if ((optionStar) || (optionMark))optionMarker=1;
   if ((optionMarker) || (optionLine)) {
      wminstep = wmin + 0.5*delta;
      Axis_t ax1,ax2,ay1,ay2;
      gPad->GetRangeAxis(ax1,ay1,ax2,ay2);
 
      Int_t ax1Pix = gPad->XtoAbsPixel(ax1);
      Int_t ax2Pix = gPad->XtoAbsPixel(ax2);
      Int_t ay1Pix = gPad->YtoAbsPixel(ay1);
      Int_t ay2Pix = gPad->YtoAbsPixel(ay2);
 
      Int_t nrPix;
      if (!optionRot)
         nrPix = ax2Pix-ax1Pix+1;
      else
         nrPix = ay2Pix-ay1Pix+1;

      // Make here decision whether it should be painted in high or low resolution
      Int_t ip, ipix, lowRes = 0;
      if (3*nrPix < last-first+1) {
         lowRes = 1;
      }
      if (optionFill2)       lowRes = 0;
      if (opt.Contains("9")) lowRes = 0;
      if (lowRes) {
         Double_t *minPix   = new Double_t[nrPix];
         Double_t *maxPix   = new Double_t[nrPix];
         Double_t *centrPix = new Double_t[nrPix];
         Int_t *nrEntries   = new Int_t[nrPix];

         for (ip = 0; ip < nrPix; ip++) {
            minPix[ip]    =  1e100;
            maxPix[ip]    = -1e100;
            nrEntries[ip] = 0;
         }

         for (ip = first; ip < last; ip++) {
            Double_t xw;
            if (!optionBins) xw = wmin + (ip-first)*delta+0.5*delta;
            else             xw = x[ip-1] + 0.5*(x[ip]-x[ip-1]);;

            if (!optionRot) {
               Int_t ix = gPad->XtoAbsPixel(gPad->XtoPad(xw))-ax1Pix;
               if (ix < 0) ix = 0;
               if (ix >= nrPix) ix = nrPix-1;
               Int_t yPixel = gPad->YtoAbsPixel(y[ip-1]);
               if (yPixel >= ay1Pix) continue;
               if (minPix[ix] > yPixel) minPix[ix] = yPixel;
               if (maxPix[ix] < yPixel) maxPix[ix] = yPixel;
               (nrEntries[ix])++;
            } else {
               Int_t iy = gPad->YtoAbsPixel(gPad->YtoPad(y[ip-1]))-ay1Pix;
               if (iy < 0) iy = 0;
               if (iy >= nrPix) iy = nrPix-1;;
               Int_t xPixel = gPad->XtoAbsPixel(gPad->XtoPad(xw));
               if (minPix[iy] > xPixel) minPix[iy] = xPixel;
               if (maxPix[iy] < xPixel) maxPix[iy] = xPixel;
               (nrEntries[iy])++;
            }
         }

         for (ipix = 0; ipix < nrPix; ipix++) {
            if (nrEntries[ipix] > 0)
               centrPix[ipix] = (minPix[ipix]+maxPix[ipix])/2.0;
            else
               centrPix[ipix] = 2*TMath::Max(TMath::Abs(minPix[ipix]),
                                             TMath::Abs(maxPix[ipix]));
         }

         Double_t *xc = new Double_t[nrPix];
         Double_t *yc = new Double_t[nrPix];

         Double_t xcadjust = 0.3*(gPad->AbsPixeltoX(ax1Pix+1) - gPad->AbsPixeltoX(ax1Pix));
         Double_t ycadjust = 0.3*(gPad->AbsPixeltoY(ay1Pix)   - gPad->AbsPixeltoY(ay1Pix+1));
         Int_t nrLine = 0;
         for (ipix = 0; ipix < nrPix; ipix++) {
            if (minPix[ipix] <= maxPix[ipix]) {
               Double_t xl[2]; Double_t yl[2];
               if (!optionRot) {
                  xc[nrLine] = gPad->AbsPixeltoX(ax1Pix+ipix) + xcadjust;
                  yc[nrLine] = gPad->AbsPixeltoY((Int_t)centrPix[ipix]);

                  xl[0]      = xc[nrLine];
                  yl[0]      = gPad->AbsPixeltoY((Int_t)minPix[ipix]);
                  xl[1]      = xc[nrLine];
                  yl[1]      = gPad->AbsPixeltoY((Int_t)maxPix[ipix]);
               } else {
                  yc[nrLine] = gPad->AbsPixeltoY(ay1Pix+ipix) + ycadjust;
                  xc[nrLine] = gPad->AbsPixeltoX((Int_t)centrPix[ipix]);
 
                  xl[0]      = gPad->AbsPixeltoX((Int_t)minPix[ipix]);
                  yl[0]      = yc[nrLine];
                  xl[1]      = gPad->AbsPixeltoX((Int_t)maxPix[ipix]);
                  yl[1]      = yc[nrLine];
               }
               if (!optionZ && gPad->GetLogx()) {
                  if (xc[nrLine] > 0) xc[nrLine] = TMath::Log10(xc[nrLine]);
                  else                xc[nrLine] = gPad->GetX1();
                  for (Int_t il = 0; il < 2; il++) {
                     if (xl[il] > 0) xl[il] = TMath::Log10(xl[il]);
                     else            xl[il] = gPad->GetX1();
                  }
               }
               if (!optionZ && gPad->GetLogy()) {
                  if (yc[nrLine] > 0) yc[nrLine] = TMath::Log10(yc[nrLine]);
                  else                yc[nrLine] = gPad->GetY1();
                  for (Int_t il = 0; il < 2; il++) {
                     if (yl[il] > 0) yl[il] = TMath::Log10(yl[il]);
                     else            yl[il] = gPad->GetY1();
                  }
               }

               gPad->PaintPolyLine(2,xl,yl,noClip);
               nrLine++;
            }
         }

         gPad->PaintPolyLine(nrLine,xc,yc,noClip);

         delete [] xc;
         delete [] yc;

         delete [] minPix;
         delete [] maxPix;
         delete [] centrPix;
         delete [] nrEntries;
      } else {
         if (!optionRot) {
            npt = 0;
            for (i=first; i<=last;i++) {
               npt++;
               if (!optionBins) gxwork[npt-1] = wmin+(i-first)*delta+0.5*delta;
               else {
                  xi1 = x[i];      xi = x[i-1];
                  if (xi1 < xi) {
                     if (i != last) Error(where, "X must be in increasing order");
                     else           Error(where, "X must have N+1 values with option N");
                     return;
                  }
                  gxwork[npt-1] = x[i-1] + 0.5*(x[i]-x[i-1]);
               }
               if (gxwork[npt-1] < uxmin || gxwork[npt-1] > uxmax) { npt--; continue;}
               if ((optionMark != 10) && (optionLine == 0)) {
                  if (y[i-1] <= rwymin)  {npt--; continue;}
               }
               gywork[npt-1] = y[i-1];
               gywork[npt]   = y[i-1]; //new
               if ((gywork[npt-1] < rwymin) || (gywork[npt-1] > rwymax) && !optionFill2) {
                  if ((gywork[npt-1] < rwymin)) gywork[npt-1] = rwymin;
                  if ((gywork[npt-1] > rwymax)) gywork[npt-1] = rwymax;
                  if (npt > 2) {
                     if (optionMarker) {
                        ComputeLogs(npt, optionZ);
                        gPad->PaintPolyMarker(npt,gxworkl,gyworkl);
                     }
                     if (optionLine) {
                        if (!optionMarker) ComputeLogs(npt, optionZ);
                        gPad->PaintPolyLine(npt,gxworkl,gyworkl,noClip);
                     }
                  }
                  gxwork[0] = gxwork[npt-1];
                  gywork[0] = gywork[npt-1];
                  npt       = 1;
                  continue;
               }

               if (npt >= 50) {
                  if (optionMarker) {
                     ComputeLogs(50, optionZ);
                     gPad->PaintPolyMarker(50,gxworkl,gyworkl);
                  }
                  if (optionLine) {
                     if (!optionMarker) ComputeLogs(50, optionZ);
                     if (optionFill2) {
                        gxworkl[npt]   = gxworkl[npt-1]; gyworkl[npt]   = rwymin;
                        gxworkl[npt+1] = gxworkl[0];     gyworkl[npt+1] = rwymin;
                        gPad->PaintFillArea(52,gxworkl,gyworkl);
                     }
                     gPad->PaintPolyLine(50,gxworkl,gyworkl);
                  }
                  gxwork[0] = gxwork[npt-1];
                  gywork[0] = gywork[npt-1];
                  npt      = 1;
               }
            }  //endfor (i=first; i<=last;i++)
            if (optionMarker && npt > 0) {
               ComputeLogs(npt, optionZ);
               gPad->PaintPolyMarker(npt,gxworkl,gyworkl);
            }
            if (optionLine && npt > 1) {
               if (!optionMarker) ComputeLogs(npt, optionZ);
               if (optionFill2) {
                  gxworkl[npt]   = gxworkl[npt-1]; gyworkl[npt]   = rwymin;
                  gxworkl[npt+1] = gxworkl[0];     gyworkl[npt+1] = rwymin;
                  gPad->PaintFillArea(npt+2,gxworkl,gyworkl);
               }
               gPad->PaintPolyLine(npt,gxworkl,gyworkl);
            }
         }
         else {
            npt = 0;
            for (i=first; i<=last;i++) {
               npt++;
               if (!optionBins) gywork[npt-1] = wminstep+(i-first)*delta+0.5*delta;
               else {
                  yi1 = y[i];      yi = y[i-1];
                  if (yi1 < yi) {
                     if (i != last) Error(where, "Y must be in increasing order");
                     else           Error(where, "Y must have N+1 values with option N");
                     return;
                  }
                  gywork[npt-1] = y[i-1] + 0.5*(y[i]-y[i-1]);
               }
               gxwork[npt-1] = x[i-1];
               if ((gxwork[npt-1] < uxmin) || (gxwork[npt-1] > uxmax)) {
                  if (npt > 2) {
                     if (optionMarker) {
                        ComputeLogs(npt, optionZ);
                        gPad->PaintPolyMarker(npt,gxworkl,gyworkl);
                     }
                     if (optionLine) {
                        if (!optionMarker) ComputeLogs(npt, optionZ);
                        gPad->PaintPolyLine(npt,gxworkl,gyworkl,noClip);
                     }
                  }
                  gxwork[0] = gxwork[npt-1];
                  gywork[0] = gywork[npt-1];
                  npt      = 1;
                  continue;
               }
               if (npt >= 50) {
                  if (optionMarker) {
                     ComputeLogs(50, optionZ);
                     gPad->PaintPolyMarker(50,gxworkl,gyworkl);
                  }
                  if (optionLine) {
                     if (!optionMarker) ComputeLogs(50, optionZ);
                     gPad->PaintPolyLine(50,gxworkl,gyworkl);
                  }
                  gxwork[0] = gxwork[npt-1];
                  gywork[0] = gywork[npt-1];
                  npt      = 1;
               }
            }  //endfor (i=first; i<=last;i++)
            if (optionMarker && npt > 0) {
               ComputeLogs(npt, optionZ);
               gPad->PaintPolyMarker(npt,gxworkl,gyworkl);
            }
            if (optionLine != 0 && npt > 1) {
               if (!optionMarker) ComputeLogs(npt, optionZ);
               gPad->PaintPolyLine(npt,gxworkl,gyworkl,noClip);
            }
         }
      }
   }

   //              Draw the histogram as a bar chart

   if (optionBar) {
      if (!optionBins) { offset = delta*baroffset; dbar = delta*barwidth; }
      else {
         if (!optionRot) {
            offset = (x[1]-x[0])*baroffset;
            dbar   = (x[1]-x[0])*barwidth;
         } else {
            offset = (y[1]-y[0])*baroffset;
            dbar   = (y[1]-y[0])*barwidth;
         }
      }
      drawbordersav = drawborder;
      gStyle->SetDrawBorder(1);
      if (!optionRot) {
         xlow  = wmin+offset;
         xhigh = wmin+offset+dbar;
         if (!optionOne) ylow = TMath::Max((Double_t)0,gPad->GetUymin());
         else            ylow = gPad->GetUymin();

         for (i=first; i<=last;i++) {
            yhigh    = y[i-1];
            gxwork[0] = xlow;
            gywork[0] = ylow;
            gxwork[1] = xhigh;
            gywork[1] = yhigh;
            ComputeLogs(2, optionZ);
            gPad->PaintBox(gxworkl[0],gyworkl[0],gxworkl[1],gyworkl[1]);
            if (!optionBins) {
               xlow  = xlow+delta;
               xhigh = xhigh+delta;
            }
            else {
               if (i < last) {
                  xi1 = x[i];      xi = x[i-1];
                  if (xi1 < xi) {
                     Error(where, "X must be in increasing order");
                     return;
                  }
                  offset  = (x[i+1]-x[i])*baroffset;
                  dbar    = (x[i+1]-x[i])*barwidth;
                  xlow    = x[i] + offset;
                  xhigh   = x[i] + offset + dbar;
               }
            }
         }  //endfor (i=first; i<=last;i++)
      }
      else {
         ylow  = wmin + offset;
         yhigh = wmin + offset + dbar;
         if (!optionOne) xlow = TMath::Max((Double_t)0,gPad->GetUxmin());
         else            xlow = gPad->GetUxmin();

         for (i=first; i<=last;i++) {
            xhigh    = x[i-1];
            gxwork[0] = xlow;
            gywork[0] = ylow;
            gxwork[1] = xhigh;
            gywork[1] = yhigh;
            ComputeLogs(2, optionZ);
            gPad->PaintBox(gxworkl[0],gyworkl[0],gxworkl[1],gyworkl[1]);
            gPad->PaintBox(xlow,ylow,xhigh,yhigh);
            if (!optionBins) {
               ylow  = ylow  + delta;
               yhigh = yhigh + delta;
            }
            else {
               if (i < last) {
                  yi1 = y[i];      yi = y[i-1];
                  if (yi1 < yi) {
                     Error(where, "Y must be in increasing order");
                     return;
                  }
                  offset  = (y[i+1]-y[i])*baroffset;
                  dbar    = (y[i+1]-y[i])*barwidth;
                  ylow    = y[i] + offset;
                  yhigh   = y[i] + offset + dbar;
               }
            }
         }  //endfor (i=first; i<=last;i++)
      }
      gStyle->SetDrawBorder(drawbordersav);
   }
   gPad->ResetBit(kClipFrame);

   delete [] gxwork;
   delete [] gywork;
   delete [] gxworkl;
   delete [] gyworkl;
}


//______________________________________________________________________________
void TGraph::PaintPolyLineHatches(Int_t n, const Double_t *x, const Double_t *y)
{
   // Draws a polyline with hatches on one side showing an exclusion
   // zone. x and y are the the vectors holding the polyline and n the
   // number of points in the polyline and w the width of the hatches.
   // w can be negative.
   // This method is not meant to be used directly. It is called 
   // automatically according to the line style convention.

   Int_t i,j,nf;
   Double_t w = (fLineWidth/100)*0.005;

   Double_t *xf = new Double_t[2*n];
   Double_t *yf = new Double_t[2*n];
   Double_t *xt = new Double_t[n];
   Double_t *yt = new Double_t[n];
   Double_t x1, x2, y1, y2, x3, y3, xm, ym, a, a1, a2, a3;

   // Compute the gPad coordinates in TRUE normalized space (NDC)
   Int_t ix1,iy1,ix2,iy2;
   Int_t iw = gPad->GetWw();
   Int_t ih = gPad->GetWh();
   Double_t x1p,y1p,x2p,y2p;
   gPad->GetPadPar(x1p,y1p,x2p,y2p);
   ix1 = (Int_t)(iw*x1p);
   iy1 = (Int_t)(ih*y1p);
   ix2 = (Int_t)(iw*x2p);
   iy2 = (Int_t)(ih*y2p);
   Double_t wndc  = TMath::Min(1.,(Double_t)iw/(Double_t)ih);
   Double_t hndc  = TMath::Min(1.,(Double_t)ih/(Double_t)iw);
   Double_t rh    = hndc/(Double_t)ih;
   Double_t rw    = wndc/(Double_t)iw;
   Double_t x1ndc = (Double_t)ix1*rw;
   Double_t y1ndc = (Double_t)iy1*rh;
   Double_t x2ndc = (Double_t)ix2*rw;
   Double_t y2ndc = (Double_t)iy2*rh;

   // Ratios to convert user space in TRUE normalized space (NDC)
   Double_t rx1,ry1,rx2,ry2;
   gPad->GetRange(rx1,ry1,rx2,ry2);
   Double_t rx = (x2ndc-x1ndc)/(rx2-rx1);
   Double_t ry = (y2ndc-y1ndc)/(ry2-ry1);

   // The first part of the filled area is made of the graph points.
   // Make sure that two adjacent points are different. 
   xf[0] = rx*(x[0]-rx1)+x1ndc;
   yf[0] = ry*(y[0]-ry1)+y1ndc;
   nf = 0;
   for (i=1; i<n; i++) {
      if (x[i]==x[i-1] && y[i]==y[i-1]) continue;
      nf++;
      xf[nf] = rx*(x[i]-rx1)+x1ndc;
      yf[nf] = ry*(y[i]-ry1)+y1ndc;
   }

   // For each graph points a shifted points is computed to build up
   // the second part of the filled area. First and last points are
   // treated as special cases, outside of the loop.
   a = TMath::ATan((yf[1]-yf[0])/(xf[1]-xf[0]));
   if (xf[0]<=xf[1]) {
      xt[0] = xf[0]-w*TMath::Sin(a);
      yt[0] = yf[0]+w*TMath::Cos(a); 
   } else {
      xt[0] = xf[0]+w*TMath::Sin(a);
      yt[0] = yf[0]-w*TMath::Cos(a); 
   }
  
   a = TMath::ATan((yf[nf]-yf[nf-1])/(xf[nf]-xf[nf-1]));
   if (xf[nf]>=xf[nf-1]) {
      xt[nf] = xf[nf]-w*TMath::Sin(a);
      yt[nf] = yf[nf]+w*TMath::Cos(a); 
   } else {
      xt[nf] = xf[nf]+w*TMath::Sin(a);
      yt[nf] = yf[nf]-w*TMath::Cos(a); 
   }

   Double_t xi0,yi0,xi1,yi1,xi2,yi2;
   for (i=1; i<nf; i++) {
      xi0 = xf[i];
      yi0 = yf[i];
      xi1 = xf[i+1];
      yi1 = yf[i+1];
      xi2 = xf[i-1];
      yi2 = yf[i-1];
      a1  = TMath::ATan((yi1-yi0)/(xi1-xi0));
      if (xi1<xi0) a1 = a1+3.14159;
      a2  = TMath::ATan((yi0-yi2)/(xi0-xi2));
      if (xi0<xi2) a2 = a2+3.14159;
      x1 = xi0-w*TMath::Sin(a1);
      y1 = yi0+w*TMath::Cos(a1); 
      x2 = xi0-w*TMath::Sin(a2);
      y2 = yi0+w*TMath::Cos(a2); 
      xm = (x1+x2)*0.5;
      ym = (y1+y2)*0.5;
      a3 = TMath::ATan((ym-yi0)/(xm-xi0));
      x3 = xi0-w*TMath::Sin(a3+1.57079);
      y3 = yi0+w*TMath::Cos(a3+1.57079);
      // Rotate (x3,y3) by PI around (xi0,yi0) if it is not on the (xm,ym) side.
      if ((xm-xi0)*(x3-xi0)<0 && (ym-yi0)*(y3-yi0)<0) {
         x3 = 2*xi0-x3; 
         y3 = 2*yi0-y3;
      }
      if ((xm==x1) && (ym==y1)) {
         x3 = xm;
         y3 = ym;
      }
      xt[i] = x3;
      yt[i] = y3;
   }

   // Close the polygon if the first and last points are the same
   if (xf[nf]==xf[0] && yf[nf]==yf[0]) {
      xm = (xt[nf]+xt[0])*0.5;
      ym = (yt[nf]+yt[0])*0.5;
      a3 = TMath::ATan((ym-yf[0])/(xm-xf[0]));
      x3 = xf[0]+w*TMath::Sin(a3+1.57079);
      y3 = yf[0]-w*TMath::Cos(a3+1.57079);
      if ((xm-xf[0])*(x3-xf[0])<0 && (ym-yf[0])*(y3-yf[0])<0) {
         x3 = 2*xf[0]-x3; 
         y3 = 2*yf[0]-y3;
      }
      xt[nf] = x3;
      xt[0]  = x3;
      yt[nf] = y3;
      yt[0]  = y3;
   }

   // Find the crossing segments and remove the useless ones
   Double_t xc, yc, c1, b1, c2, b2;
   Bool_t cross = kFALSE;
   Int_t nf2 = nf;
   for (i=nf2; i>0; i--) {
      for (j=i-1; j>0; j--) {
         if(xt[i-1]==xt[i] || xt[j-1]==xt[j]) continue;
         c1  = (yt[i-1]-yt[i])/(xt[i-1]-xt[i]);
         b1  = yt[i]-c1*xt[i];
         c2  = (yt[j-1]-yt[j])/(xt[j-1]-xt[j]);
         b2  = yt[j]-c2*xt[j];
         if (c1 != c2) {
            xc = (b2-b1)/(c1-c2);
            yc = c1*xc+b1;
            if (xc>TMath::Min(xt[i],xt[i-1]) && xc<TMath::Max(xt[i],xt[i-1]) && 
                xc>TMath::Min(xt[j],xt[j-1]) && xc<TMath::Max(xt[j],xt[j-1]) &&
                yc>TMath::Min(yt[i],yt[i-1]) && yc<TMath::Max(yt[i],yt[i-1]) && 
                yc>TMath::Min(yt[j],yt[j-1]) && yc<TMath::Max(yt[j],yt[j-1])) {
               nf++; xf[nf] = xt[i]; yf[nf] = yt[i];
               nf++; xf[nf] = xc   ; yf[nf] = yc;
               i = j;
               cross = kTRUE;
               break;
            } else {
               continue;
            }
         } else {
            continue;
         }
      }
      if (!cross) {
         nf++;
         xf[nf] = xt[i];
         yf[nf] = yt[i];
      }
      cross = kFALSE;
   }
   nf++; xf[nf] = xt[0]; yf[nf] = yt[0];

   // NDC to user coordinates
   for (i=0; i<nf+1; i++) {
      xf[i] = (1/rx)*(xf[i]-x1ndc)+rx1;
      yf[i] = (1/ry)*(yf[i]-y1ndc)+ry1;
   }

   // Draw filled area
   gPad->PaintFillArea(nf+1,xf,yf);
   TAttLine::Modify(); // In case of PaintFillAreaHatches

   delete [] xf;
   delete [] yf;
   delete [] xt;
   delete [] yt;
}


//______________________________________________________________________________
void TGraph::ComputeLogs(Int_t npoints, Int_t opt)
{
   // Convert WC from Log scales.
   //
   //   Take the LOG10 of gxwork and gywork according to the value of Options
   //   and put it in gxworkl and gyworkl.
   //
   //  npoints : Number of points in gxwork and in gywork.
   //

   Int_t i;
   memcpy(gxworkl,gxwork,npoints*8);
   memcpy(gyworkl,gywork,npoints*8);
   if (gPad->GetLogx()) {
      for (i=0;i<npoints;i++) {
         if (gxworkl[i] > 0) gxworkl[i] = TMath::Log10(gxworkl[i]);
         else                gxworkl[i] = gPad->GetX1();
      }
   }
   if (!opt && gPad->GetLogy()) {
      for (i=0;i<npoints;i++) {
         if (gyworkl[i] > 0) gyworkl[i] = TMath::Log10(gyworkl[i]);
         else                gyworkl[i] = gPad->GetY1();
      }
   }
}


//______________________________________________________________________________
void TGraph::Print(Option_t *) const
{
   // Print graph values.

   for (Int_t i=0;i<fNpoints;i++) {
      printf("x[%d]=%g, y[%d]=%g\n",i,fX[i],i,fY[i]);
   }
}

//______________________________________________________________________________
void TGraph::RecursiveRemove(TObject *obj)
{
   // Recursively remove object from the list of functions

   if (fFunctions) {
      if (!fFunctions->TestBit(kInvalidObject)) fFunctions->RecursiveRemove(obj);
   }
}


//______________________________________________________________________________
Int_t TGraph::RemovePoint()
{
   // Delete point close to the mouse position

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
   return RemovePoint(ipoint);
}


//______________________________________________________________________________
Int_t TGraph::RemovePoint(Int_t ipoint)
{
   // Delete point number ipoint

   if (ipoint < 0) return -1;
   if (ipoint >= fNpoints) return -1;

   Double_t **ps = ShrinkAndCopy(fNpoints - 1, ipoint);
   CopyAndRelease(ps, ipoint+1, fNpoints--, ipoint);
   if (gPad) gPad->Modified();
   return ipoint;
}


//______________________________________________________________________________
void TGraph::SavePrimitive(ostream &out, Option_t *option /*= ""*/)
{
    // Save primitive as a C++ statement(s) on output stream out

   char quote = '"';
   out<<"   "<<endl;
   if (gROOT->ClassSaved(TGraph::Class())) {
      out<<"   ";
   } else {
      out<<"   TGraph *";
   }
   out<<"graph = new TGraph("<<fNpoints<<");"<<endl;
   out<<"   graph->SetName("<<quote<<GetName()<<quote<<");"<<endl;
   out<<"   graph->SetTitle("<<quote<<GetTitle()<<quote<<");"<<endl;

   SaveFillAttributes(out,"graph",0,1001);
   SaveLineAttributes(out,"graph",1,1,1);
   SaveMarkerAttributes(out,"graph",1,1,1);

   for (Int_t i=0;i<fNpoints;i++) {
      out<<"   graph->SetPoint("<<i<<","<<fX[i]<<","<<fY[i]<<");"<<endl;
   }
   if (strstr(option,"multigraph")) {
      return;
   }
   static Int_t frameNumber = 0;
   if (fHistogram) {
      frameNumber++;
      TString hname = fHistogram->GetName();
      hname += frameNumber;
      fHistogram->SetName(hname.Data());
      fHistogram->SavePrimitive(out,"nodraw");
      out<<"   graph->SetHistogram("<<fHistogram->GetName()<<");"<<endl;
      out<<"   "<<endl;
   }

   // save list of functions
   TIter next(fFunctions);
   TObject *obj;
   while ((obj=next())) {
      obj->SavePrimitive(out,"nodraw");
      if (obj->InheritsFrom("TPaveStats")) {
         out<<"   graph->GetListOfFunctions()->Add(ptstats);"<<endl;
         out<<"   ptstats->SetParent(graph->GetListOfFunctions());"<<endl;
      } else {
         out<<"   graph->GetListOfFunctions()->Add("<<obj->GetName()<<");"<<endl;
      }
   }

   if (!strstr(option,"multigraph")) {
      out<<"   graph->Draw("
         <<quote<<option<<quote<<");"<<endl;
   }
}


//______________________________________________________________________________
void TGraph::Set(Int_t n)
{
   // Set number of points in the graph
   // Existing coordinates are preserved
   // New coordinates above fNpoints are preset to 0.

   if (n < 0) n = 0;
   if (n == fNpoints) return;
   Double_t **ps = Allocate(n);
   CopyAndRelease(ps, 0, TMath::Min(fNpoints,n), 0);
   if (n > fNpoints) {
      FillZero(fNpoints, n, kFALSE);
   }
   fNpoints = n;
}


//______________________________________________________________________________
Bool_t TGraph::GetEditable() const
{
   // Return kTRUE if kNotEditable bit is not set, kFALSE otherwise.

   return TestBit(kNotEditable) ? kFALSE : kTRUE;
}


//______________________________________________________________________________
void TGraph::SetEditable(Bool_t editable)
{
   // if editable=kFALSE, the graph cannot be modified with the mouse
   //  by default a TGraph is editable

   if (editable) ResetBit(kNotEditable);
   else          SetBit(kNotEditable);
}


//______________________________________________________________________________
void TGraph::SetMaximum(Double_t maximum)
{
   // Set the maximum of the graph.

   fMaximum = maximum;
   GetHistogram()->SetMaximum(maximum);
}


//______________________________________________________________________________
void TGraph::SetMinimum(Double_t minimum)
{
   // Set the minimum of the graph.

   fMinimum = minimum;
   GetHistogram()->SetMinimum(minimum);
}


//______________________________________________________________________________
void TGraph::SetPoint(Int_t i, Double_t x, Double_t y)
{
   // Set x and y values for point number i.

   if (i < 0) return;
   if (fHistogram) {
      delete fHistogram;
      fHistogram = 0;
   }
   if (i >= fMaxSize) {
      Double_t **ps = ExpandAndCopy(i+1, fNpoints);
      CopyAndRelease(ps, 0,0,0);
   }
   if (i >= fNpoints) {
      // points above i can be not initialized
      // set zero up to i-th point to avoid redefenition
      // of this method in descendant classes
      FillZero(fNpoints, i + 1);
      fNpoints = i+1;
   }
   fX[i] = x;
   fY[i] = y;
   if (gPad) gPad->Modified();
}


//______________________________________________________________________________
void TGraph::SetTitle(const char* title)
{
   // Set graph title.

   fTitle = title;
   if (fHistogram) fHistogram->SetTitle(title);
}


//______________________________________________________________________________
Double_t **TGraph::ShrinkAndCopy(Int_t size, Int_t oend)
{
   // if size*2 <= fMaxSize allocate new arrays of size points,
   // copy points [0,oend).
   // Return newarray (passed or new instance if it was zero
   // and allocations are needed)
   if (size*2 > fMaxSize || !fMaxSize) {
      return 0;
   }
   Double_t **newarrays = Allocate(size);
   CopyPoints(newarrays, 0, oend, 0);
   return newarrays;
}


//______________________________________________________________________________
void TGraph::Smooth(Int_t npoints, Double_t *x, Double_t *y, Int_t drawtype)
{
   // Smooth a curve given by N points.
   //
   //   Underlaying routine for Draw based on the CERN GD3 routine TVIPTE
   //
   //     Author - Marlow etc.   Modified by - P. Ward     Date -  3.10.1973
   //
   //   This routine draws a smooth tangentially continuous curve through
   // the sequence of data points P(I) I=1,N where P(I)=(X(I),Y(I))
   // the curve is approximated by a polygonal arc of short vectors .
   // the data points can represent open curves, P(1) != P(N) or closed
   // curves P(2) == P(N) . If a tangential discontinuity at P(I) is
   // required , then set P(I)=P(I+1) . loops are also allowed .
   //
   // Reference Marlow and Powell,Harwell report No.R.7092.1972
   // MCCONALOGUE,Computer Journal VOL.13,NO4,NOV1970Pp392 6
   //
   // _Input parameters:
   //
   //  npoints   : Number of data points.
   //  x         : Abscissa
   //  y         : Ordinate
   //
   //
   // delta is the accuracy required in constructing the curve.
   // if it is zero then the routine calculates a value other-
   // wise it uses this value. (default is 0.0)

   Int_t i, k, kp, km, npointsMax, banksize, n2, npt;
   Int_t maxiterations, finished;
   Int_t jtype, ktype, closed;
   Double_t sxmin, sxmax, symin, symax;
   Double_t delta;
   Double_t xorg, yorg;
   Double_t ratio_signs, xratio, yratio;
   Int_t flgic, flgis;
   Int_t iw, loptx;
   Double_t p1, p2, p3, p4, p5, p6;
   Double_t w1, w2, w3;
   Double_t a, b, c, r, s, t, z;
   Double_t co, so, ct, st, ctu, stu, xnt;
   Double_t dx1, dy1, dx2, dy2, dk1, dk2;
   Double_t xo, yo, dx, dy, xt, yt;
   Double_t xa, xb, ya, yb;
   Double_t u1, u2, u3, tj;
   Double_t cc, err;
   Double_t sb, sth;
   Double_t wsign, tsquare, tcube;
   c = t = co = so = ct = st = ctu = stu = dx1 = dy1 = dx2 = dy2 = 0;
   xt = yt = xa = xb = ya = yb = u1 = u2 = u3 = tj = sb = 0;

   npointsMax  = npoints*10;
   n2          = npointsMax-2;
   banksize    = n2;

   Double_t *qlx = new Double_t[npointsMax];
   Double_t *qly = new Double_t[npointsMax];
   if (!qlx || !qly) {
      Error("Smooth", "not enough space in memory");
      return;
   }

   //  Decode the type of curve according to
   //  chopt of IGHIST.
   //  ('S', 'SA', 'SA1' ,'XS', 'XSA', or 'XSA1')

   loptx = kFALSE;
   jtype  = (drawtype%1000)-10;
   if (jtype > 0) { ktype = jtype; loptx = kTRUE; }
   else             ktype = drawtype%1000;

   Double_t ruxmin = gPad->GetUxmin();
   Double_t ruymin = gPad->GetUymin();
   if (ktype == 3) {
      xorg = ruxmin;
      yorg = ruymin;
   } else {
      xorg = TMath::Max((Double_t)0,ruxmin);
      yorg = TMath::Min(TMath::Max((Double_t)0,ruymin),gPad->GetUymax());
   }

   maxiterations = 20;
   delta         = 0.00055;

   //       Scale data to the range 0-ratio_signs in X, 0-1 in Y
   //       where ratio_signs is the ratio between the number of changes
   //       of sign in Y divided by the number of changes of sign in X

   sxmin = x[0];
   sxmax = x[0];
   symin = y[0];
   symax = y[0];
   Double_t six   = 1;
   Double_t siy   = 1;
   for (i=1;i<npoints;i++) {
      if (i > 1) {
         if ((x[i]-x[i-1])*(x[i-1]-x[i-2]) < 0) six++;
         if ((y[i]-y[i-1])*(y[i-1]-y[i-2]) < 0) siy++;
      }
      if (x[i] < sxmin) sxmin = x[i];
      if (x[i] > sxmax) sxmax = x[i];
      if (y[i] < symin) symin = y[i];
      if (y[i] > symax) symax = y[i];
   }
   closed = 0;
   Double_t dx1n   = TMath::Abs(x[npoints-1]-x[0]);
   Double_t dy1n   = TMath::Abs(y[npoints-1]-y[0]);
   if (dx1n < 0.01*(sxmax-sxmin) && dy1n < 0.01*(symax-symin))  closed = 1;
   if (sxmin == sxmax) xratio = 1;
   else {
      if (six > 1) ratio_signs = siy/six;
      else         ratio_signs = 20;
      xratio = ratio_signs/(sxmax-sxmin);
   }
   if (symin == symax) yratio = 1;
   else                yratio = 1/(symax-symin);

   qlx[0] = x[0];
   qly[0] = y[0];
   for (i=0;i<npoints;i++) {
      x[i] = (x[i]-sxmin)*xratio;
      y[i] = (y[i]-symin)*yratio;
   }

   //           finished is minus one if we must draw a straight line from P(k-1)
   //           to P(k). finished is one if the last call to IPL has  <  N2
   //           points. finished is zero otherwise. npt counts the X and Y
   //           coordinates in work . When npt=N2 a call to IPL is made.

   finished = 0;
   npt      = 1;
   k        = 1;

   //           Convert coordinates back to original system

   //           Separate the set of data points into arcs P(k-1),P(k).
   //           Calculate the direction cosines. first consider whether
   //           there is a continuous tangent at the endpoints.

   if (!closed) {
      if (x[0] != x[npoints-1] || y[0] != y[npoints-1]) goto L40;
      if (x[npoints-2] == x[npoints-1] && y[npoints-2] == y[npoints-1]) goto L40;
      if (x[0] == x[1] && y[0] == y[1]) goto L40;
   }
   flgic = kFALSE;
   flgis = kTRUE;

   //           flgic is true if the curve is open and false if it is closed.
   //           flgis is true in the main loop, but is false if there is
   //           a deviation from the main loop.

   km = npoints - 1;

   //           Calculate direction cosines at P(1) using P(N-1),P(1),P(2).

   goto L100;
L40:
   flgic = kTRUE;
   flgis = kFALSE;

   //           Skip excessive consecutive equal points.

L50:
   if (k >= npoints) {
      finished = 1;  //*-*-  Prepare to clear out remaining short vectors before returning
      if (npt > 1) goto L310;
      goto L390;
   }
   k++;
   if (x[k-1] == x[k-2] && y[k-1] == y[k-2])  goto L50;
L60:
   km = k-1;
   if (k > npoints) {
      finished = 1;  //*-*-  Prepare to clear out remaining short vectors before returning
      if (npt > 1) goto L310;
      goto L390;
   }
   if (k < npoints) goto L90;
   if (!flgic) { kp = 2; goto L130;}

L80:
   if (flgis) goto L150;

   //           Draw a straight line from P(k-1) to P(k).

   finished = -1;
   goto L170;

   //           Test whether P(k) is a cusp.

L90:
   if (x[k-1] == x[k] && y[k-1] == y[k]) goto L80;
L100:
   kp = k+1;
   goto L130;

   //           Branch if the next section of the curve begins at a cusp.

L110:
   if (!flgis) goto L50;

//*-*-           Carry forward the direction cosines from the previous arc.

L120:
   co = ct;
   so = st;
   k++;
   goto L60;

   //           Calculate the direction cosines at P(k).  If k=1 then
   //           N-1 is used for k-1. If k=N then 2 is used for k+1.
   //           direction cosines at P(k) obtained from P(k-1),P(k),P(k+1).

L130:
   dx1 = x[k-1]  - x[km-1];
   dy1 = y[k-1]  - y[km-1];
   dk1 = dx1*dx1 + dy1*dy1;
   dx2 = x[kp-1] - x[k-1];
   dy2 = y[kp-1] - y[k-1];
   dk2 = dx2*dx2 + dy2*dy2;
   ctu = dx1*dk2 + dx2*dk1;
   stu = dy1*dk2 + dy2*dk1;
   xnt = ctu*ctu + stu*stu;

   //           If both ctu and stu are zero,then default.This can
   //           occur when P(k)=P(k+1). I.E. A loop.

   if (xnt < 1.E-25) {
      ctu = dy1;
      stu =-dx1;
      xnt = dk1;
   }
   //           Normalise direction cosines.

   ct = ctu/TMath::Sqrt(xnt);
   st = stu/TMath::Sqrt(xnt);
   if (flgis) goto L160;

   //           Direction cosines at P(k-1) obtained from P(k-1),P(k),P(k+1).

   w3    = 2*(dx1*dy2-dx2*dy1);
   co    = ctu+w3*dy1;
   so    = stu-w3*dx1;
   xnt   = 1/TMath::Sqrt(co*co+so*so);
   co    = co*xnt;
   so    = so*xnt;
   flgis = kTRUE;
   goto L170;

   //           Direction cosines at P(k) obtained from P(k-2),P(k-1),P(k).

L150:
   w3    = 2*(dx1*dy2-dx2*dy1);
   ct    = ctu-w3*dy2;
   st    = stu+w3*dx2;
   xnt   = 1/TMath::Sqrt(ct*ct+st*st);
   ct    = ct*xnt;
   st    = st*xnt;
   flgis = kFALSE;
   goto L170;
L160:
   if (k <= 1) goto L120;

   //           For the arc between P(k-1) and P(k) with direction cosines co,
   //           so and ct,st respectively, calculate the coefficients of the
   //           parametric cubic represented by X(t) and Y(t) where
   //           X(t)=xa*t**3 + xb*t**2 + co*t + xo
   //           Y(t)=ya*t**3 + yb*t**2 + so*t + yo

L170:
   xo = x[k-2];
   yo = y[k-2];
   dx = x[k-1] - xo;
   dy = y[k-1] - yo;

   //           Initialise the values of X(TI),Y(TI) in xt and yt respectively.

   xt = xo;
   yt = yo;
   if (finished < 0) {  //*-*- Draw a straight line between (xo,yo) and (xt,yt)
      xt += dx;
      yt += dy;
      goto L300;
   }
   c  = dx*dx+dy*dy;
   a  = co+ct;
   b  = so+st;
   r  = dx*a+dy*b;
   t  = c*6/(TMath::Sqrt(r*r+2*(7-co*ct-so*st)*c)+r);
   tsquare = t*t;
   tcube   = t*tsquare;
   xa = (a*t-2*dx)/tcube;
   xb = (3*dx-(co+a)*t)/tsquare;
   ya = (b*t-2*dy)/tcube;
   yb = (3*dy-(so+b)*t)/tsquare;

   //           If the curve is close to a straight line then use a straight
   //           line between (xo,yo) and (xt,yt).

   if (.75*TMath::Max(TMath::Abs(dx*so-dy*co),TMath::Abs(dx*st-dy*ct)) <= delta) {
      finished = -1;
      xt += dx;
      yt += dy;
      goto L300;
   }

   //           Calculate a set of values 0 == t(0).LTCT(1) <  ...  < t(M)=TC
   //           such that polygonal arc joining X(t(J)),Y(t(J)) (J=0,1,..M)
   //           is within the required accuracy of the curve

   tj = 0;
   u1 = ya*xb-yb*xa;
   u2 = yb*co-xb*so;
   u3 = so*xa-ya*co;

   //           Given t(J), calculate t(J+1). The values of X(t(J)),
   //           Y(t(J)) t(J) are contained in xt,yt and tj respectively.

L180:
   s  = t - tj;
   iw = -2;

   //           Define iw here later.

   p1 = (2*u1)*tj-u3;
   p2 = (u1*tj-u3)*3*tj+u2;
   p3 = 3*tj*ya+yb;
   p4 = (p3+yb)*tj+so;
   p5 = 3*tj*xa+xb;
   p6 = (p5+xb)*tj+co;

   //           Test D(tj,THETA). A is set to (Y(tj+s)-Y(tj))/s.b is
   //           set to (X(tj+s)-X(tj))/s.

   cc  = 0.8209285;
   err = 0.1209835;
L190:
   iw -= 2;
L200:
   a   = (s*ya+p3)*s+p4;
   b   = (s*xa+p5)*s+p6;

   //           Set z to PSI(D/delta)-cc.

   w1 = -s*(s*u1+p1);
   w2 = s*s*u1-p2;
   w3 = 1.5*w1+w2;

   //           Set the estimate of (THETA-tj)/s.Then set the numerator
   //           of the expression (EQUATION 4.4)/s. Then set the square
   //           of D(tj,tj+s)/delta. Then replace z by PSI(D/delta)-cc.

   if (w3 > 0) wsign = TMath::Abs(w1);
   else        wsign = -TMath::Abs(w1);
   sth = 0.5+wsign/(3.4*TMath::Abs(w1)+5.2*TMath::Abs(w3));
   z   = s*sth*(s-s*sth)*(w1*sth+w1+w2);
   z   = z*z/((a*a+b*b)*(delta*delta));
   z   = (z+2.642937)*z/((.3715652*z+3.063444)*z+.2441889)-cc;

   //           Branch if z has been calculated

   if (iw > 0) goto L250;
   if (z > err) goto L240;
   goto L220;
L210:
   iw -= 2;
L220:
   if (iw+2 == 0) goto L190;
   if (iw+2 >  0) goto L290;

   //           Last part of arc.

L230:
   xt = x[k-1];
   yt = y[k-1];
   s  = 0;
   goto L300;

   //           z(s). find a value of s where 0 <= s <= sb such that
   //           TMath::Abs(z(s)) < err

L240:
   kp = 0;
   c  = z;
   sb = s;
L250:
   Zero(kp,0,sb,err,s,z,maxiterations);
   if (kp == 2) goto L210;
   if (kp > 2) {
      Error("Smooth", "Attempt to plot outside plot limits");
      goto L230;
   }
   if (iw > 0) goto L200;

   //           Set z=z(s) for s=0.

   if (iw < 0) {
      z  = -cc;
      iw = 0;
      goto L250;
   }

   //           Set z=z(s) for s=sb.

   z  = c;
   iw = 1;
   goto L250;

   //           Update tj,xt and yt.

L290:
   xt = xt + s*b;
   yt = yt + s*a;
   tj = s  + tj;

   //           Convert coordinates to original system

L300:
   qlx[npt] = sxmin + xt/xratio;
   qly[npt] = symin + yt/yratio;
   npt++;

   //           If a fill area must be drawn and if the banks LX and
   //           LY are too small they are enlarged in order to draw
   //           the filled area in one go.

   if (npt < banksize)  goto L320;
   if (drawtype >= 1000 || ktype > 1) {
      Int_t newsize = banksize + n2;
      Double_t *qtemp = new Double_t[banksize];
      for (i=0;i<banksize;i++) qtemp[i] = qlx[i];
      delete [] qlx;
      qlx = new Double_t[newsize];
      for (i=0;i<banksize;i++) qlx[i]   = qtemp[i];
      for (i=0;i<banksize;i++) qtemp[i] = qly[i];
      delete [] qly;
      qly = new Double_t[newsize];
      for (i=0;i<banksize;i++) qly[i] = qtemp[i];
      delete [] qtemp;
      banksize = newsize;
      goto L320;
   }

   //           Draw the graph

L310:
   if (drawtype >= 1000) {
      gPad->PaintFillArea(npt,qlx,qly, "B");
   }
   else {
      if (ktype > 1) {
         if (!loptx) {
            qlx[npt]   = qlx[npt-1];
            qlx[npt+1] = qlx[0];
            qly[npt]   = yorg;
            qly[npt+1] = yorg;
         }
         else {
            qlx[npt]   = xorg;
            qlx[npt+1] = xorg;
            qly[npt]   = qly[npt-1];
            qly[npt+1] = qly[0];
         }
         gPad->PaintFillArea(npt+2,qlx,qly);
      }
      if (TMath::Abs(fLineWidth)>99) PaintPolyLineHatches(npt, qlx, qly);
      gPad->PaintPolyLine(npt,qlx,qly);
   }
   npt = 1;
   qlx[0] = sxmin + xt/xratio;
   qly[0] = symin + yt/yratio;
L320:
   if (finished > 0) goto L390;
   if (finished < 0) { finished = 0; goto L110;}
   if (s > 0) goto L180;
   goto L110;

   //           Convert coordinates back to original system

L390:
   for (i=0;i<npoints;i++) {
      x[i] = sxmin + x[i]/xratio;
      y[i] = symin + y[i]/yratio;
   }

   delete [] qlx;
   delete [] qly;
}


//______________________________________________________________________________
void TGraph::Sort(Bool_t (*greaterfunc)(const TGraph*, Int_t, Int_t) /*=TGraph::CompareX()*/,
                  Bool_t ascending /*=kTRUE*/, Int_t low /* =0 */, Int_t high /* =-1111 */) 
{
   // Sorts the points of this TGraph using in-place quicksort (see e.g. older glibc).
   // To compare two points the function parameter greaterfunc is used (see TGraph::CompareX for an
   // example of such a method, which is also the default comparison function for Sort). After
   // the sort, greaterfunc(this, i, j) will return kTRUE for all i>j if ascending == kTRUE, and
   // kFALSE otherwise.
   //
   // The last two parameters are used for the recursive quick sort, stating the range to be sorted
   //
   // Examples:
   //   // sort points along x axis
   //   graph->Sort();
   //   // sort points along their distance to origin
   //   graph->Sort(&TGraph::CompareRadius);
   //
   //   Bool_t CompareErrors(const TGraph* gr, Int_t i, Int_t j) {
   //     const TGraphErrors* ge=(const TGraphErrors*)gr;
   //     return (ge->GetEY()[i]>ge->GetEY()[j]); }
   //   // sort using the above comparison function, largest errors first
   //   graph->Sort(&CompareErrors, kFALSE);

   if (high == -1111) high = GetN()-1;
   //  Termination condition
   if (high <= low) return;

   int left, right;
   left = low; // low is the pivot element
   right = high;
   while (left < right) {
      // move left while item < pivot
      while(left <= high && greaterfunc(this, left, low) != ascending)
         left++;
      // move right while item > pivot
      while(right > low && greaterfunc(this, right, low) == ascending)
         right--;
      if (left < right && left < high && right > low)
         SwapPoints(left, right);
   }
   // right is final position for the pivot
   if (right > low)
      SwapPoints(low, right);
   Sort( greaterfunc, ascending, low, right-1 );
   Sort( greaterfunc, ascending, right+1, high );
}


//______________________________________________________________________________
void TGraph::Streamer(TBuffer &b)
{
   // Stream an object of class TGraph.

   if (b.IsReading()) {
      UInt_t R__s, R__c;
      Version_t R__v = b.ReadVersion(&R__s, &R__c);
      if (R__v > 2) {
         b.ReadClassBuffer(TGraph::Class(), this, R__v, R__s, R__c);
         if (fHistogram) fHistogram->SetDirectory(0);
         TIter next(fFunctions);
         TObject *obj;
         while ((obj = next())) {
            if (obj->InheritsFrom(TF1::Class())) {
               TF1 *f1 = (TF1*)obj;
               f1->SetParent(this);
            }
         }
         fMaxSize = fNpoints;
         return;
      }
      //====process old versions before automatic schema evolution
      TNamed::Streamer(b);
      TAttLine::Streamer(b);
      TAttFill::Streamer(b);
      TAttMarker::Streamer(b);
      b >> fNpoints;
      fMaxSize = fNpoints;
      fX = new Double_t[fNpoints];
      fY = new Double_t[fNpoints];
      if (R__v < 2) {
         Float_t *x = new Float_t[fNpoints];
         Float_t *y = new Float_t[fNpoints];
         b.ReadFastArray(x,fNpoints);
         b.ReadFastArray(y,fNpoints);
         for (Int_t i=0;i<fNpoints;i++) {
            fX[i] = x[i];
            fY[i] = y[i];
         }
         delete [] y;
         delete [] x;
      } else {
         b.ReadFastArray(fX,fNpoints);
         b.ReadFastArray(fY,fNpoints);
      }
      b >> fFunctions;
      b >> fHistogram;
      if (fHistogram) fHistogram->SetDirectory(0);
      if (R__v < 2) {
         Float_t mi,ma;
         b >> mi;
         b >> ma;
         fMinimum = mi;
         fMaximum = ma;
      } else {
         b >> fMinimum;
         b >> fMaximum;
      }
      b.CheckByteCount(R__s, R__c, TGraph::IsA());
      //====end of old versions

   } else {
      b.WriteClassBuffer(TGraph::Class(),this);
   }
}


//______________________________________________________________________________
void TGraph::SwapPoints(Int_t pos1, Int_t pos2)
{
   // Swap points.

   SwapValues(fX, pos1, pos2);
   SwapValues(fY, pos1, pos2);
}


//______________________________________________________________________________
void TGraph::SwapValues(Double_t* arr, Int_t pos1, Int_t pos2)
{
   // Swap values.

   Double_t tmp=arr[pos1];
   arr[pos1]=arr[pos2];
   arr[pos2]=tmp;
}


//______________________________________________________________________________
void TGraph::UseCurrentStyle()
{
   // Set current style settings in this graph
   // This function is called when either TCanvas::UseCurrentStyle
   // or TROOT::ForceStyle have been invoked.

   if (gStyle->IsReading()) {
      SetFillColor(gStyle->GetHistFillColor());
      SetFillStyle(gStyle->GetHistFillStyle());
      SetLineColor(gStyle->GetHistLineColor());
      SetLineStyle(gStyle->GetHistLineStyle());
      SetLineWidth(gStyle->GetHistLineWidth());
      SetMarkerColor(gStyle->GetMarkerColor());
      SetMarkerStyle(gStyle->GetMarkerStyle());
      SetMarkerSize(gStyle->GetMarkerSize());
   } else {
      gStyle->SetHistFillColor(GetFillColor());
      gStyle->SetHistFillStyle(GetFillStyle());
      gStyle->SetHistLineColor(GetLineColor());
      gStyle->SetHistLineStyle(GetLineStyle());
      gStyle->SetHistLineWidth(GetLineWidth());
      gStyle->SetMarkerColor(GetMarkerColor());
      gStyle->SetMarkerStyle(GetMarkerStyle());
      gStyle->SetMarkerSize(GetMarkerSize());
   }
   if (fHistogram) fHistogram->UseCurrentStyle();

   TIter next(GetListOfFunctions());
   TObject *obj;

   while ((obj = next())) {
      obj->UseCurrentStyle();
   }
}


//______________________________________________________________________________
void TGraph::Zero(Int_t &k,Double_t AZ,Double_t BZ,Double_t E2,Double_t &X,Double_t &Y
                 ,Int_t maxiterations)
{
   // Find zero of a continuous function.
   //
   //  Underlaying routine for PaintGraph
   // This function finds a real zero of the continuous real
   // function Y(X) in a given interval (A,B). See accompanying
   // notes for details of the argument list and calling sequence

   static Double_t a, b, ya, ytest, y1, x1, h;
   static Int_t j1, it, j3, j2;
   Double_t yb, x2;
   yb = 0;

   //       Calculate Y(X) at X=AZ.
   if (k <= 0) {
      a  = AZ;
      b  = BZ;
      X  = a;
      j1 = 1;
      it = 1;
      k  = j1;
      return;
   }

   //       Test whether Y(X) is sufficiently small.

   if (TMath::Abs(Y) <= E2) { k = 2; return; }

   //       Calculate Y(X) at X=BZ.

   if (j1 == 1) {
      ya = Y;
      X  = b;
      j1 = 2;
      return;
   }
   //       Test whether the signs of Y(AZ) and Y(BZ) are different.
   //       if not, begin the binary subdivision.

   if (j1 != 2) goto L100;
   if (ya*Y < 0) goto L120;
   x1 = a;
   y1 = ya;
   j1 = 3;
   h  = b - a;
   j2 = 1;
   x2 = a + 0.5*h;
   j3 = 1;
   it++;      //*-*-   Check whether (maxiterations) function values have been calculated.
   if (it >= maxiterations) k = j1;
   else                     X = x2;
   return;

   //      Test whether a bracket has been found .
   //      If not,continue the search

L100:
   if (j1 > 3) goto L170;
   if (ya*Y >= 0) {
      if (j3 >= j2) {
         h  = 0.5*h; j2 = 2*j2;
         a  = x1;  ya = y1;  x2 = a + 0.5*h; j3 = 1;
      }
      else {
         a  = X;   ya = Y;   x2 = X + h;     j3++;
      }
      it++;
      if (it >= maxiterations) k = j1;
      else                     X = x2;
      return;
   }

   //       The first bracket has been found.calculate the next X by the
   //       secant method based on the bracket.

L120:
   b  = X;
   yb = Y;
   j1 = 4;
L130:
   if (TMath::Abs(ya) > TMath::Abs(yb)) { x1 = a; y1 = ya; X  = b; Y  = yb; }
   else                                 { x1 = b; y1 = yb; X  = a; Y  = ya; }

   //       Use the secant method based on the function values y1 and Y.
   //       check that x2 is inside the interval (a,b).

L150:
   x2    = X-Y*(X-x1)/(Y-y1);
   x1    = X;
   y1    = Y;
   ytest = 0.5*TMath::Min(TMath::Abs(ya),TMath::Abs(yb));
   if ((x2-a)*(x2-b) < 0) {
      it++;
      if (it >= maxiterations) k = j1;
      else                     X = x2;
      return;
   }

   //       Calculate the next value of X by bisection . Check whether
   //       the maximum accuracy has been achieved.

L160:
   x2    = 0.5*(a+b);
   ytest = 0;
   if ((x2-a)*(x2-b) >= 0) { k = 2;  return; }
   it++;
   if (it >= maxiterations) k = j1;
   else                     X = x2;
   return;


   //       Revise the bracket (a,b).

L170:
   if (j1 != 4) return;
   if (ya*Y < 0) { b  = X; yb = Y; }
   else          { a  = X; ya = Y; }

   //       Use ytest to decide the method for the next value of X.

   if (ytest <= 0) goto L130;
   if (TMath::Abs(Y)-ytest <= 0) goto L150;
   goto L160;
}


//______________________________________________________________________________
Int_t TGraph::Merge(TCollection* li)
{
   // Adds all graphs from the collection to this graph.
   // Returns the total number of poins in the result or -1 in case of an error.

   TIter next(li);
   while (TObject* o = next()) {
      TGraph *g = dynamic_cast<TGraph*> (o);
      if (!g) {
         Error("Merge",
             "Cannot merge - an object which doesn't inherit from TGraph found in the list");
         return -1;
      }
      Double_t x, y;
      for (Int_t i = 0 ; i < g->GetN(); i++) {
         g->GetPoint(i, x, y);
         SetPoint(GetN(), x, y);
      }
   }
   return GetN();
}
