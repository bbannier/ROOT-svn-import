#import <CoreGraphics/CGContext.h>

#import "SlideView.h"

//C++ (ROOT) imports.
#import "IOSPad.h"

@implementation SlideView

//_________________________________________________________________
- (id) initWithFrame : (CGRect) frame andPad : (ROOT_iOS::Pad *)p
{
   self = [super initWithFrame : frame];
   if (self) {
      pad = p;
   }
   
   return self;
}

//_________________________________________________________________
- (void)drawRect : (CGRect)rect
{
   CGContextRef ctx = UIGraphicsGetCurrentContext();
   CGContextClearRect(ctx, rect);

   CGContextTranslateCTM(ctx, 0.f, rect.size.height);
   CGContextScaleCTM(ctx, 1.f, -1.f);

   pad->cd();   
   pad->SetViewWH(rect.size.width, rect.size.height);
   pad->SetContext(ctx);
   pad->Paint();
}

@end
