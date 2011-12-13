#ifndef ROOT_RootGUIElement
#define ROOT_RootGUIElement
//Include guards for C++ code.

@class RootQuartzView;

@protocol RootGUIElement <NSObject>
@required

- (void) addChildView : (RootQuartzView *)child;
- (void) setParentView : (id<RootGUIElement>)parent;
- (id<RootGUIElement>) parentView;

@property (nonatomic, assign) unsigned fWinID;

@end

#endif
