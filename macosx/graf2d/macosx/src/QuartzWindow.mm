#define DEBUG_ROOT_COCOA

#ifdef DEBUG_ROOT_COCOA
#import <fstream>
#endif

#import <cassert>

#import "QuartzWindow.h"
#import "QuartzPixmap.h"
#import "X11Events.h"
#import "TGWindow.h"
#import "TGClient.h"
#import "TGCocoa.h"

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
NSPoint TranslateToScreen(QuartzView *from, NSPoint point)
{
   assert(from != nil && "TranslateCoordinates, 'from' parameter is nil");
   
   //I do not know, if I can use convertToBacking ..... - have to check this.   
   const NSPoint winPoint = [from convertPoint : point toView : nil];
   NSPoint screenPoint = [[from window] convertBaseToScreen : winPoint];
 
   //This is Cocoa's coordinates, but for ROOT I have to convert.
   screenPoint.y = GlobalYCocoaToROOT(screenPoint.y);

   return screenPoint;
}

//______________________________________________________________________________
NSPoint TranslateFromScreen(NSPoint point, QuartzView *to)
{
   assert(to != nil && "TranslateCoordinates, 'to' parameter is nil");
   
   point.y = GlobalYROOTToCocoa(point.y);

   //May be I can use convertBackingTo .... have to check this.
   const NSPoint winPoint = [[to window] convertScreenToBase : point];
   return [to convertPoint : winPoint fromView : nil];
}

//______________________________________________________________________________
NSPoint TranslateCoordinates(QuartzView *from, QuartzView *to, NSPoint sourcePoint)
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
      //TODO: Must be checked yet, if I understand this correctly!
      if ([(NSObject *)window isKindOfClass : [QuartzWindow class]]) {
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
   dst->fY = win.fY;
   
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
      
      [self setContentView : fContentView];

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
- (int) fGrabButton
{
   assert(fContentView != nil && "fGrabButton, content view is nil");
   
   return fContentView.fGrabButton;
}

//______________________________________________________________________________
- (void) setFGrabButton : (int) btn
{
   assert(fContentView != nil && "setFGrabButton, content view is nil");
   
   fContentView.fGrabButton = btn;
}

//______________________________________________________________________________
- (unsigned) fGrabButtonEventMask
{
   assert(fContentView != nil && "fGrabButtonEventMask, content view is nil");
   
   return fContentView.fGrabButtonEventMask;
}

//______________________________________________________________________________
- (void) setFGrabButtonEventMask : (unsigned) mask
{
   assert(fContentView != nil && "setFGrabButtonEventMask, content view is nil");
   
   fContentView.fGrabButtonEventMask = mask;
}

//______________________________________________________________________________
- (unsigned) fGrabKeyModifiers
{
   assert(fContentView != nil && "fGrabKeyModifiers, content view is nil");
   
   return fContentView.fGrabKeyModifiers;
}

//______________________________________________________________________________
- (void) setFGrabKeyModifiers : (unsigned) mod
{
   assert(fContentView != nil && "setFGrabKeyModifiers, content view is nil");
   
   fContentView.fGrabKeyModifiers = mod;
}

//______________________________________________________________________________
- (BOOL) fOwnerEvents
{
   assert(fContentView != nil && "fOwnerEvents, content view is nil");

   return fContentView.fOwnerEvents;
}

//______________________________________________________________________________
- (void) setFOwnerEvents : (BOOL) owner
{
   assert(fContentView != nil && "setFOwnerEvents, content view is nil");

   fContentView.fOwnerEvents = owner;
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
- (int) fY
{
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
   
   [self setContentSize : newSize];
}

//______________________________________________________________________________
- (void) setX : (int) x Y : (int) y width : (unsigned) w height : (unsigned) h
{
   const NSSize newSize = {.width = w, .height = h};
   [self setContentSize : newSize];
   
   //Check how this is affected by title bar's height.
   const NSPoint topLeft = {.x = x, .y = ROOT::MacOSX::X11::GlobalYROOTToCocoa(y)};

   [self setFrameTopLeftPoint : topLeft];
}

//______________________________________________________________________________
- (void) setX : (int) x Y : (int) y
{
   const NSPoint topLeft = {.x = x, .y = ROOT::MacOSX::X11::GlobalYROOTToCocoa(y)};

   [self setFrameTopLeftPoint : topLeft];
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

@interface QuartzView ()

- (void) generateConfigureNotify : (NSRect) newFrame;

@end

@implementation QuartzView

@synthesize fBackBuffer;
@synthesize fParentView;
@synthesize fLevel;
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

@synthesize fGrabButton;
@synthesize fGrabButtonEventMask;
@synthesize fGrabKeyModifiers;
@synthesize fOwnerEvents;

@synthesize fContext;

//______________________________________________________________________________
- (id) initWithFrame : (NSRect) frame windowAttributes : (const SetWindowAttributes_t *)attr
{
   if (self = [super initWithFrame : frame]) {
      //Make this explicit (though memory is zero initialized).
      fID = 0;
      fLevel = 0;
      
      //Passive grab parameters.
      fGrabButton = -1;//0 is kAnyButton.
      fGrabButtonEventMask = 0;
      fOwnerEvents = NO;
      
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

//______________________________________________________________________________
- (BOOL) isFlipped
{
   //Now view's placement, geometry, moving and resizing can be
   //done with ROOT's (X11) coordinates without conversion.
   return YES;
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
   
   self.frame = frame;
}

//______________________________________________________________________________
- (void) setX : (int) x Y : (int) y width : (unsigned) w height : (unsigned) h
{
   assert(fParentView != nil && "setX:Y:width:height:, parent view is nil");

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
   assert(fParentView != nil && "setX:Y:, parent view is nil");
   
   NSRect newFrame = self.frame;
   newFrame.origin.x = x;
   newFrame.origin.y = y;
   
   self.frame = newFrame;
}

//______________________________________________________________________________
- (void) addChild : (QuartzView *) child
{
   assert(child != nil && "addChild, child view is nil");

   [self addSubview : child];
   child.fParentView = self;
   child.fLevel = self.fLevel + 1;
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
      if ([parent isHidden])
         return kIsUnviewable;
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

   if (fEventMask & kStructureNotifyMask)
      [self generateConfigureNotify : self.frame];
}

//______________________________________________________________________________
- (void) mapWindow
{   
   [self setHidden : NO];

   if (fEventMask & kStructureNotifyMask)
      [self generateConfigureNotify : self.frame];
}

//______________________________________________________________________________
- (void) mapSubwindows
{
   for (QuartzView * v in [self subviews]) {
      [v setHidden : NO];
 
      if (v.fEventMask & kStructureNotifyMask)
         [v generateConfigureNotify : v.frame];

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
- (Event_t) createROOTEventFor : (NSEvent *) theEvent
{
   Event_t newEvent = {};
   newEvent.fWindow = fID;
   newEvent.fTime = [theEvent timestamp];
   
   return newEvent;
}

//______________________________________________________________________________
- (void) generateConfigureNotify : (NSRect) newFrame
{
   assert(fID != 0 && "generateConfigureNotify, fID is 0");
   
   TGWindow *window = gClient->GetWindowById(fID);
   assert(window != nullptr && "generateConfigureNotify, window was not found");
   
   Event_t newEvent = {};
   newEvent.fWindow = fID;
   newEvent.fType = kConfigureNotify;         

   newEvent.fX = newFrame.origin.x;
   newEvent.fY = newFrame.origin.y;
   newEvent.fWidth = newFrame.size.width;
   newEvent.fHeight = newFrame.size.height;
   
   //TODO:
   //1. generate timestamp?
   //2. check, what's actually required from configure notify.
   window->HandleEvent(&newEvent);
}

//______________________________________________________________________________
- (void) setFrame : (NSRect) newFrame
{
   [super setFrame : newFrame];
}

//______________________________________________________________________________
- (void) setFrameSize : (NSSize) newSize
{
   //Check, if setFrameSize calls setFrame.
   
   [super setFrameSize : newSize];
   
   if ((fEventMask & kStructureNotifyMask) && self.fMapState == kIsViewable)
      [self generateConfigureNotify : self.frame];

   [self setNeedsDisplay : YES];//?
}
/*
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
- (BOOL) viewGeneratesButtonPressEvent : (EMouseButton) btn
{
   if (fEventMask & kButtonPressMask)
      return YES;

   if (btn == fGrabButton && (fGrabButtonEventMask & kButtonPressMask))
      return YES;
   
   return NO;
}

//______________________________________________________________________________
- (BOOL) viewGeneratesButtonReleaseEvent : (EMouseButton) btn
{
   if (fEventMask & kButtonReleaseMask)
      return YES;

   if (btn == fGrabButton && (fGrabButtonEventMask & kButtonReleaseMask))
      return YES;
   
   return NO;
}
*/

//______________________________________________________________________________
- (void) mouseDown : (NSEvent *) theEvent
{
   assert(fID != 0 && "mouseDown, fID is 0");
   
   TGWindow *window = gClient->GetWindowById(fID);
   assert(window != nullptr && "mouseDown, window was not found");
   

   TGCocoa *vx = dynamic_cast<TGCocoa *>(gVirtualX);
   assert(vx && "gVirtualX is either null or has type different from TGCocoa");
   vx->GetEventTranslator()->GenerateButtonPressEvent(self, theEvent, kButton1);
}

//______________________________________________________________________________
- (void) mouseUp : (NSEvent *) theEvent
{
   assert(fID != 0 && "mouseUp, fID is 0");

   TGCocoa *vx = dynamic_cast<TGCocoa *>(gVirtualX);
   assert(vx && "gVirtualX is either null or has type different from TGCocoa");
   vx->GetEventTranslator()->GenerateButtonReleaseEvent(self, theEvent, kButton1);
}

//______________________________________________________________________________
- (void) mouseEntered : (NSEvent *) theEvent
{
   assert(fID != 0 && "mouseEntered, fID is 0");
   
   TGCocoa *vx = dynamic_cast<TGCocoa *>(gVirtualX);
   assert(vx != nullptr && "mouseEntered, gVirtualX is null or not of TGCocoa type");

   vx->GetEventTranslator()->GenerateCrossingEvent(self, theEvent);  
}

//______________________________________________________________________________
- (void) mouseExited : (NSEvent *) theEvent
{
   assert(fID != 0 && "mouseExited, fID is 0");

   TGCocoa *vx = dynamic_cast<TGCocoa *>(gVirtualX);
   assert(vx != nullptr && "mouseExited, gVirtualX is null or not of TGCocoa type");

   vx->GetEventTranslator()->GenerateCrossingEvent(self, theEvent);
}

//______________________________________________________________________________
- (BOOL) acceptsFirstResponder
{
   return NO;//YES;
}

//______________________________________________________________________________
- (void) mouseMoved : (NSEvent *) theEvent
{
   assert(fID != 0 && "mouseMoved, fID is 0");
   
   if (fParentView)//Suppress events in all views, except the top-level one.
      return;      //TODO: check, that it does not create additional problems.

   TGCocoa *vx = dynamic_cast<TGCocoa *>(gVirtualX);
   assert(vx != nullptr && "mouseMoved, gVirtualX is null or not of TGCocoa type");
   
   vx->GetEventTranslator()->GeneratePointerMotionEvent(self, theEvent);
}

//______________________________________________________________________________
- (void) mouseDragged : (NSEvent *)theEvent
{
   assert(fID != 0 && "mouseDragged, fID is 0");
   
   //mouseMoved and mouseDragged work differently 
   //(drag events are generated only for one view, where drag started).
   //if (fParentView)
   //   return;
   
   TGCocoa *vx = dynamic_cast<TGCocoa *>(gVirtualX);
   assert(vx != nullptr && "mouseMoved, gVirtualX is null or not of TGCocoa type");
   
   vx->GetEventTranslator()->GeneratePointerMotionEvent(self, theEvent);   
}

@end
