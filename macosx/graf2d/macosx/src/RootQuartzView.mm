#import "RootQuartzView.h"

#import "TGWindow.h"
#import "TGClient.h"
#import "TGCocoa.h"

@implementation RootQuartzView {
   RootQuartzView *fParentView;

   CGContextRef fCurrentContext;
}

@synthesize fBackgroundColor;
@synthesize fCurrentContext;
@synthesize fWinID;

/*
//______________________________________________________________________________
- (BOOL)wantsDefaultClipping
{
   //This is very arguable decision.
   return NO;
}
*/

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
   (void)dirtyRect;

   if (fWinID) {
      if (TGWindow *window = gClient->GetWindowById(fWinID)) {
         NSGraphicsContext *nsContext = [NSGraphicsContext currentContext];

         fCurrentContext = (CGContextRef)[nsContext graphicsPort];
         
         CGContextSaveGState(fCurrentContext);
         
         TGCocoa *cocoa = static_cast<TGCocoa *>(gVirtualX);
         
         //Have a look, it's possible to set context when window selected (TGCocoa::SelectWindow),
         //or DrawXXX(windowID ....) method called (take from windowID).
         cocoa->SetContext(fCurrentContext);
         //
         gClient->NeedRedraw(window, kTRUE);
         //
         CGContextRestoreGState(fCurrentContext);
         //
 //        fCurrentContext = 0;
 //        cocoa->SetContext(0);
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

//______________________________________________________________________________
- (BOOL) fIsPixmap
{
   return FALSE;
}

@end
