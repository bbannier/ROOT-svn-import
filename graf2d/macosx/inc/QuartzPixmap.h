//Author: Timur Pocheptsov 16/02/2012

#ifndef ROOT_QuartzPixmap
#define ROOT_QuartzPixmap

#import <Cocoa/Cocoa.h>

#import "X11Drawable.h"
#import "GuiTypes.h"

///////////////////////////////////////////////////////
//                                                   //
// "Pixmap". Graphical context to draw into image.   //
//                                                   //
///////////////////////////////////////////////////////

@interface QuartzPixmap : NSObject<X11Drawable>

@property (nonatomic, assign) unsigned fID;
@property (nonatomic, readonly) BOOL   fIsPixmap;

@property (nonatomic, readonly) CGContextRef fContext;

- (id) initWithW : (unsigned) width H :(unsigned) height;
- (BOOL) resizeW : (unsigned) width H : (unsigned) height;

- (CGImageRef) createImageFromPixmap;

- (unsigned) fWidth;
- (unsigned) fHeight;
- (unsigned char *) fData;

@end

/////////////////////////////////////////////////////////
//                                                     //
// CGImageRef, created from external data source (raw  //
// data)                                               //
//                                                     //
/////////////////////////////////////////////////////////

@interface QuartzImage : NSObject<X11Drawable>

@property (nonatomic, assign) unsigned fID;
@property (nonatomic, readonly) BOOL fIsPixmap;//???

- (id) initWithW : (unsigned) width H : (unsigned) height data : (unsigned char *)data;
- (id) initMaskWithW : (unsigned) width H : (unsigned) height bitmapMask : (unsigned char *)mask;

- (void) dealloc;

- (unsigned) fWidth;
- (unsigned) fHeight;

- (CGImageRef) fImage;

- (unsigned char *) readColorBits : (Rectangle_t) area;

@end


namespace ROOT {
namespace MacOSX {
namespace X11 {//X11 emulation. But must go into quartz module later.

CGImageRef CreateSubImage(QuartzImage *image, const Rectangle_t &area);

}
}
}

#endif
