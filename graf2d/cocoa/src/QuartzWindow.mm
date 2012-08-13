// @(#)root/graf2d:$Id$
// Author: Timur Pocheptsov   16/02/2012

/*************************************************************************
 * Copyright (C) 1995-2012, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

//#define DEBUG_ROOT_COCOA

//#define NDEBUG

#ifdef DEBUG_ROOT_COCOA
#import <iostream>
#import <fstream>

#import "TClass.h"
#endif

#import <algorithm>
#import <stdexcept>
#import <cassert>

#import "QuartzWindow.h"
#import "QuartzPixmap.h"
#import "QuartzUtils.h"
#import "CocoaUtils.h"
#import "X11Buffer.h"
#import "X11Events.h"
#import "TGWindow.h"
#import "TGClient.h"
#import "TSystem.h"
#import "TGCocoa.h"

namespace ROOT {
namespace MacOSX {
namespace X11 {

//______________________________________________________________________________
QuartzWindow *CreateTopLevelWindow(Int_t x, Int_t y, UInt_t w, UInt_t h, UInt_t /*border*/, Int_t depth,
                                   UInt_t clss, void */*visual*/, SetWindowAttributes_t *attr, UInt_t)
{
   NSRect winRect = {};
   winRect.origin.x = x; 
   winRect.origin.y = GlobalYROOTToCocoa(y);
   winRect.size.width = w;
   winRect.size.height = h;

   //TODO check mask.
   const NSUInteger styleMask = NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask;
   //
   QuartzWindow * const newWindow = [[QuartzWindow alloc] initWithContentRect : winRect styleMask : styleMask backing : NSBackingStoreBuffered defer : YES windowAttributes : attr];
   if (!newWindow)
      throw std::runtime_error("CreateTopLevelWindow failed");
   //
   newWindow.fDepth = depth;
   newWindow.fClass = clss;

   return newWindow;
}

//______________________________________________________________________________
QuartzView *CreateChildView(QuartzView * /*parent*/, Int_t x, Int_t y, UInt_t w, UInt_t h, UInt_t /*border*/, Int_t /*depth*/,
                            UInt_t /*clss*/, void * /*visual*/, SetWindowAttributes_t *attr, UInt_t /*wtype*/)
{
   NSRect viewRect = {};
   viewRect.origin.x = x;
   viewRect.origin.y = y;
   viewRect.size.width = w;
   viewRect.size.height = h;
   
   QuartzView * const view = [[QuartzView alloc] initWithFrame : viewRect windowAttributes : attr];
   if (!view)
      throw std::runtime_error("CreateChildView failed");
   
   return view;
}

//______________________________________________________________________________
void GetRootWindowAttributes(WindowAttributes_t *attr)
{
   //'root' window does not exist, but we can request its attributes.
   assert(attr != 0 && "GetRootWindowAttributes, attr parameter is null");
   
   NSArray * const screens = [NSScreen screens];
   assert(screens != nil && "screens array is nil");
   
   NSScreen * const mainScreen = [screens objectAtIndex : 0];
   assert(mainScreen != nil && "screen with index 0 is nil");

   *attr = WindowAttributes_t();
   
   attr->fX = 0;
   attr->fY = 0;
   
   const NSRect frame = [mainScreen frame];
   
   attr->fWidth = frame.size.width;
   attr->fHeight = frame.size.height;
   attr->fBorderWidth = 0;
   attr->fYourEventMask = 0;
   attr->fAllEventMasks = 0;//???

   attr->fDepth = NSBitsPerPixelFromDepth([mainScreen depth]);
   attr->fVisual = 0;
   attr->fRoot = 0;
}


//Coordinate conversion.

//TODO: check how TGX11 extracts/changes window attributes.

//______________________________________________________________________________
int GlobalYCocoaToROOT(CGFloat yCocoa)
{
   NSArray * const screens = [NSScreen screens];
   assert(screens != nil && "GlobalYCocoaToROOT, screens array is nil");
   
   NSScreen * const mainScreen = [screens objectAtIndex : 0];
   assert(mainScreen != nil && "GlobalYCocoaToROOT, screen at index 0 is nil");
   
   return int(mainScreen.frame.size.height - yCocoa);
}

//______________________________________________________________________________
int GlobalYROOTToCocoa(CGFloat yROOT)
{
   //hehe :)) actually, no need in this function.
   NSArray * const screens = [NSScreen screens];
   assert(screens != nil && "GlobalYROOTToCocoa, screens array is nil");
   
   NSScreen * const mainScreen = [screens objectAtIndex : 0];
   assert(mainScreen != nil && "GlobalYROOTToCocoa, screen at index 0 is nil");
   
   return int(mainScreen.frame.size.height - yROOT);
}

//______________________________________________________________________________
int LocalYCocoaToROOT(NSView<X11Window> *parentView, CGFloat yCocoa)
{
   assert(parentView != nil && "LocalYCocoaToROOT, parent view is nil");
   
   return int(parentView.frame.size.height - yCocoa);
}

//______________________________________________________________________________
int LocalYROOTToCocoa(NSView<X11Window> *parentView, CGFloat yROOT)
{
   //:)
   assert(parentView != nil && "LocalYROOTToCocoa, parent view is nil");
   
   return int(parentView.frame.size.height - yROOT);
}


//______________________________________________________________________________
int LocalYROOTToCocoa(NSObject<X11Drawable> *drawable, CGFloat yROOT)
{
   //:)
   assert(drawable != nil && "LocalYROOTToCocoa, drawable is nil");
   
   return int(drawable.fHeight - yROOT);
}

//______________________________________________________________________________
NSPoint TranslateToScreen(NSView<X11Window> *from, NSPoint point)
{
   assert(from != nil && "TranslateToScreen, 'from' parameter is nil");
   
   //TODO: I do not know, if I can use convertToBacking ..... - have to check this.
   NSPoint winPoint = [from convertPoint : point toView : nil];
   NSPoint screenPoint = [[from window] convertBaseToScreen : winPoint];; 
   //TODO: This is Cocoa's coordinates, but for ROOT I have to convert.
   screenPoint.y = GlobalYCocoaToROOT(screenPoint.y);

   return screenPoint;
}

//______________________________________________________________________________
NSPoint TranslateFromScreen(NSPoint point, NSView<X11Window> *to)
{
   assert(to != nil && "TranslateFromScreen, 'to' parameter is nil");
   
   point.y = GlobalYROOTToCocoa(point.y);

   //May be I can use convertBackingTo .... have to check this.
   const NSPoint winPoint = [[to window] convertScreenToBase : point];
   return [to convertPoint : winPoint fromView : nil];
}

//______________________________________________________________________________
NSPoint TranslateCoordinates(NSView<X11Window> *from, NSView<X11Window> *to, NSPoint sourcePoint)
{
   //Both views are valid.
   assert(from != nil && "TranslateCoordinates, 'from' parameter is nil");
   assert(to != nil && "TranslateCoordinates, 'to' parameter is nil");

   if ([from window] == [to window]) {
      //Both views are in the same window.
      return [to convertPoint : sourcePoint fromView : from];      
   } else {
      //May be, I can do it in one call, but it's not obvious for me
      //what is 'pixel aligned backing store coordinates' and
      //if they are the same as screen coordinates.
      
      const NSPoint win1Point = [from convertPoint : sourcePoint toView : nil];
      const NSPoint screenPoint = [[from window] convertBaseToScreen : win1Point];
      const NSPoint win2Point = [[to window] convertScreenToBase : screenPoint];

      return [to convertPoint : win2Point fromView : nil];
   }
}

//______________________________________________________________________________
bool ScreenPointIsInView(NSView<X11Window> *view, Int_t x, Int_t y)
{
   assert(view != nil && "ScreenPointIsInView, view parameter is nil");
   
   NSPoint point = {};
   point.x = x, point.y = y;
   point = TranslateFromScreen(point, view);
   const NSRect viewFrame = view.frame;

   if (point.x < 0 || point.x > viewFrame.size.width)
      return false;
   if (point.y < 0 || point.y > viewFrame.size.height)
      return false;
   
   return true;
}

//______________________________________________________________________________
QuartzWindow *FindWindowInPoint(Int_t x, Int_t y)
{
   const Util::AutoreleasePool pool;//array's counter is increased, all object in array are also retained.

   NSArray * const orderedWindows = [NSApp orderedWindows];
   for (NSWindow *window in orderedWindows) {
      if (![window isKindOfClass : [QuartzWindow class]])
         continue;
      QuartzWindow * const qw = (QuartzWindow *)window;
      //Check if point is inside.
      if (ScreenPointIsInView(qw.fContentView, x, y))
         return qw;
   }
   
   return nil;
}

//______________________________________________________________________________
NSView<X11Window> *FindDNDAwareViewInPoint(NSArray *children, Window_t dragWinID, Window_t inputWinID, Int_t x, Int_t y, Int_t maxDepth)
{
   assert(children != nil && "FindDNDAwareViewInPoint, children parameter is nil");
   
   if (maxDepth <= 0)
      return nil;

   NSEnumerator * const reverseEnumerator = [children reverseObjectEnumerator];
   for (NSView<X11Window> *child in reverseEnumerator) {
      if (!ScreenPointIsInView(child, x, y))
         continue;
      if (child.fIsDNDAware && child.fID != dragWinID && child.fID != inputWinID)
         return child;//got it!
            
      NSView<X11Window> * const testView = FindDNDAwareViewInPoint([child subviews], dragWinID, inputWinID, x, y, maxDepth - 1);
      if (testView)
         return testView;
   }

   return nil;
}

//______________________________________________________________________________
NSView<X11Window> *FindDNDAwareViewInPoint(NSView *parentView, Window_t dragWinID, Window_t inputWinID, Int_t x, Int_t y, Int_t maxDepth)
{
   //X and Y are ROOT's screen coordinates (Y is inverted).
   if (maxDepth <= 0)
      return nil;

   const Util::AutoreleasePool pool;

   if (!parentView) {//Start from the screen as a 'root' window.
      NSArray * const orderedWindows = [NSApp orderedWindows];
      for (NSWindow *window in orderedWindows) {
         if (![window isKindOfClass : [QuartzWindow class]])
            continue;
         QuartzWindow * const qw = (QuartzWindow *)window;
         if (qw.fMapState != kIsViewable)
            continue;

         //First, check this view itself, my be we found what we need already.
         NSView<X11Window> *testView = qw.fContentView;
         if (!ScreenPointIsInView(testView, x, y))
            continue;
         
         if (testView.fIsDNDAware && testView.fID != dragWinID && testView.fID != inputWinID)
            return testView;

         //Recursive part, check children.
         NSArray * const children = [testView subviews];
         testView = FindDNDAwareViewInPoint(children, dragWinID, inputWinID, x, y, maxDepth - 1);
         if (testView)
            return testView;
      }

      //We did not find anything for 'root' window as parent.
      return nil;
   } else {
      //Parent view is tested already (or should not be tested at all, check children.
      return FindDNDAwareViewInPoint([parentView subviews], dragWinID, inputWinID, x, y, maxDepth);
   }
}

//______________________________________________________________________________
QuartzWindow *FindWindowUnderPointer()
{
   const Util::AutoreleasePool pool;

   NSArray * const orderedWindows = [NSApp orderedWindows];
   for (NSWindow *nsWindow in orderedWindows) {
      if (![nsWindow isKindOfClass : [QuartzWindow class]])
         continue;

      QuartzWindow * const qWindow = (QuartzWindow *)nsWindow;
      if (qWindow.fMapState != kIsViewable)//Can it be false and still in this array???
         continue;
      
      const NSPoint mousePosition = [qWindow mouseLocationOutsideOfEventStream];
      const NSSize windowSize = qWindow.frame.size;
      if (mousePosition.x >= 0 && mousePosition.x <= windowSize.width && mousePosition.y >= 0 && mousePosition.y <= windowSize.height)
         return qWindow;
   }

   return nil;
}

//______________________________________________________________________________
NSView<X11Window> *FindViewUnderPointer()
{
   //TODO: call FindViewInPoint using cursor screen coordiantes.
   const Util::AutoreleasePool pool;
   
   if (QuartzWindow *topLevel = FindWindowUnderPointer()) {
      const NSPoint mousePosition = [topLevel mouseLocationOutsideOfEventStream];
      return (NSView<X11Window> *)[[topLevel contentView] hitTest : mousePosition];
   }

   return nil;
}

//______________________________________________________________________________
QuartzWindow *FindWindowForPointerEvent(NSEvent *pointerEvent)
{
   //FindWindowForPointerEvent is required because due to grabs the receiver of the event
   //can be different from the actual window under cursor.

   assert(pointerEvent != nil && "FindWindowForPointerEvent, pointerEvent parameter is nil");

   const Util::AutoreleasePool pool;

   NSArray * const orderedWindows = [NSApp orderedWindows];
   for (NSWindow *nsWindow in orderedWindows) {
      if (![nsWindow isKindOfClass : [QuartzWindow class]])
         continue;

      QuartzWindow * const qWindow = (QuartzWindow *)nsWindow;
      if (qWindow.fMapState != kIsViewable)//Can it be false and still in this array???
         continue;

      NSPoint mousePosition = [pointerEvent locationInWindow];
      //The event has a window, so position is in this window's coordinate system,
      //convert it into screen point first.
      if ([pointerEvent window])
         mousePosition = [[pointerEvent window] convertBaseToScreen : mousePosition];
      
      mousePosition = [qWindow convertScreenToBase : mousePosition];

      const NSSize windowSize = qWindow.frame.size;
      if (mousePosition.x >= 0 && mousePosition.x <= windowSize.width && mousePosition.y >= 0 && mousePosition.y <= windowSize.height)
         return qWindow;
   }

   return nil;
}

//______________________________________________________________________________
NSView<X11Window> *FindViewForPointerEvent(NSEvent *pointerEvent)
{
   //FindViewForPointerEvent is required because of grabs - the receiver of the
   //event can be different from the actual window under cursor.
   
   assert(pointerEvent != nil && "FindViewForPointerEvent, pointerEvent parameter is nil");
   
   const Util::AutoreleasePool pool;
   
   if (QuartzWindow *topLevel = FindWindowForPointerEvent(pointerEvent)) {
      NSPoint mousePosition = [pointerEvent locationInWindow];
      if ([pointerEvent window])
         mousePosition = [[pointerEvent window] convertBaseToScreen : mousePosition];
      
      mousePosition = [topLevel convertScreenToBase : mousePosition];
      return (NSView<X11Window> *)[[topLevel contentView] hitTest : mousePosition];
   }

   return nil;
}

//______________________________________________________________________________
void SetWindowAttributes(const SetWindowAttributes_t *attr, NSObject<X11Window> *window)
{
   assert(attr != 0 && "SetWindowAttributes, attr parameter is null");
   assert(window != nil && "SetWindowAttributes, window parameter is nil");

   const Mask_t mask = attr->fMask;

   if (mask & kWABackPixel)
      window.fBackgroundPixel = attr->fBackgroundPixel;
   
   if (mask & kWAEventMask)
      window.fEventMask = attr->fEventMask;

   if (mask & kWABitGravity)
      window.fBitGravity = attr->fBitGravity;

   if (mask & kWAWinGravity)
      window.fWinGravity = attr->fWinGravity;
   
   //TODO: More attributes to set -
   //cursor for example, etc.
   if (mask & kWAOverrideRedirect) {
      //This is quite a special case.
      //TODO: Must be checked yet, if I understand this correctly!
      if ([(NSObject *)window isKindOfClass : [QuartzWindow class]]) {
         QuartzWindow * const qw = (QuartzWindow *)window;
         [qw setStyleMask : NSBorderlessWindowMask];
         [qw setAlphaValue : 0.95];
      }
   }
}

//______________________________________________________________________________
void GetWindowGeometry(NSObject<X11Window> *win, WindowAttributes_t *dst)
{
   assert(win != nil && "GetWindowGeometry, win parameter is nil");
   assert(dst != 0 && "GetWindowGeometry, dst paremeter is null");
   
   dst->fX = win.fX;
   dst->fY = win.fY;
   
   dst->fWidth = win.fWidth;
   dst->fHeight = win.fHeight;
}

//______________________________________________________________________________
void GetWindowAttributes(NSObject<X11Window> *window, WindowAttributes_t *dst)
{
   assert(window != nil && "GetWindowAttributes, window parameter is nil");
   assert(dst != 0 && "GetWindowAttributes, attr parameter is null");
   
   *dst = WindowAttributes_t();
   
   //fX, fY, fWidth, fHeight.
   GetWindowGeometry(window, dst);

   //Actually, most of them are not used by GUI.
   dst->fBorderWidth = 0;
   dst->fDepth = window.fDepth;
   //Dummy value.
   dst->fVisual = 0;
   //Dummy value.
   dst->fRoot = 0;
   dst->fClass = window.fClass;
   dst->fBitGravity = window.fBitGravity;
   dst->fWinGravity = window.fWinGravity;
   //Dummy value.
   dst->fBackingStore = kAlways;//??? CHECK
   dst->fBackingPlanes = 0;

   //Dummy value.
   dst->fBackingPixel = 0;
   
   dst->fSaveUnder = 0;

   //Dummy value.
   dst->fColormap = 0;
   //Dummy value.   
   dst->fMapInstalled = kTRUE;

   dst->fMapState = window.fMapState;

   dst->fAllEventMasks = window.fEventMask;
   dst->fYourEventMask = window.fEventMask;
   
   //Not used by GUI.
   //dst->fDoNotPropagateMask

   dst->fOverrideRedirect = 0;
   //Dummy value.
   dst->fScreen = 0;
}

//With Apple's poor man's objective C/C++ + "brilliant" Cocoa you never know, what should be 
//the linkage of callback functions, API + language dialects == MESS. I declare/define this comparators here
//as having "C++" linkage. If one good day clang will start to complane, I'll have to change this.

//______________________________________________________________________________
NSComparisonResult CompareViewsToLower(id view1, id view2, void *context)
{
    id topView = (id)context;//ARC will require _brigde cast, but NO ARC! :)
    if (view1 == topView)
        return NSOrderedAscending;
    if (view2 == topView)
        return NSOrderedDescending;

    return NSOrderedSame;
}

//______________________________________________________________________________
NSComparisonResult CompareViewsToRaise(id view1, id view2, void *context)
{
   id topView = (id)context;
   if (view1 == topView)
      return NSOrderedDescending;
   if (view2 == topView)
      return NSOrderedAscending;

   return NSOrderedSame;
}

//______________________________________________________________________________
NSPoint GetCursorHotStop(NSImage *image, ECursor cursor)
{
   assert(image != nil && "CursroHotSpot, image parameter is nil");
   
   const NSSize imageSize = image.size;

   if (cursor == kArrowRight) 
      return CGPointMake(imageSize.width, imageSize.height / 2);
   
   return CGPointMake(imageSize.width / 2, imageSize.height / 2);
}

//TGTextView is a very special window: it's a TGCompositeFrame,
//which has TGCompositeFrame inside (TGViewFrame). This TGViewFrame
//delegates Expose events to its parent, and parent tries to draw
//inside a TGViewFrame. This does not work with default 
//QuartzView -drawRect/TGCocoa. So I need a trick to identify
//this special window.

//______________________________________________________________________________
bool ViewIsTextView(unsigned viewID)
{
   const TGWindow * const window = gClient->GetWindowById(viewID);
   if (!window)
      return false;   
   return window->InheritsFrom("TGTextView");
}

//______________________________________________________________________________
bool ViewIsTextView(NSView<X11Window> *view)
{
   assert(view != nil && "ViewIsTextView, view parameter is nil");

   return ViewIsTextView(view.fID);
}

//______________________________________________________________________________
bool ViewIsTextViewFrame(NSView<X11Window> *view, bool checkParent)
{
   assert(view != nil && "ViewIsTextViewFrame, view parameter is nil");
   
   const TGWindow * const window = gClient->GetWindowById(view.fID);
   if (!window)
      return false;

   if (!window->InheritsFrom("TGViewFrame"))
      return false;
      
   if (!checkParent)
      return true;
      
   if (!view.fParentView)
      return false;
      
   return ViewIsTextView(view.fParentView);
}

//______________________________________________________________________________
NSView<X11Window> *FrameForTextView(NSView<X11Window> *textView)
{
   assert(textView != nil && "FrameForTextView, textView parameter is nil");
   
   for (NSView<X11Window> *child in [textView subviews]) {
      if (ViewIsTextViewFrame(child, false))
         return child;
   }
   
   return nil;
}

//______________________________________________________________________________
bool LockFocus(NSView<X11Window> *view)
{
   assert(view != nil && "LockFocus, view parameter is nil");
   assert([view isKindOfClass : [QuartzView class]] && "LockFocus, QuartzView is expected");
   
   if ([view lockFocusIfCanDraw]) {
      NSGraphicsContext *nsContext = [NSGraphicsContext currentContext];
      assert(nsContext != nil && "LockFocus, currentContext is nil");
      CGContextRef currContext = (CGContextRef)[nsContext graphicsPort];
      assert(currContext != 0 && "LockFocus, graphicsPort is null");//remove this assert?
      
      ((QuartzView *)view).fContext = currContext;
      
      return true;
   }
   
   return false;
}

//______________________________________________________________________________
void UnlockFocus(NSView<X11Window> *view)
{
   assert(view != nil && "UnlockFocus, view parameter is nil");
   assert([view isKindOfClass : [QuartzView class]] && "UnlockFocus, QuartzView is expected");
   
   [view unlockFocus];
   ((QuartzView *)view).fContext = 0;
}

//________________________________________________________________________________________
NSRect FindOverlapRect(const NSRect &viewRect, const NSRect &siblingViewRect)
{
   NSRect frame1 = viewRect;
   NSRect frame2 = siblingViewRect;

   //Adjust frames - move to frame1's space.
   frame2.origin.x -= frame1.origin.x;
   frame2.origin.y -= frame1.origin.y;
   frame1.origin = CGPointZero;

   NSRect overlap = {};
   
   if (frame2.origin.x < 0) {
      overlap.size.width = std::min(frame1.size.width, frame2.size.width - (frame1.origin.x - frame2.origin.x));
   } else {
      overlap.origin.x = frame2.origin.x;
      overlap.size.width = std::min(frame2.size.width, frame1.size.width - frame2.origin.x);
   }
   
   if (frame2.origin.y < 0) {
      overlap.size.height = std::min(frame1.size.height, frame2.size.height - (frame1.origin.y - frame2.origin.y));
   } else {
      overlap.origin.y = frame2.origin.y;
      overlap.size.height = std::min(frame2.size.height, frame1.size.height - frame2.origin.y);
   }
   
   return overlap;

}

//________________________________________________________________________________________
bool RectsOverlap(const NSRect &r1, const NSRect &r2)
{
   if (r2.origin.x >= r1.origin.x + r1.size.width)
      return false;
   if (r2.origin.x + r2.size.width <= r1.origin.x)
      return false;
   if (r2.origin.y >= r1.origin.y + r1.size.height)
      return false;
   if (r2.origin.y + r2.size.height <= r1.origin.y)
      return false;
   
   return true;
}

}//X11
}//MacOSX
}//ROOT

namespace Quartz = ROOT::Quartz;
namespace Util = ROOT::MacOSX::Util;
namespace X11 = ROOT::MacOSX::X11;

#ifdef DEBUG_ROOT_COCOA

namespace {

//______________________________________________________________________________
void log_attributes(const SetWindowAttributes_t *attr, unsigned winID)
{
   //This function is loggin requests, at the moment I can not set all
   //of these attributes, so I first have to check, what is actually
   //requested by ROOT.
   static std::ofstream logfile("win_attr.txt");

   const Mask_t mask = attr->fMask;   
   if (mask & kWABackPixmap)
      logfile<<"win "<<winID<<": BackPixmap\n";
   if (mask & kWABackPixel)
      logfile<<"win "<<winID<<": BackPixel\n";
   if (mask & kWABorderPixmap)
      logfile<<"win "<<winID<<": BorderPixmap\n";
   if (mask & kWABorderPixel)
      logfile<<"win "<<winID<<": BorderPixel\n";
   if (mask & kWABorderWidth)
      logfile<<"win "<<winID<<": BorderWidth\n";
   if (mask & kWABitGravity)
      logfile<<"win "<<winID<<": BitGravity\n";
   if (mask & kWAWinGravity)
      logfile<<"win "<<winID<<": WinGravity\n";
   if (mask & kWABackingStore)
      logfile<<"win "<<winID<<": BackingStore\n";
   if (mask & kWABackingPlanes)
      logfile<<"win "<<winID<<": BackingPlanes\n";
   if (mask & kWABackingPixel)
      logfile<<"win "<<winID<<": BackingPixel\n";
   if (mask & kWAOverrideRedirect)
      logfile<<"win "<<winID<<": OverrideRedirect\n";
   if (mask & kWASaveUnder)
      logfile<<"win "<<winID<<": SaveUnder\n";
   if (mask & kWAEventMask)
      logfile<<"win "<<winID<<": EventMask\n";
   if (mask & kWADontPropagate)
      logfile<<"win "<<winID<<": DontPropagate\n";
   if (mask & kWAColormap)
      logfile<<"win "<<winID<<": Colormap\n";
   if (mask & kWACursor)
      logfile<<"win "<<winID<<": Cursor\n";
}

//______________________________________________________________________________
void print_mask_info(ULong_t mask)
{
   if (mask & kButtonPressMask)
      NSLog(@"button press mask");
   if (mask & kButtonReleaseMask)
      NSLog(@"button release mask");
   if (mask & kExposureMask)
      NSLog(@"exposure mask");
   if (mask & kPointerMotionMask)
      NSLog(@"pointer motion mask");
   if (mask & kButtonMotionMask)
      NSLog(@"button motion mask");
   if (mask & kEnterWindowMask)
      NSLog(@"enter notify mask");
   if (mask & kLeaveWindowMask)
      NSLog(@"leave notify mask");
}

}
#endif


@implementation QuartzWindow {
@private
   QuartzView *fContentView;
   BOOL fDelayedTransient;
   QuartzImage *fShapeCombineMask;
}

@synthesize fMainWindow;

//QuartzWindow's life cycle.
//______________________________________________________________________________
- (id) initWithContentRect : (NSRect) contentRect styleMask : (NSUInteger) windowStyle backing : (NSBackingStoreType) bufferingType 
       defer : (BOOL) deferCreation  windowAttributes : (const SetWindowAttributes_t *)attr
{
   self = [super initWithContentRect : contentRect styleMask : windowStyle backing : bufferingType defer : deferCreation];

   if (self) {
      //ROOT's not able to draw GUI concurrently, thanks to global variables and gVirtualX itself.
      [self setAllowsConcurrentViewDrawing : NO];

      self.delegate = self;
      //create content view here.
      NSRect contentViewRect = contentRect;
      contentViewRect.origin.x = 0.f;
      contentViewRect.origin.y = 0.f;

      //TODO: OpenGL view can not be content of our QuartzWindow, check if
      //this is a problem for ROOT.
      fContentView = [[QuartzView alloc] initWithFrame : contentViewRect windowAttributes : 0];
      
      [self setContentView : fContentView];

      [fContentView release];
      fDelayedTransient = NO;
      
      if (attr)
         X11::SetWindowAttributes(attr, self);
   }
   
   return self;
}

//______________________________________________________________________________
- (void) dealloc
{
   [fShapeCombineMask release];
   [super dealloc];
}

//I want to forward a lot of property setters/getters to content view.

//______________________________________________________________________________
- (void) forwardInvocation : (NSInvocation *) anInvocation
{
   assert(fContentView != nil && "forwardInvocation:, fContentView is nil");

   if ([fContentView respondsToSelector : [anInvocation selector]]) {
      [anInvocation invokeWithTarget : fContentView];
   } else {
      [super forwardInvocation : anInvocation];
   }
}

//______________________________________________________________________________
- (NSMethodSignature*) methodSignatureForSelector : (SEL) selector
{
   NSMethodSignature *signature = [super methodSignatureForSelector : selector];

   if (!signature) {
      assert(fContentView != nil && "methodSignatureForSelector:, fContentView is nil");
      signature = [fContentView methodSignatureForSelector : selector];
   }

   return signature;
}

//______________________________________________________________________________
- (void) addTransientWindow : (QuartzWindow *) window
{
   assert(window != nil && "addTransientWindow, window parameter is nil");

   window.fMainWindow = self;
   
   if (window.fMapState != kIsViewable) {
      window.fDelayedTransient = YES;
   } else {
      [self addChildWindow : window ordered : NSWindowAbove];
      window.fDelayedTransient = NO;
   }
}

//______________________________________________________________________________
- (void) makeKeyAndOrderFront:(id)sender
{
   (void) sender;
   //The more I know Cocoa, the less I like it.
   //Window behavior between spaces is a total mess.
   //Set the window to join all spaces.
   [self setCollectionBehavior : NSWindowCollectionBehaviorCanJoinAllSpaces];
   //now bring it to the front, it will appear on the active space.
   [super makeKeyAndOrderFront : self];
   //then reset the collection behavior to default, so the window
   [self setCollectionBehavior : NSWindowCollectionBehaviorDefault];
}

//______________________________________________________________________________
- (void) setFDelayedTransient : (BOOL) d
{
   fDelayedTransient = d;
}

//______________________________________________________________________________
- (QuartzImage *) fShapeCombineMask
{
   return fShapeCombineMask;
}

//______________________________________________________________________________
- (void) setFShapeCombineMask : (QuartzImage *) mask
{
   if (mask != fShapeCombineMask) {
      [fShapeCombineMask release];
      if (mask) {
         fShapeCombineMask = [mask retain];
         
         //Check window's shadow???
      }
   }
}

///////////////////////////////////////////////////////////
//X11Drawable's protocol.

//______________________________________________________________________________
- (BOOL) fIsPixmap
{
   return NO;
}

//______________________________________________________________________________
- (BOOL) fIsOpenGLWidget
{
   return NO;
}

//______________________________________________________________________________
- (int) fX
{
   return self.frame.origin.x;
}

//______________________________________________________________________________
- (int) fY
{
   return X11::GlobalYCocoaToROOT(self.frame.origin.y + self.frame.size.height);
}

//______________________________________________________________________________
- (unsigned) fWidth
{
   return self.frame.size.width;
}

//______________________________________________________________________________
- (unsigned) fHeight
{
   //NSWindow's frame (height component) also includes title-bar.
   //So I have to use content view's height.
   //Obviously, there is a "hole" == 22 pixels.
   assert(fContentView != nil && "fHeight, content view is nil");
   
   return fContentView.frame.size.height;
}

//______________________________________________________________________________
- (void) setDrawableSize : (NSSize) newSize
{
   //Can not simply do self.frame.size = newSize.
   assert(!(newSize.width < 0) && "setDrawableSize, width is negative");
   assert(!(newSize.height < 0) && "setDrawableSize, height is negative");
   
   [self setContentSize : newSize];
}

//______________________________________________________________________________
- (void) setX : (int) x Y : (int) y width : (unsigned) w height : (unsigned) h
{
   NSSize newSize = {};
   newSize.width = w;
   newSize.height = h;
   [self setContentSize : newSize];
   
   //Check how this is affected by title bar's height.
   NSPoint topLeft = {};
   topLeft.x = x;
   topLeft.y = X11::GlobalYROOTToCocoa(y);

   [self setFrameTopLeftPoint : topLeft];
}

//______________________________________________________________________________
- (void) setX : (int) x Y : (int) y
{
   NSPoint topLeft = {};
   topLeft.x = x;
   topLeft.y = X11::GlobalYROOTToCocoa(y);

   [self setFrameTopLeftPoint : topLeft];
}

//______________________________________________________________________________
- (void) copy : (NSObject<X11Drawable> *) src area : (Rectangle_t) area withMask : (QuartzImage *)mask clipOrigin : (Point_t) clipXY toPoint : (Point_t) dstPoint
{
   assert(fContentView != nil && "copy:area:toPoint:, fContentView is nil");

   [fContentView copy : src area : area withMask : mask clipOrigin : clipXY toPoint : dstPoint];
}

//______________________________________________________________________________
- (unsigned char *) readColorBits : (Rectangle_t) area
{
   assert(fContentView != nil && "readColorBits:, fContentView is nil");
   
   return [fContentView readColorBits : area];
}

//X11Window protocol's implementation.

//______________________________________________________________________________
- (QuartzView *) fParentView
{
   return nil;
}

//______________________________________________________________________________
- (void) setFParentView : (QuartzView *) parent
{
   (void)parent;
}

//______________________________________________________________________________
- (NSView<X11Window> *) fContentView
{
   return fContentView;
}

//______________________________________________________________________________
- (QuartzWindow *) fQuartzWindow
{
   return self;
}

/////////////////////////////////////////////////////////////
//SetWindowAttributes_t/WindowAttributes_t.

//... forwards to fContentView.


//______________________________________________________________________________
- (int) fMapState
{
   //Top-level window can be only kIsViewable or kIsUnmapped (not unviewable).
   assert(fContentView != nil && "fMapState, content view is nil");
   
   if ([fContentView isHidden])
      return kIsUnmapped;
      
   return kIsViewable;
}

//______________________________________________________________________________
- (void) addChild : (NSView<X11Window> *) child
{
   assert(child != nil && "addChild, child view is nil");
 
   if (!fContentView) {
      //This can happen only in case of re-parent operation.
      assert([child isKindOfClass : [QuartzView class]] && "addChild: gl view in a top-level window as content view is not supported");
      fContentView = (QuartzView *)child;
      [self setContentView : child];
      fContentView.fParentView = nil;
   } else
      [fContentView addChild : child];
}

//______________________________________________________________________________
- (void) getAttributes : (WindowAttributes_t *) attr
{
   assert(fContentView != 0 && "getAttributes, content view is nil");
   assert(attr && "getAttributes, attr parameter is nil");

   X11::GetWindowAttributes(self, attr);
}

//______________________________________________________________________________
- (void) setAttributes : (const SetWindowAttributes_t *)attr
{
   assert(attr != 0 && "setAttributes, attr parameter is null");

#ifdef DEBUG_ROOT_COCOA
   log_attributes(attr, self.fID);
#endif

   X11::SetWindowAttributes(attr, self);
}

//______________________________________________________________________________
- (void) mapRaised
{
   assert(fContentView && "mapRaised, content view is nil");

   [self makeKeyAndOrderFront : self];
   [fContentView setHidden : NO];
   [fContentView configureNotifyTree];

   if (fDelayedTransient) {
      fDelayedTransient = NO;
      [fMainWindow addChildWindow : self ordered : NSWindowAbove];
   }
}

//______________________________________________________________________________
- (void) mapWindow
{
   assert(fContentView != nil && "mapWindow, content view is nil");

//   [self orderFront : self];
   [self makeKeyAndOrderFront : self];
   [fContentView setHidden : NO];
   [fContentView configureNotifyTree];
   
   if (fDelayedTransient) {
      fDelayedTransient = NO;
      [fMainWindow addChildWindow : self ordered : NSWindowAbove];
   }
}

//______________________________________________________________________________
- (void) mapSubwindows
{
   assert(fContentView != nil && "mapSubwindows, content view is nil");

   [fContentView mapSubwindows];
   [fContentView configureNotifyTree];
}

//______________________________________________________________________________
- (void) unmapWindow
{
   assert(fContentView != nil && "unmapWindow, content view is nil");

   [fContentView setHidden : YES];
   [self orderOut : self];
   
   if (fMainWindow && !fDelayedTransient) {
      [fMainWindow removeChildWindow : self];
      fMainWindow = nil;
   }
}

//Events.

//______________________________________________________________________________
- (void) sendEvent : (NSEvent *) theEvent
{
   assert(fContentView != nil && "sendEvent, content view is nil");

   if (theEvent.type == NSLeftMouseDown || theEvent.type == NSRightMouseDown) {
      bool generateFakeRelease = false;
      
      const NSPoint windowPoint = [theEvent locationInWindow];
      
      if (windowPoint.x <= 4 || windowPoint.x >= self.fWidth - 4)
         generateFakeRelease = true;
         
      if (windowPoint.y <= 4 || windowPoint.y >= self.fHeight - 4)
         generateFakeRelease = true;
         
      const NSPoint viewPoint =  [fContentView convertPointFromBase : windowPoint];

      if (viewPoint.y <= 0 && windowPoint.y >= 0)
         generateFakeRelease = true;

      TGCocoa * const vx = dynamic_cast<TGCocoa *>(gVirtualX);
      assert(vx != 0 && "sendEvent, gVirtualX is either null or not of TGCocoa type");
      if (vx->GetEventTranslator()->HasPointerGrab() && generateFakeRelease) {
         vx->GetEventTranslator()->GenerateButtonReleaseEvent(fContentView, theEvent, theEvent.type == NSLeftMouseDown ? kButton1 : kButton3);//yes, button release???

         //Yes, ignore this event completely (this means, you are not able to immediately start
         //resizing a window, if some popup is open. Actually, this is more or less
         //the same as with X11.app and X11 version.
         return;
      }
   }

   //Always call default version.
   [super sendEvent : theEvent];
}

//NSWindowDelegate's methods.

//______________________________________________________________________________
- (BOOL) windowShouldClose : (id) sender
{
   (void)sender;

   assert(fContentView != nil && "windowShouldClose, content view is nil");

   //TODO: check this!!! Children are
   //transient windows and ROOT does not handle
   //such a deletion properly, noop then.
   if ([[self childWindows] count])
      return NO;

   //Prepare client message for a window.
   Event_t closeEvent = {};
   closeEvent.fWindow = fContentView.fID;
   closeEvent.fType = kClientMessage;
   closeEvent.fFormat = 32;//Taken from GUI classes.
   closeEvent.fHandle = TGCocoa::fgDeleteWindowAtom;
   closeEvent.fUser[0] = TGCocoa::fgDeleteWindowAtom;
   //Place it into the queue.
   TGCocoa * const vx = dynamic_cast<TGCocoa *>(gVirtualX);
   assert(vx != 0 && "windowShouldClose, gVirtualX is either null or has a type different from TGCocoa");
   vx->SendEvent(fContentView.fID, &closeEvent);

   //Do not let AppKit to close a window.
   return NO;
}

@end

//
//
//
//
//Passive key grab info.

@implementation PassiveKeyGrab {
   unichar fKeyCode;
   NSUInteger fModifiers;
}

//______________________________________________________________________________
- (id) initWithKey : (unichar) keyCode modifiers : (NSUInteger) modifiers
{
   if (self = [super init]) {
      fKeyCode = keyCode;
      fModifiers = modifiers;
   }
   
   return self;
}

//______________________________________________________________________________
- (BOOL) matchKey : (unichar) keyCode modifiers : (NSUInteger) modifiers
{
   return keyCode == fKeyCode && modifiers == fModifiers;
}

//______________________________________________________________________________
- (BOOL) matchKey : (unichar) keyCode
{
   return keyCode == fKeyCode;
}

//______________________________________________________________________________
- (unichar) fKeyCode 
{
   return fKeyCode;
}

//______________________________________________________________________________
- (NSUInteger) fModifiers
{
   return fModifiers;
}

@end

//
//
//X11 "property".
@interface QuartzWindowProperty : NSObject {
   NSData *fPropertyData;
   Atom_t fType;
   unsigned fFormat;
}

@property (nonatomic, readonly) Atom_t fType;

@end

@implementation QuartzWindowProperty

@synthesize fType;

//______________________________________________________________________________
- (id) initWithData : (unsigned char *) data size : (unsigned) dataSize type : (Atom_t) type format : (unsigned) format
{
   if (self = [super init]) {
      //Memory is zero-initialized, but just to make it explicit:
      fPropertyData = nil;
      fType = 0;
      fFormat = 0;

      [self resetPropertyData : data size : dataSize type : type format : format];
   }

   return self;
}

//______________________________________________________________________________
- (void) dealloc
{
   [fPropertyData release];

   [super dealloc];
}

//______________________________________________________________________________
- (void) resetPropertyData : (unsigned char *) data size : (unsigned) dataSize type : (Atom_t) type format : (unsigned) format
{
   [fPropertyData release];
   
   fFormat = format;
   if (format == 16)
      dataSize *= 2;
   else if (format == 32)
      dataSize *= 4;

   fPropertyData = [[NSData dataWithBytes : data length : dataSize] retain];
   
   fType = type;
}

//______________________________________________________________________________
- (NSData *) fPropertyData
{
   return fPropertyData;
}

//______________________________________________________________________________
- (unsigned) fFormat
{
   return fFormat;
}

@end


@implementation QuartzView {
   QuartzPixmap   *fBackBuffer;
   NSMutableArray *fPassiveKeyGrabs;
   BOOL            fIsOverlapped;
   QuartzImage    *fClipMask;
   
   NSMutableDictionary   *fX11Properties;
   QuartzImage           *fBackgroundPixmap;
   
   X11::PointerGrab fCurrentGrabType;
   unsigned         fActiveGrabEventMask;
   BOOL             fActiveGrabOwnerEvents;
}

@synthesize fClipMaskIsValid;

@synthesize fID;
@synthesize fContext;
/////////////////////
//SetWindowAttributes_t/WindowAttributes_t
@synthesize fEventMask;
@synthesize fClass;
@synthesize fDepth;
@synthesize fBitGravity;
@synthesize fWinGravity;
@synthesize fBackgroundPixel;
//SetWindowAttributes_t/WindowAttributes_t
/////////////////////
@synthesize fParentView;

@synthesize fPassiveGrabButton;
@synthesize fPassiveGrabEventMask;
@synthesize fPassiveGrabKeyModifiers;
@synthesize fActiveGrabEventMask;
@synthesize fPassiveGrabOwnerEvents;
@synthesize fSnapshotDraw;
@synthesize fCurrentCursor;
@synthesize fIsDNDAware;

//______________________________________________________________________________
- (id) initWithFrame : (NSRect) frame windowAttributes : (const SetWindowAttributes_t *)attr
{
   if (self = [super initWithFrame : frame]) {
      //Make this explicit (though memory is zero initialized).
      fBackBuffer = nil;
      fClipMaskIsValid = NO;
      fClipMask = nil;

      fID = 0;
      
      //Passive grab parameters.
      fPassiveGrabButton = -1;//0 is kAnyButton.
      fPassiveGrabEventMask = 0;
      fPassiveGrabOwnerEvents = NO;
      
      fPassiveKeyGrabs = [[NSMutableArray alloc] init];
      
      [self setCanDrawConcurrently : NO];
      
      [self setHidden : YES];
      //Actually, check if view need this.      
      //
      if (attr)
         X11::SetWindowAttributes(attr, self);
         
      fCurrentCursor = kPointer;
      fX11Properties = [[NSMutableDictionary alloc] init];
      
      fCurrentGrabType = X11::kPGNoGrab;
      fActiveGrabEventMask = 0;
      fActiveGrabOwnerEvents = YES;
   }
   
   return self;
}

//______________________________________________________________________________
- (void) dealloc
{
   [fBackBuffer release];
   [fPassiveKeyGrabs release];
   [fClipMask release];
   [fX11Properties release];
   [fBackgroundPixmap release];
   [super dealloc];
}

//______________________________________________________________________________
- (void)updateTrackingAreas
{
   if (!fID)
      return;

   if (NSIsEmptyRect([self visibleRect]))
      return;

   
   const Util::AutoreleasePool pool;

   if (NSArray *trackingArray = [self trackingAreas]) {
      const NSUInteger size = [trackingArray count];
      for (NSUInteger i = 0; i < size; ++i) {
         NSTrackingArea * const t = [trackingArray objectAtIndex : i];
         [self removeTrackingArea : t];
      }
   }
   
   const NSUInteger trackerOptions = NSTrackingMouseMoved | NSTrackingMouseEnteredAndExited | NSTrackingActiveInActiveApp | NSTrackingInVisibleRect
                                    | NSTrackingEnabledDuringMouseDrag;
   NSRect frame = {};
   frame.size.width = self.fWidth;
   frame.size.height = self.fHeight;
   
   NSTrackingArea * const tracker = [[NSTrackingArea alloc] initWithRect : frame options : trackerOptions owner : self userInfo : nil];
   [self addTrackingArea : tracker];
   [tracker release];
}

//Overlap management.
//______________________________________________________________________________
- (BOOL) initClipMask
{
   const NSSize size = self.frame.size;

   if (fClipMask) {
      if ((unsigned)size.width == fClipMask.fWidth && (unsigned)size.height == fClipMask.fHeight) {
         //All pixels must be visible.
         [fClipMask clearMask];
      } else {
         [fClipMask release];
         fClipMask = nil;
      }
   }
   
   if (!fClipMask)
      fClipMask = [[QuartzImage alloc] initMaskWithW : (unsigned)size.width H : (unsigned)size.height];

   return fClipMask != nil;//BOOL is char, no pointer conversion.
}

//______________________________________________________________________________
- (QuartzImage *) fClipMask
{
   return fClipMask;
}

//______________________________________________________________________________
- (void) addOverlap : (NSRect)overlapRect
{
   assert(fClipMask != nil && "addOverlap, fClipMask is nil");
   assert(fClipMaskIsValid == YES && "addOverlap, fClipMask is invalid");
   
   [fClipMask maskOutPixels : overlapRect];
}

//X11Drawable protocol.
//______________________________________________________________________________
- (BOOL) fIsPixmap
{
   return NO;
}

//______________________________________________________________________________
- (BOOL) fIsOpenGLWidget
{
   return NO;
}

//______________________________________________________________________________
- (int) fX
{
   return self.frame.origin.x;
}

//______________________________________________________________________________
- (int) fY
{
   return self.frame.origin.y;
}

//______________________________________________________________________________
- (unsigned) fWidth
{
   return self.frame.size.width;
}

//______________________________________________________________________________
- (unsigned) fHeight
{
   return self.frame.size.height;
}

//______________________________________________________________________________
- (void) setDrawableSize : (NSSize) newSize
{
   assert(!(newSize.width < 0) && "setDrawableSize, width is negative");
   assert(!(newSize.height < 0) && "setDrawableSize, height is negative");
   
   //This will cause redraw(?)
   
   //In X11, resize changes the size, but upper-left corner is not changed.
   //In Cocoa, bottom-left is fixed.
   NSRect frame = self.frame;
   frame.size = newSize;
   
   self.frame = frame;
}

//______________________________________________________________________________
- (void) setX : (int) x Y : (int) y width : (unsigned) w height : (unsigned) h
{
   NSRect newFrame = {};
   newFrame.origin.x = x;
   newFrame.origin.y = y;
   newFrame.size.width = w;
   newFrame.size.height = h;
   
   self.frame = newFrame;
}

//______________________________________________________________________________
- (void) setX : (int) x Y : (int) y
{
   NSRect newFrame = self.frame;
   newFrame.origin.x = x;
   newFrame.origin.y = y;
   
   self.frame = newFrame;
}

//______________________________________________________________________________
- (void) copyImage : (QuartzImage *) srcImage area : (Rectangle_t) area withMask : (QuartzImage *) mask clipOrigin : (Point_t) clipXY toPoint : (Point_t) dstPoint
{
   //Check parameters.
   assert(srcImage != nil && "copyImage:area:withMask:clipOrigin:toPoint:, srcImage parameter is nil");
   assert(srcImage.fImage != nil && "copyImage:area:withMask:clipOrigin:toPoint:, srcImage.fImage is nil");

   //Check self.
   assert(self.fContext != 0 && "copyImage:area:withMask:clipOrigin:toPoint:, self.fContext is null");
   
   //No RAII for subImage, since it can be really subimage or image itself and
   //in these cases there is no need to release image.
   CGImageRef subImage = 0;
   bool needSubImage = false;
   if (area.fX || area.fY || area.fWidth != srcImage.fWidth || area.fHeight != srcImage.fHeight) {
      needSubImage = true;
      subImage = X11::CreateSubImage(srcImage, area);
      if (!subImage) {
         NSLog(@"QuartzView: -copyImage:area:withMask:clipOrigin:toPoint:, subimage creation failed");
         return;
      }
   } else
      subImage = srcImage.fImage;

   //Save context state.
   const Quartz::CGStateGuard ctxGuard(self.fContext);

   //Scale and translate to undo isFlipped.
   CGContextTranslateCTM(self.fContext, 0., self.fHeight); 
   CGContextScaleCTM(self.fContext, 1., -1.);
   //Set clip mask on a context.
   
   if (mask) {
      assert(mask.fImage != nil && "copyImage:area:withMask:clipOrigin:toPoint:, mask.fImage is nil");
      assert(CGImageIsMask(mask.fImage) == true && "copyImage:area:withMask:clipOrigin:toPoint:, mask.fImage is not a mask");
      clipXY.fY = X11::LocalYROOTToCocoa(self, clipXY.fY + mask.fHeight);
      const CGRect clipRect = CGRectMake(clipXY.fX, clipXY.fY, mask.fWidth, mask.fHeight);
      CGContextClipToMask(self.fContext, clipRect, mask.fImage);
   }
   
   //Convert from X11 to Cocoa (as soon as we scaled y * -1).
   dstPoint.fY = X11::LocalYROOTToCocoa(self, dstPoint.fY + area.fHeight);
   const CGRect imageRect = CGRectMake(dstPoint.fX, dstPoint.fY, area.fWidth, area.fHeight);
   CGContextDrawImage(self.fContext, imageRect, subImage);

   if (needSubImage)
      CGImageRelease(subImage);
}

//______________________________________________________________________________
- (void) copyView : (QuartzView *) srcView area : (Rectangle_t) area toPoint : (Point_t) dstPoint
{
   //To copy one "window" to another "window", I have to ask source QuartzView to draw intself into
   //bitmap, and copy this bitmap into the destination view.

   assert(srcView != nil && "copyView:area:toPoint:, srcView parameter is nil");

   const NSRect frame = [srcView frame];   
   //imageRep is in autorelease pool now.
   NSBitmapImageRep * const imageRep = [srcView bitmapImageRepForCachingDisplayInRect : frame];
   if (!imageRep) {
      NSLog(@"QuartzView: -copyView:area:toPoint failed");
      return;
   }
   
   assert(srcView != nil && "copyView:area:toPoint:, srcView parameter is nil");
   assert(self.fContext != 0 && "copyView:area:toPoint, self.fContext is null");

   //It can happen, that src and self are the same.
   //cacheDisplayInRect calls drawRect with bitmap context 
   //(and this will reset self.fContext: I have to save/restore it.
   CGContextRef ctx = srcView.fContext;
   srcView.fSnapshotDraw = YES;
   [srcView cacheDisplayInRect : frame toBitmapImageRep : imageRep];
   srcView.fSnapshotDraw = NO;
   srcView.fContext = ctx;

   const CGRect subImageRect = CGRectMake(area.fX, area.fY, area.fWidth, area.fHeight);
   const Util::CFScopeGuard<CGImageRef> subImage(CGImageCreateWithImageInRect(imageRep.CGImage, subImageRect));

   if (!subImage.Get()) {
      NSLog(@"QuartzView: -copyView:area:toPoint, CGImageCreateWithImageInRect failed");
      return;
   }

   const Quartz::CGStateGuard ctxGuard(self.fContext);
   const CGRect imageRect = CGRectMake(dstPoint.fX, [self visibleRect].size.height - (dstPoint.fY + area.fHeight), area.fWidth, area.fHeight);

   CGContextTranslateCTM(self.fContext, 0., [self visibleRect].size.height); 
   CGContextScaleCTM(self.fContext, 1., -1.);

   CGContextDrawImage(self.fContext, imageRect, subImage.Get());
}

//______________________________________________________________________________
- (void) copyPixmap : (QuartzPixmap *) srcPixmap area : (Rectangle_t) area withMask : (QuartzImage *) mask clipOrigin : (Point_t) clipXY toPoint : (Point_t) dstPoint
{
   //Check parameters.  
   assert(srcPixmap != nil && "copyPixmap:area:withMask:clipOrigin:toPoint:, srcPixmap parameter is nil");
   
   //More difficult case: pixmap already contains reflected image.
   area.fY = X11::LocalYROOTToCocoa(srcPixmap, area.fY) - area.fHeight;
   
   if (!X11::AdjustCropArea(srcPixmap, area)) {
      NSLog(@"QuartzView: -copyPixmap:area:withMask:clipOrigin:toPoint, no intersection between pixmap rectangle and cropArea");
      return;
   }

   //Check self.
   assert(self.fContext != 0 && "copyPixmap:area:withMask:clipOrigin:toPoint:, self.fContext is null");
   
   const Util::CFScopeGuard<CGImageRef> imageFromPixmap([srcPixmap createImageFromPixmap : area]);
   assert(imageFromPixmap.Get() != 0 && "copyPixmap:area:withMask:clipOrigin:toPoint:, createImageFromPixmap failed");

   //Save context state.
   const Quartz::CGStateGuard ctxGuard(self.fContext);
   
   if (mask) {
      assert(mask.fImage != nil && "copyPixmap:area:withMask:clipOrigin:toPoint:, mask.fImage is nil");
      assert(CGImageIsMask(mask.fImage) == true && "copyPixmap:area:withMask:clipOrigin:toPoint:, mask.fImage is not a mask");

      const CGRect clipRect = CGRectMake(clipXY.fX, clipXY.fY, mask.fWidth, mask.fHeight);
      CGContextClipToMask(self.fContext, clipRect, mask.fImage);
   }
   
   const CGRect imageRect = CGRectMake(dstPoint.fX, dstPoint.fY, area.fWidth, area.fHeight);
   CGContextDrawImage(self.fContext, imageRect, imageFromPixmap.Get());
}


//______________________________________________________________________________
- (void) copyImage : (QuartzImage *) srcImage area : (Rectangle_t) area toPoint : (Point_t) dstPoint
{
   assert(srcImage != nil && "copyImage:area:toPoint:, srcImage parameter is nil");
   assert(srcImage.fImage != nil && "copyImage:area:toPoint:, srcImage.fImage is nil");
   assert(self.fContext != 0 && "copyImage:area:toPoint:, fContext is null");

   if (!X11::AdjustCropArea(srcImage, area)) {
      NSLog(@"QuartzView: -copyImage:area:toPoint, image and copy area do not intersect");
      return;
   }

   CGImageRef subImage = 0;
   bool needSubImage = false;
   if (area.fX || area.fY || area.fWidth != srcImage.fWidth || area.fHeight != srcImage.fHeight) {
      needSubImage = true;
      subImage = X11::CreateSubImage(srcImage, area);
      if (!subImage) {
         NSLog(@"QuartzView: -copyImage:area:toPoint:, subimage creation failed");
         return;
      }
   } else
      subImage = srcImage.fImage;

   const Quartz::CGStateGuard ctxGuard(self.fContext);

   CGContextTranslateCTM(self.fContext, 0., self.fHeight); 
   CGContextScaleCTM(self.fContext, 1., -1.);

   dstPoint.fY = X11::LocalYCocoaToROOT(self, dstPoint.fY + area.fHeight);
   const CGRect imageRect = CGRectMake(dstPoint.fX, dstPoint.fY, area.fWidth, area.fHeight);
   CGContextDrawImage(self.fContext, imageRect, subImage);
   
   if (needSubImage)
      CGImageRelease(subImage);
}

//______________________________________________________________________________
- (void) copy : (NSObject<X11Drawable> *) src area : (Rectangle_t) area withMask : (QuartzImage *)mask clipOrigin : (Point_t) clipXY toPoint : (Point_t) dstPoint
{
   assert(src != nil && "copy:area:withMask:clipOrigin:toPoint:, src parameter is nil");
   
   if ([src isKindOfClass : [QuartzWindow class]]) {
      //Forget about mask (can I have it???)
      QuartzWindow * const qw = (QuartzWindow *)src;
      //Will not work with OpenGL.
      [self copyView : (QuartzView *)qw.fContentView area : area toPoint : dstPoint];
   } else if ([src isKindOfClass : [QuartzView class]]) {
      //Forget about mask (can I have it???)
      [self copyView : (QuartzView *)src area : area toPoint : dstPoint];
   } else if ([src isKindOfClass : [QuartzPixmap class]]) {
      [self copyPixmap : (QuartzPixmap *)src area : area withMask : mask clipOrigin : clipXY toPoint : dstPoint];
   } else if ([src isKindOfClass : [QuartzImage class]]) {
      [self copyImage : (QuartzImage *)src area : area withMask : mask clipOrigin : clipXY toPoint : dstPoint];
   } else {
      assert(0 && "copy:area:withMask:clipOrigin:toPoint:, src is of unknown type");
   }
}

//______________________________________________________________________________
- (unsigned char *) readColorBits : (Rectangle_t) area
{
   //TODO: make the part, reading pixels
   //from NSBitmapImageRep not so lame.
   const NSRect visRect = [self visibleRect];
   Rectangle_t srcRect = {};
   srcRect.fX = visRect.origin.x;
   srcRect.fY = visRect.origin.y;
   srcRect.fWidth = visRect.size.width;
   srcRect.fHeight = visRect.size.height;
   
   if (!X11::AdjustCropArea(srcRect, area)) {
      NSLog(@"QuartzView: -readColorBits:, visible rect of view and copy area do not intersect");
      return 0;
   }

   NSBitmapImageRep * const imageRep = [self bitmapImageRepForCachingDisplayInRect : visRect];//imageRect is autoreleased.
   if (!imageRep) {
      NSLog(@"QuartzView: -readColorBits:, bitmapImageRepForCachingDisplayInRect failed");
      return 0;
   }
   
   CGContextRef ctx = self.fContext; //Save old context if any.
   [self cacheDisplayInRect : visRect toBitmapImageRep : imageRep];
   self.fContext = ctx; //Restore old context.
   //
   const unsigned char *srcData = [imageRep bitmapData];
   //We have a source data now. Let's allocate buffer for ROOT's GUI and convert source data.
   unsigned char *data = 0;
   
   try {
      data = new unsigned char[area.fWidth * area.fHeight * 4];//bgra?
   } catch (const std::bad_alloc &) {
      NSLog(@"QuartzView: -readColorBits:, memory allocation failed");
      return 0;
   }
   
   const NSInteger bitsPerPixel = [imageRep bitsPerPixel];
   //TODO: ohhh :(((
   assert(bitsPerPixel == 32 && "-readColorBits:, no alpha channel???");

   const NSInteger bytesPerRow = [imageRep bytesPerRow];
   const unsigned dataWidth = bytesPerRow / (bitsPerPixel / 8);//assume an octet :(
   
   unsigned char *dstPixel = data;
   const unsigned char *line = srcData + area.fY * dataWidth * 4;
   const unsigned char *srcPixel = line + area.fX * 4;
      
   for (UShort_t i = 0; i < area.fHeight; ++i) {
      for (UShort_t j = 0; j < area.fWidth; ++j, srcPixel += 4, dstPixel += 4) {
         dstPixel[0] = srcPixel[2];
         dstPixel[1] = srcPixel[1];
         dstPixel[2] = srcPixel[0];
         dstPixel[3] = srcPixel[3];
      }

      line += dataWidth * 4;
      srcPixel = line + area.fX * 4;
   }
   
   return data;
}


//______________________________________________________________________________
- (void) setFBackgroundPixmap : (QuartzImage *) pixmap
{
   if (fBackgroundPixmap != pixmap) {
      [fBackgroundPixmap release];
      if (pixmap)
         fBackgroundPixmap = [pixmap retain];
      else
         fBackgroundPixmap = nil;
   }
}

//______________________________________________________________________________
- (QuartzImage *) fBackgroundPixmap
{
   //I do not autorelease, screw this idiom!

   return fBackgroundPixmap;
}


//______________________________________________________________________________
- (int) fMapState
{
   if ([self isHidden])
      return kIsUnmapped;

   for (QuartzView *parent = fParentView; parent; parent = parent.fParentView) {
      if ([parent isHidden])
         return kIsUnviewable;
   }

   return kIsViewable;
}

//______________________________________________________________________________
- (QuartzPixmap *) fBackBuffer
{
   return fBackBuffer;//No autorelease, I know the object's lifetime myself.
}

//______________________________________________________________________________
- (void) setFBackBuffer : (QuartzPixmap *) backBuffer
{
   if (fBackBuffer != backBuffer) {
      [fBackBuffer release];
      
      if (backBuffer)
         fBackBuffer = [backBuffer retain];
      else
         fBackBuffer = nil;
   }
}

//______________________________________________________________________________
- (NSView<X11Window> *) fContentView
{
   return self;
}

//______________________________________________________________________________
- (QuartzWindow *) fQuartzWindow
{
   return (QuartzWindow *)[self window];
}

//______________________________________________________________________________
- (void) activatePassiveGrab
{
   fCurrentGrabType = X11::kPGPassiveGrab;
}

//______________________________________________________________________________
- (void) activateImplicitGrab
{
   fCurrentGrabType = X11::kPGImplicitGrab;
}

//______________________________________________________________________________
- (void) activateGrab : (unsigned) eventMask ownerEvents : (BOOL) ownerEvents
{
   fCurrentGrabType = X11::kPGActiveGrab;
   fActiveGrabEventMask = eventMask;
   fActiveGrabOwnerEvents = ownerEvents;
}

//______________________________________________________________________________
- (void) cancelGrab
{
   fCurrentGrabType = X11::kPGNoGrab;
   fActiveGrabEventMask = 0;
   fActiveGrabOwnerEvents = YES;
}

//______________________________________________________________________________
- (BOOL) acceptsCrossingEvents : (unsigned) eventMask
{
   bool accepts = fEventMask & eventMask;
   if (fCurrentGrabType == X11::kPGPassiveGrab)//In ROOT passive grabs are always owner_events == true.
      accepts = accepts || (fPassiveGrabEventMask & eventMask);

   if (fCurrentGrabType == X11::kPGActiveGrab) {
      if (fActiveGrabOwnerEvents)
         accepts = accepts || (fActiveGrabOwnerEvents & eventMask);
      else
         accepts = fActiveGrabOwnerEvents & eventMask;
   }

   return accepts;
}

//______________________________________________________________________________
- (void) addChild : (NSView<X11Window> *) child
{
   assert(child != nil && "addChild, child view is nil");

   [self addSubview : child];
   child.fParentView = self;
}

//______________________________________________________________________________
- (void) getAttributes : (WindowAttributes_t *)attr
{
   assert(attr != 0 && "getAttributes, attr parameter is null");
   
   X11::GetWindowAttributes(self, attr);
}

//______________________________________________________________________________
- (void) setAttributes : (const SetWindowAttributes_t *)attr
{
   assert(attr != 0 && "setAttributes, attr parameter is null");

#ifdef DEBUG_ROOT_COCOA
   log_attributes(attr, fID);
#endif

   X11::SetWindowAttributes(attr, self);
}

//______________________________________________________________________________
- (void) mapRaised
{
   //Move view to the top of subviews (in UIKit there is a special method).   
   QuartzView * const parent = fParentView;
   [self removeFromSuperview];
   [parent addSubview : self];
   [self setHidden : NO];
}

//______________________________________________________________________________
- (void) mapWindow
{   
   [self setHidden : NO];
}

//______________________________________________________________________________
- (void) mapSubwindows
{
   for (QuartzView * v in [self subviews]) {
      [v setHidden : NO]; 
      //[v mapSubwindows];
   }
}

//______________________________________________________________________________
- (void) unmapWindow
{
   [self setHidden : YES];
}

//______________________________________________________________________________
- (BOOL) fIsOverlapped
{
   return fIsOverlapped;
}

//______________________________________________________________________________
- (void) setOverlapped : (BOOL) overlap
{
   fIsOverlapped = overlap;
   for (NSView<X11Window> *child in [self subviews])
      [child setOverlapped : overlap];
}

//______________________________________________________________________________
- (void) raiseWindow
{
   using namespace X11;//Comparators.

   for (QuartzView *sibling in [fParentView subviews]) {
      if (self == sibling)
         continue;
      if ([sibling isHidden])
         continue;
      //TODO: equal test is not good :) I have a baaad feeling about this ;)
      if (CGRectEqualToRect(sibling.frame, self.frame)) {
         [sibling setOverlapped : YES];
         //
         [sibling setHidden : YES];
         //
      }
   }

   [self setOverlapped : NO];
   //
   [self setHidden : NO];
   //
   [fParentView sortSubviewsUsingFunction : CompareViewsToRaise context : (void *)self];
   [self setNeedsDisplay : YES];//?
}

//______________________________________________________________________________
- (void) lowerWindow
{
   using namespace X11;

   NSEnumerator * const reverseEnumerator = [[fParentView subviews] reverseObjectEnumerator];
   for (QuartzView *sibling in reverseEnumerator) {
      if (sibling == self)
         continue;

      //TODO: equal test is not good :) I have a baaad feeling about this ;)
      if (CGRectEqualToRect(sibling.frame, self.frame)) {
         [sibling setOverlapped : NO];
         //
         [sibling setHidden : NO];
         //
         [sibling setNeedsDisplay : YES];
         [self setOverlapped : YES];
         //
         [self setHidden : YES];
         //
         break;
      }
   }
   
   [fParentView sortSubviewsUsingFunction : CompareViewsToLower context : (void*)self];//NO ARC! No __bridge!
}

//______________________________________________________________________________
- (BOOL) isFlipped
{
   //Now view's placement, geometry, moving and resizing can be
   //done with ROOT's (X11) coordinates without conversion.
   return YES;
}

//______________________________________________________________________________
- (void) configureNotifyTree
{
   if (self.fMapState == kIsViewable || fIsOverlapped == YES) {
      if (fEventMask & kStructureNotifyMask) {
         TGCocoa * const vx = dynamic_cast<TGCocoa *>(gVirtualX);
         assert(vx && "configureNotifyTree, gVirtualX is either null or has type different from TGCocoa");
         vx->GetEventTranslator()->GenerateConfigureNotifyEvent(self, self.frame);
      }

      for (NSView<X11Window> *v in [self subviews])
         [v configureNotifyTree];
   }
}

//Key grabs.
//______________________________________________________________________________
- (void) addPassiveKeyGrab : (unichar) keyCode modifiers : (NSUInteger) modifiers
{
   //Remove and add (not to traverse twice).
   [self removePassiveKeyGrab : keyCode modifiers : modifiers];
   PassiveKeyGrab * const newGrab = [[PassiveKeyGrab alloc] initWithKey : keyCode modifiers : modifiers];
   [fPassiveKeyGrabs addObject : newGrab];
   [newGrab release];
}

//______________________________________________________________________________
- (void) removePassiveKeyGrab : (unichar) keyCode modifiers : (NSUInteger) modifiers
{
   const NSUInteger count = [fPassiveKeyGrabs count];
   for (NSUInteger i = 0; i < count; ++i) {
      PassiveKeyGrab *grab = [fPassiveKeyGrabs objectAtIndex : i];
      if ([grab matchKey : keyCode modifiers : modifiers]) {
         [fPassiveKeyGrabs removeObjectAtIndex : i];
         break;
      }
   }
}

//______________________________________________________________________________
- (PassiveKeyGrab *) findPassiveKeyGrab : (unichar) keyCode modifiers : (NSUInteger) modifiers
{
   NSEnumerator * const enumerator = [fPassiveKeyGrabs objectEnumerator];
   while (PassiveKeyGrab *grab = (PassiveKeyGrab *)[enumerator nextObject]) {
      if ([grab matchKey : keyCode modifiers : modifiers])
         return grab;
   }

   return nil;
}

//______________________________________________________________________________
- (PassiveKeyGrab *) findPassiveKeyGrab : (unichar) keyCode
{
   //Do not check modifiers.
   NSEnumerator * const enumerator = [fPassiveKeyGrabs objectEnumerator];
   while (PassiveKeyGrab *grab = (PassiveKeyGrab *)[enumerator nextObject]) {
      if ([grab matchKey : keyCode])
         return grab;
   }

   return nil;
}

//Painting mechanics.

//______________________________________________________________________________
- (void) drawRect : (NSRect) dirtyRect
{
   using namespace X11;

   (void)dirtyRect;

   if (fID) {
      if (TGWindow * const window = gClient->GetWindowById(fID)) {
         if (ViewIsTextViewFrame(self, true))//It's never painted, parent renders child. true == check the parent also.
            return;

         NSGraphicsContext * const nsContext = [NSGraphicsContext currentContext];
         assert(nsContext != nil && "drawRect, currentContext returned nil");

         TGCocoa * const vx = (TGCocoa *)gVirtualX;
         vx->CocoaDrawON();

         fContext = (CGContextRef)[nsContext graphicsPort];
         assert(fContext != 0 && "drawRect, graphicsPort returned null");

         const Quartz::CGStateGuard ctxGuard(fContext);

         Util::CFScopeGuard<CGImageRef> clipImageGuard;
         QuartzWindow * const topLevelParent = self.fQuartzWindow;
         if (topLevelParent.fShapeCombineMask) {
            //Attach clip mask to the context.
            const NSRect clipRect  = [self convertRect : [self visibleRect] toView : nil];
            clipImageGuard.Reset(CGImageCreateWithImageInRect(topLevelParent.fShapeCombineMask.fImage, clipRect));
            //TODO: this geometry looks suspicious, check!
            CGContextClipToMask(fContext, CGRectMake(0, 0, clipRect.size.width, clipRect.size.height), clipImageGuard.Get());
         }


         if (window->InheritsFrom("TGContainer"))//It always has an ExposureMask.
            vx->GetEventTranslator()->GenerateExposeEvent(self, [self visibleRect]);

         if (fEventMask & kExposureMask) {
            if (ViewIsTextView(self)) {
               //Send Expose event, using child view (this is how it's done in GUI :( ).
               NSView<X11Window> * const viewFrame = FrameForTextView(self);
               if (viewFrame)
                  vx->GetEventTranslator()->GenerateExposeEvent(viewFrame, [viewFrame visibleRect]);//Now we set fExposedRegion for TGView.
            }

            //Ask ROOT's widget/window to draw itself.
            gClient->NeedRedraw(window, kTRUE);
            
            if (!fSnapshotDraw && !ViewIsTextView(self)) {
               //If Cocoa repaints widget, cancel all ROOT's "outside of paint event"
               //rendering into this widget ... Except it's a text view :)
               gClient->CancelRedraw(window);
               vx->GetCommandBuffer()->RemoveGraphicsOperationsForWindow(fID);
            }
         }

         if (fBackBuffer) {
            //Very "special" window.
            CGImageRef image = [fBackBuffer createImageFromPixmap];// CGBitmapContextCreateImage(fBackBuffer.fContext);
            if (image) {
               const CGRect imageRect = CGRectMake(0, 0, fBackBuffer.fWidth, fBackBuffer.fHeight);
               CGContextDrawImage(fContext, imageRect, image);
               CGImageRelease(image);
            }
         }
     
         vx->CocoaDrawOFF();
#ifdef DEBUG_ROOT_COCOA
         CGContextSetRGBStrokeColor(fContext, 1., 0., 0., 1.);
         CGContextStrokeRect(fContext, dirtyRect);
#endif

         fContext = 0;         
      } else {
#ifdef DEBUG_ROOT_COCOA
         NSLog(@"QuartzView: -drawRect method, no window for id %u was found", fID);
#endif
      }
   }
}

//Event handling.

//______________________________________________________________________________
- (void) setFrame : (NSRect) newFrame
{
   //In case of TBrowser, setFrame started infinite recursion:
   //HandleConfigure for embedded main frame emits signal, slot
   //calls layout, layout calls setFrame -> HandleConfigure and etc. etc.
   if (CGRectEqualToRect(newFrame, self.frame))
      return;

   [super setFrame : newFrame];
}

//______________________________________________________________________________
- (void) setFrameSize : (NSSize) newSize
{
   //Check, if setFrameSize calls setFrame.
   
   [super setFrameSize : newSize];
   
   if ((fEventMask & kStructureNotifyMask) && (self.fMapState == kIsViewable || fIsOverlapped == YES)) {
      TGCocoa * const vx = dynamic_cast<TGCocoa *>(gVirtualX);
      assert(vx != 0 && "setFrameSize:, gVirtualX is either null or has a type, different from TGCocoa");
      vx->GetEventTranslator()->GenerateConfigureNotifyEvent(self, self.frame);
   }

   [self setNeedsDisplay : YES];//?
}

//______________________________________________________________________________
- (void) mouseDown : (NSEvent *) theEvent
{
   assert(fID != 0 && "mouseDown, fID is 0");

   TGCocoa * const vx = dynamic_cast<TGCocoa *>(gVirtualX);
   assert(vx != 0 && "mouseDown, gVirtualX is either null or has a type, different from TGCocoa");
   vx->GetEventTranslator()->GenerateButtonPressEvent(self, theEvent, kButton1);
}

//______________________________________________________________________________
- (void) scrollWheel : (NSEvent*) theEvent
{
   assert(fID != 0 && "scrollWheel, fID is 0");

   TGCocoa * const vx = dynamic_cast<TGCocoa *>(gVirtualX);
   assert(vx != 0 && "scrollWheel, gVirtualX is either null or has a type, different from TGCocoa");

   const CGFloat deltaY = [theEvent deltaY];
   if (deltaY < 0) {
      vx->GetEventTranslator()->GenerateButtonPressEvent(self, theEvent, kButton5);
      vx->GetEventTranslator()->GenerateButtonReleaseEvent(self, theEvent, kButton5);
   } else if (deltaY > 0) {
      vx->GetEventTranslator()->GenerateButtonPressEvent(self, theEvent, kButton4);
      vx->GetEventTranslator()->GenerateButtonReleaseEvent(self, theEvent, kButton4);
   }
}

#ifdef DEBUG_ROOT_COCOA
//______________________________________________________________________________
- (void) printViewInformation
{
   assert(fID != 0 && "printWindowInformation, fID is 0");
   const TGWindow * const window = gClient->GetWindowById(fID);
   assert(window != 0 && "printWindowInformation, window not found");

   NSLog(@"-----------------View %u info:---------------------", fID);
   NSLog(@"ROOT's window class is %s", window->IsA()->GetName());
   NSLog(@"event mask is:");
   print_mask_info(fEventMask);
   NSLog(@"grab mask is:");
   print_mask_info(fGrabButtonEventMask);
   NSLog(@"----------------End of view info------------------");
}
#endif

//______________________________________________________________________________
- (void) rightMouseDown : (NSEvent *) theEvent
{
   assert(fID != 0 && "rightMouseDown, fID is 0");

#ifdef DEBUG_ROOT_COCOA
   [self printViewInformation];
#endif

   TGCocoa * const vx = dynamic_cast<TGCocoa *>(gVirtualX);
   assert(vx != 0 && "rightMouseDown, gVirtualX is either null or has type different from TGCocoa");
   vx->GetEventTranslator()->GenerateButtonPressEvent(self, theEvent, kButton3);
}

//______________________________________________________________________________
- (void) mouseUp : (NSEvent *) theEvent
{
   assert(fID != 0 && "mouseUp, fID is 0");

   TGCocoa * const vx = dynamic_cast<TGCocoa *>(gVirtualX);
   assert(vx && "mouseUp, gVirtualX is either null or has type different from TGCocoa");
   vx->GetEventTranslator()->GenerateButtonReleaseEvent(self, theEvent, kButton1);
}

//______________________________________________________________________________
- (void) rightMouseUp : (NSEvent *) theEvent
{

   assert(fID != 0 && "rightMouseUp, fID is 0");

   TGCocoa * const vx = dynamic_cast<TGCocoa *>(gVirtualX);
   assert(vx != 0 && "rightMouseUp, gVirtualX is either null or has type different from TGCocoa");
   vx->GetEventTranslator()->GenerateButtonReleaseEvent(self, theEvent, kButton3);
}

//______________________________________________________________________________
- (void) mouseEntered : (NSEvent *) theEvent
{
   assert(fID != 0 && "mouseEntered, fID is 0");

   TGCocoa * const vx = dynamic_cast<TGCocoa *>(gVirtualX);
   assert(vx != 0 && "mouseEntered, gVirtualX is null or not of TGCocoa type");

   vx->GetEventTranslator()->GenerateCrossingEvent(theEvent);
}

//______________________________________________________________________________
- (void) mouseExited : (NSEvent *) theEvent
{
   assert(fID != 0 && "mouseExited, fID is 0");

   TGCocoa * const vx = dynamic_cast<TGCocoa *>(gVirtualX);
   assert(vx != 0 && "mouseExited, gVirtualX is null or not of TGCocoa type");

   vx->GetEventTranslator()->GenerateCrossingEvent(theEvent);
}

//______________________________________________________________________________
- (void) mouseMoved : (NSEvent *) theEvent
{
   assert(fID != 0 && "mouseMoved, fID is 0");
   
   if (fParentView)//Suppress events in all views, except the top-level one.
      return;      //TODO: check, that it does not create additional problems.

   TGCocoa *vx = dynamic_cast<TGCocoa *>(gVirtualX);
   assert(vx != 0 && "mouseMoved, gVirtualX is null or not of TGCocoa type");
   
   vx->GetEventTranslator()->GeneratePointerMotionEvent(theEvent);
}

//______________________________________________________________________________
- (void) mouseDragged : (NSEvent *)theEvent
{
   assert(fID != 0 && "mouseDragged, fID is 0");
   
   TGCocoa * const vx = dynamic_cast<TGCocoa *>(gVirtualX);
   assert(vx != 0 && "mouseMoved, gVirtualX is null or not of TGCocoa type");
   
   vx->GetEventTranslator()->GeneratePointerMotionEvent(theEvent);   
}

//______________________________________________________________________________
- (void) rightMouseDragged : (NSEvent *)theEvent
{
   assert(fID != 0 && "rightMouseDragged, fID is 0");

   TGCocoa * const vx = dynamic_cast<TGCocoa *>(gVirtualX);
   assert(vx != 0 && "rightMouseMoved, gVirtualX is null or not of TGCocoa type");
   
   vx->GetEventTranslator()->GeneratePointerMotionEvent(theEvent);
}

//______________________________________________________________________________
- (void) keyDown:(NSEvent *)theEvent
{
   assert(fID != 0 && "keyDown, fID is 0");
  
   TGCocoa * const vx = dynamic_cast<TGCocoa *>(gVirtualX);
   assert(vx != 0 && "keyDown, gVirtualX is null or not of TGCocoa type");
   vx->GetEventTranslator()->GenerateKeyPressEvent(self, theEvent);
}

//______________________________________________________________________________
- (void) keyUp:(NSEvent *)theEvent
{
   (void)theEvent;
}

//First responder staff.

//______________________________________________________________________________
- (BOOL) acceptsFirstMouse : (NSEvent *)theEvent
{
   (void)theEvent;
   return YES;
}

//______________________________________________________________________________
- (BOOL) acceptsFirstResponder
{
   return YES;
}

//______________________________________________________________________________
- (BOOL) becomeFirstResponder
{
   //Change focus.
   NSView<X11Window> *focusView = nil;
   for (NSView<X11Window> *view = self; view; view = view.fParentView) {
      if (view.fEventMask & kFocusChangeMask) {
         focusView = view;
         break;
      }
   }

   if (!focusView)
      focusView = ((QuartzWindow *)[self window]).fContentView;
   
   TGCocoa * const vx = dynamic_cast<TGCocoa *>(gVirtualX);
   assert(vx != 0 && "becomeFirstResponder, gVirtualX is null or not of TGCocoa type");
   vx->GetEventTranslator()->GenerateFocusChangeEvent(focusView);

   return YES;
}

//______________________________________________________________________________
- (BOOL) resignFirstResponder
{
   //Change focus.
   
   //NSResponder returns YES, so do I.
   return YES;
}


//Cursors.
//______________________________________________________________________________
- (void) setFCurrentCursor : (ECursor) cursor
{
   if (cursor != fCurrentCursor) {
      fCurrentCursor = cursor;
      [self.fQuartzWindow invalidateCursorRectsForView : self];
   }
}

//______________________________________________________________________________
- (NSCursor *) createCustomCursor 
{
   const char *pngFileName = 0;

   switch (fCurrentCursor) {
   case kMove:
      pngFileName = "move_cursor.png";
      break;
   case kArrowHor:
      pngFileName = "hor_arrow_cursor.png";
      break;
   case kArrowVer:
      pngFileName = "ver_arrow_cursor.png";
      break;
   case kArrowRight:
      pngFileName = "right_arrow_cursor.png";
      break;
   case kBottomLeft:
   case kTopRight:
      pngFileName = "top_right_cursor.png";
      break;
   case kTopLeft:
   case kBottomRight:
      pngFileName = "top_left_cursor.png";
      break;
   default:;
   }
   
   if (pngFileName) {
      const char * const path = gSystem->Which("$ROOTSYS/icons", pngFileName, kReadPermission);//This must be deleted.
      const Util::ScopedArray<const char> arrayGuard(path);

      if (!path || path[0] == 0) {
         //File was not found.
         return nil;
      }
      
      NSString *nsPath = [NSString stringWithFormat : @"%s", path];//in autorelease pool.
      NSImage * const cursorImage = [[NSImage alloc] initWithContentsOfFile : nsPath];//must call release.

      if (!cursorImage)
         return nil;

      NSPoint hotSpot = X11::GetCursorHotStop(cursorImage, fCurrentCursor);
      NSCursor * const customCursor = [[[NSCursor alloc] initWithImage : cursorImage hotSpot : hotSpot] autorelease];//in autorelease pool.
      
      [cursorImage release];
      
      return customCursor;
   }

   return nil;
}

//______________________________________________________________________________
- (void) resetCursorRects
{
   //Cursors from TVirtaulX:
   // kBottomLeft, kBottomRight, kTopLeft,  kTopRight,
   // kBottomSide, kLeftSide,    kTopSide,  kRightSide,
   // kMove,       kCross,       kArrowHor, kArrowVer,
   // kHand,       kRotate,      kPointer,  kArrowRight,
   // kCaret,      kWatch
   
   NSCursor *cursor = nil;
   
   switch (fCurrentCursor) {
   case kCross:
      cursor = [NSCursor crosshairCursor];
      break;
   case kPointer:
      //Use simple arrow (or this special cursor will be even on GUI widgets).
      break;
   case kHand:
      cursor = [NSCursor openHandCursor];
      break;
   case kLeftSide:
      cursor = [NSCursor resizeLeftCursor];
      break;
   case kRightSide:
      cursor = [NSCursor resizeRightCursor];
      break;
   case kTopSide:
      cursor = [NSCursor resizeUpCursor];
      break;
   case kBottomSide:
      cursor = [NSCursor resizeDownCursor];
      break;
   case kCaret:
      cursor = [NSCursor IBeamCursor];
      break;
   case kRotate:
   case kWatch:
   default:
      cursor = [self createCustomCursor];
   }
   
   if (cursor)
      [self addCursorRect : self.visibleRect cursor : cursor];
   else
      [super resetCursorRects];
}

//______________________________________________________________________________
- (void) setProperty : (const char *) propName data : (unsigned char *) propData size : (unsigned) dataSize forType : (Atom_t) dataType format : (unsigned) format
{
   assert(propName != 0 && "setProperty:data:size:forType:, propName parameter is null");
   assert(propData != 0 && "setProperty:data:size:forType:, propData parameter is null");
   assert(dataSize != 0 && "setProperty:data:size:forType:, dataSize parameter is 0");

   NSString * const key = [NSString stringWithCString : propName encoding : NSASCIIStringEncoding];
   QuartzWindowProperty * property = (QuartzWindowProperty *)[fX11Properties valueForKey : key];
   
   //At the moment (and I think this will never change) TGX11 always calls XChangeProperty with PropModeReplace.
   if (property)
      [property resetPropertyData : propData size : dataSize type : dataType format : format];
   else {
      //No property found, add a new one.
      property = [[QuartzWindowProperty alloc] initWithData : propData size : dataSize type : dataType format : format];
      [fX11Properties setObject : property forKey : key];
      [property release];
   }
}

//______________________________________________________________________________
- (BOOL) hasProperty : (const char *) propName
{
   assert(propName != 0 && "hasProperty, propName parameter is null");
   
   NSString * const key = [NSString stringWithCString : propName encoding : NSASCIIStringEncoding];
   QuartzWindowProperty * const property = (QuartzWindowProperty *)[fX11Properties valueForKey : key];

   return property != nil;
}

//______________________________________________________________________________
- (unsigned char *) getProperty : (const char *) propName returnType : (Atom_t *) type 
   returnFormat : (unsigned *) format nElements : (unsigned *) nElements
{
   assert(propName != 0 && "getProperty:returnType:returnFormat:nElements, propName parameter is null");
   assert(type != 0 && "getProperty:returnType:returnFormat:nElements, type parameter is null");
   assert(format != 0 && "getProperty:returnType:returnFormat:nElements, format parameter is null");
   assert(nElements != 0 && "getProperty:returnType:returnFormat:nElements, nElements parameter is null");

   NSString * const key = [NSString stringWithCString : propName encoding : NSASCIIStringEncoding];
   QuartzWindowProperty * const property = (QuartzWindowProperty *)[fX11Properties valueForKey : key];
   assert(property != 0 && "getProperty:returnType:returnFormat:nElements, property not found");

   NSData * const propData = property.fPropertyData;
   
   const NSUInteger dataSize = [propData length];
   unsigned char *buff = 0;
   try {
      buff = new unsigned char[dataSize]();
   } catch (const std::bad_alloc &) {
      NSLog(@"QuartzWindow: -getProperty:returnType:returnFormat:nElements, memory allocation failed");//Hmm, can I log, if new failed? :)
      return 0;
   }

   [propData getBytes : buff length : dataSize];
   *format = property.fFormat;
   
   *nElements = dataSize;
   
   if (*format == 16)
      *nElements= dataSize / 2;
   else if (*format == 32)
      *nElements = dataSize / 4;
      
   *type = property.fType;

   return buff;
}

//______________________________________________________________________________
- (void) removeProperty : (const char *) propName
{
   assert(propName != 0 && "removeProperty:, propName parameter is null");
   
   NSString * const key = [NSString stringWithCString : propName encoding : NSASCIIStringEncoding];
   [fX11Properties removeObjectForKey : key];
}

//DND
//______________________________________________________________________________
- (NSDragOperation) draggingEntered : (id<NSDraggingInfo>) sender
{
   NSPasteboard * const pasteBoard = [sender draggingPasteboard];
   const NSDragOperation sourceDragMask = [sender draggingSourceOperationMask];
   
   if ([[pasteBoard types] containsObject : NSFilenamesPboardType] && (sourceDragMask & NSDragOperationCopy))
      return NSDragOperationCopy;
      
   return NSDragOperationNone;
}

//______________________________________________________________________________
- (BOOL) performDragOperation : (id<NSDraggingInfo>) sender
{
   //We can drag some files (images, pdfs, source code files) from
   //finder to ROOT's window (mainly TCanvas or text editor).
   //The logic is totally screwed here :((( - ROOT will try to
   //read a property of some window (not 'self', unfortunately) -
   //this works since on Window all data is in a global clipboard
   //(on X11 it simply does not work at all).
   //I'm attaching the file name as a property for the top level window,
   //there is no other way to make this data accessible for ROOT.

   NSPasteboard * const pasteBoard = [sender draggingPasteboard];
   const NSDragOperation sourceDragMask = [sender draggingSourceOperationMask];

   if ([[pasteBoard types] containsObject : NSFilenamesPboardType] && (sourceDragMask & NSDragOperationCopy)) {

      //Here I try to put string ("file://....") into window's property to make
      //it accesible from ROOT's GUI.
      const Atom_t textUriAtom = gVirtualX->InternAtom("text/uri-list", kFALSE);

      NSArray * const files = [pasteBoard propertyListForType : NSFilenamesPboardType];
      for (NSString *path in files) {
         //ROOT can not process several files, use the first one.
         NSString * const item = [@"file://" stringByAppendingString : path];
         //Yes, only ASCII encoding, but after all, ROOT's not able to work with NON-ASCII strings.
         const NSUInteger len = [item lengthOfBytesUsingEncoding : NSASCIIStringEncoding] + 1;
         try {
            std::vector<unsigned char> propertyData(len);
            [item getCString : (char *)&propertyData[0] maxLength : propertyData.size() encoding : NSASCIIStringEncoding];
            //There is no any guarantee, that this will ever work, logic in TGDNDManager is totally crazy.
            NSView<X11Window> * const targetView = self.fQuartzWindow.fContentView;
            [targetView setProperty : "_XC_DND_DATA" data : &propertyData[0] size : propertyData.size() forType : textUriAtom format : 8];
         } catch (const std::bad_alloc &) {
            NSLog(@"QuartzView: -performDragOperation, memory allocation failed");//Hehe, can I log something in case of bad_alloc??? ;)
            return NO;
         }

         break;
      }
      
      //Property is attached now.

      //Gdk on windows creates three events on file drop (WM_DROPFILES): XdndEnter, XdndPosition, XdndDrop.
      //1. Dnd enter.
      Event_t event1 = {};
      event1.fType = kClientMessage;
      event1.fWindow = fID;
      event1.fHandle = gVirtualX->InternAtom("XdndEnter", kFALSE);
      event1.fUser[0] = long(fID);
      event1.fUser[2] = textUriAtom;//gVirtualX->InternAtom("text/uri-list", kFALSE);
      //
      gVirtualX->SendEvent(fID, &event1);

      //2. Dnd position.
      Event_t event2 = {};
      event2.fType = kClientMessage;
      event2.fWindow = fID;
      event2.fHandle = gVirtualX->InternAtom("XdndPosition", kFALSE);
      event2.fUser[0] = long(fID);
      event2.fUser[2] = 0;//Here I have to pack x and y for drop coordinates, shifting by 16 bits.
      NSPoint dropPoint = [sender draggingLocation];
      dropPoint = [self convertPointFromBase : dropPoint];
      dropPoint = X11::TranslateToScreen(self, dropPoint);
      event2.fUser[2] = UShort_t(dropPoint.y) | (UShort_t(dropPoint.x) << 16);
      
      gVirtualX->SendEvent(fID, &event2);

      Event_t event3 = {};
      event3.fType = kClientMessage;
      event3.fWindow = fID;
      event3.fHandle = gVirtualX->InternAtom("XdndDrop", kFALSE);

      gVirtualX->SendEvent(fID, &event3);
   }

   return YES;//Always ok, even if file type is not supported - no need in "animation".
}

@end
