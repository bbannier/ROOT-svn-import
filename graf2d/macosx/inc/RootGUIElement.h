#ifndef ROOT_RootGUIElement
#define ROOT_RootGUIElement
//Include guards for C++ code.

@class RootQuartzView;

@protocol RootGUIElement <NSObject>
@required

- (void) addChildView : (RootQuartzView *)child;
- (void) setParentView : (id<RootGUIElement>)parent;
- (id<RootGUIElement>) parentView;

//This is a test function to check, if I can draw at any moment I want.
- (void) clearWidget;

@property (nonatomic, assign) unsigned fWinID;

@end

#endif
