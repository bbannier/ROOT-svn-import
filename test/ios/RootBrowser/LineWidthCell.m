#import <CoreGraphics/CGContext.h>

#import "LineWidthCell.h"


@implementation LineWidthCell

//_________________________________________________________________
- (id)initWithFrame:(CGRect)frame width : (CGFloat)w
{
   self = [super initWithFrame:frame];

   if (self) {
      lineWidth = w;
      
      backgroundImage = [UIImage imageNamed : @"line_cell.png"];
      [backgroundImage retain];
   }

   return self;
}

//_________________________________________________________________
- (void)drawRect:(CGRect)rect
{
   CGContextRef ctx = UIGraphicsGetCurrentContext();

   //Fill the background.   
   [backgroundImage drawInRect : rect];
   //Draw the line.
   CGContextSetLineCap(ctx, kCGLineCapRound);
   CGContextSetLineWidth(ctx, lineWidth);
   CGContextSetRGBStrokeColor(ctx, 0.f, 0.f, 0.f, 1.f);
   CGContextBeginPath(ctx);
   CGContextMoveToPoint(ctx, 10.f, rect.size.height / 2);
   CGContextAddLineToPoint(ctx, rect.size.width - 10, rect.size.height / 2);
   CGContextStrokePath(ctx);
   
   NSString *label = [NSString stringWithFormat:@"(%d)", (int)lineWidth];
   CGContextSetRGBFillColor(ctx, 0.f, 0.f, 1.f, 1.f);
   [label drawInRect:CGRectMake(rect.size.width / 2 - 10.f, rect.size.height / 2 - 15.f, 40.f, 60.f) withFont : [UIFont systemFontOfSize : 10]];
}

//_________________________________________________________________
- (void)dealloc
{
   [backgroundImage release];
   [super dealloc];
}

@end
