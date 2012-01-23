// @(#)root/graf2d:$Id$
// Author: Olivier Couet, 23/01/2012

/*************************************************************************
 * Copyright (C) 1995-2011, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/


#include "QuartzFillArea.h"
#include "RStipples.h"

//#ifndef ROOT_Rtypes
//#include "Rtypes.h"
//#endif

static Int_t gFillHollow;  // Flag if fill style is hollow
static Int_t gFillPattern; // Fill pattern

namespace ROOT {
namespace Quartz {

   
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
void DrawFillArea(CGContextRef ctx, Int_t n, TPoint * xy)
{
   // Draw a filled area through all points.
   // n         : number of points
   // xy        : list of points
                  
   CGContextBeginPath (ctx);
      
   CGContextMoveToPoint (ctx, xy[0].fX, xy[0].fY);
      
   for (Int_t i=1; i<n; i++) CGContextAddLineToPoint (ctx, xy[i].fX, xy[i].fY);
      
   if (gFillHollow) CGContextStrokePath(ctx);
   else             CGContextFillPath(ctx);
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
