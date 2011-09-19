#import "ColorCell.h"

@implementation ColorCell

//____________________________________________________________________________________________________
+ (CGFloat) cellAlpha 
{
   return 0.8f;
}

//____________________________________________________________________________________________________
- (id)initWithFrame:(CGRect)frame
{
   self = [super initWithFrame:frame];
   
   if (self)
      self.backgroundColor = [UIColor clearColor];

   return self;
}

//____________________________________________________________________________________________________
- (void)dealloc
{
   [super dealloc];
}

//____________________________________________________________________________________________________
- (void) setRGB : (const double *) newRgb
{
   rgb[0] = newRgb[0];
   rgb[1] = newRgb[1];
   rgb[2] = newRgb[2];
}

//____________________________________________________________________________________________________
- (void)drawRect:(CGRect)rect
{
   CGContextRef ctx = UIGraphicsGetCurrentContext();
   if (!ctx) {
      //Log error: ctx is nil.
      return;
   }

   CGContextSetRGBFillColor(ctx, rgb[0], rgb[1], rgb[2], [ColorCell cellAlpha]);
   CGContextFillRect(ctx, rect);
}

@end
