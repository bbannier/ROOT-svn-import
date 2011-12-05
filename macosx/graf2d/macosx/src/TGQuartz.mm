#include <algorithm>

#include <Cocoa/Cocoa.h>

//DEBUG ONLY
#include <iostream>
//

//#include "Cocoa"

#include "TGQuartz.h"
#include "TPoint.h"
#include "TColor.h"
#include "TMath.h"
#include "TROOT.h"

const Double_t kPI = TMath::Pi();

ClassImp(TGQuartz)

//______________________________________________________________________________
TGQuartz::TGQuartz()
{
   NSLog(@"TGQuartz default ctor");
}

//______________________________________________________________________________
TGQuartz::TGQuartz(const char *name, const char *title)
            : TGCocoa(name, title)
{
   NSLog(@"TGQuartz was created %s %s", name, title);
}

//______________________________________________________________________________
void TGQuartz::DrawBox(Int_t x1, Int_t y1, Int_t x2, Int_t y2, EBoxMode mode)
{
   //This is just a sketch implementation.
   
   //TODO: check fill style (it can be polygon with some stipple).
   //Think about gradient filling and transparency.
   //What about outline? (TODO).
   //What about different color spaces? (using RGB now, but ...)

   CGContextRef ctx = (CGContextRef)fCtx;//This is context from CoreGraphics, it's up to me (TGCocoa) to manage it.

   //This function modifies different state variables in context.
   //The same context will be used by other functions.
   //So either save and restore state (that's what I do here) or
   //REMEMBER to set these states later to correct values 
   //(correct from next operation's POV). 
   CGContextSaveGState(ctx);

   if (mode == kFilled) {
      //Fill the rectangle with solid color.
   
      const Float_t alpha = 1.f; //Alpha for filled area must be set externally, for example,
                                 //add transparency into TAttFill, add global fill transparency
                                 //and check them here.
      Float_t red = 1.f, green = 1.f, blue = 1.f; //White by default.
      //Color in gVirtualX is already set by primitive or by pad,
      //just extract RGB triplet here.
      if (const TColor *color = gROOT->GetColor(GetFillColor()))
         color->GetRGB(red, green, blue); 

      CGContextSetRGBFillColor(ctx, red, green, blue, alpha);
      CGContextFillRect(ctx, CGRectMake(x1, y1, x2 - x1, y2 - y1));
   } else {
      //Find better way to specify line attributes like line caps
      //and joins - TODO: this must go somehow to TAttLine, so we can set/get
      //these parameters externally/internally. In this example - just hardcoded values.
      CGContextSetLineCap(ctx, kCGLineCapRound);
      CGContextSetLineJoin(ctx, kCGLineJoinMiter);
   
      if (GetLineWidth() > 1.f)
         CGContextSetLineWidth(ctx, GetLineWidth());

      const Float_t alpha = 1.f; //Alpha for line must be set externally, for example,
                                 //add transparency into TAttLine, add global line transparency
                                 //and check them here.

      Float_t red = 0.f, green = 0.f, blue = 0.f;//Black line by default.
   
      if (const TColor *color = gROOT->GetColor(GetLineColor()))
         color->GetRGB(red, green, blue);

      CGContextSetRGBStrokeColor(ctx, red, green, blue, alpha);

      if (y1 > y2)
         std::swap(y1, y2);

      CGContextStrokeRect(ctx, CGRectMake(x1, y1, x2 - x1, y2 - y1));
   }
   
   CGContextRestoreGState(ctx);
}

//______________________________________________________________________________
void TGQuartz::DrawCellArray(Int_t /*x1*/, Int_t /*y1*/, Int_t /*x2*/, Int_t /*y2*/, Int_t /*nx*/, Int_t /*ny*/, Int_t */*ic*/)
{
//   CGContextRef ctx = (CGContextRef)fCtx;
}

//______________________________________________________________________________
void TGQuartz::DrawFillArea(Int_t /*n*/, TPoint * /*xy*/)
{
   //CGContextRef ctx = (CGContextRef)fCtx;
}

//______________________________________________________________________________
void TGQuartz::DrawLine(Int_t x1, Int_t y1, Int_t x2, Int_t y2)
{
   // Draw a line.
   // x1,y1        : begin of line
   // x2,y2        : end of line
      
   CGContextRef ctx = (CGContextRef)fCtx;
   
   const Float_t alpha = 1.f;
   
   Float_t red = 0.f, green = 0.f, blue = 0.f;//Black line by default.
   
   if (const TColor *color = gROOT->GetColor(GetLineColor()))
      color->GetRGB(red, green, blue);
   
   CGContextSetRGBStrokeColor(ctx, red, green, blue, alpha);
   
   CGContextBeginPath (ctx);
   CGContextMoveToPoint(ctx, x1, y1);
   CGContextAddLineToPoint(ctx, x2, y2);
   CGContextStrokePath(ctx);
}

//______________________________________________________________________________
void TGQuartz::DrawPolyLine(Int_t n, TPoint *xy)
{
   // Draw a line through all points.
   // n         : number of points
   // xy        : list of points   
   
   CGContextRef ctx = (CGContextRef)fCtx;
   
   const Float_t alpha = 1.f;
   
   Float_t red = 0.f, green = 0.f, blue = 0.f;//Black line by default.
   
   if (const TColor *color = gROOT->GetColor(GetLineColor()))
      color->GetRGB(red, green, blue);
   
   CGContextSetRGBStrokeColor(ctx, red, green, blue, alpha);
   
   CGContextBeginPath (ctx);
   for (Int_t i=1; i<n; i++) {
      CGContextMoveToPoint    (ctx, xy[i-1].fX, xy[i-1].fY);
      CGContextAddLineToPoint (ctx, xy[i].fX  , xy[i].fY);
   }
   CGContextStrokePath(ctx);
}

//______________________________________________________________________________
void TGQuartz::DrawPolyMarker(Int_t /*n*/, TPoint * /*xy*/)
{
   //CGContextRef ctx = (CGContextRef)fCtx;
}

//______________________________________________________________________________
void TGQuartz::DrawText(Int_t x, Int_t y, Float_t angle, Float_t mgn, const char *text, ETextMode /*mode*/)
{
   // Draw text
   
   CGContextRef ctx = (CGContextRef)fCtx;
 
   CGContextSelectFont (ctx, 
                        "Helvetica-Bold",
                        10,
                        kCGEncodingMacRoman);

   
   // Text color
   const Float_t alpha = 1.f;
   Float_t red = 0.f, green = 0.f, blue = 0.f;//Black line by default.

   if (const TColor *color = gROOT->GetColor(GetTextColor())) color->GetRGB(red, green, blue);
//   CGContextSetRGBStrokeColor (ctx, red, green, blue, alpha);
   CGContextSetRGBFillColor   (ctx, red, green, blue, alpha);

   // Text drawing mode
//   CGContextSetTextDrawingMode (ctx, kCGTextFillStroke); 
   CGContextSetTextDrawingMode (ctx, kCGTextFill); 



   // Text rotation
   CGAffineTransform tm; 
   tm =  CGAffineTransformMakeRotation (-(angle*kPI)/180.);
   tm =  CGAffineTransformScale (tm, 1., -1.); 
   CGContextSetTextMatrix (ctx, tm);    
   // Draw the text
   CGContextShowTextAtPoint (ctx, (Float_t)x, (Float_t)y, text, strlen(text)); 
}

//______________________________________________________________________________
void TGQuartz::SetContext(void *ctx)
{
   fCtx = ctx;
}
