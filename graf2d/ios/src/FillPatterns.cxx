#include <CoreGraphics/CGContext.h>

#include "TVirtualX.h"

#include "GraphicUtils.h"
#include "FillPatterns.h"

namespace ROOT_iOS {
namespace GraphicUtils {

//
// Fill patterns, defined in RStipples.h.
// Pattern in RStipple.h is 32x32.
// Order is: say, the first two numbers are 1: 0x42 2: 0x42
// NUMBER:      1        2     
// BITS:   0100 0010  0100 0010 //bits from lower to higher.
// IN HEX   2     4    2     4

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

//______________________________________________________________________________
void draw_pattern_11(void *, CGContextRef ctx)
{
   Float_t red = 0.f, green = 0.f, blue = 0.f, alpha = 1.f;
   GetColorForIndex(gVirtualX->GetFillColor(), red, green, blue);
   CGContextSetRGBFillColor(ctx, red, green, blue, alpha);

   CGContextFillRect(ctx, CGRectMake(0.f, 4.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(1.f, 3.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(1.f, 5.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(2.f, 2.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(2.f, 6.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(4.f, 6.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(4.f, 2.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(5.f, 5.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(5.f, 3.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(6.f, 4.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(7.f, 1.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(7.f, 2.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(7.f, 3.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(7.f, 4.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(7.f, 5.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(7.f, 6.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(7.f, 7.f, 1.f, 1.f));

   CGContextFillRect(ctx, CGRectMake(0.f, 10.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(0.f, 14.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(1.f, 11.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(1.f, 13.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(2.f, 12.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(3.f, 9.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(3.f, 10.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(3.f, 11.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(3.f, 12.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(3.f, 13.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(3.f, 14.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(3.f, 15.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(4.f, 12.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(5.f, 11.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(5.f, 13.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(6.f, 10.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(6.f, 14.f, 1.f, 1.f));

}

//______________________________________________________________________________
CGPatternRef create_pattern_11()
{
   CGPatternCallbacks patternCallbacks;
   patternCallbacks.version = 0;
   patternCallbacks.drawPattern = draw_pattern_11;
   patternCallbacks.releaseInfo = 0;
   
   return CGPatternCreate(0, CGRectMake(0.f, 0.f, 8.f, 16.f), CGAffineTransformIdentity, 8.f, 16.f, kCGPatternTilingConstantSpacingMinimalDistortion, 1, &patternCallbacks);
}

//______________________________________________________________________________
void draw_pattern_12(void *, CGContextRef ctx)
{
   Float_t red = 0.f, green = 0.f, blue = 0.f, alpha = 1.f;
   GetColorForIndex(gVirtualX->GetFillColor(), red, green, blue);
   CGContextSetRGBFillColor(ctx, red, green, blue, alpha);

   CGContextFillRect(ctx, CGRectMake(0.f, 1.f,  1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(0.f, 7.f,  1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(0.f, 11.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(0.f, 12.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(0.f, 13.f, 1.f, 1.f));
   
   CGContextFillRect(ctx, CGRectMake(1.f, 2.f,  1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(1.f, 6.f,  1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(1.f, 10.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(1.f, 14.f, 1.f, 1.f));

   CGContextFillRect(ctx, CGRectMake(2.f, 3.f,  1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(2.f, 4.f,  1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(2.f, 5.f,  1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(2.f, 9.f,  1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(2.f, 15.f, 1.f, 1.f));
   
   CGContextFillRect(ctx, CGRectMake(3.f, 9.f,  1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(3.f, 15.f, 1.f, 1.f));
   
   CGContextFillRect(ctx, CGRectMake(4.f, 3.f,  1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(4.f, 4.f,  1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(4.f, 5.f,  1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(4.f, 9.f,  1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(4.f, 15.f, 1.f, 1.f));

   CGContextFillRect(ctx, CGRectMake(5.f, 2.f,  1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(5.f, 6.f,  1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(5.f, 10.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(5.f, 14.f, 1.f, 1.f));

   CGContextFillRect(ctx, CGRectMake(6.f, 1.f,  1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(6.f, 7.f,  1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(6.f, 11.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(6.f, 12.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(6.f, 13.f, 1.f, 1.f));

   CGContextFillRect(ctx, CGRectMake(7.f, 1.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(7.f, 7.f, 1.f, 1.f));
}

//______________________________________________________________________________
CGPatternRef create_pattern_12()
{
   CGPatternCallbacks patternCallbacks;
   patternCallbacks.version = 0;
   patternCallbacks.drawPattern = draw_pattern_12;
   patternCallbacks.releaseInfo = 0;
   
   return CGPatternCreate(0, CGRectMake(0.f, 0.f, 8.f, 16.f), CGAffineTransformIdentity, 8.f, 16.f, kCGPatternTilingConstantSpacingMinimalDistortion, 1, &patternCallbacks);
}

//______________________________________________________________________________
void draw_pattern_13(void *, CGContextRef ctx)
{
   Float_t red = 0.f, green = 0.f, blue = 0.f, alpha = 1.f;
   GetColorForIndex(gVirtualX->GetFillColor(), red, green, blue);
   CGContextSetRGBFillColor(ctx, red, green, blue, alpha);

   CGContextFillRect(ctx, CGRectMake(0.f, 7.f,  1.f, 1.f));
   
   CGContextFillRect(ctx, CGRectMake(1.f, 0.f,  1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(1.f, 6.f,  1.f, 1.f));
   
   CGContextFillRect(ctx, CGRectMake(2.f, 1.f,  1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(2.f, 5.f,  1.f, 1.f));

   CGContextFillRect(ctx, CGRectMake(3.f, 2.f,  1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(3.f, 4.f,  1.f, 1.f));
   
   CGContextFillRect(ctx, CGRectMake(4.f, 3.f,  1.f, 1.f));

   CGContextFillRect(ctx, CGRectMake(5.f, 2.f,  1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(5.f, 4.f,  1.f, 1.f));

   CGContextFillRect(ctx, CGRectMake(6.f, 1.f,  1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(6.f, 5.f,  1.f, 1.f));

   CGContextFillRect(ctx, CGRectMake(7.f, 0.f,  1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(7.f, 6.f,  1.f, 1.f));
}

//______________________________________________________________________________
CGPatternRef create_pattern_13()
{
   CGPatternCallbacks patternCallbacks;
   patternCallbacks.version = 0;
   patternCallbacks.drawPattern = draw_pattern_13;
   patternCallbacks.releaseInfo = 0;
   
   return CGPatternCreate(0, CGRectMake(0.f, 0.f, 8.f, 8.f), CGAffineTransformIdentity, 8.f, 8.f, kCGPatternTilingConstantSpacingMinimalDistortion, 1, &patternCallbacks);
}

//______________________________________________________________________________
void draw_pattern_14(void *, CGContextRef ctx)
{
   Float_t red = 0.f, green = 0.f, blue = 0.f, alpha = 1.f;
   GetColorForIndex(gVirtualX->GetFillColor(), red, green, blue);
   CGContextSetRGBFillColor(ctx, red, green, blue, alpha);

   for (UInt_t i = 0; i < 16; ++i) {
      CGContextFillRect(ctx, CGRectMake(0.f, i, 1.f, 1.f));
      CGContextFillRect(ctx, CGRectMake(12.f, i, 1.f, 1.f));
      
      CGContextFillRect(ctx, CGRectMake(i, 3.f, 1.f, 1.f));
      CGContextFillRect(ctx, CGRectMake(i, 15.f, 1.f, 1.f));
   }
   
   for (UInt_t i = 0; i < 8; ++i) {
      CGContextFillRect(ctx, CGRectMake(i, 7.f, 1.f, 1.f));
      CGContextFillRect(ctx, CGRectMake(i + 4, 11.f, 1.f, 1.f));
      CGContextFillRect(ctx, CGRectMake(4.f, 8.f + i, 1.f, 1.f));
      CGContextFillRect(ctx, CGRectMake(8.f, 4.f + i, 1.f, 1.f));
   }
   
}

//______________________________________________________________________________
CGPatternRef create_pattern_14()
{
   CGPatternCallbacks patternCallbacks;
   patternCallbacks.version = 0;
   patternCallbacks.drawPattern = draw_pattern_14;
   patternCallbacks.releaseInfo = 0;
   
   return CGPatternCreate(0, CGRectMake(0.f, 0.f, 16.f, 16.f), CGAffineTransformIdentity, 16.f, 16.f, kCGPatternTilingConstantSpacingMinimalDistortion, 1, &patternCallbacks);
}

//______________________________________________________________________________
void draw_pattern_15(void *, CGContextRef ctx)
{
   Float_t red = 0.f, green = 0.f, blue = 0.f, alpha = 1.f;
   GetColorForIndex(gVirtualX->GetFillColor(), red, green, blue);
   CGContextSetRGBFillColor(ctx, red, green, blue, alpha);

   CGContextFillRect(ctx, CGRectMake(0.f, 2.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(0.f, 6.f, 1.f, 1.f));

   CGContextFillRect(ctx, CGRectMake(1.f, 5.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(1.f, 7.f, 1.f, 1.f));
   
   CGContextFillRect(ctx, CGRectMake(2.f, 2.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(2.f, 6.f, 1.f, 1.f));
   
   CGContextFillRect(ctx, CGRectMake(3.f, 1.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(3.f, 3.f, 1.f, 1.f));
   
   CGContextFillRect(ctx, CGRectMake(4.f, 2.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(4.f, 6.f, 1.f, 1.f));

   CGContextFillRect(ctx, CGRectMake(5.f, 5.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(5.f, 7.f, 1.f, 1.f));

   CGContextFillRect(ctx, CGRectMake(6.f, 2.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(6.f, 6.f, 1.f, 1.f));

   CGContextFillRect(ctx, CGRectMake(7.f, 1.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(7.f, 3.f, 1.f, 1.f));
}

//______________________________________________________________________________
CGPatternRef create_pattern_15()
{
   CGPatternCallbacks patternCallbacks;
   patternCallbacks.version = 0;
   patternCallbacks.drawPattern = draw_pattern_15;
   patternCallbacks.releaseInfo = 0;
   
   return CGPatternCreate(0, CGRectMake(0.f, 0.f, 8.f, 8.f), CGAffineTransformIdentity, 8.f, 8.f, kCGPatternTilingConstantSpacingMinimalDistortion, 1, &patternCallbacks);
}

//______________________________________________________________________________
void draw_pattern_16(void *, CGContextRef ctx)
{
   Float_t red = 0.f, green = 0.f, blue = 0.f;
   GetColorForIndex(gVirtualX->GetFillColor(), red, green, blue);
   CGContextSetRGBFillColor(ctx, red, green, blue, 1.f);
   
   CGContextFillRect(ctx, CGRectMake(0.f, 2.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(0.f, 6.f, 1.f, 1.f));

   CGContextFillRect(ctx, CGRectMake(1.f, 3.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(1.f, 7.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(2.f, 3.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(2.f, 7.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(3.f, 3.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(3.f, 7.f, 1.f, 1.f));

   CGContextFillRect(ctx, CGRectMake(4.f, 2.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(4.f, 6.f, 1.f, 1.f));

   CGContextFillRect(ctx, CGRectMake(5.f, 1.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(5.f, 5.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(6.f, 1.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(6.f, 5.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(7.f, 1.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(7.f, 5.f, 1.f, 1.f));
}

//______________________________________________________________________________
CGPatternRef create_pattern_16()
{
   CGPatternCallbacks patternCallbacks;
   patternCallbacks.version = 0;
   patternCallbacks.drawPattern = draw_pattern_16;
   patternCallbacks.releaseInfo = 0;
   
   return CGPatternCreate(0, CGRectMake(0.f, 0.f, 8.f, 8.f), CGAffineTransformIdentity, 8.f, 8.f, kCGPatternTilingConstantSpacingMinimalDistortion, 1, &patternCallbacks);
}

//______________________________________________________________________________
void draw_pattern_17(void *, CGContextRef ctx)
{
   Float_t red = 0.f, green = 0.f, blue = 0.f;
   GetColorForIndex(gVirtualX->GetFillColor(), red, green, blue);
   CGContextSetRGBFillColor(ctx, red, green, blue, 1.f);
   
   CGContextFillRect(ctx, CGRectMake(0.f, 3.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(1.f, 2.f, 1.f, 1.f));
   CGContextFillRect(ctx, CGRectMake(2.f, 1.f, 1.f, 1.f));
}

//______________________________________________________________________________
CGPatternRef create_pattern_17()
{
   CGPatternCallbacks patternCallbacks;
   patternCallbacks.version = 0;
   patternCallbacks.drawPattern = draw_pattern_17;
   patternCallbacks.releaseInfo = 0;
   
   return CGPatternCreate(0, CGRectMake(0.f, 0.f, 4.f, 4.f), CGAffineTransformIdentity, 4.f, 4.f, kCGPatternTilingConstantSpacingMinimalDistortion, 1, &patternCallbacks);
}

//______________________________________________________________________________
CGPatternRef create_pattern_18()
{
   CGPatternCallbacks patternCallbacks;
   patternCallbacks.version = 0;
   patternCallbacks.drawPattern = draw_pattern_17;
   patternCallbacks.releaseInfo = 0;
   
   return CGPatternCreate(0, CGRectMake(0.f, 0.f, 4.f, 4.f), CGAffineTransformMakeScale(-1.f, 1.f), 4.f, 4.f, kCGPatternTilingConstantSpacingMinimalDistortion, 1, &patternCallbacks);
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
                                                                  create_pattern_10,
                                                                  create_pattern_11,
                                                                  create_pattern_12,
                                                                  create_pattern_13,
                                                                  create_pattern_14,
                                                                  create_pattern_15,
                                                                  create_pattern_16,
                                                                  create_pattern_17,
                                                                  create_pattern_18};

}
}