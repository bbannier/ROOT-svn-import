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
      self.backgroundImage = [UIImage imageNamed : @"marker_cell_bkn.png"];
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
   CGContextSetAllowsAntialiasing(ctx, 0);
   TPoint markerPosition(rect.size.width / 2, rect.size.height / 2);
   ROOT_iOS::GraphicUtils::DrawPolyMarker(ctx, 1, &markerPosition, kBlack, 2.5, Style_t(markerStyle));//2.5 is the marker size.
}

@end
