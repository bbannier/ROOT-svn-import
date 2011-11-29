#define DEBUG_ROOT_COCOA

#ifdef DEBUG_ROOT_COCOA
#include <iostream>
#endif

#include <stdexcept>

#include <Cocoa/Cocoa.h>

#include "CocoaPrivate.h"
#include "CocoaUtils.h"

namespace ROOT {
namespace MacOSX {
namespace Details {

namespace {

/*

// Window attributes that can be inquired
struct WindowAttributes_t {
   Int_t      fX, fY;                 // location of window
   Int_t      fWidth, fHeight;        // width and height of window
   Int_t      fBorderWidth;           // border width of window
   Int_t      fDepth;                 // depth of window +
   void      *fVisual;                // the associated visual structure
   Window_t   fRoot;                  // root of screen containing window
   Int_t      fClass;                 // kInputOutput, kInputOnly
   Int_t      fBitGravity;            // one of bit gravity values
   Int_t      fWinGravity;            // one of the window gravity values
   Int_t      fBackingStore;          // kNotUseful, kWhenMapped, kAlways
   ULong_t    fBackingPlanes;         // planes to be preserved if possible
   ULong_t    fBackingPixel;          // value to be used when restoring planes
   Bool_t     fSaveUnder;             // boolean, should bits under be saved?
   Colormap_t fColormap;              // color map to be associated with window
   Bool_t     fMapInstalled;          // boolean, is color map currently installed
   Int_t      fMapState;              // kIsUnmapped, kIsUnviewable, kIsViewable
   Long_t     fAllEventMasks;         // set of events all people have interest in
   Long_t     fYourEventMask;         // my event mask
   Long_t     fDoNotPropagateMask;    // set of events that should not propagate
   Bool_t     fOverrideRedirect;      // boolean value for override-redirect
   void      *fScreen;                // back pointer to correct screen
};
//
*/

//______________________________________________________________________________
void GetWindowAttributesFromScreen(NSScreen *screen, CocoaWindowAttributes &cocoaWin)
{
}

}


//______________________________________________________________________________
CocoaWindowAttributes::CocoaWindowAttributes()
                         : fROOTWindowAttribs()
{
}

//______________________________________________________________________________
CocoaPrivate::CocoaPrivate()
{
   //Init NSApplication.
   Util::AutoreleasePool pool;
   
   [NSApplication sharedApplication];
   
   InitX11RootWindow();
}

//______________________________________________________________________________
CocoaPrivate::~CocoaPrivate()
{
}

//______________________________________________________________________________
void CocoaPrivate::InitX11RootWindow()
{
   //Fake (at the moment?) 'ROOT' window for X11 emulation.
   NSArray *screens = [NSScreen screens];
   if (screens) {
      NSScreen *mainScreen = [screens objectAtIndex : 0];
      if (mainScreen) {
         fWindows.push_back(CocoaWindowAttributes());
         CocoaWindowAttributes &cocoaWin = fWindows.back();

         WindowAttributes_t &rootAttr = cocoaWin.fROOTWindowAttribs;
         const NSRect frame = [mainScreen frame];
         
         //These are parameters, used in TGFrame, when it's created as "fake" window
         //for X11's root window. 
         
         //TODO: Set other parameters later.
         rootAttr.fX = frame.origin.x;
         rootAttr.fY = frame.origin.y;
         rootAttr.fWidth = frame.size.width;
         rootAttr.fHeight = frame.size.height;
         rootAttr.fBorderWidth = 0;
         rootAttr.fYourEventMask = 0;
         //
         rootAttr.fDepth = NSBitsPerPixelFromDepth([mainScreen depth]);
         rootAttr.fVisual = &cocoaWin;
         rootAttr.fRoot = Window_t();
         //
      } else
         throw std::runtime_error("screen at index 0 is nil");
   } else 
      throw std::runtime_error("-screens returned nil");
}

}
}
}
