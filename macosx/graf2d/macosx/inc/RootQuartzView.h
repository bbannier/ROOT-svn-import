#ifndef ROOT_RootQuartzView
#define ROOT_RootQuartzView
//Include guards for C++ code.

#import <Cocoa/Cocoa.h>

#import "RootGUIElement.h"

@interface RootQuartzView : NSView<RootGUIElement>
- (void) addChildView : (RootQuartzView *)childView;
- (void) setParentView : (RootQuartzView *)parent;
- (RootQuartzView *) parentView;

- (NSView *)contentView;

@property (nonatomic, assign) unsigned fWinID;

//NSView does not have backgroundColor?
@property (nonatomic, assign) unsigned long fBackgroundColor;

@property (nonatomic, readonly) BOOL fIsPixmap;
@property (nonatomic, assign) CGContextRef fCurrentContext;

@end


#endif