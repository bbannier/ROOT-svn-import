// @(#)root/graf:$Name:  $:$Id: TCurlyLine.cxx,v 1.2 2000/06/13 10:49:14 brun Exp $
// Author: Otto Schaile   20/11/99

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

//________________________________________________________________________
//
// This class implements curly or wavy polylines typically used to draw Feynman diagrams.
// Amplitudes and wavelengths may be specified in the constructors,
// via commands or interactively from popup menus.
// The class make use of TPolyLine by inheritance, ExecuteEvent methods
// are highly inspired from the methods used in TPolyLine and TArc.
// The picture below has been generated by the tutorial feynman.
//Begin_Html
/*
<img src="gif/feynman.gif">
*/
//End_Html
//________________________________________________________________________

#include "IOStream.h"
#include "TCurlyLine.h"
#include "TROOT.h"
#include "TVirtualPad.h"
#include "TVirtualX.h"
#include "TMath.h"

ClassImp(TCurlyLine)

//______________________________________________________________________________
TCurlyLine::TCurlyLine(Double_t x1, Double_t y1, Double_t x2, Double_t y2, Double_t tl, Double_t rad)
{
 // create a new TCurlyLine with starting point (x1, y1), end point (x2,y2)
 // The wavelength and amplitude are given in percent of the pad height

   fX1         = x1;
   fY1         = y1;
   fX2         = x2;
   fY2         = y2;
   fWaveLength = tl;
   fAmplitude  = rad;
   fIsCurly    = kTRUE;
   Build();
}

//______________________________________________________________________________
void TCurlyLine::Build()
{
//*-*-*-*-*-*-*-*-*-*-*Create a curly (Gluon) or wavy (Gamma) line*-*-*-*-*-*
//*-*                  ===========================================

   Double_t pixwave,pixampl;
   if (gPad) {
      Double_t hpixels  = gPad->GetAbsHNDC()*gPad->GetWh();
      Int_t px1         = gPad->XtoAbsPixel(fX1);
      Int_t py1         = gPad->YtoAbsPixel(fY1);
      Int_t px2         = gPad->XtoAbsPixel(fX2);
      Int_t py2         = gPad->YtoAbsPixel(fY2);
      Double_t pl2      = Double_t((px2-px1)*(px2-px1) + (py1-py2)*(py1-py2));
      Double_t pixlength = TMath::Sqrt(pl2);
      pixwave           = hpixels*fWaveLength/pixlength;
      pixampl           = hpixels*fAmplitude/pixlength;
   } else {
      pixwave           = fWaveLength;
      pixampl           = fAmplitude;
   }
   Double_t anglestep = 20;
   Double_t phimaxle  = TMath::Pi() * 2. / anglestep ;
   Double_t length    = TMath::Sqrt((fX2-fX1)*(fX2-fX1) + (fY2-fY1)*(fY2-fY1));
   Double_t  ampl     = length*pixampl;
   Double_t wave      = length*pixwave;
   Double_t dx        = wave/25;
   Double_t len2pi    = dx * anglestep;
   if(length <= 4 * ampl){
      cout << "CurlyLine:: too short " << length << endl;
      SetBit(kTooShort);
      return;
   }
   Double_t angle = TMath::ATan2(fY2-fY1, fX2-fX1);
   if(angle < 0) angle += 2*TMath::Pi();
   Double_t  lengthcycle = 0.5*len2pi + 2*ampl;
   Int_t nperiods = (Int_t)((length - lengthcycle) / len2pi);
   Double_t restlenght = 0.5 * (length - nperiods * len2pi - lengthcycle);
   fNsteps = (Int_t)(anglestep * nperiods + anglestep / 2 +4);
   SetPolyLine(fNsteps);
   Double_t *xv = GetX();
   Double_t *yv = GetY();
   xv[0] = 0;          yv[0] = 0;
   xv[1] = restlenght; yv[1] = 0;
   Double_t phase =  1.5 * TMath::Pi();
   Double_t x0 = ampl + restlenght;
   Int_t i;
   for(i = 2; i < fNsteps-1; i++){
//  distinguish between curly and wavy
      if(fIsCurly) xv[i] = x0 + ampl*TMath::Sin(phase);
      else         xv[i] = x0;
      yv[i]  = ampl*TMath::Cos(phase);
      phase += phimaxle;
      x0    += dx;
   }
   xv[fNsteps-1] = length; yv[fNsteps-1] = 0;
   Double_t cosang = TMath::Cos(angle);
   Double_t sinang = TMath::Sin(angle);
   Double_t xx, yy;
   for(i = 0; i < fNsteps; i++){
      xx = xv[i] * cosang - yv[i] * sinang;
      yy = xv[i] * sinang + yv[i] * cosang;
      xv[i] = xx + fX1;
      yv[i] = yy + fY1;
   }
   if (gPad) gPad->Modified();
}

//______________________________________________________________________________
 Int_t TCurlyLine::DistancetoPrimitive(Int_t px, Int_t py)
{
//*-*-*-*-*-*-*-*-*-*-*Compute distance from point px,py to a line*-*-*-*-*-*
//*-*                  ===========================================

   return DistancetoLine(px,py,fX1,fY1,fX2,fY2);
}

//______________________________________________________________________________
void TCurlyLine::ExecuteEvent(Int_t event, Int_t px, Int_t py)
{
//*-*-*-*-*-*-*-*-*-*-*Execute action corresponding to one event*-*-*-*
//*-*                  =========================================
//  This member function is called when a  TCurlyLine is clicked with the locator
//
//  If Left button clicked on one of the line end points, this point
//     follows the cursor until button is released.
//
//  if Middle button clicked, the line is moved parallel to itself
//     until the button is released.
//

   Int_t kMaxDiff = 20;
   static Int_t d1,d2,px1,px2,py1,py2;
   static Int_t pxold, pyold, px1old, py1old, px2old, py2old;
   static Bool_t P1, P2, L;
   Int_t dx, dy;


   switch (event) {

   case kButton1Down:
      gVirtualX->SetLineColor(-1);
      TAttLine::Modify();  //Change line attributes only if necessary

      // No break !!!

   case kMouseMotion:

      px1 = gPad->XtoAbsPixel(fX1);
      py1 = gPad->YtoAbsPixel(fY1);
      px2 = gPad->XtoAbsPixel(fX2);
      py2 = gPad->YtoAbsPixel(fY2);

      P1 = P2 = L = kFALSE;

      d1  = TMath::Abs(px1 - px) + TMath::Abs(py1-py); //simply take sum of pixels differences
      if (d1 < kMaxDiff) { //*-*================>OK take point number 1
         px1old = px1; py1old = py1;
         P1 = kTRUE;
         gPad->SetCursor(kPointer);
         return;
      }
      d2  = TMath::Abs(px2 - px) + TMath::Abs(py2-py); //simply take sum of pixels differences
      if (d2 < kMaxDiff) { //*-*================>OK take point number 2
         px2old = px2; py2old = py2;
         P2 = kTRUE;
         gPad->SetCursor(kPointer);
         return;
      }

      L = kTRUE;
      pxold = px; pyold = py;
      gPad->SetCursor(kMove);

      break;

   case kButton1Motion:

      if (P1) {
         gVirtualX->DrawLine(px1old, py1old, px2, py2);
         gVirtualX->DrawLine(px, py, px2, py2);
         px1old = px;
         py1old = py;
      }
      if (P2) {
         gVirtualX->DrawLine(px1, py1, px2old, py2old);
         gVirtualX->DrawLine(px1, py1, px, py);
         px2old = px;
         py2old = py;
      }
      if (L) {
         gVirtualX->DrawLine(px1, py1, px2, py2);
         dx = px-pxold;  dy = py-pyold;
         px1 += dx; py1 += dy; px2 += dx; py2 += dy;
         gVirtualX->DrawLine(px1, py1, px2, py2);
         pxold = px;
         pyold = py;
      }
      break;

   case kButton1Up:

      if (P1) {
         fX1 = gPad->AbsPixeltoX(px);
         fY1 = gPad->AbsPixeltoY(py);
      }
      if (P2) {
         fX2 = gPad->AbsPixeltoX(px);
         fY2 = gPad->AbsPixeltoY(py);
      }
      if (L) {
         fX1 = gPad->AbsPixeltoX(px1);
         fY1 = gPad->AbsPixeltoY(py1);
         fX2 = gPad->AbsPixeltoX(px2);
         fY2 = gPad->AbsPixeltoY(py2);
      }
      Build();
      gPad->Modified();
      gVirtualX->SetLineColor(-1);
   }
}

//_____________________________________________________________________________________
void TCurlyLine::SavePrimitive(ofstream &out, Option_t *){
    // Save primitive as a C++ statement(s) on output stream out

   if (gROOT->ClassSaved(TCurlyLine::Class())) {
       out<<"   ";
   } else {
       out<<"   TCurlyLine *";
   }
   out<<"curlyline = new TCurlyLine("
     <<fX1<<","<<fY1<<","<<fX2<<","<<fY2<<","
     <<fWaveLength<<","<<fAmplitude<<");"<<endl;
   if (!fIsCurly) {
      out<<"   curlyline->SetWavy();"<<endl;
   }
   SaveLineAttributes(out,"curlyline",1,1,1);
   out<<"   curlyline->Draw();"<<endl;
}

//_____________________________________________________________________________________
void TCurlyLine::SetCurly()
{
   fIsCurly = kTRUE;
   Build();
}
void TCurlyLine::SetWavy()
{
   fIsCurly = kFALSE;
   Build();
}
void TCurlyLine::SetWaveLength(Double_t x)
{
   fWaveLength = x;
   Build();
}
void TCurlyLine::SetAmplitude(Double_t x)
{
   fAmplitude = x;
   Build();
}
void TCurlyLine::SetStartPoint(Double_t x, Double_t y)
{
   fX1 = x;
   fY1 = y;
   Build();
}
void TCurlyLine::SetEndPoint(Double_t x, Double_t y)
{
   fX2 = x;
   fY2 = y;
   Build();
}
