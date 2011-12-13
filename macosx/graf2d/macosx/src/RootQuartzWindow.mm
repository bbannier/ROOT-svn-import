#import "RootQuartzWindow.h"
#import "RootQuartzView.h"

@implementation RootQuartzWindow

@synthesize fTopLevelView;

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
   NSLog(@"will min");
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
- (void) windowDidResize : (NSNotification *)notification
{
   (void)notification;
   NSLog(@"window did resize\n");
}

//______________________________________________________________________________
- (void) windowDidMove : (NSNotification *)notification
{
   (void)notification;
   NSLog(@"window did move\n");
}

@end
