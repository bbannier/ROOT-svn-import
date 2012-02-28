//Author: Timur Pocheptsov 16/02/2012

#ifndef ROOT_X11Drawable
#define ROOT_X11Drawable

#import <utility>//HOHO!

#import <Cocoa/Cocoa.h>

#import "GuiTypes.h"

namespace ROOT {
namespace MacOSX {
namespace X11 {

//CGSize/NSSize use double.
typedef std::pair<unsigned, unsigned> DrawableSize_t;

//CGPoint/NSPoint use double.
typedef std::pair<int, int> Point_t;

}
}
}

@class QuartzPixmap;
@class QuartzView;

/////////////////////////////////////////////////////////////////////////////////////
//                                                                                 //
// Protocol for "drawables". It can be window, view (child window), pixmap. //
//                                                                                 //
/////////////////////////////////////////////////////////////////////////////////////

@protocol X11Drawable 
@optional

@property (nonatomic, assign) QuartzPixmap *fBackBuffer;
@property (nonatomic, assign) QuartzView *fParentView;
@property (nonatomic, assign) unsigned fID; //Identifier used by TGCocoa and ROOT's GUI classes.

//As soon as I have to somehow emulate X11's behavior to make GUI happy,
//I have this bunch of properties here to be set/read from a window.
//Some of them are used, some are just pure "emulation".
//Properties, which are used, are commented in a declaration.

/////////////////////////////////////////////////////////////////
//SetWindowAttributes_t/WindowAttributes_t

@property (nonatomic, assign) long          fEventMask; //Specifies which events must be processed by widget.

@property (nonatomic, assign) int           fClass;
@property (nonatomic, assign) int           fDepth;

@property (nonatomic, assign) int           fBitGravity;
@property (nonatomic, assign) int           fWinGravity;

@property (nonatomic, assign) unsigned long fBackgroundPixel;//At the moment used in a TGCocoa::ClearArea, for example.

@property (nonatomic, readonly) int         fMapState;


//End of SetWindowAttributes_t/WindowAttributes_t
/////////////////////////////////////////////////////////////////


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

@property (nonatomic, assign) int fGrabButton;
@property (nonatomic, assign) unsigned fGrabButtonEventMask;
@property (nonatomic, assign) unsigned fGrabKeyModifiers;
@property (nonatomic, assign) BOOL fOwnerEvents;
//modifier also.

//Geometry: (readonly)
- (int)      fX;
- (int)      fY; //top-left corner system.

- (unsigned) fWidth;
- (unsigned) fHeight;
- (NSSize)   fSize;

//Geometry: setters, parameters are in top-level system
//(will be converted into Cocoa system internally).
- (void)     setDrawableSize : (NSSize) newSize;
- (void)     setX : (int) x Y : (int) y width : (unsigned) w height : (unsigned) h;
- (void)     setX : (int) x Y : (int) y;

//Nested views ("windows").
- (void)     addChild : (QuartzView *)child;

//X11/ROOT GUI's attributes
- (void)     getAttributes : (WindowAttributes_t *) attr;
- (void)     setAttributes : (const SetWindowAttributes_t *) attr;

//
- (void)     mapRaised;
- (void)     mapWindow;
- (void)     mapSubwindows;

- (void)     unmapWindow;
//

- (void)     copy : (id<X11Drawable>) src fromPoint : (ROOT::MacOSX::X11::Point_t) srcPoint 
             size : (ROOT::MacOSX::X11::DrawableSize_t) size toPoint : (ROOT::MacOSX::X11::Point_t) dstPoint;

@end

#endif
