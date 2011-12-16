#import "RootQuartzView.h"

#import "TGWindow.h"
#import "TGClient.h"
#import "TGCocoa.h"

@implementation RootQuartzView {
   RootQuartzView *fParentView;
}

@synthesize fWinID;


//______________________________________________________________________________
- (void) drawRect : (NSRect)dirtyRect
{
//   
   if (fWinID) {
      if (TGWindow *window = gClient->GetWindowById(fWinID)) {
         NSGraphicsContext *nsContext = [NSGraphicsContext currentContext];
         CGContextRef ctx = (CGContextRef)[nsContext graphicsPort];
         TGCocoa *cocoa = static_cast<TGCocoa *>(gVirtualX);
         cocoa->SetContext(ctx);
         //
         gClient->NeedRedraw(window, kTRUE);
         //
      }
   }
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

//______________________________________________________________________________
- (void) clearWidget
{

}

#pragma mark - Event handling.
//______________________________________________________________________________
- (void) setFrameSize : (NSSize)newSize
{
   //Generate ConfigureNotify event and send it to ROOT's TGWindow.
   if (fWinID) {
      if (TGWindow *window = gClient->GetWindowById(fWinID)) {
         NSLog(@"send configure notify to TGWindow");
         //Should I also send setNeedsDisplay or not?
      }
   }
   
   [super setFrameSize : newSize];
}


@end
