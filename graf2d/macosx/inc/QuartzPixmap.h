#ifndef ROOT_QuartzPixmap
#define ROOT_QuartzPixmap

#import <Cocoa/Cocoa.h>

#import "X11Drawable.h"
//
//
//"Pixmap".
//
//

@interface QuartzPixmap : NSObject<X11Drawable>

@property (nonatomic, assign) unsigned fID;
@property (nonatomic, readonly) BOOL   fIsPixmap;

@property (nonatomic, readonly) CGContextRef fContext;

- (id) initWithSize : (NSSize) frame;
- (BOOL) resize : (NSSize) newSize;

- (unsigned) fWidth;
- (unsigned) fHeight;
- (NSSize) fSize;

@end

#endif
