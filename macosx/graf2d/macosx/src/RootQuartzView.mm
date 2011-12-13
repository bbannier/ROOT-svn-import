#import "RootQuartzView.h"

@implementation RootQuartzView {
   RootQuartzView *fParentView;
}

@synthesize fWinID;

//______________________________________________________________________________
- (void) drawRect : (NSRect)dirtyRect
{
   NSGraphicsContext *nsContext = [NSGraphicsContext currentContext];
   CGContextRef ctx = (CGContextRef)[nsContext graphicsPort];
   
   CGContextSetRGBFillColor(ctx, 1.f, 0.3f, 0.f, 1.f);
   CGContextFillRect(ctx, dirtyRect);
}

//______________________________________________________________________________
- (void) addChildView : (RootQuartzView *)childView
{
   [self addSubview : childView];
   [childView setParentView : self];
}

//______________________________________________________________________________
- (RootQuartzView *) parentView
{
   return fParentView;
}

//______________________________________________________________________________
- (void) setParentView : (RootQuartzView *)parent
{
   fParentView = parent;
}

@end
