#import "ROOTApplicationDelegate.h"

@implementation ROOTApplicationDelegate

//______________________________________________________________________________
- (id) init
{
   if (self = [super init]) {
      [NSApp setDelegate : self];
   }
   
   return self;
}

//______________________________________________________________________________
- (void) dealloc
{
   [NSApp setDelegate : nil];//?
   [super dealloc];
}


//NSApplicationDelegate.

//______________________________________________________________________________
- (void) applicationWillResignActive : (NSNotification *) aNotification
{
   //Popup windows, menus, color-selectors, etc. - they all have
   //a problem: due to some reason, Cocoa changes the z-stack order
   //of such a window while switching between applications (using alt-tab, for example).
   //This leads to a very annoying effect: you open a menu, alt-tab,
   //alt-tab back and ... popup or menu is now behind the main window.
   //I have to save/restore this z-stack order here.

   //Popups were fixed using transient hint, noop now.
   (void) aNotification;
}

//______________________________________________________________________________
- (void) applicationDidBecomeActive : (NSNotification *) aNotification
{
   //Popup windows, menus, color-selectors, etc. - they all have
   //a problem: due to some reason, Cocoa changes the z-stack order
   //of such a window while switching between applications (using alt-tab, for example).
   //This leads to a very annoying effect: you open a menu, alt-tab,
   //alt-tab back and ... popup or menu is now behind the main window.
   //I have to save/restore this z-stack order here.

   //Popups were fixed using transient hint, noop now.
   (void) aNotification;
}

//______________________________________________________________________________
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
{
   [NSApp stop : nil];
   NSEvent* stopEvent = [NSEvent otherEventWithType: NSApplicationDefined
                                     location: NSMakePoint(0,0)
                               modifierFlags: 0
                                   timestamp: 0.0
                                 windowNumber: 0
                                     context: nil
                                     subtype: 0
                                       data1: 0
                                       data2: 0];
   [NSApp postEvent : stopEvent atStart: true];

   return NSTerminateCancel;
}

@end
