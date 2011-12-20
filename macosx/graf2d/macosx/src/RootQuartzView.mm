#import "RootQuartzView.h"

#import "TGWindow.h"
#import "TGClient.h"
#import "TGCocoa.h"

@implementation RootQuartzView {
   RootQuartzView *fParentView;
}

@synthesize fBackgroundColor;
@synthesize fWinID;

//______________________________________________________________________________
- (BOOL)wantsDefaultClipping
{
   //This is very arguable decision.
   return NO;
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

#pragma mark - Event handling.

//______________________________________________________________________________
- (void) drawRect : (NSRect)dirtyRect
{
//   
   if (fWinID) {
      if (TGWindow *window = gClient->GetWindowById(fWinID)) {
         NSGraphicsContext *nsContext = [NSGraphicsContext currentContext];
         CGContextRef ctx = (CGContextRef)[nsContext graphicsPort];
         
         CGContextSaveGState(ctx);
         
         TGCocoa *cocoa = static_cast<TGCocoa *>(gVirtualX);
         cocoa->SetContext(ctx);
         //
         gClient->NeedRedraw(window, kTRUE);
         //
         CGContextRestoreGState(ctx);
      }
   }
}

//______________________________________________________________________________
- (void) setFrameSize : (NSSize)newSize
{
   //Generate ConfigureNotify event and send it to ROOT's TGWindow.
   [super setFrameSize : newSize];
   
   if (fWinID) {
      if (TGWindow *window = gClient->GetWindowById(fWinID)) {
         //Should I also send setNeedsDisplay or not?
         //
         Event_t newEvent = {};
         newEvent.fType = kConfigureNotify;
         
         newEvent.fWindow = fWinID;
         newEvent.fX = self.frame.origin.x;
         newEvent.fY = self.frame.origin.y;
         newEvent.fWidth = newSize.width;
         newEvent.fHeight = newSize.height;
         
         window->HandleEvent(&newEvent);
         //
      }
   }
}

//______________________________________________________________________________
- (NSView *)contentView
{
   return self;
}

@end
