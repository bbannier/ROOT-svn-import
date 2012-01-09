#ifndef ROOT_CocoaPixmap
#define ROOT_CocoaPixmap

#import <Cocoa/Cocoa.h>

#import "RootGUIElement.h"

@interface CocoaPixmap : NSObject<RootGUIElement>

- (id) initWithSize : (NSSize) pixmapSize;
- (BOOL) resizePixmap : (NSSize) newPixmapSize;

//Fake RootGUIElement.
- (void) addChildView : (RootQuartzView *)child;
- (void) setParentView : (id<RootGUIElement>)parent;
- (id<RootGUIElement>) parentView;
- (NSView *) contentView;

@property (nonatomic, readonly) unsigned fWidth;
@property (nonatomic, readonly) unsigned fHeight;
@property (nonatomic, readonly) CGImageRef fImage;

//Fake RootGUIElement.
@property (nonatomic, assign) unsigned fWinID;

@property (nonatomic, readonly) BOOL fIsPixmap;

@end

#endif
