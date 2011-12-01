#import "QuartzView.h"

#import "QuartzTest.h"

@implementation QuartzView {
   QuartzTest *test;
}

- (id)initWithFrame:(NSRect)frame
{
   if (self = [super initWithFrame : frame]) {
      // Initialization code here.
      test = new QuartzTest(frame.size.width, frame.size.height);
   }
    
   return self;
}

- (void) drawRect : (NSRect)dirtyRect
{
   // Drawing code here.
   NSGraphicsContext *nsContext = [NSGraphicsContext currentContext];
   CGContextRef cgContext = (CGContextRef)[nsContext graphicsPort];
   
   CGContextSetRGBFillColor(cgContext, 1.f, 0.3f, 0.f, 1.f);
   CGContextFillRect(cgContext, dirtyRect);
}

@end
