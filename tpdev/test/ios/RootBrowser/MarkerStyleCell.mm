#import <CoreGraphics/CGContext.h>

#import "MarkerStyleCell.h"

#import "IOSMarkers.h"
#import "TPoint.h"

@implementation MarkerStyleCell

@synthesize backgroundImage;

//____________________________________________________________________________________________________
- (id) initWithFrame : (CGRect)frame andMarkerStyle : (EMarkerStyle)style
{
   self = [super initWithFrame : frame];
   if (self) {
      markerStyle = style;
      self.backgroundImage = [UIImage imageNamed : @"line_cell.png"];
   }

   return self;
}

//____________________________________________________________________________________________________
- (void) dealloc
{
   self.backgroundImage = nil;
   [super dealloc];
}

//____________________________________________________________________________________________________
- (void)drawRect : (CGRect)rect
{
   //Drawing code.
   [backgroundImage drawInRect : rect];
   //
   CGContextRef ctx = UIGraphicsGetCurrentContext();
   
   CGContextTranslateCTM(ctx, 0.f, rect.size.height);
   CGContextScaleCTM(ctx, 1.f, -1.f);
   
   CGContextSetAllowsAntialiasing(ctx, 0);
   CGContextSetRGBFillColor(ctx, 0.f, 0.f, 0.f, 1.f);
   CGContextSetRGBStrokeColor(ctx, 0.f, 0.f, 0.f, 1.f);
   
   TPoint markerPosition(rect.size.width / 2, rect.size.height / 2);
   ROOT::iOS::GraphicUtils::DrawPolyMarker(ctx, 1, &markerPosition, 2.5, Style_t(markerStyle));//2.5 is the marker size.
}

@end
