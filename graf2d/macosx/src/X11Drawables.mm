#define DEBUG_ROOT_COCOA

#ifdef DEBUG_ROOT_COCOA
#import <fstream>
#endif

#import <cassert>

#import "X11Drawables.h"
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
         //
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

   if (mask & kWABackingPlanes)
      window.fBackingPlanes = attr->fBackingPlanes;
      
   if (mask & kWASaveUnder)
      window.fSaveUnder = attr->fSaveUnder;
      
   if (mask & kWAOverrideRedirect)
      window.fOverrideRedirect = attr->fOverrideRedirect;
   
   //TODO: More attributes to set -
   //cursor for example, etc.
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
   dst->fBackingPlanes = window.fBackingPlanes;

   //Dummy value.
   dst->fBackingPixel = 0;
   
   dst->fSaveUnder = window.fSaveUnder;

   //Dummy value.
   dst->fColormap = 0;
   //Dummy value.   
   dst->fMapInstalled = kTRUE;

   dst->fMapState = window.fIsMapped ? kIsViewable : kIsUnmapped;

   //Dummy value. Actually, never used in ROOT's GUI.
   dst->fAllEventMasks = 0;
   dst->fYourEventMask = window.fEventMask;
   
   //Not used by GUI.
   //dst->fDoNotPropagateMask

   dst->fOverrideRedirect = window.fOverrideRedirect;

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
- (unsigned long) fBackingPlanes
{
   assert(fContentView != nil && "fBackingPlanes, content view is nil");
   
   return fContentView.fBackingPlanes;
}

//______________________________________________________________________________
- (void) setFBackingPlanes : (unsigned long) planes
{
   assert(fContentView != nil && "setFBackingPlanes, content view is nil");
   
   fContentView.fBackingPlanes = planes;
}

//______________________________________________________________________________
- (BOOL) fSaveUnder
{
   assert(fContentView != nil && "fSaveUnder, content view is nil");
   
   return fContentView.fSaveUnder;
}

//______________________________________________________________________________
- (void) setFSaveUnder : (BOOL) save
{
   assert(fContentView != nil && "setFSaveUnder, content view is nil");
   
   fContentView.fSaveUnder = save;
}

//______________________________________________________________________________
- (BOOL) fIsMapped
{
   assert(fContentView != nil && "fIsMapped, content view is nil");
   
   return fContentView.fIsMapped;
}

//______________________________________________________________________________
- (void) setFIsMapped : (BOOL) mapped
{
   assert(fContentView != nil && "setFIsMapped, content view is nil");
   
   fContentView.fIsMapped = mapped;
}

//______________________________________________________________________________
- (BOOL) fOverrideRedirect
{
   assert(fContentView != nil && "fOverrideRedirect, content view is nil");
   
   return fContentView.fOverrideRedirect;
}

//______________________________________________________________________________
- (void) setFOverrideRedirect : (BOOL) redir
{
   assert(fContentView != nil && "setFOverrideRedirect, content view is nil");
   
   fContentView.fOverrideRedirect = redir;
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
   
   [self setContentSize : newSize];
}

//______________________________________________________________________________
- (void) setX : (int) x rootY : (int) y width : (unsigned) w height : (unsigned) h
{
   const NSSize newSize = {.width = w, .height = h};
   [self setContentSize : newSize];
   
   //Check how this is affected by title bar's height.
   const NSPoint topLeft = {.x = x, .y = ROOT::MacOSX::X11::GlobalYROOTToCocoa(y)};
   [self setFrameTopLeftPoint : topLeft];
}

//______________________________________________________________________________
- (void) setX : (int) x rootY : (int) y
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
   self.fIsMapped = YES;
   //
   [self makeKeyAndOrderFront : self];
}

//______________________________________________________________________________
- (void) unmapWindow
{
   self.fIsMapped = NO;
   
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

@implementation QuartzView

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
@synthesize fBackingPlanes;
@synthesize fSaveUnder;
@synthesize fIsMapped;
@synthesize fOverrideRedirect;
//SetWindowAttributes_t/WindowAttributes_t
/////////////////////

@synthesize fContext;

//______________________________________________________________________________
- (id) initWithFrame : (NSRect) frame windowAttributes : (const SetWindowAttributes_t *)attr
{
   if (self = [super initWithFrame : frame]) {
      [self setCanDrawConcurrently : NO];
   
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
   const CGFloat yShift = newSize.height - frame.size.height;
//   if (fID == 38) {
//      NSLog(@"EXTENAL RESIZE: ")
//   }
 //  frame.origin.y -= yShift;
   frame.size = newSize;
    
   self.frame = frame;
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
   
   self.frame = newFrame;
}

//______________________________________________________________________________
- (void) setX : (int) x rootY : (int) y
{
   assert(fParentView != nil && "setX:rootY:, parent view is nil");
   
   NSRect newFrame = self.frame;
   newFrame.origin.x = x;
   newFrame.origin.y = ROOT::MacOSX::X11::LocalYROOTToCocoa(fParentView, y + newFrame.size.height);
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
- (void) mapRaised
{
   fIsMapped = YES;
   //
   [self setHidden : NO];
}

//______________________________________________________________________________
- (void) unmapWindow
{
   fIsMapped = NO;
   
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
      if (fID == 38) {
         NSGraphicsContext *nsContext = [NSGraphicsContext currentContext];
         assert(nsContext != nil && "drawRect, currentContext returned nil");

         fContext = (CGContextRef)[nsContext graphicsPort];
         assert(fContext != nullptr && "drawRect, graphicsPort returned null");
         
         CGContextSetRGBFillColor(fContext, 1.f, 0.f, 0.f, 1.f);
         CGContextFillRect(fContext, dirtyRect);
         fContext = nullptr;
         
         return;
      }
      if (TGWindow *window = gClient->GetWindowById(fID)) {
         NSGraphicsContext *nsContext = [NSGraphicsContext currentContext];
         assert(nsContext != nil && "drawRect, currentContext returned nil");

         fContext = (CGContextRef)[nsContext graphicsPort];
         assert(fContext != nullptr && "drawRect, graphicsPort returned null");
         
         CGContextSaveGState(fContext);

         //Ask ROOT's widget/window to draw.
         gClient->NeedRedraw(window, kTRUE);
         //
         
         CGContextRestoreGState(fContext);

         fContext = nullptr;
      } else {
         NSLog(@"Warning: RootQuartzView, -drawRect method, no window for id %u was found", fID);
      }
   }
}

//Event handling.

- (void) setFrame : (NSRect)newFrame
{
   [super setFrame : newFrame];
   if (fID == 38) {
      NSLog(@"-------------- %g %g %g %g", newFrame.origin.x, newFrame.origin.y, newFrame.size.width, newFrame.size.height);
      if (newFrame.origin.y > 40.f) {
   //      int * pp = 0;
   //      pp[100] = 100;
      }
   }
}


//______________________________________________________________________________
- (void) setFrameSize : (NSSize)newSize
{
   //Generate ConfigureNotify event and send it to ROOT's TGWindow.

   
   [super setFrameSize : newSize];
   
   if (fEventMask & kStructureNotifyMask) {//Check, if window wants such events.
      if (fID) {
         if (TGWindow *window = gClient->GetWindowById(fID)) {
            Event_t newEvent = {};

            newEvent.fType = kConfigureNotify;         
            newEvent.fWindow = fID;
            newEvent.fX = self.frame.origin.x;
            newEvent.fY = self.frame.origin.y;
            newEvent.fWidth = newSize.width;
            newEvent.fHeight = newSize.height;
         
            //TODO:
            //1. generate time.
            //2. check, what's actually required from configure notify.
            window->HandleEvent(&newEvent);
         } else {
            NSLog(@"Warning: RootQuartzView, -setFrameSize method, no window for id %u was found", fID);
         }
      }
   }
}

- (void) mouseDown : (NSEvent *)theEvent
{
   (void)theEvent;
   NSLog(@"I'm %u, geometry: %g %g %g %g", fID, self.frame.origin.x, self.frame.origin.y, self.frame.size.width, self.frame.size.height);
   if (fID == 38) {
      NSRect frame = self.frame;
      frame.origin.y = 0;
      self.frame = frame;
   }
      
}

@end


@implementation QuartzPixmap {
@private
}

@synthesize fID;

//______________________________________________________________________________
- (BOOL) fIsPixmap
{
   return YES;
}

@end