#include <algorithm>

#include <Cocoa/Cocoa.h>

//DEBUG ONLY
#include <iostream>
//

#include "TGQuartz.h"
#include "TColor.h"
#include "TROOT.h"

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
   CGContextRef ctx = (CGContextRef)fCtx;

   if (mode == kFilled) {
      const Float_t alpha = 1.f; //must be in gVirtualX.
      Float_t red = 1.f, green = 1.f, blue = 1.f; //White by default.
      if (const TColor *color = gROOT->GetColor(GetFillColor()))
         color->GetRGB(red, green, blue); 

      CGContextSetRGBFillColor(ctx, red, green, blue, alpha);
      CGContextSetRGBStrokeColor(ctx, red, green, blue, alpha);

      CGContextSetLineWidth(ctx, 1.f);
      CGContextFillRect(ctx, CGRectMake(x1, y1, x2 - x1, y2 - y1));
   } else {
      CGContextSetLineCap(ctx, kCGLineCapRound);
      CGContextSetLineJoin(ctx, kCGLineJoinMiter);
   
      if (GetLineWidth() > 1.f)
         CGContextSetLineWidth(ctx, GetLineWidth());

      const Float_t alpha = 1.f;//Must come from gVirtualX.
      Float_t red = 0.f, green = 0.f, blue = 0.f;//Black line by default.
   
      if (const TColor *color = gROOT->GetColor(gVirtualX->GetLineColor()))
         color->GetRGB(red, green, blue);

      CGContextSetRGBStrokeColor(ctx, red, green, blue, alpha);
      if (y1 > y2)
         std::swap(y1, y2);
      CGContextStrokeRect(ctx, CGRectMake(x1, y1, x2 - x1, y2 - y1));
   }
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
void TGQuartz::DrawLine(Int_t /*x1*/, Int_t /*y1*/, Int_t /*x2*/, Int_t /*y2*/)
{
   //CGContextRef ctx = (CGContextRef)fCtx; 
}

//______________________________________________________________________________
void TGQuartz::DrawPolyLine(Int_t /*n*/, TPoint * /*xy*/)
{
   //CGContextRef ctx = (CGContextRef)fCtx;
}

//______________________________________________________________________________
void TGQuartz::DrawPolyMarker(Int_t /*n*/, TPoint * /*xy*/)
{
   //CGContextRef ctx = (CGContextRef)fCtx;
}

//______________________________________________________________________________
void TGQuartz::DrawText(Int_t /*x*/, Int_t /*y*/, Float_t /*angle*/, Float_t /*mgn*/, const char * /*text*/, ETextMode /*mode*/)
{
   //CGContextRef ctx = (CGContextRef)fCtx;
}

//______________________________________________________________________________
void TGQuartz::SetContext(void *ctx)
{
   fCtx = ctx;
}
