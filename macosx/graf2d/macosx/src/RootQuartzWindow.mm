#import <stdlib.h>

#import "RootQuartzWindow.h"
#import "RootQuartzView.h"


//C++ (ROOT) imports.
#import "TGCocoa.h"

@implementation RootQuartzWindow

@synthesize fTopLevelView;
@synthesize fWinID;

//______________________________________________________________________________
- (void) setFWinID : (unsigned) winID
{
   fTopLevelView.fWinID = winID;
   fWinID = winID;
}

//______________________________________________________________________________
- (id) initWithContentRect : (NSRect)contentRect styleMask : (NSUInteger)windowStyle backing : (NSBackingStoreType)bufferingType defer : (BOOL)deferCreation
{
   self = [super initWithContentRect : contentRect styleMask : windowStyle backing : bufferingType defer : deferCreation];
   if (self)
      self.delegate = self;
   
   return self;
}

//______________________________________________________________________________
- (void) windowWillMiniaturize : (NSNotification *)notification
{
   (void)notification;//As soon as I'm compiling as Obj-C++, I have a warning about unused parameters (never happens with Obj-C).
}

//______________________________________________________________________________
- (void) setFTopLevelView : (RootQuartzView *)view
{
   [self setContentView : view];
   fTopLevelView = view;
}

//______________________________________________________________________________
- (void) addChildView : (RootQuartzView *)view
{
   [fTopLevelView addChildView : view];
}

//______________________________________________________________________________
- (void) setParentView : (RootQuartzView *)parent
{
   //
   (void)parent;
}

//______________________________________________________________________________
- (RootQuartzView *) parentView
{
   return nil;
}

#pragma mark - NSWindowDelegate methods.

//______________________________________________________________________________
- (void) windowDidResize : (NSNotification *)notification
{
   (void)notification;
}

//______________________________________________________________________________
- (void) windowDidMove : (NSNotification *)notification
{
   (void)notification;
}

//______________________________________________________________________________
- (void) windowDidBecomeKey : (NSNotification *)notification
{
   //Let's generate ConfigureNotify event.
   (void)notification;
}

@end
