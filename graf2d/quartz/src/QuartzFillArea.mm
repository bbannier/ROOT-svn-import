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
#include <cassert>
#include <vector>

#include "QuartzFillArea.h"
#include "TColorGradient.h"
#include "CocoaUtils.h"
#include "RStipples.h"
#include "TError.h"
#include "TROOT.h"

static Int_t gFillHollow;  // Flag if fill style is hollow
static Int_t gFillPattern; // Fill pattern

namespace ROOT {
namespace Quartz {

namespace {

const CGSize shadowOffset = CGSizeMake(10., 10.);
const CGFloat shadowBlur = 5.;

//______________________________________________________________________________
void ReadGradientColors(const TColorGradient *extendedColor, std::vector<CGFloat> &colors)
{
   assert(extendedColor != nullptr && "ReadGradientColors, extendedColor parameter is null");
   
   typedef TColorGradient::SizeType_t size_type;
   
   colors.resize(3 * 4);
   const Color_t *colorIndices = extendedColor->GetColors();
   Float_t rgb[3];
   for (size_type i = 0, pos = 0, e = extendedColor->GetNumberOfSteps(); i < e; ++i, pos += 4) {
      if (const TColor *clearColor = gROOT->GetColor(colorIndices[i])) {
         clearColor->GetRGB(rgb[0], rgb[1], rgb[2]);
         colors[pos] = rgb[0];
         colors[pos + 1] = rgb[1];
         colors[pos + 2] = rgb[2];
         colors[pos + 3] = clearColor->GetAlpha();
      } else {
         ::Warning("ReadGradientColors", "No color for index %d was found in gROOT's list of colors", colorIndices[i]);
         colors[pos] = 0.;
         colors[pos + 1] = 0.;
         colors[pos + 2] = 0.;
         colors[pos + 3] = 1.;
      }
   }
}

}

   
//______________________________________________________________________________
void DrawBox(CGContextRef ctx, Int_t x1, Int_t y1, Int_t x2, Int_t y2,
             Int_t mode)
{
   // Draw a box
            
   if (x1 > x2) std::swap(x1, x2);
   if (y1 > y2) std::swap(y1, y2);

   if (mode) CGContextFillRect(ctx, CGRectMake(x1, y1, x2 - x1, y2 - y1));
   else      CGContextStrokeRect(ctx, CGRectMake(x1, y1, x2 - x1, y2 - y1));
}

//______________________________________________________________________________
void DrawBoxGradient(CGContextRef ctx, Int_t x1, Int_t y1, Int_t x2, Int_t y2, const TColorGradient *extendedColor)
{
   using ROOT::MacOSX::Util::CFScopeGuard;

   assert(ctx != nullptr && "DrawBoxGradient, ctx parameter is null");
   assert(extendedColor != nullptr && "DrawBoxGradient, extendedColor parameter is null");
   assert(extendedColor->GetNumberOfSteps() != 0 && "DrawBoxGradient, no colors in extendedColor");
   
   CGPoint startPoint = CGPointZero;
   CGPoint endPoint = CGPointZero;
   
   if (extendedColor->GetGradientDirection() == TColorGradient::kGDHorizontal)
      endPoint.x = x2;
   else
      endPoint.y = y2;
      
   if (extendedColor->HasShadow()) {
      //To have shadow and gradient at the same time,
      //I first have to fill polygon, and after that
      //draw gradient (since gradient fills the whole area
      //with clip path and generates no shadow).
      CGContextSetRGBFillColor(ctx, 1., 1., 1., 0.5);
      CGContextSetShadow(ctx, shadowOffset, shadowBlur);
      CGContextFillRect(ctx, CGRectMake(x1, y1, x2 - x1, y2 - y1));
   }
   
   CGContextBeginPath(ctx);
   CGContextAddRect(ctx, CGRectMake(x1, y1, x2 - x1, y2 - y1));
   CGContextClosePath(ctx);
   CGContextClip(ctx);
   

   //Create a gradient.
   const CFScopeGuard<CGColorSpaceRef> baseSpace(CGColorSpaceCreateDeviceRGB());
   
   std::vector<CGFloat> colors;
   ReadGradientColors(extendedColor, colors);

   const CFScopeGuard<CGGradientRef> gradient(CGGradientCreateWithColorComponents(baseSpace.Get(), &colors[0], extendedColor->GetColorPositions(), extendedColor->GetNumberOfSteps()));
   CGContextDrawLinearGradient(ctx, gradient.Get(), startPoint, endPoint, 0);
}

//______________________________________________________________________________
void DrawFillArea(CGContextRef ctx, Int_t n, TPoint * xy, Bool_t shadow)
{
   // Draw a filled area through all points.
   // n         : number of points
   // xy        : list of points
                  
   CGContextBeginPath (ctx);
      
   CGContextMoveToPoint (ctx, xy[0].fX, xy[0].fY);
   for (Int_t i = 1; i < n; ++i) 
      CGContextAddLineToPoint (ctx, xy[i].fX, xy[i].fY);

   CGContextClosePath(ctx);
      
   if (gFillHollow) 
      CGContextStrokePath(ctx);
   else {
      if (shadow)
         CGContextSetShadow(ctx, shadowOffset, shadowBlur);
      
      CGContextFillPath(ctx);
   }
}

//______________________________________________________________________________
void DrawFillAreaGradient(CGContextRef ctx, Int_t nPoints, const TPoint *xy, const TColorGradient *extendedColor)
{
   using ROOT::MacOSX::Util::CFScopeGuard;

   assert(ctx != nullptr && "DrawFillAreaGradient, ctx parameter is null");
   assert(nPoints != 0 && "DrawFillAreaGradient, nPoints parameter is 0");
   assert(xy != nullptr && "DrawFillAreaGradient, xy parameter is null");
   assert(extendedColor != nullptr && "DrawFillAreaGradient, extendedColor parameter is null");

   const CFScopeGuard<CGMutablePathRef> path(CGPathCreateMutable());
   if (!path.Get()) {
      ::Error("DrawFillAreaGradient", "CGPathCreateMutable failed");
      return;
   }

   CGPathMoveToPoint(path.Get(), nullptr, xy[0].fX, xy[0].fY);
   for (Int_t i = 1; i < nPoints; ++i)
      CGPathAddLineToPoint(path.Get(), nullptr, xy[i].fX, xy[i].fY);
   
   CGPathCloseSubpath(path.Get());

   //Calculate gradient's start and end point (either X or Y coordinate,
   //depending on gradient type). Also, fill CGPath object.
   CGPoint startPoint = CGPointZero;
   CGPoint endPoint = CGPointZero;
   
   if (extendedColor->GetGradientDirection() == TColorGradient::kGDHorizontal) {
      startPoint = CGPointMake(xy[0].fX, 0.);
      endPoint = CGPointMake(xy[0].fX, 0.);
   
      for (Int_t i = 1; i < nPoints; ++i) {
         startPoint.x = std::min(startPoint.x, CGFloat(xy[i].fX));
         endPoint.x = std::max(endPoint.x, CGFloat(xy[i].fX));
      }
   } else {
      startPoint = CGPointMake(0., xy[0].fY);
      endPoint = CGPointMake(0., xy[0].fY);
   
      for (Int_t i = 1; i < nPoints; ++i) {
         startPoint.y = std::min(startPoint.y, CGFloat(xy[i].fY));
         endPoint.y = std::max(endPoint.y, CGFloat(xy[i].fY));
      }
   }
   
   if (extendedColor->HasShadow()) {
      //To have shadow and gradient at the same time,
      //I first have to fill polygon, and after that
      //draw gradient (since gradient fills the whole area
      //with clip path and generates no shadow).
      CGContextSetRGBFillColor(ctx, 1., 1., 1., 0.5);
      CGContextBeginPath(ctx);
      CGContextAddPath(ctx, path.Get());
      CGContextSetShadow(ctx, shadowOffset, shadowBlur);
      CGContextFillPath(ctx);
   }

   CGContextBeginPath(ctx);
   CGContextAddPath(ctx, path.Get());
   CGContextClip(ctx);

   //Create a gradient.
   const CFScopeGuard<CGColorSpaceRef> baseSpace(CGColorSpaceCreateDeviceRGB());
   std::vector<CGFloat> colors;
   ReadGradientColors(extendedColor, colors);
   const CFScopeGuard<CGGradientRef> gradient(CGGradientCreateWithColorComponents(baseSpace.Get(), &colors[0],
                                                                                  extendedColor->GetColorPositions(),
                                                                                  extendedColor->GetNumberOfSteps()));
   CGContextDrawLinearGradient(ctx, gradient.Get(), startPoint, endPoint, 0);
}
   
//______________________________________________________________________________
void SetFillStyle(CGContextRef ctx, Int_t style, 
                  Float_t r, Float_t g, Float_t b, Float_t a)

{
   // Set fill area style.
   //
   // style - compound fill area interior style
   //         style = 1000 * interiorstyle + styleindex
      
   Int_t fais = style/1000;
   Int_t fasi = style%1000;   
   
   gFillHollow  = 0;
   gFillPattern = 0;     

   switch (fais) {
      case 1:         // solid
         break;
            
      case 2:         // pattern
         gFillHollow = 1;
         break;
            
      case 3:         // hatch
         gFillHollow  = 0;
         gFillPattern = fasi;
         SetStencilPattern(ctx, r, g, b, a);
         break;
            
      default:
         gFillHollow = 1;
         break;
   }
}

   
//______________________________________________________________________________
void DrawStencil (void *sti, CGContextRef ctx)
{
   // Draw a stencil pattern from gStipples
      
   int i,j;
      
   int *st = static_cast<int *>(sti);
      
   int x , y=0; 
   for (i=0; i<31; i=i+2) {
      x = 0;
      for (j=0; j<8; j++) {
         if (gStipples[*st][i] & (1<<j)) 
            CGContextFillRect(ctx, CGRectMake(x, y, 1, 1));
         x++;
      }
      for (j=0; j<8; j++) {
         if (gStipples[*st][i+1] & (1<<j)) 
            CGContextFillRect(ctx, CGRectMake(x, y, 1, 1));
         x++;
      }
      y++;
   }
}


//______________________________________________________________________________
void SetStencilPattern(CGContextRef ctx, 
                       Float_t r, Float_t g, Float_t b, Float_t a)
{
   // Set the fill pattern
      
   CGPatternRef pattern;
   CGColorSpaceRef baseSpace;
   CGColorSpaceRef patternSpace;
            
   CGFloat RGB[4];
   RGB[0] = r;
   RGB[1] = g;
   RGB[2] = b;
   RGB[3] = a;
   CGPatternCallbacks callbacks = {0, &DrawStencil, NULL};
      
   baseSpace    = CGColorSpaceCreateDeviceRGB ();
   patternSpace = CGColorSpaceCreatePattern (baseSpace);
   CGContextSetFillColorSpace (ctx, patternSpace);
   CGColorSpaceRelease (patternSpace);
   CGColorSpaceRelease (baseSpace);
      
   pattern = CGPatternCreate(&gFillPattern, CGRectMake(0, 0, 16, 16),
                             CGAffineTransformIdentity, 16, 16,
                             kCGPatternTilingConstantSpacing,
                             false, &callbacks);
   CGContextSetFillPattern (ctx, pattern, RGB);
   CGPatternRelease (pattern);
}

}//namespace Quartz
}//namespace ROOT
