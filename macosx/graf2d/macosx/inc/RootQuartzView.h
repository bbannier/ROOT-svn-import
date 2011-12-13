#ifndef ROOT_RootQuartzView
#define ROOT_RootQuartzView
//Include guards for C++ code.

#import <Cocoa/Cocoa.h>

#import "RootGUIElement.h"

@interface RootQuartzView : NSView<RootGUIElement>
- (void) addChildView : (RootQuartzView *)childView;
- (void) setParentView : (RootQuartzView *)parent;
- (RootQuartzView *) parentView;

//This is a test function to check, if I can draw at any moment I want.
- (void) clearWidget;

@property (nonatomic, assign) unsigned fWinID;

@end


#endif