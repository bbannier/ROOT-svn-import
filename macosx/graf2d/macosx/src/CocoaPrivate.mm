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

}


//______________________________________________________________________________
CocoaWindowAttributes::CocoaWindowAttributes()
                         : fROOTWindowAttribs()
{
}

//______________________________________________________________________________
CocoaWindowAttributes::CocoaWindowAttributes(const WindowAttributes_t &winAttr, NSObject *nsWin)
                         : fROOTWindowAttribs(winAttr),
                           fCocoaWindow(nsWin)
{
}

//______________________________________________________________________________
CocoaPrivate::CocoaPrivate()
               : fCurrentWindowID(1)//0 is for 'ROOT', any real window has id > 0.
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
         fWindows[0] = CocoaWindowAttributes();
         CocoaWindowAttributes &cocoaWin = fWindows[0];

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

//______________________________________________________________________________
unsigned CocoaPrivate::RegisterWindow(NSObject *nsWin, const WindowAttributes_t &winAttr)
{
   //In case of X11, gVirtualX->CreateWindow returns some 'descriptor' (integer number),
   //which is valid for X11 calls and window can be identified by this descriptor.
   //With Cocoa, we have NSWindow pointers, which can not be simply returned from CreateWindow.
   //So I need some mapping between real NSObjects and ROOT's integers.
   //I have an internal numbering - just subsequent numbers. NSWindow has -windowNumber
   //method - I'll probably use it in future, but still the machinery will be almost the same.
   //If window is closed, it's id will go to fFreeWindowIDs array. So, check it first:
   
   unsigned newID = 0;

   if (fFreeWindowIDs.size()) {
      newID = fFreeWindowIDs.back();
      fFreeWindowIDs.pop_back();
   } else
      newID = fCurrentWindowID++;

#ifdef DEBUG_ROOT_COCOA
   if (fWindows.find(newID) != fWindows.end()) {
      NSLog(@"new window ID %u is still in use\n", newID);
      throw std::runtime_error("window id is still in use");
   }
#endif

   fWindows[newID] = CocoaWindowAttributes(winAttr, nsWin);
   
   return newID;
}

//______________________________________________________________________________
id<RootGUIElement> CocoaPrivate::GetWindow(unsigned winID)
{
   auto winIter = fWindows.find(winID);

#ifdef DEBUG_ROOT_COCOA
   if (winIter == fWindows.end()) {
      NSLog(@"Requested non-registered window with ID %u", winID);
      throw std::runtime_error("Non-existing window requested");
   }
#endif
   return (id<RootGUIElement>)winIter->second.fCocoaWindow.Get();
}

//______________________________________________________________________________
void CocoaPrivate::DeleteWindow(unsigned winID)
{
   auto winIter = fWindows.find(winID);

#ifdef DEBUG_ROOT_COCOA
   if (winIter == fWindows.end()) {
      NSLog(@"Attempt to delete window, which is not registered : %u", winID);
      throw std::runtime_error("Non existing winID in DeleteWindow");
   }
#endif
   
   //Probably, I'll need some additional cleanup here later. Now just delete NSWindow and
   //reuse its id.
   id window = winIter->second.fCocoaWindow.Get();
   if ([window respondsToSelector : @selector(removeFromSuperview)])
      [window removeFromSuperview];

   fFreeWindowIDs.push_back(winID);
   fWindows.erase(winIter);//StrongReference should do its work here.
}

}
}
}
