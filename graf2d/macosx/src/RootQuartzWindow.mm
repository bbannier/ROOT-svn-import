#define DEBUG_ROOT_COCOA

#import "RootQuartzWindow.h"
#import "RootQuartzView.h"


//C++ (ROOT) imports.
#import "TGCocoa.h"

@implementation RootQuartzWindow

@synthesize fTopLevelView;
@synthesize fWinID;

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
   [fTopLevelView addSubview : view];
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
- (void) queueConfigureNotify : (NSNotification *)notification
{
   TGCocoa *cocoa = (TGCocoa *)gVirtualX;//Uh-oh!
   RootQuartzWindow *win = (RootQuartzWindow *)notification.object;
   const NSRect frame = win.frame;
   
   Event_t newEvent = {};
   newEvent.fX = (Int_t)frame.origin.x;
   newEvent.fY = cocoa->CocoaToRootY((Int_t)frame.origin.y);
   newEvent.fWidth = (Int_t)frame.size.width;
   newEvent.fHeight = (Int_t)frame.size.height;
   newEvent.fType = kConfigureNotify;
   newEvent.fWindow = win.fWinID;
   
   cocoa->QueueEvent(newEvent);
   
   newEvent.fType = kExpose;//Baby did a bad bad thing :)))
   cocoa->QueueEvent(newEvent);
}

//______________________________________________________________________________
- (void) windowDidResize : (NSNotification *)notification
{
   //Let's generate ConfigureNotify event.
   //Event_t newEvent = {};
   if ([notification.object isKindOfClass : [RootQuartzWindow class]]) {
      [self queueConfigureNotify : notification];
   }
#ifdef DEBUG_ROOT_COCOA
   else
      NSLog(@"windowDidResize: Object %@ is not a RootQuartzWindow", notification.object);
#endif
}

//______________________________________________________________________________
- (void) windowDidMove : (NSNotification *)notification
{
   //Let's generate ConfigureNotify event.
   if ([notification.object isKindOfClass : [RootQuartzWindow class]]) {
      [self queueConfigureNotify : notification];
   }
#ifdef DEBUG_ROOT_COCOA
   else
      NSLog(@"windowDidResize: Object %@ is not a RootQuartzWindow", notification.object);
#endif
}

//______________________________________________________________________________
- (void) windowDidBecomeKey : (NSNotification *)notification
{
   //Let's generate ConfigureNotify event.
   if ([notification.object isKindOfClass : [RootQuartzWindow class]]) {
      TGCocoa *cocoa = (TGCocoa *)gVirtualX;//Uh-oh!
      RootQuartzWindow *win = (RootQuartzWindow *)notification.object;
      const NSRect frame = win.frame;
   
      Event_t newEvent = {};
      newEvent.fX = (Int_t)frame.origin.x;
      newEvent.fY = cocoa->CocoaToRootY((Int_t)frame.origin.y);
      newEvent.fWidth = (Int_t)frame.size.width;
      newEvent.fHeight = (Int_t)frame.size.height;
      newEvent.fType = kExpose;
      newEvent.fWindow = win.fWinID;
      cocoa->QueueEvent(newEvent);
   }
#ifdef DEBUG_ROOT_COCOA
   else
      NSLog(@"windowDidResize: Object %@ is not a RootQuartzWindow", notification.object);
#endif

}

@end
