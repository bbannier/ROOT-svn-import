#ifndef ROOT_RootQuartzWindow
#define ROOT_RootQuartzWindow
//Include guards for C++ code.

#import <Cocoa/Cocoa.h>

#import "RootGUIElement.h"

@class RootQuartzView;

@interface RootQuartzWindow : NSWindow<RootGUIElement>

- (void) addChildView : (RootQuartzView *)childView;

@property (nonatomic, assign) RootQuartzView *fTopLevelView;
@end

#endif
