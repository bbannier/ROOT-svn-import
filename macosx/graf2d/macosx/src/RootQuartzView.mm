#import "RootQuartzView.h"

@implementation RootQuartzView

//______________________________________________________________________________
- (void) drawRect : (NSRect)dirtyRect
{
   NSGraphicsContext *nsContext = [NSGraphicsContext currentContext];
   CGContextRef ctx = (CGContextRef)[nsContext graphicsPort];
   
   CGContextSetRGBFillColor(ctx, 1.f, 0.3f, 0.f, 1.f);
   CGContextFillRect(ctx, dirtyRect);
}

@end