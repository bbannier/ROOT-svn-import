#import <CoreGraphics/CGContext.h>

//C++ (ROOT) imports.
#import "IOSFillPatterns.h"

#import "PatternCell.h"

@implementation PatternCell

//____________________________________________________________________________________________________
- (id)initWithFrame:(CGRect)frame andPattern : (unsigned) index
{
   self = [super initWithFrame:frame];
    
   if (self) {
      patternIndex = index;
      solid = NO;
   }

   return self;
}

//____________________________________________________________________________________________________
- (void)dealloc
{
   [super dealloc];
}

//____________________________________________________________________________________________________
- (void) setAsSolid
{
   solid = YES;
}

//____________________________________________________________________________________________________
- (void)drawRect:(CGRect)rect
{
   CGContextRef ctx = UIGraphicsGetCurrentContext();

   //Fill view with pattern.
   CGContextSetRGBFillColor(ctx, 1.f, 1.f, 1.f, 1.f);
   CGContextFillRect(ctx, rect);

   if (!solid) { //Solid fill - no pattern.
      float rgb[] = {0.f, 0.f, 0.f};
      CGPatternRef pattern = ROOT::iOS::GraphicUtils::gPatternGenerators[patternIndex](rgb);
   
      CGColorSpaceRef colorSpace = CGColorSpaceCreatePattern(0);
      const float alpha = 1.f;
   
      CGContextSetFillColorSpace(ctx, colorSpace);
      CGContextSetFillPattern(ctx, pattern, &alpha);
      CGContextFillRect(ctx, rect);

      CGColorSpaceRelease(colorSpace);
      CGPatternRelease(pattern);
   }
}

@end
