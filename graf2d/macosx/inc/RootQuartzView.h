#ifndef ROOT_RootQuartzView
#define ROOT_RootQuartzView
//Include guards for C++ code.

#import <Cocoa/Cocoa.h>

#import "RootGUIElement.h"

@interface RootQuartzView : NSView<RootGUIElement>
- (void) addChildView : (RootQuartzView *)childView;
- (void) setParentView : (RootQuartzView *)parent;
- (RootQuartzView *) parentView;
@end


#endif