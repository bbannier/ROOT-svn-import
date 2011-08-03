#include "TAttMarker.h"
#include "TVirtualX.h"
#include "TColor.h"
#include "TROOT.h"

#include "GraphicUtils.h"

namespace ROOT_iOS {
namespace GraphicUtils {

//______________________________________________________________________________
void draw_pattern_1(void *, CGContextRef ctx)
{
   Float_t red = 0.f, green = 0.f, blue = 0.f, alpha = 1.f;
   GetColorForIndex(gVirtualX->GetFillColor(), red, green, blue);
   CGContextSetRGBFillColor(ctx, red, green, blue, alpha);
   
   CGContextFillRect(ctx, CGRectMake(0.f, 0.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(1.f, 1.f, 1.f, 1.f));
}

//______________________________________________________________________________
CGPatternRef create_pattern_1()
{
   CGPatternCallbacks patternCallbacks;
   patternCallbacks.version = 0;
   patternCallbacks.drawPattern = draw_pattern_1;
   patternCallbacks.releaseInfo = 0;
   
   return CGPatternCreate(0, CGRectMake(0.f, 0.f, 2.f, 2.f), CGAffineTransformIdentity, 2.f, 2.f, kCGPatternTilingConstantSpacingMinimalDistortion, 1, &patternCallbacks);
}

//______________________________________________________________________________
void draw_pattern_2(void *, CGContextRef ctx)
{
   Float_t red = 0.f, green = 0.f, blue = 0.f, alpha = 1.f;
   GetColorForIndex(gVirtualX->GetFillColor(), red, green, blue);
   CGContextSetRGBFillColor(ctx, red, green, blue, alpha);
   
   CGContextFillRect(ctx, CGRectMake(2.f, 0.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(0.f, 1.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(2.f, 2.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(0.f, 3.f, 1.f, 1.f));

}

//______________________________________________________________________________
CGPatternRef create_pattern_2()
{
   CGPatternCallbacks patternCallbacks;
   patternCallbacks.version = 0;
   patternCallbacks.drawPattern = draw_pattern_2;
   patternCallbacks.releaseInfo = 0;
   
   return CGPatternCreate(0, CGRectMake(0.f, 0.f, 4.f, 4.f), CGAffineTransformIdentity, 4.f, 4.f, kCGPatternTilingConstantSpacingMinimalDistortion, 1, &patternCallbacks);
}

//______________________________________________________________________________
void draw_pattern_3(void *, CGContextRef ctx)
{
   Float_t red = 0.f, green = 0.f, blue = 0.f, alpha = 1.f;
   GetColorForIndex(gVirtualX->GetFillColor(), red, green, blue);
   CGContextSetRGBFillColor(ctx, red, green, blue, alpha);
   
   CGContextFillRect(ctx, CGRectMake(0.f, 0.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(2.f, 2.f, 1.f, 1.f));
}

//______________________________________________________________________________
CGPatternRef create_pattern_3()
{
   CGPatternCallbacks patternCallbacks;
   patternCallbacks.version = 0;
   patternCallbacks.drawPattern = draw_pattern_3;
   patternCallbacks.releaseInfo = 0;
   
   return CGPatternCreate(0, CGRectMake(0.f, 0.f, 4.f, 4.f), CGAffineTransformIdentity, 4.f, 4.f, kCGPatternTilingConstantSpacingMinimalDistortion, 1, &patternCallbacks);
}

//______________________________________________________________________________
void draw_pattern_4(void *, CGContextRef ctx)
{
   Float_t red = 0.f, green = 0.f, blue = 0.f, alpha = 1.f;
   GetColorForIndex(gVirtualX->GetFillColor(), red, green, blue);
   CGContextSetRGBFillColor(ctx, red, green, blue, alpha);
   
   CGContextFillRect(ctx, CGRectMake(0.f, 7.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(1.f, 6.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(2.f, 5.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(3.f, 4.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(4.f, 3.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(5.f, 2.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(6.f, 1.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(7.f, 0.f, 1.f, 1.f));
}

//______________________________________________________________________________
CGPatternRef create_pattern_4()
{
   CGPatternCallbacks patternCallbacks;
   patternCallbacks.version = 0;
   patternCallbacks.drawPattern = draw_pattern_4;
   patternCallbacks.releaseInfo = 0;
   
   return CGPatternCreate(0, CGRectMake(0.f, 0.f, 8.f, 8.f), CGAffineTransformIdentity, 8.f, 8.f, kCGPatternTilingConstantSpacingMinimalDistortion, 1, &patternCallbacks);
}

//______________________________________________________________________________
void draw_pattern_5(void *, CGContextRef ctx)
{
   Float_t red = 0.f, green = 0.f, blue = 0.f, alpha = 1.f;
   GetColorForIndex(gVirtualX->GetFillColor(), red, green, blue);
   CGContextSetRGBFillColor(ctx, red, green, blue, alpha);
   
   CGContextFillRect(ctx, CGRectMake(0.f, 0.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(1.f, 1.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(2.f, 2.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(3.f, 3.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(4.f, 4.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(5.f, 5.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(6.f, 6.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(7.f, 7.f, 1.f, 1.f));
}

//______________________________________________________________________________
CGPatternRef create_pattern_5()
{
   CGPatternCallbacks patternCallbacks;
   patternCallbacks.version = 0;
   patternCallbacks.drawPattern = draw_pattern_5;
   patternCallbacks.releaseInfo = 0;
   
   return CGPatternCreate(0, CGRectMake(0.f, 0.f, 8.f, 8.f), CGAffineTransformIdentity, 8.f, 8.f, kCGPatternTilingConstantSpacingMinimalDistortion, 1, &patternCallbacks);
}

//______________________________________________________________________________
void draw_pattern_6(void *, CGContextRef ctx)
{
   Float_t red = 0.f, green = 0.f, blue = 0.f, alpha = 1.f;
   GetColorForIndex(gVirtualX->GetFillColor(), red, green, blue);
   CGContextSetRGBFillColor(ctx, red, green, blue, alpha);
   
   CGContextFillRect(ctx, CGRectMake(2.f, 0.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(2.f, 1.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(2.f, 2.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(2.f, 3.f, 1.f, 1.f));
}

//______________________________________________________________________________
CGPatternRef create_pattern_6()
{
   CGPatternCallbacks patternCallbacks;
   patternCallbacks.version = 0;
   patternCallbacks.drawPattern = draw_pattern_6;
   patternCallbacks.releaseInfo = 0;
   
   return CGPatternCreate(0, CGRectMake(0.f, 0.f, 4.f, 4.f), CGAffineTransformIdentity, 4.f, 4.f, kCGPatternTilingConstantSpacingMinimalDistortion, 1, &patternCallbacks);
}

//______________________________________________________________________________
void draw_pattern_7(void *, CGContextRef ctx)
{
   Float_t red = 0.f, green = 0.f, blue = 0.f, alpha = 1.f;
   GetColorForIndex(gVirtualX->GetFillColor(), red, green, blue);
   CGContextSetRGBFillColor(ctx, red, green, blue, alpha);
   
   CGContextFillRect(ctx, CGRectMake(0.f, 1.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(1.f, 1.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(2.f, 1.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(3.f, 1.f, 1.f, 1.f));
}

//______________________________________________________________________________
CGPatternRef create_pattern_7()
{
   CGPatternCallbacks patternCallbacks;
   patternCallbacks.version = 0;
   patternCallbacks.drawPattern = draw_pattern_7;
   patternCallbacks.releaseInfo = 0;
   
   return CGPatternCreate(0, CGRectMake(0.f, 0.f, 4.f, 4.f), CGAffineTransformIdentity, 4.f, 4.f, kCGPatternTilingConstantSpacingMinimalDistortion, 1, &patternCallbacks);
}

//______________________________________________________________________________
void draw_pattern_8(void *, CGContextRef ctx)
{
   Float_t red = 0.f, green = 0.f, blue = 0.f, alpha = 1.f;
   GetColorForIndex(gVirtualX->GetFillColor(), red, green, blue);
   CGContextSetRGBFillColor(ctx, red, green, blue, alpha);
   
   //0x11   
   CGContextFillRect(ctx, CGRectMake(0.f, 0.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(4.f, 0.f, 1.f, 1.f));

   //0xb8   
   CGContextFillRect(ctx, CGRectMake(0.f, 1.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(1.f, 1.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(3.f, 1.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(7.f, 1.f, 1.f, 1.f));

   //0x7c
   CGContextFillRect(ctx, CGRectMake(0.f, 2.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(1.f, 2.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(2.f, 2.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(6.f, 2.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(7.f, 2.f, 1.f, 1.f));
   
   //0x3a
   CGContextFillRect(ctx, CGRectMake(0.f, 3.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(1.f, 3.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(5.f, 3.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(7.f, 3.f, 1.f, 1.f));
   
   //0x11
   CGContextFillRect(ctx, CGRectMake(0.f, 4.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(4.f, 4.f, 1.f, 1.f));
   
   //0xa3
   CGContextFillRect(ctx, CGRectMake(1.f, 5.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(3.f, 5.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(4.f, 5.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(5.f, 5.f, 1.f, 1.f));
   
   //0xc7
   CGContextFillRect(ctx, CGRectMake(2.f, 6.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(3.f, 6.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(4.f, 6.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(5.f, 6.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(6.f, 6.f, 1.f, 1.f));
   
   //0x8b
   CGContextFillRect(ctx, CGRectMake(3.f, 7.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(4.f, 7.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(5.f, 7.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(7.f, 7.f, 1.f, 1.f));
}

//______________________________________________________________________________
CGPatternRef create_pattern_8()
{
   CGPatternCallbacks patternCallbacks;
   patternCallbacks.version = 0;
   patternCallbacks.drawPattern = draw_pattern_8;
   patternCallbacks.releaseInfo = 0;
   
   return CGPatternCreate(0, CGRectMake(0.f, 0.f, 8.f, 8.f), CGAffineTransformIdentity, 8.f, 8.f, kCGPatternTilingConstantSpacingMinimalDistortion, 1, &patternCallbacks);
}

//______________________________________________________________________________
void draw_pattern_9(void *, CGContextRef ctx)
{
   Float_t red = 0.f, green = 0.f, blue = 0.f, alpha = 1.f;
   GetColorForIndex(gVirtualX->GetFillColor(), red, green, blue);
   CGContextSetRGBFillColor(ctx, red, green, blue, alpha);
   
   CGContextFillRect(ctx, CGRectMake(0.f, 0.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(0.f, 1.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(1.f, 2.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(2.f, 3.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(3.f, 3.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(4.f, 3.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(5.f, 3.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(6.f, 3.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(7.f, 2.f, 1.f, 1.f));
   
   CGContextFillRect(ctx, CGRectMake(4.f, 4.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(4.f, 5.f, 1.f, 1.f));
      
   CGContextFillRect(ctx, CGRectMake(3.f, 6.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(5.f, 6.f, 1.f, 1.f));
   
   CGContextFillRect(ctx, CGRectMake(0.f, 7.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(1.f, 7.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(2.f, 7.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(6.f, 7.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(7.f, 7.f, 1.f, 1.f));
}

//______________________________________________________________________________
CGPatternRef create_pattern_9()
{
   CGPatternCallbacks patternCallbacks;
   patternCallbacks.version = 0;
   patternCallbacks.drawPattern = draw_pattern_9;
   patternCallbacks.releaseInfo = 0;
   
   return CGPatternCreate(0, CGRectMake(0.f, 0.f, 8.f, 8.f), CGAffineTransformIdentity, 8.f, 8.f, kCGPatternTilingConstantSpacingMinimalDistortion, 1, &patternCallbacks);
}

//______________________________________________________________________________
void draw_pattern_10(void *, CGContextRef ctx)
{
   Float_t red = 0.f, green = 0.f, blue = 0.f, alpha = 1.f;
   GetColorForIndex(gVirtualX->GetFillColor(), red, green, blue);
   CGContextSetRGBFillColor(ctx, red, green, blue, alpha);
   
   CGContextFillRect(ctx, CGRectMake(0.f, 0.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(0.f, 1.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(0.f, 2.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(0.f, 3.f, 1.f, 1.f));
   
   CGContextFillRect(ctx, CGRectMake(1.f, 3.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(2.f, 3.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(3.f, 3.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(4.f, 3.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(5.f, 3.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(6.f, 3.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(7.f, 3.f, 1.f, 1.f));
   
   CGContextFillRect(ctx, CGRectMake(4.f, 4.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(4.f, 5.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(4.f, 6.f, 1.f, 1.f));

   CGContextFillRect(ctx, CGRectMake(0.f, 7.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(1.f, 7.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(2.f, 7.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(3.f, 7.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(4.f, 7.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(5.f, 7.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(6.f, 7.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(7.f, 7.f, 1.f, 1.f));

}

//______________________________________________________________________________
CGPatternRef create_pattern_10()
{
   CGPatternCallbacks patternCallbacks;
   patternCallbacks.version = 0;
   patternCallbacks.drawPattern = draw_pattern_10;
   patternCallbacks.releaseInfo = 0;
   
   return CGPatternCreate(0, CGRectMake(0.f, 0.f, 8.f, 8.f), CGAffineTransformIdentity, 8.f, 8.f, kCGPatternTilingConstantSpacingMinimalDistortion, 1, &patternCallbacks);
}

PatternGenerator_t gPatternGenerators[kPredefinedFillPatterns] = {create_pattern_1,
                                                                  create_pattern_2,
                                                                  create_pattern_3,
                                                                  create_pattern_4,
                                                                  create_pattern_5,
                                                                  create_pattern_6,
                                                                  create_pattern_7,
                                                                  create_pattern_8,
                                                                  create_pattern_9,
                                                                  create_pattern_10};

namespace {

typedef std::vector<TPoint>::size_type size_type;

}

//______________________________________________________________________________
void DrawMarkerDot(CGContextRef /*ctx*/, const std::vector<TPoint> &/*xy*/)
{
}

//______________________________________________________________________________
void DrawMarkerPlus(CGContextRef /*ctx*/, const std::vector<TPoint> &/*xy*/)
{
}

//______________________________________________________________________________
void DrawMarkerStar(CGContextRef /*ctx*/, const std::vector<TPoint> &/*xy*/)
{
}

//______________________________________________________________________________
void DrawMarkerCircle(CGContextRef /*ctx*/, const std::vector<TPoint> &/*xy*/)
{
}

//______________________________________________________________________________
void DrawMarkerX(CGContextRef /*ctx*/, const std::vector<TPoint> &/*xy*/)
{
}

//______________________________________________________________________________
void DrawMarkerFullDotSmall(CGContextRef /*ctx*/, const std::vector<TPoint> &/*xy*/)
{
}

//______________________________________________________________________________
void DrawMarkerFullDotMedium(CGContextRef /*ctx*/, const std::vector<TPoint> &/*xy*/)
{
}

//______________________________________________________________________________
void DrawMarkerFullDotLarge(CGContextRef /*ctx*/, const std::vector<TPoint> &/*xy*/)
{
}

//______________________________________________________________________________
void DrawMarkerFullSquare(CGContextRef /*ctx*/, const std::vector<TPoint> &/*xy*/)
{
}

//______________________________________________________________________________
void DrawMarkerFullTrianlgeUp(CGContextRef /*ctx*/, const std::vector<TPoint> &/*xy*/)
{
}

//______________________________________________________________________________
void DrawMarkerFullTrianlgeDown(CGContextRef /*ctx*/, const std::vector<TPoint> &/*xy*/)
{
}

//______________________________________________________________________________
void DrawMarkerDiamond(CGContextRef /*ctx*/, const std::vector<TPoint> &/*xy*/)
{
}

//______________________________________________________________________________
void DrawMarkerCross(CGContextRef /*ctx*/, const std::vector<TPoint> &/*xy*/)
{
}

//______________________________________________________________________________
void DrawMarkerFullStar(CGContextRef ctx, const std::vector<TPoint> &xy)
{
   // HIGZ full star pentagone
   const Size_t markerSize = gVirtualX->GetMarkerSize();
   const Int_t im  = Int_t(4 * markerSize + 0.5);
   const Int_t im1 = Int_t(0.66 * markerSize + 0.5);
   const Int_t im2 = Int_t(2.00 * markerSize + 0.5);
   const Int_t im3 = Int_t(2.66 * markerSize + 0.5);
   const Int_t im4 = Int_t(1.33 * markerSize + 0.5);
   
   Float_t r = 0.f, g = 0.f, b = 0.f, a = 1.f;
   GetColorForIndex(gVirtualX->GetMarkerColor(), r, g, b);
   CGContextSetRGBFillColor(ctx, r, g, b, a);
   
   for (size_type i = 0, e = xy.size(); i < e; ++i) {
      const Double_t x = xy[i].fX;
      const Double_t y = xy[i].fY;

      CGContextBeginPath(ctx);
      CGContextMoveToPoint(ctx, x - im, y - im4);
      CGContextAddLineToPoint(ctx, x - im2, y + im1);
      CGContextAddLineToPoint(ctx, x - im4, y - im4);
      CGContextDrawPath(ctx, kCGPathFillStroke);
 
      CGContextBeginPath(ctx);
      CGContextMoveToPoint(ctx, x - im2, y + im1);//1
      CGContextAddLineToPoint(ctx, x - im3, y + im);//2
      CGContextAddLineToPoint(ctx, x, y + im2);//3
      CGContextDrawPath(ctx, kCGPathFillStroke);
      
      CGContextBeginPath(ctx);
      CGContextMoveToPoint(ctx, x, y + im2);//3
      CGContextAddLineToPoint(ctx, x + im3, y + im);//4
      CGContextAddLineToPoint(ctx, x + im2, y + im1);//5
      CGContextDrawPath(ctx, kCGPathFillStroke);
      
      CGContextBeginPath(ctx);
      CGContextMoveToPoint(ctx, x + im2, y + im1);//5
      CGContextAddLineToPoint(ctx, x + im, y - im4);//6
      CGContextAddLineToPoint(ctx,x + im4, y - im4);//7
      CGContextDrawPath(ctx, kCGPathFillStroke);
      
      CGContextBeginPath(ctx);
      CGContextMoveToPoint(ctx, x + im4, y - im4);//7
      CGContextAddLineToPoint(ctx, x, y - im);//8
      CGContextAddLineToPoint(ctx, x - im4, y - im4);//9
      CGContextDrawPath(ctx, kCGPathFillStroke);
            
      CGContextBeginPath(ctx);
      CGContextMoveToPoint(ctx, x - im4, y - im4);//9
      CGContextAddLineToPoint(ctx, x - im2, y + im1);//1
      CGContextAddLineToPoint(ctx, x, y + im2);//3
      CGContextDrawPath(ctx, kCGPathFillStroke);
            
      CGContextBeginPath(ctx);
      CGContextMoveToPoint(ctx, x - im4, y - im4);//9
      CGContextAddLineToPoint(ctx, x, y + im2);//3
      CGContextAddLineToPoint(ctx, x + im2, y + im1);//5
      CGContextDrawPath(ctx, kCGPathFillStroke);
            
      CGContextBeginPath(ctx);
      CGContextMoveToPoint(ctx, x - im4, y - im4);//9
      CGContextAddLineToPoint(ctx, x + im2, y + im1);//5
      CGContextAddLineToPoint(ctx, x + im4, y - im4);//7
      CGContextDrawPath(ctx, kCGPathFillStroke);   
   }
}

//______________________________________________________________________________
void DrawMarkerOpenStar(CGContextRef /*ctx*/, const std::vector<TPoint> &/*xy*/)
{
}

//______________________________________________________________________________
void DrawPolyMarker(CGContextRef ctx, const std::vector<TPoint> &xy)
{
   const Style_t markerStyle = gVirtualX->GetMarkerStyle();
   
   switch (markerStyle) {
   case kDot:
      DrawMarkerDot(ctx, xy);
      break;
   case kPlus:
      DrawMarkerPlus(ctx, xy);
      break;
   case kStar:
      DrawMarkerStar(ctx, xy);
      break;
   case kCircle:
   case kOpenCircle:
      DrawMarkerCircle(ctx, xy);
      break;
   case kMultiply:
      DrawMarkerX(ctx, xy);
      break;
   case kFullDotSmall://"Full dot small"
      DrawMarkerFullDotSmall(ctx, xy);
      break;
   case kFullDotMedium:
      DrawMarkerFullDotMedium(ctx, xy);
      break;
   case kFullDotLarge:
   case kFullCircle:
      DrawMarkerFullDotLarge(ctx, xy);
      break;
   case kFullSquare:
      DrawMarkerFullSquare(ctx, xy);
      break;
   case kFullTriangleUp:
      DrawMarkerFullTrianlgeUp(ctx, xy);
      break;
   case kFullTriangleDown:
      DrawMarkerFullTrianlgeDown(ctx, xy);
      break;
   case kOpenSquare:
      DrawMarkerFullSquare(ctx, xy);
      break;
   case kOpenTriangleUp:
      DrawMarkerFullTrianlgeUp(ctx, xy);
      break;
   case kOpenDiamond:
      DrawMarkerDiamond(ctx, xy);
      break;
   case kOpenCross:
      DrawMarkerCross(ctx, xy);
      break;
   case kFullStar:
      DrawMarkerFullStar(ctx, xy);
      break;
   case kOpenStar:
      DrawMarkerOpenStar(ctx, xy);
      break;
   }   
}

//______________________________________________________________________________
void GetColorForIndex(Color_t colorIndex, Float_t &r, Float_t &g, Float_t &b)
{
   if (const TColor *color = gROOT->GetColor(colorIndex))
      color->GetRGB(r, g, b);
}

//IDEncoder.
//______________________________________________________________
IDEncoder::IDEncoder(UInt_t radix, UInt_t channelSize)
            : fRadix(radix),
              fRadix2(radix * radix),
              fChannelSize(channelSize),
              fStepSize(channelSize / (radix - 1)),
              fMaxID(radix * radix * radix)
{
}

//______________________________________________________________
Bool_t IDEncoder::IdToColor(UInt_t id, Float_t *rgb) const
{
   if (id >= fMaxID)
      return kFALSE;

   const UInt_t red = id / fRadix2;
   const UInt_t green = (id - red * fRadix2) / fRadix;
   const UInt_t blue = (id - red * fRadix2 - green * fRadix) % fRadix;
   
   rgb[0] = red * fStepSize / Float_t(fChannelSize);
   rgb[1] = green * fStepSize / Float_t(fChannelSize);
   rgb[2] = blue * fStepSize / Float_t(fChannelSize);
   
   return kTRUE;
}

//______________________________________________________________
UInt_t IDEncoder::ColorToId(UInt_t r, UInt_t g, UInt_t b) const
{
   const UInt_t red   = FixValue(r);
   const UInt_t green = FixValue(g);
   const UInt_t blue  = FixValue(b);
   
   return fRadix2 * red + fRadix * green + blue;
}

//______________________________________________________________
UInt_t IDEncoder::FixValue(UInt_t val) const
{
   const UInt_t orig = val / fStepSize;
   
   if (orig * fStepSize != val) {
      const UInt_t top = (orig + 1) * fStepSize - val;
      const UInt_t bottom = val - orig * fStepSize;

      if (top > bottom || orig + 1 >= fRadix)
         return orig;
         
      return orig + 1;
   } else
      return orig;
}

}
}
