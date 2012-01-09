#ifndef ROOT_RootGUIElement
#define ROOT_RootGUIElement
//Include guards for C++ code.

//TODO: better include separate headers for CGContextRef/NSObject.
#import <Cocoa/Cocoa.h>

@class RootQuartzView;

//TODO: better protocol for this method is X11Drawable or something like this,
//and such a drawable can be NSWindow-derived object, NSView-derived, or
//NSImage (or NSObject-derived wrapper for CGImage).
//In this case, methods' names must change from addChildView, to addChildDrawable and etc.
//Some concept needed to name contentView, parentView, etc.

//Methods like GetDrawableAttributes can be added (instead of current version of CocoaPrivate, 
//which saves WindowAttributes_t.

@protocol RootGUIElement <NSObject>
@required

- (void) addChildView : (RootQuartzView *)child;
- (void) setParentView : (id<RootGUIElement>)parent;
- (id<RootGUIElement>) parentView;
- (NSView *) contentView;

@property (nonatomic, assign) unsigned fWinID;

//Ugly hack.
@property (nonatomic, readonly) BOOL fIsPixmap;

//Can it be readonly here, and assign in a class, adopting this protocol?
@property (nonatomic, assign) CGContextRef fCurrentContext;


@end

#endif
