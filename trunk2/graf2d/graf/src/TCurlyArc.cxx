// @(#)root/graf:$Id$
// Author: Otto Schaile   20/11/99

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

//______________________________________________________________________________
//
// This class implements curly or wavy arcs typically used to draw Feynman diagrams.
// Amplitudes and wavelengths may be specified in the constructors,
// via commands or interactively from popup menus.
// The class make use of TCurlyLine by inheritance, ExecuteEvent methods
// are highly inspired from the methods used in TPolyLine and TArc.
// The picture below has been generated by the tutorial feynman.
//Begin_Html
/*
<img src="gif/feynman.gif">
*/
//End_Html
//______________________________________________________________________________

#include "Riostream.h"
#include "TCurlyArc.h"
#include "TROOT.h"
#include "TVirtualPad.h"
#include "TVirtualX.h"
#include "TMath.h"

Double_t TCurlyArc::fgDefaultWaveLength = 0.02;
Double_t TCurlyArc::fgDefaultAmplitude  = 0.01; 
Bool_t   TCurlyArc::fgDefaultIsCurly    = kTRUE;   

ClassImp(TCurlyArc)


//______________________________________________________________________________
TCurlyArc::TCurlyArc()
{
   // Default constructor
   
   fR1     = 0.;
   fPhimin = 0.;
   fPhimax = 0.;
   fTheta  = 0.;
}


//______________________________________________________________________________
TCurlyArc::TCurlyArc(Double_t x1, Double_t y1,
                   Double_t rad, Double_t phimin, Double_t phimax,
                   Double_t wl, Double_t amp)
         : fR1(rad), fPhimin(phimin),fPhimax(phimax)
{
   // create a new TCurlyarc with center (x1, y1) and radius rad.
   // The wavelength and amplitude are given in percent of the line length
   // phimin and phimax are given in degrees.

   fX1         = x1;
   fY1         = y1;
   fIsCurly    = fgDefaultIsCurly;
   fAmplitude  = amp;
   fWaveLength = wl;
   fTheta      = 0;
   Build();
}


//______________________________________________________________________________
void TCurlyArc::Build()
{
   // Create a curly (Gluon) or wavy (Gamma) arc.

   Double_t pixeltoX = 1;
   Double_t pixeltoY = 1;
   Double_t rPix = fR1;
   if (gPad) {
   	Double_t ww = (Double_t)gPad->GetWw();
   	Double_t wh = (Double_t)gPad->GetWh();
   	Double_t pxrange = gPad->GetAbsWNDC()*ww;
   	Double_t pyrange = - gPad->GetAbsHNDC()*wh;
   	Double_t xrange  = gPad->GetX2() - gPad->GetX1();
   	Double_t yrange  = gPad->GetY2() - gPad->GetY1();
   	pixeltoX  = xrange / pxrange;
   	pixeltoY  = yrange/pyrange;
      rPix = fR1 / pixeltoX;
   }
   Double_t dang = fPhimax - fPhimin;
   if(dang < 0) dang += 360;
   Double_t length = TMath::Pi() * fR1 * dang/180;
   Double_t x1sav = fX1;
   Double_t y1sav = fY1;
   fX1 = fY1 = 0;
   fX2 = length;
   fY2 = 0;
   TCurlyLine::Build();
   fX1 = x1sav;
   fY1 = y1sav;
   Double_t *xv= GetX();
   Double_t *yv= GetY();
   Double_t xx, yy, angle;
   for(Int_t i = 0; i < fNsteps; i++){
      angle = xv[i] / rPix + fPhimin * TMath::Pi()/180;
      xx    = (yv[i] + rPix) * cos(angle);
      yy    = (yv[i] + rPix) * sin(angle);
      xx *= pixeltoX;
      yy *= TMath::Abs(pixeltoY);
      xv[i] = xx + fX1;
      yv[i] = yy + fY1;
   }
   if (gPad) gPad->Modified();
}


//______________________________________________________________________________
Int_t TCurlyArc::DistancetoPrimitive(Int_t px, Int_t py)
{
   // Compute distance from point px,py to an arc.
   //
   //  Compute the closest distance of approach from point px,py to this arc.
   //  The distance is computed in pixels units.
   //

   // Compute distance of point to center of arc
   Int_t pxc    = gPad->XtoAbsPixel(fX1);
   Int_t pyc    = gPad->YtoAbsPixel(fY1);
   Double_t dist = TMath::Sqrt(Double_t((pxc-px)*(pxc-px)+(pyc-py)*(pyc-py)));
   Double_t cosa = (px - pxc)/dist;
   Double_t sina = (pyc - py)/dist;
   Double_t phi  = TMath::ATan2(sina,cosa);
   if(phi < 0) phi += 2 * TMath::Pi();
   phi = phi * 180 / TMath::Pi();
   if(fPhimax > fPhimin){
      if(phi < fPhimin || phi > fPhimax) return 9999;
   } else {
      if(phi > fPhimin && phi < fPhimax) return 9999;
   }
   Int_t pxr = gPad->XtoPixel(fR1)- gPad->XtoPixel(0);
   Double_t distr = TMath::Abs(dist-pxr);
   return Int_t(distr);
}


//______________________________________________________________________________
void TCurlyArc::ExecuteEvent(Int_t event, Int_t px, Int_t py)
{
   // Execute action corresponding to one event.
   //
   //  This member function is called when a TCurlyArc is clicked with the locator
   //
   //  If Left button clicked on one of the line end points, this point
   //     follows the cursor until button is released.
   //
   //  if Middle button clicked, the line is moved parallel to itself
   //     until the button is released.

   Int_t kMaxDiff = 10;
   const Int_t np = 10;
   const Double_t pi = 3.141592;
   static Int_t x[np+3], y[np+3];
   static Int_t px1,py1,npe,r1;
   static Int_t pxold, pyold;
   Int_t i, dpx, dpy;
   Double_t angle,dx,dy,dphi,rLx,rRx;
   Double_t  phi0;
   static Bool_t pTop, pL, pR, pBot, pINSIDE;
   static Int_t pTx,pTy,pLx,pLy,pRx,pRy,pBx,pBy;

   switch (event) {

   case kButton1Down:
      gVirtualX->SetLineColor(-1);
      TAttLine::Modify();
      dphi = (fPhimax-fPhimin) * pi / 180;
      if(dphi<0) dphi += 2 * pi;
      dphi /= np;
      phi0 = fPhimin * pi / 180;
      for (i=0;i<=np;i++) {
         angle = Double_t(i)*dphi + phi0;
         dx    = fR1*TMath::Cos(angle);
         dy    = fR1*TMath::Sin(angle);
         Int_t rpixY = gPad->XtoAbsPixel(dy) - gPad->XtoAbsPixel(0);
         x[i]  = gPad->XtoAbsPixel(fX1 + dx);
         y[i]  = gPad->YtoAbsPixel(fY1) + rpixY;
      }
      if (fPhimax-fPhimin >= 360 ) {
         x[np+1] = x[0];
         y[np+1] = y[0];
         npe = np;
      } else {
         x[np+1]   = gPad->XtoAbsPixel(fX1);
         y[np+1]   = gPad->YtoAbsPixel(fY1);
         x[np+2] = x[0];
         y[np+2] = y[0];
         npe = np + 2;
      }
      px1 = gPad->XtoAbsPixel(fX1);
      py1 = gPad->YtoAbsPixel(fY1);
      pTx = pBx = px1;
      pLy = pRy = py1;
      pLx = gPad->XtoAbsPixel(-fR1+fX1);
      pRx = gPad->XtoAbsPixel( fR1+fX1);
      r1 = TMath::Abs(pLx-pRx)/2;
      // a circle in pixels, radius measured along X     
      pTy = gPad->YtoAbsPixel(fY1) + r1;
      pBy = gPad->YtoAbsPixel(fY1) - r1;

      gVirtualX->DrawLine(pRx+4, py1+4, pRx-4, py1+4);
      gVirtualX->DrawLine(pRx-4, py1+4, pRx-4, py1-4);
      gVirtualX->DrawLine(pRx-4, py1-4, pRx+4, py1-4);
      gVirtualX->DrawLine(pRx+4, py1-4, pRx+4, py1+4);
      gVirtualX->DrawLine(pLx+4, py1+4, pLx-4, py1+4);
      gVirtualX->DrawLine(pLx-4, py1+4, pLx-4, py1-4);
      gVirtualX->DrawLine(pLx-4, py1-4, pLx+4, py1-4);
      gVirtualX->DrawLine(pLx+4, py1-4, pLx+4, py1+4);
      gVirtualX->DrawLine(px1+4, pBy+4, px1-4, pBy+4);
      gVirtualX->DrawLine(px1-4, pBy+4, px1-4, pBy-4);
      gVirtualX->DrawLine(px1-4, pBy-4, px1+4, pBy-4);
      gVirtualX->DrawLine(px1+4, pBy-4, px1+4, pBy+4);
      gVirtualX->DrawLine(px1+4, pTy+4, px1-4, pTy+4);
      gVirtualX->DrawLine(px1-4, pTy+4, px1-4, pTy-4);
      gVirtualX->DrawLine(px1-4, pTy-4, px1+4, pTy-4);
      gVirtualX->DrawLine(px1+4, pTy-4, px1+4, pTy+4);
      // No break !!!

   case kMouseMotion:
      px1 = gPad->XtoAbsPixel(fX1);
      py1 = gPad->YtoAbsPixel(fY1);
      pTx = pBx = px1;
      pLy = pRy = py1;
      pLx = gPad->XtoAbsPixel(-fR1+fX1);
      pRx = gPad->XtoAbsPixel( fR1+fX1);
      
      pTy = gPad->YtoAbsPixel(fY1) + TMath::Abs(pLx-pRx)/2;
      pBy = gPad->YtoAbsPixel(fY1) - TMath::Abs(pLx-pRx)/2;

      pTop = pL = pR = pBot = pINSIDE = kFALSE;
      if ((TMath::Abs(px - pTx) < kMaxDiff) &&
          (TMath::Abs(py - pTy) < kMaxDiff)) {             // top edge
         pTop = kTRUE;
         gPad->SetCursor(kTopSide);
      }
      else
      if ((TMath::Abs(px - pBx) < kMaxDiff) &&
          (TMath::Abs(py - pBy) < kMaxDiff)) {             // bottom edge
         pBot = kTRUE;
         gPad->SetCursor(kBottomSide);
      }
      else
      if ((TMath::Abs(py - pLy) < kMaxDiff) &&
          (TMath::Abs(px - pLx) < kMaxDiff)) {             // left edge
         pL = kTRUE;
         gPad->SetCursor(kLeftSide);
      }
      else
      if ((TMath::Abs(py - pRy) < kMaxDiff) &&
          (TMath::Abs(px - pRx) < kMaxDiff)) {             // right edge
         pR = kTRUE;
         gPad->SetCursor(kRightSide);
      }
      else {pINSIDE= kTRUE; gPad->SetCursor(kMove); }
      pxold = px;  pyold = py;

      break;

   case kButton1Motion:
      gVirtualX->DrawLine(pRx+4, py1+4, pRx-4, py1+4);
      gVirtualX->DrawLine(pRx-4, py1+4, pRx-4, py1-4);
      gVirtualX->DrawLine(pRx-4, py1-4, pRx+4, py1-4);
      gVirtualX->DrawLine(pRx+4, py1-4, pRx+4, py1+4);
      gVirtualX->DrawLine(pLx+4, py1+4, pLx-4, py1+4);
      gVirtualX->DrawLine(pLx-4, py1+4, pLx-4, py1-4);
      gVirtualX->DrawLine(pLx-4, py1-4, pLx+4, py1-4);
      gVirtualX->DrawLine(pLx+4, py1-4, pLx+4, py1+4);
      gVirtualX->DrawLine(px1+4, pBy+4, px1-4, pBy+4);
      gVirtualX->DrawLine(px1-4, pBy+4, px1-4, pBy-4);
      gVirtualX->DrawLine(px1-4, pBy-4, px1+4, pBy-4);
      gVirtualX->DrawLine(px1+4, pBy-4, px1+4, pBy+4);
      gVirtualX->DrawLine(px1+4, pTy+4, px1-4, pTy+4);
      gVirtualX->DrawLine(px1-4, pTy+4, px1-4, pTy-4);
      gVirtualX->DrawLine(px1-4, pTy-4, px1+4, pTy-4);
      gVirtualX->DrawLine(px1+4, pTy-4, px1+4, pTy+4);
      for (i=0;i<npe;i++) gVirtualX->DrawLine(x[i], y[i], x[i+1], y[i+1]);
      if (pTop) {
         r1 -= (py - pyold);
      }
      if (pBot) {
         r1 += (py - pyold);
      }
      if (pL) {
         r1 -= (px - pxold);
      }
      if (pR) {
         r1 += (px - pxold);
      }
      if (pTop || pBot || pL || pR) {
         gVirtualX->SetLineColor(-1);
         TAttLine::Modify();
         dphi = (fPhimax-fPhimin) * pi / 180;
         if(dphi<0) dphi += 2 * pi;
         dphi /= np;
         phi0 = fPhimin * pi / 180;
         Double_t ur1 = r1;
         Int_t pX1   = gPad->XtoAbsPixel(fX1);
         Int_t pY1   = gPad->YtoAbsPixel(fY1);
         for (i=0;i<=np;i++) {
            angle = Double_t(i)*dphi + phi0;
            dx    = ur1 * TMath::Cos(angle);
            dy    = ur1 * TMath::Sin(angle);
            x[i]  = pX1 + (Int_t)dx;
            y[i]  = pY1 + (Int_t)dy;
         }
         if (fPhimax-fPhimin >= 360 ) {
            x[np+1] = x[0];
            y[np+1] = y[0];
            npe = np;
         } else {
            x[np+1]   = pX1;
            y[np+1]   = pY1;
            x[np+2] = x[0];
            y[np+2] = y[0];
            npe = np + 2;
         }
         for (i=0;i<npe;i++) {
            gVirtualX->DrawLine(x[i], y[i], x[i+1], y[i+1]);
         }
      }
      if (pINSIDE) {
         dpx  = px-pxold;  dpy = py-pyold;
         px1 += dpx; py1 += dpy;
         for (i=0;i<=npe;i++) { x[i] += dpx; y[i] += dpy;}
         for (i=0;i<npe;i++) gVirtualX->DrawLine(x[i], y[i], x[i+1], y[i+1]);
      }
      pTx = pBx = px1;
      pRx = px1+r1;
      pLx = px1-r1;
      pRy = pLy = py1;
      pTy = py1-r1;
      pBy = py1+r1;
      gVirtualX->DrawLine(pRx+4, py1+4, pRx-4, py1+4);
      gVirtualX->DrawLine(pRx-4, py1+4, pRx-4, py1-4);
      gVirtualX->DrawLine(pRx-4, py1-4, pRx+4, py1-4);
      gVirtualX->DrawLine(pRx+4, py1-4, pRx+4, py1+4);
      gVirtualX->DrawLine(pLx+4, py1+4, pLx-4, py1+4);
      gVirtualX->DrawLine(pLx-4, py1+4, pLx-4, py1-4);
      gVirtualX->DrawLine(pLx-4, py1-4, pLx+4, py1-4);
      gVirtualX->DrawLine(pLx+4, py1-4, pLx+4, py1+4);
      gVirtualX->DrawLine(px1+4, pBy+4, px1-4, pBy+4);
      gVirtualX->DrawLine(px1-4, pBy+4, px1-4, pBy-4);
      gVirtualX->DrawLine(px1-4, pBy-4, px1+4, pBy-4);
      gVirtualX->DrawLine(px1+4, pBy-4, px1+4, pBy+4);
      gVirtualX->DrawLine(px1+4, pTy+4, px1-4, pTy+4);
      gVirtualX->DrawLine(px1-4, pTy+4, px1-4, pTy-4);
      gVirtualX->DrawLine(px1-4, pTy-4, px1+4, pTy-4);
      gVirtualX->DrawLine(px1+4, pTy-4, px1+4, pTy+4);
      pxold = px;
      pyold = py;
      break;

   case kButton1Up:
      fX1 = gPad->AbsPixeltoX(px1);
      fY1 = gPad->AbsPixeltoY(py1);
      //rBy = gPad->AbsPixeltoY(py1+r1);
      //rTy = gPad->AbsPixeltoY(py1-r1);
      rLx = gPad->AbsPixeltoX(px1+r1);
      rRx = gPad->AbsPixeltoX(px1-r1);
      fR1 = TMath::Abs(rRx-rLx)/2;
      Build();
      gPad->Modified(kTRUE);
      gVirtualX->SetLineColor(-1);
   }
}


//______________________________________________________________________________
void TCurlyArc::SavePrimitive(std::ostream &out, Option_t * /*= ""*/)
{
   // Save primitive as a C++ statement(s) on output stream out

   if (gROOT->ClassSaved(TCurlyArc::Class())) {
      out<<"   ";
   } else {
      out<<"   TCurlyArc *";
   }
   out<<"curlyarc = new TCurlyArc("
     <<fX1<<","<<fY1<<","<<fR1<<","<<fPhimin<<","<<fPhimax<<","
      <<fWaveLength<<","<<fAmplitude<<");"<<std::endl;
   if (!fIsCurly) {
      out<<"   curlyarc->SetWavy();"<<std::endl;
   }
   SaveLineAttributes(out,"curlyarc",1,1,1);
   out<<"   curlyarc->Draw();"<<std::endl;
}


//______________________________________________________________________________
void TCurlyArc::SetCenter(Double_t x, Double_t y)
{
   // Set Curly Arc center.

   fX1 = x;
   fY1 = y;
   Build();
}


//______________________________________________________________________________
void TCurlyArc::SetRadius(Double_t x)
{
   // Set Curly Arc radius.

   fR1 = x;
   Build();
}


//______________________________________________________________________________
void TCurlyArc::SetPhimin(Double_t x)
{
   // Set Curly Arc minimum Phi.

   fPhimin = x;
   Build();
}


//______________________________________________________________________________
void TCurlyArc::SetPhimax(Double_t x)
{
   // Set Curly Arc maximum Phi.

   fPhimax = x;
   Build();
}


//______________________________________________________________________________
void TCurlyArc::SetDefaultWaveLength(Double_t WaveLength)
{
   // Set default wave length.

   fgDefaultWaveLength = WaveLength;
}


//______________________________________________________________________________
void TCurlyArc::SetDefaultAmplitude(Double_t Amplitude)
{
   // Set default wave amplitude.

   fgDefaultAmplitude = Amplitude ;
}


//______________________________________________________________________________
void TCurlyArc::SetDefaultIsCurly(Bool_t IsCurly)
{
   // Set default "IsCurly".

   fgDefaultIsCurly = IsCurly;
}


//______________________________________________________________________________
Double_t TCurlyArc::GetDefaultWaveLength()
{
   // Get default wave length.

   return fgDefaultWaveLength;
}


//______________________________________________________________________________
Double_t TCurlyArc::GetDefaultAmplitude()
{
   // Get default wave amplitude.

   return fgDefaultAmplitude;
}


//______________________________________________________________________________
Bool_t TCurlyArc::GetDefaultIsCurly()
{
   // Get default "IsCurly".

   return fgDefaultIsCurly;
}
