#ifndef ROOT_RootGUIElement
#define ROOT_RootGUIElement
//Include guards for C++ code.

@class RootQuartzView;

@protocol RootGUIElement <NSObject>
@required

- (void) addChildView : (RootQuartzView *)child;

- (RootQuartzView *) getTopLevelView;

- (void) setParent : (id<RootGUIElement>)parent;
- (id<RootGUIElement>) parent;

@end

#endif
