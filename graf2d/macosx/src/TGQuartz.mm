// @(#)root/graf2d:$Id$
// Author: Olivier Couet, 23/01/2012

/*************************************************************************
 * Copyright (C) 1995-2011, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/


#include <algorithm>

#include <Cocoa/Cocoa.h>

//DEBUG ONLY
#include <iostream>
//

//#include "Cocoa"

#include "QuartzText.h"
#include "QuartzMarker.h"
#include "QuartzFillArea.h"
#include "CocoaPrivate.h"
#include "X11Drawables.h"
#include "QuartzLine.h"

#include "TGQuartz.h"
#include "TPoint.h"
#include "TColor.h"
#include "TMath.h"
#include "TStyle.h"
#include "TString.h"
#include "TObjString.h"
#include "TObjArray.h"
#include "TROOT.h"

const Double_t kPI = TMath::Pi();

using namespace ROOT;

ClassImp(TGQuartz)


//______________________________________________________________________________
TGQuartz::TGQuartz()
{
   // TGQuartz default constructor
}


//______________________________________________________________________________
TGQuartz::TGQuartz(const char *name, const char *title)
            : TGCocoa(name, title)
{
   // TGQuartz normal constructor
}


//______________________________________________________________________________
void TGQuartz::DrawBox(Int_t x1, Int_t y1, Int_t x2, Int_t y2, EBoxMode mode)
{
   // Draw a box

   CGContextRef ctx = (CGContextRef)GetCurrentContext();

   SetContextFillColor(GetFillColor());
   
   SetContextStrokeColor(GetLineColor());
   
   TColor *color = gROOT->GetColor(GetFillColor());
   if (!color) return;
   
   Float_t r = 0.f;
   Float_t g = 0.f;
   Float_t b = 0.f;
   Float_t a = 1.f;
   
   color->GetRGB(r, g, b);
   
   Quartz::SetFillStyle(ctx, GetFillStyle(), r, g, b, a);

   Quartz::DrawBox(ctx, x1, y1, x2, y2, (Int_t)mode);
}


//______________________________________________________________________________
void TGQuartz::DrawFillArea(Int_t n, TPoint * xy)
{
   // Draw a filled area through all points.
   // n         : number of points
   // xy        : list of points

   CGContextRef ctx = (CGContextRef)GetCurrentContext();

   SetContextStrokeColor(GetFillColor());

   SetContextFillColor(GetFillColor());

   TColor *color = gROOT->GetColor(GetFillColor());
   if (!color) return;
   
   Float_t r = 0.f;
   Float_t g = 0.f;
   Float_t b = 0.f;
   Float_t a = 1.f;

   color->GetRGB(r, g, b);

   Quartz::SetFillStyle(ctx, GetFillStyle(), r, g, b, a);

   Quartz::DrawFillArea(ctx, n, xy);
}


//______________________________________________________________________________
void TGQuartz::DrawCellArray(Int_t /*x1*/, Int_t /*y1*/, Int_t /*x2*/, Int_t /*y2*/, 
                             Int_t /*nx*/, Int_t /*ny*/, Int_t */*ic*/)
{
   // Draw CellArray
   
   //CGContextRef ctx = (CGContextRef)GetCurrentContext();
}


//______________________________________________________________________________
void TGQuartz::DrawLine(Int_t x1, Int_t y1, Int_t x2, Int_t y2)
{
   // Draw a line.
   // x1,y1        : begin of line
   // x2,y2        : end of line
      
   CGContextRef ctx = (CGContextRef)GetCurrentContext();
   
   SetContextStrokeColor(GetLineColor());
   
   Quartz::SetLineStyle(ctx, GetLineStyle());
   
   Quartz::SetLineWidth(ctx, GetLineWidth());
   
   Quartz::DrawLine(ctx, x1, y1, x2, y2);
}


//______________________________________________________________________________
void TGQuartz::DrawPolyLine(Int_t n, TPoint *xy)
{
   // Draw a line through all points.
   // n         : number of points
   // xy        : list of points   
   
   CGContextRef ctx = (CGContextRef)GetCurrentContext();
   
   SetContextStrokeColor(GetLineColor());
   
   Quartz::SetLineStyle(ctx, GetLineStyle());
   
   Quartz::SetLineWidth(ctx, GetLineWidth());

   Quartz::DrawPolyLine(ctx, n, xy);
}


//______________________________________________________________________________
void TGQuartz::DrawPolyMarker(Int_t n, TPoint *xy)
{
   // Draw PolyMarker
   // n         : number of points
   // xy        : list of points   
   
   CGContextRef ctx = (CGContextRef)GetCurrentContext();

   SetContextFillColor(GetMarkerColor());
   
   SetContextStrokeColor(GetMarkerColor());
   
   Quartz::SetLineStyle(ctx, 1);
   
   Quartz::SetLineWidth(ctx, 1);

   Quartz::DrawPolyMarker(ctx, n, xy, GetMarkerSize(), GetMarkerStyle());
}


//______________________________________________________________________________
void TGQuartz::DrawText(Int_t x, Int_t y, Float_t angle, Float_t /*mgn*/, 
                        const char *text, ETextMode /*mode*/)
{
   // Draw text
   
   CGContextRef ctx = (CGContextRef)GetCurrentContext();
 
   // Text color
   SetContextFillColor(GetTextColor());

   assert(fSelectedDrawable != 0 && "no pixmap selected");
   id<X11Drawable> pixmap = fPimpl->GetWindow(fSelectedDrawable);
   assert(pixmap.fIsPixmap == YES && "selected drawable is not a pixmap");
   
   CGContextSaveGState(ctx);
   
   CGContextTranslateCTM(ctx, 0.f, pixmap.fHeight);
   CGContextScaleCTM(ctx, 1.f, -1.f);
   
   Quartz::DrawText(ctx, (Double_t)x, ROOT::MacOSX::X11::LocalYROOTToCocoa(pixmap, y), angle, text);
   
   CGContextRestoreGState(ctx);

/*
 // Text rotation
   CGAffineTransform tm; 
   tm =  CGAffineTransformMakeRotation (-(angle*kPI)/180.);
   tm =  CGAffineTransformScale (tm, 1., -1.); 
   CGContextSetTextMatrix (ctx, tm); 
   
   // Draw the text
   CGContextShowTextAtPoint (ctx, (Float_t)x, (Float_t)y, text, strlen(text)); 
   
 CTFontRef currentFont = fFontManager->SelectFont(gVirtualX->GetTextFont(), 
 gVirtualX->GetTextSize());
   
  //    if (!fSelectedFont)
  //    throw std::runtime_error("GetTextBounds: font not selected");
   
   Font_t fontIndex = gVirtualX->GetTextFont() / 10 - 1;

   UInt_t w = 0, h = 0;
//   if (fontIndex == 11) {
//      CTLineGuard ctLine(text, fSelectedFont, fSymbolMap);
//      ctLine.GetBounds(w, h);
//   } else {
      CTLineGuard ctLine(text, fSelectedFont);
      ctLine.GetBounds(w, h);
//   }
   
   
   CGContextSaveGState(ctx);

   CGContextSetTextPosition(ctx, 0.f, 0.f);
   CGContextTranslateCTM(ctx, x, y);  
   CGContextRotateCTM(ctx, gVirtualX->GetTextAngle() * TMath::DegToRad());
   CGContextTranslateCTM(ctx, xc, yc);
   CGContextTranslateCTM(ctx, -0.5 * w, -0.5 * h);

   CTLineDraw(ctLine.fCTLine, ctx);
   CGContextRestoreGState(ctx);
 */
}

/*
//_________________________________________________________________
void Painter::DrawText(Double_t x, Double_t y, const CTLineGuard &ctLine)
{
   UInt_t w = 0, h = 0;
   ctLine.GetBounds(w, h);
   
   x = fConverter.XToView(x);
   y = fConverter.YToView(y);
   
   Double_t xc = 0., yc = 0.;
   
   const UInt_t hAlign = UInt_t(gVirtualX->GetTextAlign() / 10);   
   switch (hAlign) {
   case 1:
      xc = 0.5 * w;
      break;
   case 2:
      break;
   case 3:
      xc = -0.5 * w;
      break;
   }

   const UInt_t vAlign = UInt_t(gVirtualX->GetTextAlign() % 10);
   switch (vAlign) {
   case 1:
      yc = 0.5 * h;
      break;
   case 2:
      break;
   case 3:
      yc = -0.5 * h;
      break;
   }
   
   CGContextSaveGState(fCtx);

   CGContextSetTextPosition(fCtx, 0.f, 0.f);
   CGContextTranslateCTM(fCtx, x, y);  
   CGContextRotateCTM(fCtx, gVirtualX->GetTextAngle() * TMath::DegToRad());
   CGContextTranslateCTM(fCtx, xc, yc);
   CGContextTranslateCTM(fCtx, -0.5 * w, -0.5 * h);

   CTLineDraw(ctLine.fCTLine, fCtx);
   CGContextRestoreGState(fCtx);
}*/


//______________________________________________________________________________
void TGQuartz::SetContextFillColor(Int_t ci)
{
   // Set the current fill color in the current context.

   CGContextRef ctx = (CGContextRef)GetCurrentContext();

   TColor *color = gROOT->GetColor(ci);
   if (!color) return;

   const Float_t a = 1.f;
   Float_t r = 0.f;
   Float_t g = 0.f;
   Float_t b = 0.f;

   color->GetRGB(r, g, b);

   CGContextSetRGBFillColor (ctx, r, g, b, a);
}


//______________________________________________________________________________
void TGQuartz::SetContextStrokeColor(Int_t ci)
{
   // Set the current fill color in the current context.

   CGContextRef ctx = (CGContextRef)GetCurrentContext();

   TColor *color = gROOT->GetColor(ci);
   if (!color) return;

   const Float_t a = 1.f;
   Float_t r = 0.f;
   Float_t g = 0.f;
   Float_t b = 0.f;

   color->GetRGB(r, g, b);

   CGContextSetRGBStrokeColor (ctx, r, g, b, a);
}


//______________________________________________________________________________
void TGQuartz::SetLineColor(Color_t cindex)
{
   // Set color index "cindex" for drawing lines.

   TAttLine::SetLineColor(cindex);
}


//______________________________________________________________________________
void TGQuartz::SetLineStyle(Style_t lstyle)
{
   // Set line style.
   
   TAttLine::SetLineStyle(lstyle);
}


//______________________________________________________________________________
void TGQuartz::SetLineWidth(Width_t width)
{
   // Set the line width.
   
   TAttLine::SetLineWidth(width);
}


//______________________________________________________________________________
void TGQuartz::SetFillColor(Color_t cindex)
{
   // Set color index "cindex" for fill areas.

   TAttFill::SetFillColor(cindex);
}


//______________________________________________________________________________
void TGQuartz::SetFillStyle(Style_t style)
{
   // Set fill area style.
   
   TAttFill::SetFillStyle(style);
}


//______________________________________________________________________________
void TGQuartz::SetMarkerColor(Color_t cindex)
{
   // Set color index "cindex" for markers.

   TAttMarker::SetMarkerColor(cindex);
}


//______________________________________________________________________________
void TGQuartz::SetMarkerSize(Float_t markersize)
{
   // Set marker size index.
   //
   // markersize - the marker scale factor

   TAttMarker::SetMarkerSize(markersize);
}


//______________________________________________________________________________
void TGQuartz::SetMarkerStyle(Style_t markerstyle)
{
   // Set marker style.

   TAttMarker::SetMarkerStyle(markerstyle);
}


//______________________________________________________________________________
void TGQuartz::SetTextAlign(Short_t talign)
{
   // Set the text alignment.
   //
   // talign = txalh horizontal text alignment
   // talign = txalv vertical text alignment

   TAttText::SetTextAlign(talign);
}


//______________________________________________________________________________
void TGQuartz::SetTextColor(Color_t cindex)
{
   // Set the color index "cindex" for text.

   TAttText::SetTextColor(cindex);
}


//______________________________________________________________________________
void TGQuartz::SetTextFont(Font_t fontnumber)
{
   // Set the current text font number.

   TAttText::SetTextFont(fontnumber);
}


//______________________________________________________________________________
void TGQuartz::SetTextSize(Float_t textsize)
{
   // Set the current text size to "textsize"
   
   TAttText::SetTextSize(textsize);
}


//______________________________________________________________________________
void TGQuartz::SetOpacity(Int_t /*percent*/)
{
   // Set opacity of the current window. This image manipulation routine
   // works by adding to a percent amount of neutral to each pixels RGB.
   // Since it requires quite some additional color map entries is it
   // only supported on displays with more than > 8 color planes (> 256
   // colors).
}


//______________________________________________________________________________
Int_t TGQuartz::SetTextFont(char * /*fontname*/, ETextSetMode /*mode*/)
{
   // Set text font to specified name "fontname".This function returns 0 if
   // the specified font is found, 1 if it is not.
   //
   // mode - loading flag
   //        mode = 0 search if the font exist (kCheck)
   //        mode = 1 search the font and load it if it exists (kLoad)
   
   return 0;
}
