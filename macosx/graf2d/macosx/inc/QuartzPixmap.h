//Author: Timur Pocheptsov 16/02/2012

#ifndef ROOT_QuartzPixmap
#define ROOT_QuartzPixmap

#import <Cocoa/Cocoa.h>

#import "X11Drawable.h"

///////////////////////////////////////////////////////
//                                                   //
// "Pixmap". Graphical context to draw into image.   //
//                                                   //
///////////////////////////////////////////////////////

//TODO: names are misleading and wrong.

@interface QuartzPixmap : NSObject<X11Drawable>

@property (nonatomic, assign) unsigned fID;
@property (nonatomic, readonly) BOOL   fIsPixmap;

@property (nonatomic, readonly) CGContextRef fContext;

- (id) initWithW : (unsigned) width H :(unsigned) height;
- (BOOL) resizeW : (unsigned) width H : (unsigned) height;

- (unsigned) fWidth;
- (unsigned) fHeight;
//- (NSSize) fSize;

@end

//TODO: names are misleading and wrong.
//This is only a temporary non-generic solution to
//make TGCocoa work (CreatePixmapFromData, etc. - 
//image created from data provided by
//ASImage.

@interface QuartzImage : NSObject<X11Drawable>

@property (nonatomic, assign) unsigned fID;
@property (nonatomic, readonly) BOOL fIsPixmap;//???

- (id) initWithW : (unsigned) width H : (unsigned) height data : (unsigned char *)data;
- (void) dealloc;

- (unsigned) fWidth;
- (unsigned) fHeight;

- (CGImageRef) fImage;

@end


#endif
