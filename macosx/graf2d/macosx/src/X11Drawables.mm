#define DEBUG_ROOT_COCOA

#ifdef DEBUG_ROOT_COCOA
#import <fstream>
#endif

#import <cassert>

#import "X11Drawables.h"
#import "TGWindow.h"
#import "TGClient.h"
#import "TGCocoa.h"
#import "TClass.h"

namespace ROOT {
namespace MacOSX {
namespace X11 {

//______________________________________________________________________________
void GetRootWindowAttributes(WindowAttributes_t *attr)
{
   //'root' window does not exist, but we can request its attributes.
   assert(attr != nullptr && "GetRootWindowAttributes, attr parameter is null");
   
   NSArray *screens = [NSScreen screens];
   assert(screens != nil && "screens array is nil");
   
   NSScreen *mainScreen = [screens objectAtIndex : 0];
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
   NSArray *screens = [NSScreen screens];
   assert(screens != nil && "GlobalYCocoaToROOT, screens array is nil");
   
   NSScreen *mainScreen = [screens objectAtIndex : 0];
   assert(mainScreen != nil && "GlobalYCocoaToROOT, screen at index 0 is nil");
   
   return int(mainScreen.frame.size.height - yCocoa);
}

//______________________________________________________________________________
int GlobalYROOTToCocoa(CGFloat yROOT)
{
   //hehe :)) actually, no need in this function.
   NSArray *screens = [NSScreen screens];
   assert(screens != nil && "GlobalYROOTToCocoa, screens array is nil");
   
   NSScreen *mainScreen = [screens objectAtIndex : 0];
   assert(mainScreen != nil && "GlobalYROOTToCocoa, screen at index 0 is nil");
   
   return int(mainScreen.frame.size.height - yROOT);
}

//______________________________________________________________________________
int LocalYCocoaToROOT(QuartzView *parentView, CGFloat yCocoa)
{
   assert(parentView != nil && "LocalYCocoaToROOT, parent view is nil");
   
   return int(parentView.frame.size.height - yCocoa);
}

//______________________________________________________________________________
int LocalYROOTToCocoa(QuartzView *parentView, CGFloat yROOT)
{
   //:)
   assert(parentView != nil && "LocalYROOTToCocoa, parent view is nil");
   
   return int(parentView.frame.size.height - yROOT);
}


//______________________________________________________________________________
int LocalYROOTToCocoa(id<X11Drawable> drawable, CGFloat yROOT)
{
   //:)
   assert(drawable != nil && "LocalYROOTToCocoa, drawable is nil");
   
   return int(drawable.fHeight - yROOT);
}

//______________________________________________________________________________
void SetWindowAttributes(const SetWindowAttributes_t *attr, id<X11Drawable> window)
{
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
      //Unfortunately, as soon as this is Objective-C++, I 
      //can not do the following test:
      //if ([window isKindOfClass : [QuartzWindow : class]]) {
      if (!window.fParentView) {
         QuartzWindow *qw = (QuartzWindow *)window;
         [qw setStyleMask : NSBorderlessWindowMask];
      }
   }
}

//______________________________________________________________________________
void GetWindowGeometry(id<X11Drawable> win, WindowAttributes_t *dst)
{
   assert(win != nil && "GetWindowGeometry, win parameter is nil");
   assert(dst != nullptr && "GetWindowGeometry, dst paremeter is null");
   
   dst->fX = win.fX;
   dst->fY = win.fYROOT;
   
   dst->fWidth = win.fWidth;
   dst->fHeight = win.fHeight;
}

//______________________________________________________________________________
void GetWindowAttributes(id<X11Drawable> window, WindowAttributes_t *dst)
{
   assert(window != nil && "GetWindowAttributes, window parameter is nil");
   assert(dst != nullptr && "GetWindowAttributes, attr parameter is null");
   
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

}
}
}

#ifdef DEBUG_ROOT_COCOA

namespace {

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

}
#endif


@implementation QuartzWindow {
@private
   QuartzView *fContentView;
}

@synthesize fBackBuffer;
@synthesize fContext;


//RootQuartzWindow's life cycle.

//______________________________________________________________________________
- (id) initWithContentRect : (NSRect) contentRect styleMask : (NSUInteger) windowStyle backing : (NSBackingStoreType) bufferingType 
       defer : (BOOL) deferCreation  windowAttributes : (const SetWindowAttributes_t *)attr
{
   self = [super initWithContentRect : contentRect styleMask : windowStyle backing : bufferingType defer : deferCreation];

   if (self) {
      //ROOT's not able to draw GUI concurrently, thanks to global variables and gVirtualX itself.
      [self setAllowsConcurrentViewDrawing : NO];
   
      fContext = nullptr;
      //self.delegate = ...
      //create content view here.
      NSRect contentViewRect = contentRect;
      contentViewRect.origin.x = 0.f;
      contentViewRect.origin.y = 0.f;
      fContentView = [[QuartzView alloc] initWithFrame : contentViewRect windowAttributes : 0];
      
      fContentView.fResizedByROOT = YES;//If it will do any resize
      [self setContentView : fContentView];
      fContentView.fResizedByROOT = NO;

      [fContentView release];
      
      if (attr)//TODO: what about deferCreation? at the moment, deferCreation is always 'NO'.
         ROOT::MacOSX::X11::SetWindowAttributes(attr, self);
   }
   
   return self;
}

//______________________________________________________________________________
- (void) dealloc
{
   [super dealloc];
}

///////////////////////////////////////////////////////////
//X11Drawable's protocol.

//______________________________________________________________________________
- (QuartzView *) fParentView
{
   return nil;
}

//______________________________________________________________________________
- (void) setFParentView : (QuartzView *)parent
{
   (void)parent;
}

//______________________________________________________________________________
- (unsigned) fID 
{
   assert(fContentView != nil && "fID, content view is nil");

   return fContentView.fID;
}

/////////////////////////////////////////////////////////////
//SetWindowAttributes_t/WindowAttributes_t.

//______________________________________________________________________________
- (unsigned long) fBackgroundPixel
{
   assert(fContentView != nil && "fBackgroundPixel, content view is nil");
   
   return fContentView.fBackgroundPixel;
}

//______________________________________________________________________________
- (void) setFBackgroundPixel : (unsigned long) pixel
{
   assert(fContentView != nil && "SetFBackgroundPixel, content view is nil");
   
   fContentView.fBackgroundPixel = pixel;
}

//______________________________________________________________________________
- (long) fEventMask
{
   assert(fContentView != nil && "fEventMask, content view is nil");
   
   return fContentView.fEventMask;
}

//______________________________________________________________________________
- (void) setFEventMask : (long)mask 
{
   assert(fContentView != nil && "setFEventMask, content view is nil");
   
   fContentView.fEventMask = mask;
}

//______________________________________________________________________________
- (int) fClass
{
   assert(fContentView != nil && "fClass, content view is nil");
   
   return fContentView.fClass;
}

//______________________________________________________________________________
- (void) setFClass : (int) windowClass
{
   assert(fContentView != nil && "setFClass, content view is nil");
   
   fContentView.fClass = windowClass;
}

//______________________________________________________________________________
- (void) setFID : (unsigned) winID
{
   assert(fContentView != nil && "setFID, content view is nil");
   
   fContentView.fID = winID;
}

//______________________________________________________________________________
- (int) fDepth
{
   assert(fContentView != nil && "fDepth, content view is nil");
   
   return fContentView.fDepth;
}

//______________________________________________________________________________
- (void) setFDepth : (int) depth
{
   assert(fContentView != nil && "setFDepth, content view is nil");
   
   fContentView.fDepth = depth;
}

//______________________________________________________________________________
- (int) fBitGravity
{
   assert(fContentView != nil && "fBitGravity, content view is nil");
   
   return fContentView.fBitGravity;
}

//______________________________________________________________________________
- (void) setFBitGravity : (int) bit
{
   assert(fContentView != nil && "setFBitGravity, content view is nil");

   fContentView.fBitGravity = bit;
}

//______________________________________________________________________________
- (int) fWinGravity
{
   assert(fContentView != nil && "fWinGravity, content view is nil");
   
   return fContentView.fWinGravity;
}

//______________________________________________________________________________
- (void) setFWinGravity : (int) bit
{
   assert(fContentView != nil && "setFWinGravity, content view is nil");
   
   fContentView.fWinGravity = bit;
}

//______________________________________________________________________________
- (int) fMapState
{
   //Top-level window can be only kIsViewable or kIsUnmapped (not unviewable).
   assert(fContentView != nil && "fMapState, content view is nil");
   
   if ([fContentView isHidden])
      return kIsUnmapped;
      
   return kIsViewable;
}

//End of SetWindowAttributes_t/WindowAttributes_t.
/////////////////////////////////////////////////////////////

//______________________________________________________________________________
- (BOOL) fIsPixmap
{
   return NO;
}

//______________________________________________________________________________
- (QuartzView *) fContentView
{
   return fContentView;
}

//______________________________________________________________________________
- (int) fX
{
   return self.frame.origin.x;
}

//______________________________________________________________________________
- (int) fYCocoa
{
   return self.frame.origin.y;
}

//______________________________________________________________________________
- (int) fYROOT
{
   //self.frame.origin.y is bottom-left corner's coordinate in a bottom-left system.
   NSArray *screens = [NSScreen screens];
   assert(screens != nil && "fYROOT, screens array is nil");
   
   NSScreen *mainScreen = [screens objectAtIndex : 0];
   assert(mainScreen != nil && "fYROOT, main screen (index 0) is nil");
   
   return ROOT::MacOSX::X11::GlobalYCocoaToROOT(self.frame.origin.y + self.frame.size.height);
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
- (NSSize) fSize
{
   //NSWindow's frame includes title-bar.
   //I have to use content view's frame.
   assert(fContentView != nil && "fSize, content view is nil");
   
   return fContentView.frame.size;
}

//______________________________________________________________________________
- (void) setDrawableSize : (NSSize) newSize
{
   //Can not simply do self.frame.size = newSize.
   assert(!(newSize.width < 0) && "setDrawableSize, width is negative");
   assert(!(newSize.height < 0) && "setDrawableSize, height is negative");
   
   fContentView.fResizedByROOT = YES;
   [self setContentSize : newSize];
   fContentView.fResizedByROOT = NO;
}

//______________________________________________________________________________
- (void) setX : (int) x rootY : (int) y width : (unsigned) w height : (unsigned) h
{
   const NSSize newSize = {.width = w, .height = h};
   [self setContentSize : newSize];
   
   //Check how this is affected by title bar's height.
   const NSPoint topLeft = {.x = x, .y = ROOT::MacOSX::X11::GlobalYROOTToCocoa(y)};

   fContentView.fResizedByROOT = YES;
   [self setFrameTopLeftPoint : topLeft];
   fContentView.fResizedByROOT = NO;
}

//______________________________________________________________________________
- (void) setX : (int) x rootY : (int) y
{
   const NSPoint topLeft = {.x = x, .y = ROOT::MacOSX::X11::GlobalYROOTToCocoa(y)};
   fContentView.fResizedByROOT = YES;
   [self setFrameTopLeftPoint : topLeft];
   fContentView.fResizedByROOT = NO;
}

//______________________________________________________________________________
- (void) addChild : (QuartzView *) child
{
   assert(fContentView != nil && "addChild, content view is nil");
   assert(child != nil && "addChild, child view is nil");
   
   [fContentView addChild : child];
}

//______________________________________________________________________________
- (void) getAttributes : (WindowAttributes_t *) attr
{
   assert(fContentView != nullptr && "getAttributes, content view is nil");
   assert(attr && "getAttributes, attr parameter is nil");

   ROOT::MacOSX::X11::GetWindowAttributes(self, attr);
}

//______________________________________________________________________________
- (void) setAttributes : (const SetWindowAttributes_t *)attr
{
   assert(attr != nullptr && "setAttributes, attr parameter is null");

#ifdef DEBUG_ROOT_COCOA
   log_attributes(attr, self.fID);
#endif

   ROOT::MacOSX::X11::SetWindowAttributes(attr, self);
}

//______________________________________________________________________________
- (void) mapRaised
{
   assert(fContentView && "mapRaised, content view is nil");

   [self orderFront : self];
   [fContentView setHidden : NO];
}

//______________________________________________________________________________
- (void) mapWindow
{
   assert(fContentView != nil && "mapWindow, content view is nil");

   [self orderFront : self];
   [fContentView setHidden : NO];
}

//______________________________________________________________________________
- (void) mapSubwindows
{
   assert(fContentView != nil && "mapSubwindows, content view is nil");

//   [fContentView setHidden : NO];
   [fContentView mapSubwindows];
}

//______________________________________________________________________________
- (void) unmapWindow
{
   assert(fContentView != nil && "unmapWindow, content view is nil");

   [fContentView setHidden : YES];
   [self orderOut : self];
}

//End of X11Drawable's protocol.
///////////////////////////////////////////////////////////


//NSWindowDelegate's methods here.

@end

//
//
//
//

@implementation QuartzView {
   BOOL fHandleConfigureSent;
}

@synthesize fResizedByROOT;
@synthesize fBackBuffer;
@synthesize fParentView;
@synthesize fID;

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

@synthesize fContext;

//______________________________________________________________________________
- (id) initWithFrame : (NSRect) frame windowAttributes : (const SetWindowAttributes_t *)attr
{
   if (self = [super initWithFrame : frame]) {
      //Make this explicit (though memory is zero initialized).
      fID = 0;
   
      [self setCanDrawConcurrently : NO];
      
      [self setHidden : YES];
      //Actually, check if view need this.
      const NSUInteger trackerOptions = NSTrackingMouseMoved | NSTrackingMouseEnteredAndExited | NSTrackingActiveInActiveApp | NSTrackingInVisibleRect;
      frame.origin = CGPointZero;
      NSTrackingArea *tracker = [[NSTrackingArea alloc] initWithRect : frame options : trackerOptions owner : self userInfo : nil];
      [self addTrackingArea : tracker];
      [tracker release];
      //
      if (attr)
         ROOT::MacOSX::X11::SetWindowAttributes(attr, self);
   }
   
   return self;
}

/////////////////////////////////////////////////////////////
//X11Drawable protocol.

//______________________________________________________________________________
- (BOOL) fIsPixmap
{
   return NO;
}

//______________________________________________________________________________
- (QuartzView *) fContentView
{
   return self;
}

//______________________________________________________________________________
- (int) fX
{
   return self.frame.origin.x;
}

//______________________________________________________________________________
- (int) fYCocoa
{
   return self.frame.origin.y;
}

//______________________________________________________________________________
- (int) fYROOT
{
   assert(fParentView != nil && "fYROOT, parent view is nil");
   
   return ROOT::MacOSX::X11::LocalYCocoaToROOT(fParentView, self.frame.origin.y + self.frame.size.height);
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
- (NSSize) fSize
{
   return self.frame.size;
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
   
   fResizedByROOT = YES;
   self.frame = frame;
   fResizedByROOT = NO;
}

//______________________________________________________________________________
- (void) setX : (int) x rootY : (int) y width : (unsigned) w height : (unsigned) h
{
   assert(fParentView != nil && "setX:rootY:width:height:, parent view is nil");

   NSRect newFrame = {};
   newFrame.origin.x = x;
   newFrame.origin.y = ROOT::MacOSX::X11::LocalYROOTToCocoa(fParentView, y + h);
   newFrame.size.width = w;
   newFrame.size.height = h;
   
   fResizedByROOT = YES;
   self.frame = newFrame;
   fResizedByROOT = NO;
}

//______________________________________________________________________________
- (void) setX : (int) x rootY : (int) y
{
   assert(fParentView != nil && "setX:rootY:, parent view is nil");
   
   NSRect newFrame = self.frame;
   newFrame.origin.x = x;
   newFrame.origin.y = ROOT::MacOSX::X11::LocalYROOTToCocoa(fParentView, y + newFrame.size.height);
   
   fResizedByROOT = YES;
   self.frame = newFrame;
   fResizedByROOT = NO;
}

//______________________________________________________________________________
- (void) addChild : (QuartzView *) child
{
   assert(child != nil && "addChild, child view is nil");

   [self addSubview : child];
   child.fParentView = self;
}

//______________________________________________________________________________
- (void) getAttributes : (WindowAttributes_t *)attr
{
   assert(attr != nullptr && "getAttributes, attr parameter is null");
   
   ROOT::MacOSX::X11::GetWindowAttributes(self, attr);
}

//______________________________________________________________________________
- (void) setAttributes : (const SetWindowAttributes_t *)attr
{
   assert(attr != nullptr && "setAttributes, attr parameter is null");

#ifdef DEBUG_ROOT_COCOA
   log_attributes(attr, fID);
#endif

   ROOT::MacOSX::X11::SetWindowAttributes(attr, self);
}

//______________________________________________________________________________
- (int) fMapState
{
   if ([self isHidden])
      return kIsUnmapped;

   for (QuartzView *parent = fParentView; parent; parent = parent.fParentView) {
      if ([parent isHidden]) {
         return kIsUnviewable;
      }
   }

   return kIsViewable;
}

//______________________________________________________________________________
- (void) mapRaised
{
   //Move view to the top of subviews (in UIKit there is a special method).   
   QuartzView *parent = fParentView;
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
      [v mapSubwindows];
   }
}

//______________________________________________________________________________
- (void) unmapWindow
{
   [self setHidden : YES];
}

//End of X11Drawable protocol.
/////////////////////////////////////////////////////////////

//Painting mechanics.

//______________________________________________________________________________
- (void) drawRect : (NSRect) dirtyRect
{
   (void)dirtyRect;//Not used at the moment.

   if (fID) {
      if (TGWindow *window = gClient->GetWindowById(fID)) {
         NSGraphicsContext *nsContext = [NSGraphicsContext currentContext];
         assert(nsContext != nil && "drawRect, currentContext returned nil");

         fContext = (CGContextRef)[nsContext graphicsPort];
         assert(fContext != nullptr && "drawRect, graphicsPort returned null");
         
         CGContextSaveGState(fContext);

         if (fEventMask & kExposureMask) {
            //Ask ROOT's widget/window to draw itself.
            gClient->NeedRedraw(window, kTRUE);
         }

         if (fBackBuffer) {
            //Very "special" window.
            CGImageRef image = CGBitmapContextCreateImage(fBackBuffer.fContext);
            const CGRect imageRect = CGRectMake(0, 0, fBackBuffer.fWidth, fBackBuffer.fHeight);

            CGContextDrawImage(fContext, imageRect, image);
            CGImageRelease(image);
         }

         CGContextRestoreGState(fContext);

         fContext = nullptr;
      } else {
         NSLog(@"Warning: QuartzView, -drawRect method, no window for id %u was found", fID);
      }
   }
}

//Event handling.

//______________________________________________________________________________
- (void) generateConfigureNotify : (NSRect) newFrame
{
   if (self.fMapState == kIsUnmapped)
      return;

   if (fID) {
      if (TGWindow *window = gClient->GetWindowById(fID)) {
         Event_t newEvent = {};

         newEvent.fType = kConfigureNotify;         
         newEvent.fWindow = fID;
         newEvent.fX = newFrame.origin.x;
         newEvent.fY = newFrame.origin.y;
         newEvent.fWidth = newFrame.size.width;
         newEvent.fHeight = newFrame.size.height;
         
         //TODO:
         //1. generate timestamp?
         //2. check, what's actually required from configure notify.
         window->HandleEvent(&newEvent);
      } else {
         NSLog(@"Warning: QuartzView, -generateConfigureNotify method, no window for id %u was found", fID);
      }
   }

}

//______________________________________________________________________________
- (void) setFrame : (NSRect) newFrame
{
   [super setFrame : newFrame];

   if (fEventMask & kStructureNotifyMask) {
     // [self generateConfigureNotify : newFrame];
   }
}


//______________________________________________________________________________
- (void) setFrameSize : (NSSize) newSize
{
   //Check, if setFrameSize calls setFrame.
   
   [super setFrameSize : newSize];
   
   if (fEventMask & kStructureNotifyMask) {
      [self generateConfigureNotify : self.frame];
   }
   
   //?
   [self setNeedsDisplay : YES];
}

//______________________________________________________________________________
- (void) locationForEvent : (NSEvent *) cocoaEvent toROOTEvent : (Event_t *) rootEvent
{
   assert(cocoaEvent != nil && "locationForEvent, cocoaEvent parameter is nil");
   assert(rootEvent != nullptr && "locationForEvent, rootEvent parameter is null");

   const NSPoint clickPoint = [self convertPoint : [cocoaEvent locationInWindow] fromView : nil];
   rootEvent->fX = clickPoint.x;
   rootEvent->fY = ROOT::MacOSX::X11::LocalYCocoaToROOT(self, clickPoint.y);
}

//______________________________________________________________________________
- (Event_t) createROOTEventFor : (NSEvent *) theEvent
{
   Event_t newEvent = {};
   newEvent.fWindow = fID;
   newEvent.fTime = [theEvent timestamp];
   
   return newEvent;
}

//______________________________________________________________________________
- (void) mouseDown : (NSEvent *) theEvent
{

   if (fID) {
      if (TGWindow *window = gClient->GetWindowById(fID)) {
       if (fEventMask & kButtonPressMask) {
            Event_t newEvent = [self createROOTEventFor : theEvent];
            newEvent.fType = kButtonPress;
            [self locationForEvent : theEvent toROOTEvent : &newEvent];
            /*
            const NSPoint clickPoint = [self convertPoint : [theEvent locationInWindow] fromView : nil];
            
            newEvent.fX = clickPoint.x;
            newEvent.fY = ROOT::MacOSX::X11::LocalYCocoaToROOT(self, clickPoint.y);
            */
            window->HandleEvent(&newEvent);
         } else //We can also check fDoNoPropagate mask and block the event (TODO).
            [super mouseDown : theEvent];//Pass to the parent view.
      } else {
         NSLog(@"Warning: QuartzView, -mouseDown method, no window for id %u was found", fID);
      }
   } else
      [super mouseDown : theEvent];//Will pass to parent view.
}

//______________________________________________________________________________
- (void) mouseUp : (NSEvent *) theEvent
{
   if (fID) {
      if (TGWindow *window = gClient->GetWindowById(fID)) {
         if (fEventMask & kButtonReleaseMask) {
            Event_t newEvent = [self createROOTEventFor : theEvent];
            newEvent.fType = kButtonRelease;
            //TODO: coordinates?
            [self locationForEvent : theEvent toROOTEvent : &newEvent];
            window->HandleEvent(&newEvent);
         } else
            [super mouseUp : theEvent];//Pass to the parent view.
      } else {
         NSLog(@"Warning: QuartzView, -mouseDown method, no window for id %u was found", fID);
      }
   } else
      [super mouseUp : theEvent];
}

//______________________________________________________________________________
- (void) mouseEntered : (NSEvent *) theEvent
{
   if (fID) {
      if (TGWindow *window = gClient->GetWindowById(fID)) {
         if (fEventMask & kEnterWindowMask) {
            Event_t newEvent = [self createROOTEventFor : theEvent];
            newEvent.fType = kEnterNotify;
            [self locationForEvent : theEvent toROOTEvent : &newEvent];
            
            window->HandleEvent(&newEvent);
         }
      } else {
         NSLog(@"Warning: QuartzView, -mouseEntered method, no window for id %u was found", fID);
      }
   }
}

//______________________________________________________________________________
- (void) mouseExited : (NSEvent *) theEvent
{
   if (fID) {
      if (TGWindow *window = gClient->GetWindowById(fID)) {
         if (fEventMask & kLeaveWindowMask) {
            Event_t newEvent = [self createROOTEventFor : theEvent];
            newEvent.fType = kLeaveNotify;
            [self locationForEvent : theEvent toROOTEvent : &newEvent];
            
            window->HandleEvent(&newEvent);
         }
      } else {
         NSLog(@"Warning: QuartzView, -mouseExited method, no window for id %u was found", fID);
      }
   }
}

//______________________________________________________________________________
- (BOOL) acceptsFirstResponder
{
   return YES;
}

//______________________________________________________________________________
- (void) mouseMoved : (NSEvent *) theEvent
{
   const NSPoint windowPoint = [theEvent locationInWindow];
   NSView *candidateView = [[[self window] contentView] hitTest : windowPoint];

   if (candidateView != self)
      return;

   if (fID) {
      if (TGWindow *window = gClient->GetWindowById(fID)) {
         if (fEventMask & kPointerMotionMask) {
            Event_t newEvent = [self createROOTEventFor : theEvent];
            newEvent.fType = kMotionNotify;
            [self locationForEvent : theEvent toROOTEvent : &newEvent];
            
            window->HandleEvent(&newEvent);
         }
      } else {
         NSLog(@"Warning: QuartzView, -mouseMoved, no window for id %u was found", fID);
      }
   }
}

@end

//
//
//
//
//
//

@implementation QuartzPixmap {
@private
   unsigned fWidth;
   unsigned fHeight;
   
   CGContextRef fContext;
}

@synthesize fID;

//______________________________________________________________________________
- (id) initWithSize : (NSSize) pixmapSize flipped : (BOOL) flip
{
   if (self = [super init]) {
      fWidth = 0;
      fHeight = 0;
      
      if ([self resize : pixmapSize flipped : flip])
         return self;
   }
   
   //Yes, if context creation failed, the calling code should use
   //separate alloc/init statements to check this.
   return nil;
}

//______________________________________________________________________________
- (void) dealloc
{
   if (fContext)
      CGContextRelease(fContext);
   [super dealloc];
}

//______________________________________________________________________________
- (BOOL) resize : (NSSize) newSize flipped : (BOOL) flip
{
   assert(newSize.width > 0 && "Pixmap width must be positive");
   assert(newSize.height > 0 && "Pixmap height must be positive");

   CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();//[1]

   if (!colorSpace) {
      assert(colorSpace && "CGColorSpaceCreateDeviceRGB failed");
      return NO;
   }
   
   CGContextRef ctx = CGBitmapContextCreate(nullptr, newSize.width, newSize.height, 8, 0, colorSpace, kCGImageAlphaPremultipliedLast);//[2]

   if (!ctx) {
      CGColorSpaceRelease(colorSpace);//[1], ![2]
      assert(ctx && "CGBitmapContextCreate failed");
      return NO;
   }

   //
   //For debug only: fill bitmap with green color.
   //
//   CGContextSetRGBFillColor(ctx, 0.f, 1.f, 0.f, 1.f);
//   CGContextFillRect(ctx, CGRectMake(0.f, 0.f, newSize.width, newSize.height));

   
   if (fContext) {
      //New context was created OK, we can release now the old one.
      CGContextRelease(fContext);//[2]
   }

   //Size to be used later - to identify,
   //if we really have to resize.
   fWidth = newSize.width;
   fHeight = newSize.height;
   
   fContext = ctx;//[2]

   if (flip) {
      CGContextTranslateCTM(fContext, 0.f, fHeight);
      CGContextScaleCTM(fContext, 1.f, -1.f);
   }

   CGColorSpaceRelease(colorSpace);//[1]

   return YES;

}

//______________________________________________________________________________
- (BOOL) fIsPixmap
{
   return YES;
}

//______________________________________________________________________________
- (unsigned) fWidth
{
   assert(fContext && "fWidth, called for bad pixmap");
   return fWidth;
}

//______________________________________________________________________________
- (unsigned) fHeight
{
   assert(fContext != nullptr && "fHeight, called for bad pixmap");
   return fHeight;
}

//______________________________________________________________________________
- (NSSize) fSize
{
   NSSize size = {};
   size.width = fWidth;
   size.height = fHeight;
   return size;
}

//______________________________________________________________________________
- (CGContextRef) fContext
{
   assert(fContext != nullptr && "fContext, called for bad pixmap");   
   return fContext;
}

@end
