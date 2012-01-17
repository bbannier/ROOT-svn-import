#ifndef ROOT_X11Drawables
#define ROOT_X11Drawables

#import <Cocoa/Cocoa.h>

#import "GuiTypes.h"

@class QuartzView;

namespace ROOT {
namespace MacOSX {
namespace X11 {

//Coordinate conversion.
int GlobalYCocoaToROOT(CGFloat yCocoa);
//:)
int GlobalYROOTToCocoa(CGFloat yROOT);

int LocalYCocoaToROOT(QuartzView *parentView, CGFloat yCocoa);
//:)
int LocalYROOTToCocoa(QuartzView *parentView, CGFloat yROOT);

}
}
}


//Protocol to emulate "drawables". It can be window, view (child window), pixmap.
@class QuartzPixmap;

@protocol X11Drawable 
@optional

@property (nonatomic, assign) QuartzPixmap *fBackBuffer;
@property (nonatomic, assign) QuartzView *fParentView;
@property (nonatomic, assign) unsigned fID; //Identifier used by TGCocoa and ROOT's GUI classes.
@property (nonatomic, assign) unsigned fEventMask; //Specifies which events must be processed by widget.

//
@property (nonatomic, assign) unsigned long fBackgroundPixel;

//In X11 drawable is window or pixmap, ROOT's GUI
//also has this ambiguity. So I have a property
//to check in TGCocoa, what's the object.
@property (nonatomic, readonly) BOOL fIsPixmap;
//Window has a content view, self is a content view for a view.
@property (nonatomic, readonly) QuartzView *fContentView;

//If we draw "in a normal mode" == drawing operation were called
//from view's drawRect method, this property is not nil.
//TGCocoa, when some drawing method is called, will ask view/window
//about context, if it's not nil - we'll use it, but if it is nil,
//we try to get current context and lock focus on the current view.
@property (nonatomic, readonly) CGContextRef fContext;

//Geometry: (readonly)
- (int)      fX;

//X11 (== ROOT's GUI) and Cocoa uses different coordinate systems:
//in X11's window (0, 0) is a top-left corner,
//in Cocoa's window it's bottom left.

- (int)      fYCocoa; //bottom-left system.
- (int)      fYROOT;  //top-left system.

- (unsigned) fWidth;
- (unsigned) fHeight;
- (NSSize)   fSize;

//Geometry: setters, parameters are in top-level system
//(will be converted into Cocoa system internally).
- (void)     setDrawableSize : (NSSize) newSize;
- (void)     setX : (int) x rootY : (int) y width : (unsigned) w height : (unsigned) h;

//Nested views ("windows").
- (void)     addChild : (QuartzView *)child;

//X11/ROOT GUI's attributes
- (void)     getAttributes : (WindowAttributes_t *) attr;
- (void)     setAttributes : (const SetWindowAttributes_t *) attr;

@end

//
//
//RootQuartzWindow class : top-level window. 
//
//

@interface QuartzWindow : NSWindow<X11Drawable>//, NSWindowDelegate>

@property (nonatomic, assign) QuartzPixmap *fBackBuffer;
@property (nonatomic, assign) QuartzView *fParentView;
@property (nonatomic, assign) unsigned fID;
@property (nonatomic, assign) unsigned fEventMask;

@property (nonatomic, assign) unsigned long fBackgroundPixel;

@property (nonatomic, readonly) BOOL fIsPixmap;
@property (nonatomic, readonly) QuartzView *fContentView;

@property (nonatomic, readonly) CGContextRef fContext;

//Geometry.
- (int)      fX;
- (int)      fYCocoa;
- (int)      fYROOT;

- (unsigned) fWidth;
- (unsigned) fHeight;
- (NSSize)   fSize;

- (void)     setDrawableSize : (NSSize) newSize;
- (void)     setX : (int) x rootY : (int) y width : (unsigned) w height : (unsigned) h;

//Children subviews.
- (void)     addChild : (QuartzView *)child;

//X11/ROOT GUI's attributes.
- (void)     getAttributes : (WindowAttributes_t *)attr;
- (void)     setAttributes : (const SetWindowAttributes_t *)attr;

@end

//
//
//Child window.
//
//

@interface QuartzView : NSView<X11Drawable>

@property (nonatomic, assign) QuartzPixmap *fBackBuffer;
@property (nonatomic, assign) QuartzView *fParentView;

@property (nonatomic, assign) unsigned fID;
@property (nonatomic, assign) unsigned fEventMask;

@property (nonatomic, assign) unsigned long fBackgroundPixel;

@property (nonatomic, readonly) BOOL fIsPixmap;
@property (nonatomic, readonly) QuartzView *fContentView;

@property (nonatomic, readonly) CGContextRef fContext;

//Geometry.
- (int)      fX;
- (int)      fYCocoa;
- (int)      fYROOT;

- (unsigned) fWidth;
- (unsigned) fHeight;
- (NSSize)   fSize;

- (void)     setDrawableSize : (NSSize) newSize;
- (void)     setX : (int) x rootY : (int) y width : (unsigned) w height : (unsigned) h;

//Children subviews.
- (void)     addChild : (QuartzView *)child;

//X11/ROOT GUI's attributes.
- (void)     getAttributes : (WindowAttributes_t *)attr;
- (void)     setAttributes : (const SetWindowAttributes_t *)attr;

@end

//
//
//"Pixmap".
//
//

@interface QuartzPixmap : NSObject<X11Drawable>

@property (nonatomic, assign) unsigned fID;
@property (nonatomic, readonly) BOOL fIsPixmap;

@end

#endif
