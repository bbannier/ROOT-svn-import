#ifndef ROOT_RootQuartzWindow
#define ROOT_RootQuartzWindow
//Include guards for C++ code.

#import <Cocoa/Cocoa.h>

#import "RootGUIElement.h"

@class RootQuartzView;

@interface RootQuartzWindow : NSWindow<RootGUIElement, NSWindowDelegate>

- (void) addChildView : (RootQuartzView *)childView;
- (void) setParentView : (RootQuartzView *)parent;
- (RootQuartzView *) parentView;

//NSWindowDelegate.
- (void) windowDidResize : (NSNotification *)notification;
- (void) windowDidMove : (NSNotification *)notification;

@property (nonatomic, assign) RootQuartzView *fTopLevelView;
@property (nonatomic, assign) unsigned fWinID;

@property (nonatomic, readonly) BOOL fIsPixmap;
@property (nonatomic, assign) CGContextRef fCurrentContext;

@end

#endif
