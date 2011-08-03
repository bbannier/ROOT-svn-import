#include <CoreGraphics/CGContext.h>

#include "TVirtualX.h"

#include "GraphicUtils.h"
#include "FillPatterns.h"

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

}
}