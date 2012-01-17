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
void SetWindowAttributes(const SetWindowAttributes_t *attr, QuartzView *view)
{
   if (attr->fMask & kWABackPixel)
      view.fBackgroundPixel = attr->fBackgroundPixel;
   
   if (attr->fMask & kWAEventMask)
      view.fEventMask = attr->fEventMask;
   
   //TODO: More attributes to set.
}

//______________________________________________________________________________
void SetWindowAttributes(const SetWindowAttributes_t *attr, QuartzWindow *window)
{
   assert(attr != nullptr && "SetWindowAttributes, attr parameter is null");
   assert(window != nil && "SetWindowAttributes, window parameter is nil");

   if (attr->fMask & kWABorderWidth) {
      //Set border width.
   }
   
   if (attr->fMask & kWAEventMask)
      window.fEventMask = attr->fEventMask;
      
   //TODO: More attributes to set.
   
   SetWindowAttributes(attr, window.fContentView);
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
void GetWindowAttributes(QuartzWindow *window, WindowAttributes_t *dst)
{
   assert(window != nil && "GetWindowAttributes, window parameter is nil");
   assert(dst != nullptr && "GetWindowAttributes, attr parameter is null");
   
   GetWindowGeometry(window, dst);

   //TODO: border.
   //attr->fBorderWidth = [window ...]
   //TODO: kInputOnly/kInputOutput
   //attr->fClass
   //TODO: depth
   //attr->fDepth = 
   //TODO: BitGravity
   //attr->fBitGravity = ;
   //TODO: WinGravity
   //attr->fWinGravity = ;
   
   dst->fYourEventMask = window.fEventMask;
}

//______________________________________________________________________________
void GetWindowAttributes(QuartzView *view, WindowAttributes_t *dst)
{
   assert(view != nil && "GetWindowAttributes, view parameter is nil");
   assert(dst != nullptr && "GetWindowAttributes, attr parameter is null");
   
   GetWindowGeometry(view, dst);

   //TODO: other parameters.
   
   dst->fYourEventMask = view.fEventMask;
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
@synthesize fEventMask;
@synthesize fContext;
@synthesize fBackgroundPixel;

//RootQuartzWindow's life cycle.

//______________________________________________________________________________
- (id) initWithContentRect : (NSRect) contentRect styleMask : (NSUInteger) windowStyle backing : (NSBackingStoreType) bufferingType 
       defer : (BOOL) deferCreation  windowAttributes : (const SetWindowAttributes_t *)attr
{
   self = [super initWithContentRect : contentRect styleMask : windowStyle backing : bufferingType defer : deferCreation];

   if (self) {
      fContext = nullptr;
      //self.delegate = ...
      //create content view here.
      NSRect contentViewRect = contentRect;
      contentViewRect.origin = CGPointZero;
      QuartzView *view = [[QuartzView alloc] initWithFrame : contentViewRect windowAttributes : attr];
      [self setContentView : view];
      [view release];
      
      if (attr)//TODO: what about deferCreation?
         ROOT::MacOSX::X11::SetWindowAttributes(attr, self);
   }
   
   return self;
}

//______________________________________________________________________________
- (void) dealloc
{
   [super dealloc];
}

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

//______________________________________________________________________________
- (void) setFID : (unsigned) winID
{
   assert(fContentView != nil && "setFID, content view is nil");
   
   fContentView.fID = winID;
}

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

   (void)attr;
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
@synthesize fEventMask;
@synthesize fContext;
@synthesize fBackgroundPixel;

//______________________________________________________________________________
- (id) initWithFrame : (NSRect) frame windowAttributes : (const SetWindowAttributes_t *)attr
{
   if (self = [super initWithFrame : frame]) {
      if (attr)
         ROOT::MacOSX::X11::SetWindowAttributes(attr, self);
   }
   
   return self;
}

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
   [self setFrameSize : newSize];
}

//______________________________________________________________________________
- (void) setX : (int) x rootY : (int) y width : (unsigned) w height : (unsigned) h
{
   assert(fParentView != nil && "setX:rootY:width:height:, parent view is nil");

   NSRect newFrame = {};
   newFrame.origin.x = x;
   newFrame.origin.y = ROOT::MacOSX::X11::LocalYROOTToCocoa(fParentView, y);
   newFrame.size.width = w;
   newFrame.size.height = h;
   
   self.frame = newFrame;
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
   (void)attr;
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

         //Ask ROOT's widget/window to draw.
         gClient->NeedRedraw(window, kTRUE);
         //
         
         CGContextRestoreGState(fContext);

         fContext = 0;
      } else {
         NSLog(@"Warning: RootQuartzView, -drawRect method, no window for id %u was found", fID);
      }
   }
}

//Event handling.

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