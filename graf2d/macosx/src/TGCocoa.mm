// Author: Timur Pocheptsov 22/11/2011

#include <algorithm>
#include <stdexcept>

//
#include <iostream>
#include <fstream>
//

#include  <Cocoa/Cocoa.h>

#include "CocoaPrivate.h"
#include "QuartzWindow.h"
#include "QuartzPixmap.h"
#include "X11Drawable.h"
#include "QuartzText.h"
#include "CocoaUtils.h"
#include "X11Events.h"
#include "X11Buffer.h"
#include "TGClient.h"
#include "TGWindow.h"
#include "TGCocoa.h"
#include "TError.h"

ClassImp(TGCocoa)

namespace Details = ROOT::MacOSX::Details;
namespace X11 = ROOT::MacOSX::X11;//Oh, I hope, there is not such a stupid name in a global space :)


namespace {

class CGStateGuard {
public:
   CGStateGuard(CGContextRef ctx)
      : fCtx(ctx)
   {
      CGContextSaveGState(ctx);
   }
   ~CGStateGuard()
   {
      CGContextRestoreGState(fCtx);
   }
   
private:
   CGContextRef fCtx;
   
   CGStateGuard(const CGStateGuard &rhs) = delete;
   CGStateGuard &operator = (const CGStateGuard &rhs) = delete;
};

//______________________________________________________________________________
void PixelToRGB(Pixel_t pixelColor, CGFloat *rgb)
{
   //TODO: something not so lame!
   rgb[0] = (pixelColor >> 16 & 0xff) / 255.f;
   rgb[1] = (pixelColor >> 8 & 0xff) / 255.f;
   rgb[2] = (pixelColor & 0xff) / 255.f;
}

//______________________________________________________________________________
void SetStrokeParametersFromX11Context(CGContextRef ctx, const GCValues_t &gcVals)
{
   //This is initial version, must be more complex: dashes, joins, caps, etc. TODO.
   //Also, fFunction can affect this, etc.

   assert(ctx != nullptr && "SetStrokeParametersFromX11Context, context parameter is null");

   const Mask_t mask = gcVals.fMask;
   
   if ((mask & kGCLineWidth) && gcVals.fLineWidth > 1)
      CGContextSetLineWidth(ctx, gcVals.fLineWidth);
   else
      CGContextSetLineWidth(ctx, 1.);

   CGFloat rgb[3] = {};
   if (mask & kGCForeground)
      PixelToRGB(gcVals.fForeground, rgb);
   else
      ::Warning("SetStrokeParametersFromX11Context", "x11 context does not have line color information");

   CGContextSetRGBStrokeColor(ctx, rgb[0], rgb[1], rgb[2], 1.f);
}

//______________________________________________________________________________
void SetFilledAreaParametersFromX11Context(CGContextRef ctx, const GCValues_t &gcVals)
{
   //This is initial version, will be more complex - fill stype, patterns, tiles, etc. etc. TODO.

   assert(ctx != nullptr && "SetFilledAreaParametersFromX11Context, context parameter is null");
   
   const Mask_t mask = gcVals.fMask;
   
   CGFloat rgb[3] = {};
   if (mask & kGCForeground)
      PixelToRGB(gcVals.fForeground, rgb);
   else
      ::Warning("SetFilledAreaParametersFromX11Context", "no fill color found in x11 context");
   
   CGContextSetRGBFillColor(ctx, rgb[0], rgb[1], rgb[2], 1.f);
}

//______________________________________________________________________________
void BgraToRgba(unsigned char *data, unsigned w, unsigned h)
{
   //This function works only for TGCocoa::CreatePixmapFromData == number of components, types,
   //byte order, etc. are fixed and known.
   assert(data != nullptr && "BgraToRgba, image data is null");
   assert(w != 0 && "BgraToRgba, image width is 0");
   assert(h != 0 && "BgraToRgba, image height is 0");
   
   unsigned char *p = data;
   for (unsigned i = 0, e = w * h; i < e; ++i, p += 4)
      std::swap(p[0], p[2]);
}

//______________________________________________________________________________
QuartzWindow *CreateTopLevelWindow(Int_t x, Int_t y, UInt_t w, UInt_t h, UInt_t /*border*/, Int_t depth,
                                   UInt_t clss, void */*visual*/, SetWindowAttributes_t *attr, UInt_t)
{
   NSRect winRect = {};
   winRect.origin.x = x; 
   winRect.origin.y = ROOT::MacOSX::X11::GlobalYROOTToCocoa(y);
   winRect.size.width = w;
   winRect.size.height = h;

   //TODO check mask.
   const NSUInteger styleMask = NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask;
   //
   QuartzWindow *newWindow = [[QuartzWindow alloc] initWithContentRect : winRect styleMask : styleMask backing : NSBackingStoreBuffered defer : YES windowAttributes : attr];
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
   
   QuartzView *view = [[QuartzView alloc] initWithFrame : viewRect windowAttributes : attr];
   
   return view;
}

}

//______________________________________________________________________________
TGCocoa::TGCocoa()
            : fSelectedDrawable(0),
              fForegroundProcess(false)
              
{
   try {
      fPimpl.reset(new Details::CocoaPrivate);
   } catch (const std::exception &) {
      throw;
   }
}

//______________________________________________________________________________
TGCocoa::TGCocoa(const char *name, const char *title)
            : TVirtualX(name, title),
              fSelectedDrawable(0),
              fForegroundProcess(false)              
{
   try {
      fPimpl.reset(new Details::CocoaPrivate);
   } catch (const std::exception &) {
      throw;   
   }
}

//______________________________________________________________________________
TGCocoa::~TGCocoa()
{
   //
}

//______________________________________________________________________________
void TGCocoa::GetWindowAttributes(Window_t wid, WindowAttributes_t &attr)
{
   if (!wid)//X11's None?
      return;

   if (fPimpl->IsRootWindow(wid)) {
      //'root' window.
      ROOT::MacOSX::X11::GetRootWindowAttributes(&attr);
   } else {
      id<X11Drawable> window = fPimpl->GetDrawable(wid);
      [window getAttributes : &attr];
   }
}

//______________________________________________________________________________
Bool_t TGCocoa::ParseColor(Colormap_t /*cmap*/, const char *colorName, ColorStruct_t &color)
{
   //"Color" passed as colorName, can be one of the names, defined in X11/rgb.txt,
   //or rgb triplet, which looks like: #rgb #rrggbb #rrrgggbbb #rrrrggggbbbb,
   //where r, g, and b - are hex digits.
   return fPimpl->fX11ColorParser.ParseColor(colorName, color);
}

//______________________________________________________________________________
Bool_t TGCocoa::AllocColor(Colormap_t /*cmap*/, ColorStruct_t &color)
{
   const unsigned red = unsigned(double(color.fRed) / 0xFFFF * 0xFF);
   const unsigned green = unsigned(double(color.fGreen) / 0xFFFF * 0xFF);
   const unsigned blue = unsigned(double(color.fBlue) / 0xFFFF * 0xFF);
   color.fPixel = red << 16 | green << 8 | blue;
   return kTRUE;
}

//______________________________________________________________________________
void TGCocoa::QueryColor(Colormap_t /*cmap*/, ColorStruct_t & color)
{
   // Returns the current RGB value for the pixel in the "color" structure
   color.fRed = (color.fPixel >> 16 & 0xFF) * 0xFFFF / 0xFF;
   color.fGreen = (color.fPixel >> 8 & 0xFF) * 0xFFFF / 0xFF;
   color.fBlue = (color.fPixel & 0xFF) * 0xFFFF / 0xFF;
}

//______________________________________________________________________________
void TGCocoa::NextEvent(Event_t &/*event*/)
{
}

//______________________________________________________________________________
void TGCocoa::GetPasteBuffer(Window_t /*id*/, Atom_t /*atom*/, TString &/*text*/, Int_t &/*nchar*/, Bool_t /*del*/)
{
   // Gets contents of the paste buffer "atom" into the string "text".
   // (nchar = number of characters) If "del" is true deletes the paste
   // buffer afterwards.
}

//______________________________________________________________________________
Bool_t TGCocoa::Init(void * /*display*/)
{
   // Initializes the Cocoa and Quartz system. Returns kFALSE in case of failure.
   return kFALSE;
}

//______________________________________________________________________________
void TGCocoa::ClearWindow()
{
   // Clears the entire area of the current window.
}

//______________________________________________________________________________
void TGCocoa::CloseWindow()
{
   // Deletes current window.
}

//______________________________________________________________________________
void TGCocoa::ClosePixmap()
{

   // Deletes current pixmap.
}

//______________________________________________________________________________
void TGCocoa::CopyPixmap(Int_t wid, Int_t xpos, Int_t ypos)
{
   //const ROOT::MacOSX::Util::AutoreleasePool pool;
   id<X11Drawable> source = fPimpl->GetDrawable(wid);
   assert(source.fIsPixmap == YES && "CopyPixmap, source is not a pixmap");
   
   QuartzPixmap *pixmap = (QuartzPixmap *)source;
   
   id<X11Drawable> window = fPimpl->GetDrawable(fSelectedDrawable);
   
   if (window.fBackBuffer) {      
      CGImageRef image = [pixmap createImageFromPixmap];// CGBitmapContextCreateImage(pixmap.fContext);
      if (image) {
         CGContextRef dstCtx = window.fBackBuffer.fContext;
         assert(dstCtx != nullptr && "CopyPixmap, destination context is null");

         const CGRect imageRect = CGRectMake(xpos, ypos, pixmap.fWidth, pixmap.fHeight);

         CGContextDrawImage(dstCtx, imageRect, image);
         CGContextFlush(dstCtx);

         CGImageRelease(image);
      }
   } else {
      Warning("CopyPixmap", "Operation skipped, since destination window is not double buffered");
   }
}

//______________________________________________________________________________
void TGCocoa::CreateOpenGLContext(Int_t /*wid*/)
{
   // Creates OpenGL context for window "wid"
}

//______________________________________________________________________________
void TGCocoa::DeleteOpenGLContext(Int_t /*wid*/)
{
   // Deletes OpenGL context for window "wid"
}

//______________________________________________________________________________
UInt_t TGCocoa::ExecCommand(TGWin32Command * /*code*/)
{
   // Executes the command "code" coming from the other threads (Win32)
   return 0;
}

//______________________________________________________________________________
Int_t TGCocoa::GetDoubleBuffer(Int_t /*wid*/)
{
   // Queries the double buffer value for the window "wid".
   return 0;
}

//______________________________________________________________________________
void TGCocoa::GetCharacterUp(Float_t &chupx, Float_t &chupy)
{
   // Returns character up vector.
   chupx = chupy = 0;
}

//______________________________________________________________________________
void TGCocoa::GetGeometry(Int_t wid, Int_t & x, Int_t &y, UInt_t &w, UInt_t &h)
{
   if (wid < 0 || fPimpl->IsRootWindow(wid)) {
      //Comment in TVirtualX suggests, that wid can be < 0.
      //This will be screen's geometry.
      WindowAttributes_t attr = {};
      ROOT::MacOSX::X11::GetRootWindowAttributes(&attr);
      x = attr.fX;
      y = attr.fY;
      w = attr.fWidth;
      h = attr.fHeight;
   } else {
      id<X11Drawable> window = fPimpl->GetDrawable(wid);
      x = window.fX;
      y = window.fY;
      w = window.fWidth;
      h = window.fHeight;
   }
}

//______________________________________________________________________________
const char *TGCocoa::DisplayName(const char *)
{
   // Returns hostname on which the display is opened.
   return "dummy";
}

//______________________________________________________________________________
Handle_t  TGCocoa::GetNativeEvent() const
{
   // Returns the current native event handle.
   return 0;
}

//______________________________________________________________________________
ULong_t TGCocoa::GetPixel(Color_t /*cindex*/)
{
   // Returns pixel value associated to specified ROOT color number "cindex".
   return 0;
}

//______________________________________________________________________________
void TGCocoa::GetPlanes(Int_t & /*nplanes*/)
{
   // Returns the maximum number of planes.
}

//______________________________________________________________________________
void TGCocoa::GetRGB(Int_t /*index*/, Float_t &/*r*/, Float_t &/*g*/, Float_t &/*b*/)
{
   // Returns RGB values for color "index".
}


//______________________________________________________________________________
Bool_t TGCocoa::HasTTFonts() const
{
   // Returns True when TrueType fonts are used
   return kFALSE;
}

//______________________________________________________________________________
Window_t TGCocoa::GetWindowID(Int_t wid)
{
   //In case of X11, there is a mixture of 
   //casted X11 ids (Window_t) and index in some internal array (in TGX11), which
   //contains such an id. On Mac, I always have indices. Yes, I'm smart.
   return wid;
}

//______________________________________________________________________________
Int_t TGCocoa::AddWindow(ULong_t /*qwid*/, UInt_t /*w*/, UInt_t /*h*/)
{
   // Registers a window created by Qt as a ROOT window
   //
   // w, h - the width and height, which define the window size
   return 0;
}

//______________________________________________________________________________
Int_t TGCocoa::AddPixmap(ULong_t /*pixind*/, UInt_t /*w*/, UInt_t /*h*/)
{
   // Registers a pixmap created by TGLManager as a ROOT pixmap
   //
   // w, h - the width and height, which define the pixmap size
   return 0;
}


//______________________________________________________________________________
void TGCocoa::RemoveWindow(ULong_t /*qwid*/)
{
   // Removes the created by Qt window "qwid".
}

//______________________________________________________________________________
void TGCocoa::MoveWindow(Int_t wid, Int_t x, Int_t y)
{
   // Moves the window "wid" to the specified x and y coordinates.
   // It does not change the window's size, raise the window, or change
   // the mapping state of the window.
   //
   // x, y - coordinates, which define the new position of the window
   //        relative to its parent.

   if (!wid)//From TGX11.
      return;

   assert(!fPimpl->IsRootWindow(wid) && "MoveWindow, called for 'root' window");

   [fPimpl->GetDrawable(wid) setX : x Y : y];
}

//______________________________________________________________________________
Int_t TGCocoa::OpenPixmap(UInt_t w, UInt_t h)
{
   //Two stage creation.
   NSSize newSize = {};
   newSize.width = w;
   newSize.height = h;

   QuartzPixmap *obj = [QuartzPixmap alloc];
   if (QuartzPixmap *pixmap = [obj initWithW : w H : h]) {
      pixmap.fID = fPimpl->RegisterDrawable(pixmap);
      [pixmap release];
      return (Int_t)pixmap.fID;
   } else {
      Error("OpenPixmap", "Pixmap initialization failed");
      [obj release];
      return -1;
   }
}

//______________________________________________________________________________
void TGCocoa::QueryPointer(Int_t & /*ix*/, Int_t &/*iy*/)
{
   // Returns the pointer position.
}

//______________________________________________________________________________
Pixmap_t TGCocoa::ReadGIF(Int_t /*x0*/, Int_t /*y0*/, const char * /*file*/, Window_t /*id*/)
{
   // If id is NULL - loads the specified gif file at position [x0,y0] in the
   // current window. Otherwise creates pixmap from gif file

   return 0;
}

//______________________________________________________________________________
Int_t TGCocoa::RequestLocator(Int_t /*mode*/, Int_t /*ctyp*/, Int_t &/*x*/, Int_t &/*y*/)
{
   // Requests Locator position.
   // x,y  - cursor position at moment of button press (output)
   // ctyp - cursor type (input)
   //        ctyp = 1 tracking cross
   //        ctyp = 2 cross-hair
   //        ctyp = 3 rubber circle
   //        ctyp = 4 rubber band
   //        ctyp = 5 rubber rectangle
   //
   // mode - input mode
   //        mode = 0 request
   //        mode = 1 sample
   //
   // The returned value is:
   //        in request mode:
   //                       1 = left is pressed
   //                       2 = middle is pressed
   //                       3 = right is pressed
   //        in sample mode:
   //                       11 = left is released
   //                       12 = middle is released
   //                       13 = right is released
   //                       -1 = nothing is pressed or released
   //                       -2 = leave the window
   //                     else = keycode (keyboard is pressed)

   return 0;
}

//______________________________________________________________________________
Int_t TGCocoa::RequestString(Int_t /*x*/, Int_t /*y*/, char * /*text*/)
{
   // Requests string: text is displayed and can be edited with Emacs-like
   // keybinding. Returns termination code (0 for ESC, 1 for RETURN)
   //
   // x,y  - position where text is displayed
   // text - displayed text (as input), edited text (as output)
   return 0;
}

//______________________________________________________________________________
void TGCocoa::RescaleWindow(Int_t /*wid*/, UInt_t /*w*/, UInt_t /*h*/)
{
   // Rescales the window "wid".
   //
   // wid - window identifier
   // w   - the width
   // h   - the heigth
}

//______________________________________________________________________________
Int_t TGCocoa::ResizePixmap(Int_t wid, UInt_t w, UInt_t h)
{
   assert(!fPimpl->IsRootWindow(wid) && "ResizePixmap, called for 'root' window");

   id<X11Drawable> drawable = fPimpl->GetDrawable(wid);
   assert(drawable.fIsPixmap == YES && "ResizePixmap, object is not a pixmap");

   QuartzPixmap *pixmap = (QuartzPixmap *)drawable;
   if (w == pixmap.fWidth && h == pixmap.fHeight)
      return 1;
   
   if ([pixmap resizeW : w H : h])
      return 1;

   return -1;
}

//______________________________________________________________________________
void TGCocoa::ResizeWindow(Int_t wid)
{
   // Resizes the window "wid" if necessary.
   if (!wid)//From TGX11.
      return;
   
   id<X11Drawable> window = fPimpl->GetDrawable(wid);
   if (window.fBackBuffer) {
      int currentDrawable = fSelectedDrawable;
      fSelectedDrawable = wid;
      SetDoubleBufferON();
      fSelectedDrawable = currentDrawable;
   }
}

//______________________________________________________________________________
void TGCocoa::SelectWindow(Int_t wid)
{
   //This function can be called from pad/canvas, both for window and for pixmap.
   //This makes things more difficult, since pixmap has it's own context,
   //not related to context from RootQuartzView's -drawRect method.
   //
   assert(wid > fPimpl->GetRootWindowID() && "SelectWindow, called for 'root' window");

   fSelectedDrawable = wid;
}

//______________________________________________________________________________
void TGCocoa::SelectPixmap(Int_t pixid)
{
   assert(pixid > fPimpl->GetRootWindowID() && "SelectPixmap, 'root' window can not be selected");

   fSelectedDrawable = pixid;
}

//______________________________________________________________________________
void TGCocoa::SetCharacterUp(Float_t /*chupx*/, Float_t /*chupy*/)
{
   // Sets character up vector.
}

//______________________________________________________________________________
void TGCocoa::SetClipOFF(Int_t /*wid*/)
{
   // Turns off the clipping for the window "wid".
}

//______________________________________________________________________________
void TGCocoa::SetClipRegion(Int_t /*wid*/, Int_t /*x*/, Int_t /*y*/, UInt_t /*w*/, UInt_t /*h*/)
{
   // Sets clipping region for the window "wid".
   //
   // wid  - window indentifier
   // x, y - origin of clipping rectangle
   // w, h - the clipping rectangle dimensions

}

//______________________________________________________________________________
void TGCocoa::SetCursor(Int_t /*win*/, ECursor /*cursor*/)
{
   // The cursor "cursor" will be used when the pointer is in the
   // window "wid".
}

//______________________________________________________________________________
void TGCocoa::SetDoubleBuffer(Int_t wid, Int_t mode)
{
   // Sets the double buffer on/off on the window "wid".
   // wid  - window identifier.
   //        999 means all opened windows.
   // mode - the on/off switch
   //        mode = 1 double buffer is on
   //        mode = 0 double buffer is off
   
   assert(wid > fPimpl->GetRootWindowID() && "SetDoubleBuffer called for 'root' window");
   
   if (wid == 999) {
      NSLog(@"***** SET DOUBLE BUFFER FOR ALL WINDOWS *****");
   } else {
      fSelectedDrawable = wid;
      mode ? SetDoubleBufferON() : SetDoubleBufferOFF();
   }   
}

//______________________________________________________________________________
void TGCocoa::SetDoubleBufferOFF()
{
   // Turns double buffer mode off.
   assert(fSelectedDrawable > fPimpl->GetRootWindowID() && "SetDoubleBufferOFF, called, but no correct window was selected before");
   
   id<X11Drawable> obj = fPimpl->GetDrawable(fSelectedDrawable);
   assert(obj.fIsPixmap == NO && "SetDoubleBufferOFF, selected drawable is a pixmap, it can not have a back buffer");
   
   QuartzPixmap *buffer = obj.fBackBuffer;
   assert(buffer != nil && "SetDoubleBufferOFF, window does not have back buffer");

   fPimpl->DeleteDrawable(buffer.fID);
   obj.fBackBuffer = nil;
}

//______________________________________________________________________________
void TGCocoa::SetDoubleBufferON()
{
   // Turns double buffer mode on.
   assert(fSelectedDrawable > fPimpl->GetRootWindowID() && "SetDoubleBufferON, called, but no correct window was selected before");
   
   id<X11Drawable> window = fPimpl->GetDrawable(fSelectedDrawable);
   
   assert(window.fIsPixmap == NO && "SetDoubleBufferON, selected drawable is a pixmap, can not attach pixmap to pixmap");
   
   const unsigned currW = window.fWidth;
   const unsigned currH = window.fHeight;
   
   if (QuartzPixmap *currentPixmap = window.fBackBuffer) {
      if (currH == currentPixmap.fHeight && currW == currentPixmap.fWidth)
         return;
   }
   
   QuartzPixmap *mem = [QuartzPixmap alloc];
   if (!mem) {
      Error("SetDoubleBufferON", "QuartzPixmap alloc failed");
      return;
   }
   
   if (QuartzPixmap *pixmap = [mem initWithW : currW H : currH]) {
      pixmap.fID = fPimpl->RegisterDrawable(pixmap);
      [pixmap release];

      if (window.fBackBuffer)//Now we can delete the old one, since the new was created.
         fPimpl->DeleteDrawable(window.fBackBuffer.fID);

      window.fBackBuffer = pixmap;
   } else {
      [mem dealloc];
      Error("SetDoubleBufferON", "Can't create a pixmap");
   }
}

//______________________________________________________________________________
void TGCocoa::SetDrawMode(EDrawMode /*mode*/)
{
   // Sets the drawing mode.
   //
   // mode = 1 copy
   // mode = 2 xor
   // mode = 3 invert
   // mode = 4 set the suitable mode for cursor echo according to the vendor
}

//______________________________________________________________________________
void TGCocoa::SetRGB(Int_t /*cindex*/, Float_t /*r*/, Float_t /*g*/, Float_t /*b*/)
{
   // Sets color intensities the specified color index "cindex".
   //
   // cindex  - color index
   // r, g, b - the red, green, blue intensities between 0.0 and 1.0
}

//______________________________________________________________________________
void TGCocoa::SetTextMagnitude(Float_t /*mgn*/)
{
   // Sets the current text magnification factor to "mgn"
}

//______________________________________________________________________________
void TGCocoa::Sync(Int_t /*mode*/)
{
   // Set synchronisation on or off.
   // mode : synchronisation on/off
   //    mode=1  on
   //    mode<>0 off
}

//______________________________________________________________________________
void TGCocoa::UpdateWindow(Int_t /*mode*/)
{
   // Updates or synchronises client and server once (not permanent).
   // according to "mode".
   //    mode = 1 update
   //    mode = 0 sync
   assert(fSelectedDrawable > fPimpl->GetRootWindowID() && "UpdateWindow, no window was selected, can not update 'root' window");
   
   id<X11Drawable> window = fPimpl->GetDrawable(fSelectedDrawable);

   if (QuartzPixmap *pixmap = window.fBackBuffer) {
      QuartzView *dstView = window.fContentView;
      assert(dstView != nil && "UpdateWindow, destination view is nil");
      
      if (dstView.fContext) {
         //We can draw directly.
         CGImageRef image = [pixmap createImageFromPixmap];//CGBitmapContextCreateImage(pixmap.fContext);
         if (image) {
            const CGRect imageRect = CGRectMake(0, 0, pixmap.fWidth, pixmap.fHeight);
            CGContextDrawImage(dstView.fContext, imageRect, image);
            CGImageRelease(image);
         }
      } else {
         //Have to wait.
         fPimpl->fX11CommandBuffer.AddUpdateWindow(dstView);
         Update(1);
      }
   }
}

//______________________________________________________________________________
void TGCocoa::Warp(Int_t /*ix*/, Int_t /*iy*/, Window_t /*id*/)
{
   // Sets the pointer position.
   // ix - new X coordinate of pointer
   // iy - new Y coordinate of pointer
   // Coordinates are relative to the origin of the window id
   // or to the origin of the current window if id == 0.
}

//______________________________________________________________________________
Int_t TGCocoa::WriteGIF(char * /*name*/)
{
   // Writes the current window into GIF file.
   // Returns 1 in case of success, 0 otherwise.

   return 0;
}

//______________________________________________________________________________
void TGCocoa::WritePixmap(Int_t /*wid*/, UInt_t /*w*/, UInt_t /*h*/, char * /*pxname*/)
{
   // Writes the pixmap "wid" in the bitmap file "pxname".
   //
   // wid    - the pixmap address
   // w, h   - the width and height of the pixmap.
   // pxname - the file name
}


//GUI
//______________________________________________________________________________
void TGCocoa::MapWindow(Window_t wid)
{
   // Maps the window "wid" and all of its subwindows that have had map
   // requests. This function has no effect if the window is already mapped.
   
   assert(!fPimpl->IsRootWindow(wid) && "MapWindow, called for 'root' window");
   
   if (MakeProcessForeground()) {
      id<X11Drawable> window = fPimpl->GetDrawable(wid);
      [window mapWindow];
   }
}

//______________________________________________________________________________
void TGCocoa::MapSubwindows(Window_t wid)
{
   // Maps all subwindows for the specified window "wid" in top-to-bottom
   // stacking order.   
   
   assert(!fPimpl->IsRootWindow(wid) && "MapSubwindows, called for 'root' window");
   
   if (MakeProcessForeground()) {
      id<X11Drawable> window = fPimpl->GetDrawable(wid);
      [window mapSubwindows];
   }

}

//______________________________________________________________________________
void TGCocoa::MapRaised(Window_t wid)
{
   // Maps the window "wid" and all of its subwindows that have had map
   // requests on the screen and put this window on the top of of the
   // stack of all windows.
   
   assert(!fPimpl->IsRootWindow(wid) && "MapRaised, called for 'root' window");
   //ROOT::MacOSX::Util::AutoreleasePool pool;//TODO

   if (MakeProcessForeground()) {
      id<X11Drawable> window = fPimpl->GetDrawable(wid);
      [window mapRaised];
   }
}

//______________________________________________________________________________
void TGCocoa::UnmapWindow(Window_t wid)
{
   // Unmaps the specified window "wid". If the specified window is already
   // unmapped, this function has no effect. Any child window will no longer
   // be visible (but they are still mapped) until another map call is made
   // on the parent.
   assert(!fPimpl->IsRootWindow(wid) && "UnmapWindow, called for 'root' window");
   
   ROOT::MacOSX::Util::AutoreleasePool pool;//TODO
   
   id<X11Drawable> window = fPimpl->GetDrawable(wid);
   [window unmapWindow];
}

//______________________________________________________________________________
void TGCocoa::DestroyWindow(Window_t wid)
{
   //
   assert(!fPimpl->IsRootWindow(wid) && "DestroyWindow, called for 'root' window");
   
   id<X11Drawable> window = fPimpl->GetDrawable(wid);
   assert(window.fIsPixmap == NO && "DestroyWindow, called for pixmap");
   
   if (window.fBackBuffer)
      fPimpl->DeleteDrawable(window.fBackBuffer.fID);
   
   fPimpl->DeleteDrawable(wid);
}

//______________________________________________________________________________
void TGCocoa::DestroySubwindows(Window_t /*wid*/)
{
   // The DestroySubwindows function destroys all inferior windows of the
   // specified window, in bottom-to-top stacking order.
}

//______________________________________________________________________________
void TGCocoa::RaiseWindow(Window_t /*wid*/)
{
   // Raises the specified window to the top of the stack so that no
   // sibling window obscures it.
}

//______________________________________________________________________________
void TGCocoa::LowerWindow(Window_t /*wid*/)
{
   // Lowers the specified window "wid" to the bottom of the stack so
   // that it does not obscure any sibling windows.
}

//______________________________________________________________________________
void TGCocoa::MoveWindow(Window_t wid, Int_t x, Int_t y)
{
   // Moves the specified window to the specified x and y coordinates.
   // It does not change the window's size, raise the window, or change
   // the mapping state of the window.
   //
   // x, y - coordinates, which define the new position of the window
   //        relative to its parent.
   assert(!fPimpl->IsRootWindow(wid) && "MoveWindow, called for 'root' window");
   
   [fPimpl->GetDrawable(wid) setX : x Y : y];
}

//______________________________________________________________________________
void TGCocoa::MoveResizeWindow(Window_t wid, Int_t x, Int_t y, UInt_t w, UInt_t h)
{
   // Changes the size and location of the specified window "wid" without
   // raising it.
   //
   // x, y - coordinates, which define the new position of the window
   //        relative to its parent.
   // w, h - the width and height, which define the interior size of
   //        the window
   
   if (!wid)//From TGX11.
      return;
   
   assert(!fPimpl->IsRootWindow(wid) && "MoveResizeWindow, called for 'root' window");
   
   [fPimpl->GetDrawable(wid) setX : x Y : y width : w height : h];
}

//______________________________________________________________________________
void TGCocoa::ResizeWindow(Window_t wid, UInt_t w, UInt_t h)
{
   if (!wid)//From TGX11.
      return;

   assert(!fPimpl->IsRootWindow(wid) && "ResizeWindow, called for 'root' window");
   
   const NSSize newSize = {.width = w, .height = h};
   [fPimpl->GetDrawable(wid) setDrawableSize : newSize];
}

//______________________________________________________________________________
void TGCocoa::IconifyWindow(Window_t /*wid*/)
{
   // Iconifies the window "wid".
}
//______________________________________________________________________________
Bool_t TGCocoa::NeedRedraw(ULong_t /*tgwindow*/, Bool_t /*force*/)
{
   // Notify the low level GUI layer ROOT requires "tgwindow" to be
   // updated
   //
   // Returns kTRUE if the notification was desirable and it was sent
   //
   // At the moment only Qt4 layer needs that
   //
   // One needs explicitly cast the first parameter to TGWindow to make
   // it working in the implementation.
   //
   // One needs to process the notification to confine
   // all paint operations within "expose" / "paint" like low level event
   // or equivalent

   return kFALSE;
}

//______________________________________________________________________________
void TGCocoa::ReparentChild(Window_t wid, Window_t pid, Int_t x, Int_t y)
{
   assert(!fPimpl->IsRootWindow(wid) && "ReparentChild, can not re-parent 'root' window");
   
   //TODO: does ROOT cares about reparent X11 events?

   QuartzView *view = fPimpl->GetDrawable(wid).fContentView;
   if (fPimpl->IsRootWindow(pid)) {
      //Make a top-level view from a child view.
      [view retain];
      [view removeFromSuperview];
      view.fParentView = nil;
      
      NSRect frame = view.frame;
      frame.origin = CGPointZero;
      const NSUInteger styleMask = NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask;
      QuartzWindow *newTopLevel = [[QuartzWindow alloc] initWithContentRect : frame styleMask : styleMask backing : NSBackingStoreBuffered defer : NO];
      
      [view setX : x Y : y];
      [newTopLevel setContentView : view];
      fPimpl->ReplaceDrawable(wid, newTopLevel);

      [view updateLevel : 0];

      [view release];
      [newTopLevel release];
   } else {
      [view retain];
      [view removeFromSuperview];
      //
      id<X11Drawable> newParent = fPimpl->GetDrawable(pid);
      assert(newParent.fIsPixmap == NO && "ReparentChild, pixmap can not be a new parent");
      [view setX : x Y : y];
      [newParent addChild : view];
      [view release];
   }
}

//______________________________________________________________________________
void TGCocoa::ReparentTopLevel(Window_t wid, Window_t pid, Int_t x, Int_t y)
{
   assert(0 && "ReparentTopLevel, not implemented yet");
   //I have to delete QuartzWindow here and place in its slot view + 
   //reparent this view into pid.
   if (fPimpl->IsRootWindow(pid))//Nothing to do, wid is already a top-level window.
      return;
   
   QuartzView *contentView = fPimpl->GetDrawable(wid).fContentView;
   [contentView retain];
   QuartzWindow *topLevel = (QuartzWindow *)[contentView window];
   [topLevel setContentView : nil];
   fPimpl->ReplaceDrawable(wid, contentView);
   [contentView setX : x Y : y];
   [fPimpl->GetDrawable(pid) addChild : contentView];
   [contentView release];
}

//______________________________________________________________________________
void TGCocoa::ReparentWindow(Window_t wid, Window_t pid, Int_t x, Int_t y)
{
   // If the specified window is mapped, ReparentWindow automatically
   // performs an UnmapWindow request on it, removes it from its current
   // position in the hierarchy, and inserts it as the child of the specified
   // parent. The window is placed in the stacking order on top with respect
   // to sibling windows.
   
   assert(!fPimpl->IsRootWindow(wid) && "ReparentWindow, can not re-parent 'root' window");

   QuartzView *view = fPimpl->GetDrawable(wid).fContentView;
   if (view.fParentView) {
      //wi
      ReparentChild(wid, pid, x, y);
   } else {
      //wid is a top-level window (or content view of such a window).
      ReparentTopLevel(wid, pid, x, y);
   }
}

//______________________________________________________________________________
void TGCocoa::SetWindowBackground(Window_t wid, ULong_t color)
{
   assert(!fPimpl->IsRootWindow(wid) && "SetWindowBackground, can not set color for 'root' window");

   id<X11Drawable> window = fPimpl->GetDrawable(wid);
   window.fBackgroundPixel = color;
}

//______________________________________________________________________________
void TGCocoa::SetWindowBackgroundPixmap(Window_t /*wid*/, Pixmap_t /*pxm*/)
{
   // Sets the background pixmap of the window "wid" to the specified
   // pixmap "pxm".
}

//______________________________________________________________________________
Int_t TGCocoa::InitWindow(ULong_t parentID)
{
   // Creates a new window and return window number.
   // Returns -1 if window initialization fails.

   WindowAttributes_t attr = {};

   if (fPimpl->IsRootWindow(parentID)) {
      ROOT::MacOSX::X11::GetRootWindowAttributes(&attr);   
   } else {
      id<X11Drawable> parentWin = fPimpl->GetDrawable(parentID);
      [parentWin getAttributes : &attr];
   }

   return CreateWindow(parentID, 0, 0, attr.fWidth, attr.fHeight, 0, attr.fDepth, attr.fClass, nullptr, nullptr, 0);
}

//______________________________________________________________________________
Window_t TGCocoa::CreateWindow(Window_t parentID, Int_t x, Int_t y, UInt_t w, UInt_t h, UInt_t border, Int_t depth,
                               UInt_t clss, void *visual, SetWindowAttributes_t *attr, UInt_t wtype)
{
   //Do not know at the moment, what to do with ALL these possible X11 parameters, which
   //means nothing for Cocoa. TODO: create window correctly to emulate what ROOT wants from TGCocoa.
   //This implementation is just a sketch to try.
   //
   //Check if really need this.
   ROOT::MacOSX::Util::AutoreleasePool pool;
   
   if (fPimpl->IsRootWindow(parentID)) {//parent == root window.
      QuartzWindow *newWindow = CreateTopLevelWindow(x, y, w, h, border, depth, clss, visual, attr, wtype);
      
      const Window_t result = fPimpl->RegisterDrawable(newWindow);
      newWindow.fID = result;
      [newWindow release];//Owned by fPimpl now.

      return result;
   } else {
      id<X11Drawable> parentWin = fPimpl->GetDrawable(parentID);
      
      QuartzView *childView = CreateChildView(parentWin.fContentView, x, y, w, h, border, depth, clss, visual, attr, wtype);
      const Window_t result = fPimpl->RegisterDrawable(childView);
      
      childView.fID = result;
      [parentWin addChild : childView];

      [childView release];//Owned by fPimpl now.

      return result;
   }
}

//______________________________________________________________________________
Int_t TGCocoa::OpenDisplay(const char * /*dpyName*/)
{
   //
   return 0;
}

//______________________________________________________________________________
void TGCocoa::CloseDisplay()
{
   // Closes connection to display server and destroys all windows.
}

//______________________________________________________________________________
Display_t TGCocoa::GetDisplay() const
{
   // Returns handle to display (might be usefull in some cases where
   // direct X11 manipulation outside of TGCocoa is needed, e.g. GL
   // interface).

   return 0;
}

//______________________________________________________________________________
Visual_t TGCocoa::GetVisual() const
{
   // Returns handle to visual.
   //
   // Might be usefull in some cases where direct X11 manipulation outside
   // of TGCocoa is needed, e.g. GL interface.

   return 0;
}

//______________________________________________________________________________
Int_t TGCocoa::GetScreen() const
{
   // Returns screen number.
   //
   // Might be usefull in some cases where direct X11 manipulation outside
   // of TGCocoa is needed, e.g. GL interface.

   return 0;
}

//______________________________________________________________________________
Int_t TGCocoa::GetDepth() const
{
   // Returns depth of screen (number of bit planes).
   // Equivalent to GetPlanes().

   return 0;
}

//______________________________________________________________________________
Colormap_t TGCocoa::GetColormap() const
{
   // Returns handle to colormap.
   //
   // Might be usefull in some cases where direct X11 manipulation outside
   // of TGCocoa is needed, e.g. GL interface.

   return 0;
}

//______________________________________________________________________________
Window_t TGCocoa::GetDefaultRootWindow() const
{
   // Returns handle to the default root window created when calling
   // XOpenDisplay().
   

   return 1;
}

//______________________________________________________________________________
Atom_t  TGCocoa::InternAtom(const char * /*atom_name*/, Bool_t /*only_if_exist*/)
{
   // Returns the atom identifier associated with the specified "atom_name"
   // string. If "only_if_exists" is False, the atom is created if it does
   // not exist. If the atom name is not in the Host Portable Character
   // Encoding, the result is implementation dependent. Uppercase and
   // lowercase matter; the strings "thing", "Thing", and "thinG" all
   // designate different atoms.

   return 0;
}

//______________________________________________________________________________
Window_t TGCocoa::GetParent(Window_t /*wid*/) const
{
   // Returns the parent of the window "wid".

   return 0;
}

//______________________________________________________________________________
FontStruct_t TGCocoa::LoadQueryFont(const char *fontName)
{
   //fontName is in XLFD format:
   //-foundry-family- ..... etc., some components can be omitted and replaced by *.

   ROOT::MacOSX::X11::XLFDName xlfd = {};
   if (ParseXLFDName(fontName, xlfd))
      return fPimpl->fFontManager.LoadFont(xlfd);

   return 0;
}

//______________________________________________________________________________
FontH_t TGCocoa::GetFontHandle(FontStruct_t fs)
{
   return (FontH_t)fs;
}

//______________________________________________________________________________
void TGCocoa::DeleteFont(FontStruct_t fs)
{
   fPimpl->fFontManager.UnloadFont(fs);
}

//______________________________________________________________________________
GContext_t TGCocoa::CreateGC(Drawable_t /*wid*/, GCValues_t *gval)
{
   //Here I have to imitate graphics context that exists in X11.
   fX11Contexts.push_back(*gval);
   return fX11Contexts.size();
}

//______________________________________________________________________________
void TGCocoa::ChangeGC(GContext_t gc, GCValues_t *gval)
{
   //
   assert(gc <= fX11Contexts.size() && gc > 0 && "ChangeGC - stange context id");
   assert(gval != nullptr && "ChangeGC, gval parameter is null");
   
   GCValues_t &x11Context = fX11Contexts[gc - 1];
   const Mask_t &mask = gval->fMask;
   x11Context.fMask |= mask;
   
   //Not all of GCValues_t members are used, but
   //all can be copied/set without any problem.
   
   if (mask & kGCFunction)
      x11Context.fFunction = gval->fFunction;
   if (mask & kGCPlaneMask)
      x11Context.fPlaneMask = gval->fPlaneMask;
   if (mask & kGCForeground)
      x11Context.fForeground = gval->fForeground;
   if (mask & kGCBackground)
      x11Context.fBackground = gval->fBackground;
   if (mask & kGCLineWidth)
      x11Context.fLineWidth = gval->fLineWidth;
   if (mask & kGCLineStyle)
      x11Context.fLineStyle = gval->fLineStyle;
   if (mask & kGCCapStyle)//nobody uses
      x11Context.fCapStyle = gval->fCapStyle;
   if (mask & kGCJoinStyle)//nobody uses
      x11Context.fJoinStyle = gval->fJoinStyle;
   if (mask & kGCFillStyle)
      x11Context.fFillStyle = gval->fFillStyle;
   if (mask & kGCFillRule)//nobody uses
      x11Context.fFillRule = gval->fFillRule;
   if (mask & kGCArcMode)//nobody uses
      x11Context.fArcMode = gval->fArcMode;
   if (mask & kGCTile)
      x11Context.fTile = gval->fTile;
   if (mask & kGCStipple)//nobody
      x11Context.fStipple = gval->fStipple;
   if (mask & kGCTileStipXOrigin)
      x11Context.fTsXOrigin = gval->fTsXOrigin;
   if (mask & kGCTileStipYOrigin)
      x11Context.fTsYOrigin = gval->fTsYOrigin;
   if (mask & kGCFont)
      x11Context.fFont = gval->fFont;
   if (mask & kGCSubwindowMode)
      x11Context.fSubwindowMode = gval->fSubwindowMode;
   if (mask & kGCGraphicsExposures)
      x11Context.fGraphicsExposures = gval->fGraphicsExposures;
   if (mask & kGCClipXOrigin)
      x11Context.fClipXOrigin = gval->fClipXOrigin;
   if (mask & kGCClipYOrigin)
      x11Context.fClipYOrigin = gval->fClipYOrigin;
   if (mask & kGCClipMask)
      x11Context.fClipMask = gval->fClipMask;
   if (mask & kGCDashOffset)
      x11Context.fDashOffset = gval->fDashOffset;
   if (mask & kGCDashList) {
      const unsigned nDashes = sizeof x11Context.fDashes / sizeof x11Context.fDashes[0];
      for (unsigned i = 0; i < nDashes; ++i)
         x11Context.fDashes[i] = gval->fDashes[i];
      x11Context.fDashLen = gval->fDashLen;
   }
}

//______________________________________________________________________________
void TGCocoa::CopyGC(GContext_t src, GContext_t dst, Mask_t mask)
{
   assert(src <= fX11Contexts.size() && src > 0 && "CopyGC, bad source context");   
   assert(dst <= fX11Contexts.size() && dst > 0 && "CopyGC, bad destination context");
   
   GCValues_t srcContext = fX11Contexts[src - 1];
   srcContext.fMask = mask;
   
   ChangeGC(dst, &srcContext);
}

//______________________________________________________________________________
void TGCocoa::DeleteGC(GContext_t /*gc*/)
{
   // Deletes the specified GC "gc".
}

//______________________________________________________________________________
Cursor_t TGCocoa::CreateCursor(ECursor /*cursor*/)
{
   // Creates the specified cursor. (just return cursor from cursor pool).
   // The cursor can be:
   //
   // kBottomLeft, kBottomRight, kTopLeft,  kTopRight,
   // kBottomSide, kLeftSide,    kTopSide,  kRightSide,
   // kMove,       kCross,       kArrowHor, kArrowVer,
   // kHand,       kRotate,      kPointer,  kArrowRight,
   // kCaret,      kWatch

   return 0;
}

//______________________________________________________________________________
void TGCocoa::SetCursor(Window_t /*wid*/, Cursor_t /*curid*/)
{
   // Sets the cursor "curid" to be used when the pointer is in the
   // window "wid".
}

//______________________________________________________________________________
Pixmap_t TGCocoa::CreatePixmap(Drawable_t /*wid*/, UInt_t /*w*/, UInt_t /*h*/)
{
   // Creates a pixmap of the specified width and height and returns
   // a pixmap ID that identifies it.

   return kNone;
}
//______________________________________________________________________________
Pixmap_t TGCocoa::CreatePixmap(Drawable_t /*wid*/, const char * /*bitmap*/,
                               UInt_t /*width*/, UInt_t /*height*/,
                               ULong_t /*forecolor*/, ULong_t /*backcolor*/,
                               Int_t /*depth*/)
{
   // Creates a pixmap from bitmap data of the width, height, and depth you
   // specified and returns a pixmap that identifies it. The width and height
   // arguments must be nonzero. The depth argument must be one of the depths
   // supported by the screen of the specified drawable.
   //
   // wid           - specifies which screen the pixmap is created on
   // bitmap        - the data in bitmap format
   // width, height - define the dimensions of the pixmap
   // forecolor     - the foreground pixel values to use
   // backcolor     - the background pixel values to use
   // depth         - the depth of the pixmap

   return 0;
}

//______________________________________________________________________________
Pixmap_t TGCocoa::CreateBitmap(Drawable_t /*wid*/, const char * /*bitmap*/,
                                 UInt_t /*width*/, UInt_t /*height*/)
{
   // Creates a bitmap (i.e. pixmap with depth 1) from the bitmap data.
   //
   // wid           - specifies which screen the pixmap is created on
   // bitmap        - the data in bitmap format
   // width, height - define the dimensions of the pixmap

   return 0;
}

//______________________________________________________________________________
void TGCocoa::DeletePixmap(Pixmap_t /*pmap*/)
{
   // Explicitely deletes the pixmap resource "pmap".
}

//______________________________________________________________________________
Bool_t TGCocoa::CreatePictureFromFile(Drawable_t /*wid*/,
                                      const char * /*filename*/,
                                      Pixmap_t &/*pict*/,
                                      Pixmap_t &/*pict_mask*/,
                                      PictureAttributes_t &/*attr*/)
{
   // Creates a picture pict from data in file "filename". The picture
   // attributes "attr" are used for input and output. Returns kTRUE in
   // case of success, kFALSE otherwise. If the mask "pict_mask" does not
   // exist it is set to kNone.

   return kFALSE;
}

//______________________________________________________________________________
Bool_t TGCocoa::CreatePictureFromData(Drawable_t /*wid*/, char ** /*data*/,
                                      Pixmap_t &/*pict*/,
                                      Pixmap_t &/*pict_mask*/,
                                      PictureAttributes_t & /*attr*/)
{
   // Creates a picture pict from data in bitmap format. The picture
   // attributes "attr" are used for input and output. Returns kTRUE in
   // case of success, kFALSE otherwise. If the mask "pict_mask" does not
   // exist it is set to kNone.

   return kFALSE;
}
//______________________________________________________________________________
Bool_t TGCocoa::ReadPictureDataFromFile(const char * /*filename*/, char *** /*ret_data*/)
{
   // Reads picture data from file "filename" and store it in "ret_data".
   // Returns kTRUE in case of success, kFALSE otherwise.

   return kFALSE;
}

//______________________________________________________________________________
void TGCocoa::DeletePictureData(void * /*data*/)
{
   // Delete picture data created by the function ReadPictureDataFromFile.
}

//______________________________________________________________________________
void TGCocoa::SetDashes(GContext_t /*gc*/, Int_t /*offset*/, const char * /*dash_list*/, Int_t /*n*/)
{
   // Sets the dash-offset and dash-list attributes for dashed line styles
   // in the specified GC. There must be at least one element in the
   // specified dash_list. The initial and alternating elements (second,
   // fourth, and so on) of the dash_list are the even dashes, and the
   // others are the odd dashes. Each element in the "dash_list" array
   // specifies the length (in pixels) of a segment of the pattern.
   //
   // gc        - specifies the GC (see GCValues_t structure)
   // offset    - the phase of the pattern for the dashed line-style you
   //             want to set for the specified GC.
   // dash_list - the dash-list for the dashed line-style you want to set
   //             for the specified GC
   // n         - the number of elements in dash_list
   // (see also the GCValues_t structure)
}

//______________________________________________________________________________
void TGCocoa::FreeColor(Colormap_t /*cmap*/, ULong_t /*pixel*/)
{
   // Frees color cell with specified pixel value.
}

//______________________________________________________________________________
Int_t TGCocoa::EventsPending()
{
   // Returns the number of events that have been received from the X server
   // but have not been removed from the event queue.
   return 0;
}

//______________________________________________________________________________
void TGCocoa::Bell(Int_t /*percent*/)
{
   // Sets the sound bell. Percent is loudness from -100% .. 100%.
}

//______________________________________________________________________________
void TGCocoa::ChangeWindowAttributes(Window_t wid, SetWindowAttributes_t *attr)
{
   if (!wid)//From TGX11
      return;

   assert(!fPimpl->IsRootWindow(wid) && "ChangeWindowAttributes, called for the 'root' window");
   assert(attr != nullptr && "ChangeWindowAttributes, attr parameter is null");
   
   [fPimpl->GetDrawable(wid) setAttributes : attr];
}

//______________________________________________________________________________
void TGCocoa::ChangeProperty(Window_t /*wid*/, Atom_t /*property*/,
                             Atom_t /*type*/, UChar_t * /*data*/,
                             Int_t /*len*/)
{
   // Alters the property for the specified window and causes the X server
   // to generate a PropertyNotify event on that window.
   //
   // wid       - the window whose property you want to change
   // property - specifies the property name
   // type     - the type of the property; the X server does not
   //            interpret the type but simply passes it back to
   //            an application that might ask about the window
   //            properties
   // data     - the property data
   // len      - the length of the specified data format
}

//
//GUI rendering part.
//

//______________________________________________________________________________
void TGCocoa::DrawLineAux(Drawable_t wid, const GCValues_t &gcVals, Int_t x1, Int_t y1, Int_t x2, Int_t y2)
{
   //Can be called directly of when flushing command buffer.
   assert(!fPimpl->IsRootWindow(wid) && "DrawLineAux, called for 'root' window");
   
   QuartzView *view = fPimpl->GetDrawable(wid).fContentView;

   const CGStateGuard ctxGuard(view.fContext);//Will restore state back.
   //Draw a line.
   //This draw line is a special GUI method, it's used not by ROOT's graphics, but
   //widgets. The problem is:
   //-I have to switch off anti-aliasing, since if anti-aliasing is on,
   //the line is thick and has different color.
   //-As soon as I switch-off anti-aliasing, and line is precise, I can not
   //draw a line [0, 0, -> w, 0].
   //I use a small translation, after all, this is ONLY gui method and it
   //will not affect anything except GUI.
   CGContextTranslateCTM(view.fContext, 0.f, 1.);
   CGContextSetAllowsAntialiasing(view.fContext, 0);//Smoothed line is of wrong color and in a wrong position - this is bad for GUI.

   SetStrokeParametersFromX11Context(view.fContext, gcVals);
   CGContextSetLineWidth(view.fContext, 1.);   
   CGContextBeginPath(view.fContext);
   CGContextMoveToPoint(view.fContext, x1, y1);
   CGContextAddLineToPoint(view.fContext, x2, y2);
   CGContextStrokePath(view.fContext);
}

//______________________________________________________________________________
void TGCocoa::DrawLine(Drawable_t wid, GContext_t gc, Int_t x1, Int_t y1, Int_t x2, Int_t y2)
{
   //This function can be called:
   //a)'normal' way - from view's drawRect method.
   //b) for 'direct rendering' - operation was initiated by ROOT's GUI, not by 
   //   drawRect.   
   //This code is just a hack to show a button or other widgets.
   
   if (!wid) //From TGX11.
      return;
   
   assert(!fPimpl->IsRootWindow(wid) && "DrawLine, called for 'root' window");   
   assert(gc > 0 && gc <= fX11Contexts.size() && "DrawLine, strange context index");

   const GCValues_t &gcVals = fX11Contexts[gc - 1];
   QuartzView *view = fPimpl->GetDrawable(wid).fContentView;
   
   if (!view.fContext) {
      fPimpl->fX11CommandBuffer.AddDrawLine(wid, gcVals, x1, y1, x2, y2);
      return;
   }

   DrawLineAux(wid, gcVals, x1, y1, x2, y2);
}

//______________________________________________________________________________
void TGCocoa::DrawRectangleAux(Drawable_t wid, const GCValues_t &gcVals, Int_t x, Int_t y, UInt_t w, UInt_t h)
{
   //Can be called directly or during flushing command buffer.
   assert(!fPimpl->IsRootWindow(wid) && "DrawRectangleAux, called for the 'root' window");

   QuartzView *view = fPimpl->GetDrawable(wid).fContentView;
   
   const CGStateGuard ctxGuard(view.fContext);//Will restore context state.

   //Line color from X11 context.
   SetStrokeParametersFromX11Context(view.fContext, gcVals);
      
   const CGRect rect = CGRectMake(x, y, w, h);
   CGContextStrokeRect(view.fContext, rect);
}


//______________________________________________________________________________
void TGCocoa::DrawRectangle(Drawable_t wid, GContext_t gc, Int_t x, Int_t y, UInt_t w, UInt_t h)
{
   //Can be called in a 'normal way' - from drawRect method (QuartzView)
   //or directly by ROOT.
   
   if (!wid)//From X11.
      return;

   assert(!fPimpl->IsRootWindow(wid) && "DrawRectangle, called for the 'root' window");
   assert(gc > 0 && gc <= fX11Contexts.size() && "DrawRectangle, bad GContext_t");

   const GCValues_t &gcVals = fX11Contexts[gc - 1];
   QuartzView *view = fPimpl->GetDrawable(wid).fContentView;
   if (!view.fContext) {
      fPimpl->fX11CommandBuffer.AddDrawRectangle(wid, gcVals, x, y, w, h);
      return;
   }

   DrawRectangleAux(wid, gcVals, x, y, w, h);
}

//______________________________________________________________________________
void TGCocoa::FillRectangleAux(Drawable_t wid, const GCValues_t &gcVals, Int_t x, Int_t y, UInt_t w, UInt_t h)
{
   //Can be called directly or when flushing command buffer.
   if (!wid)//From TGX11.
      return;

   assert(!fPimpl->IsRootWindow(wid) && "FillRectangleAux, called for the 'root' window");

   QuartzView *view = fPimpl->GetDrawable(wid).fContentView;
   const CGStateGuard ctxGuard(view.fContext);//Will restore context state.
   //Fill color from X11 context.
   SetFilledAreaParametersFromX11Context(view.fContext, gcVals);
   const CGRect fillRect = CGRectMake(x, y, w, h);
   CGContextFillRect(view.fContext, fillRect);
}

//______________________________________________________________________________
void TGCocoa::FillRectangle(Drawable_t wid, GContext_t gc, Int_t x, Int_t y, UInt_t w, UInt_t h)
{
   //Can be called in a 'normal way' - from drawRect method (QuartzView)
   //or directly by ROOT.

   if (!wid)//From TGX11.
      return;

   assert(!fPimpl->IsRootWindow(wid) && "FillRectangle, called for the 'root' window");
   assert(gc > 0 && gc <= fX11Contexts.size() && "FillRectangle, bad GContext_t");

   const GCValues_t &gcVals = fX11Contexts[gc - 1];   
   QuartzView *view = fPimpl->GetDrawable(wid).fContentView;
   if (!view.fContext) {
      fPimpl->fX11CommandBuffer.AddFillRectangle(wid, gcVals, x, y, w, h);
      return;
   }
   
   FillRectangleAux(wid, gcVals, x, y, w, h);
}

//______________________________________________________________________________
void TGCocoa::CopyAreaAux(Drawable_t src, Drawable_t dst, const GCValues_t &/*gcVals*/, Int_t srcX, Int_t srcY, UInt_t width, UInt_t height, Int_t dstX, Int_t dstY)
{
   //Called directly or when flushing command buffer.
   if (!src || !dst)//Can this happen? From TGX11.
      return;
      
   assert(!fPimpl->IsRootWindow(src) && "CopyAreaAux, src parameter is 'root' window");
   assert(!fPimpl->IsRootWindow(dst) && "CopyAreaAux, dst parameter is 'root' window");
   
   id<X11Drawable> srcDrawable = fPimpl->GetDrawable(src);
   id<X11Drawable> dstDrawable = fPimpl->GetDrawable(dst);
   
   Point_t dstPoint = {};
   dstPoint.fX = dstX;
   dstPoint.fY = dstY;
   
   Rectangle_t copyArea = {};
   copyArea.fX = srcX;
   copyArea.fY = srcY;
   copyArea.fWidth = (UShort_t)width;//!
   copyArea.fHeight = (UShort_t)height;//!

   //Check gc also???
   [dstDrawable copy : srcDrawable area : copyArea toPoint : dstPoint];
}

//______________________________________________________________________________
void TGCocoa::CopyArea(Drawable_t src, Drawable_t dst, GContext_t /*gc*/, Int_t srcX, Int_t srcY, UInt_t width, UInt_t height, Int_t dstX, Int_t dstY)
{
   if (!src || !dst)//Can this happen? From TGX11.
      return;
      
   assert(!fPimpl->IsRootWindow(src) && "CopyArea, src parameter is 'root' window");
   assert(!fPimpl->IsRootWindow(dst) && "CopyArea, dst parameter is 'root' window");
   
   id<X11Drawable> srcDrawable = fPimpl->GetDrawable(src);
   id<X11Drawable> dstDrawable = fPimpl->GetDrawable(dst);
   
   QuartzView *view = nil;
   if ([(NSObject *)dstDrawable isKindOfClass : [QuartzView class]] || [(NSObject *)dstDrawable isKindOfClass : [QuartzWindow class]])
      view = dstDrawable.fContentView;
   
   if (view && !view.fContext) {
      fPimpl->fX11CommandBuffer.AddCopyArea(src, dst, GCValues_t(), srcX, srcY, width, height, dstX, dstY);
      return;
   }
   
   Point_t dstPoint = {};
   dstPoint.fX = dstX;
   dstPoint.fY = dstY;
   
   Rectangle_t copyArea = {};
   copyArea.fX = srcX;
   copyArea.fY = srcY;
   copyArea.fWidth = (UShort_t)width;//!
   copyArea.fHeight = (UShort_t)height;//!

   //Check gc also???
   [dstDrawable copy : srcDrawable area : copyArea toPoint : dstPoint];
}

//______________________________________________________________________________
void TGCocoa::DrawStringAux(Drawable_t wid, const GCValues_t &gcVals, Int_t x, Int_t y, const char *text, Int_t len)
{
   //Can be called by ROOT directly, or indirectly by AppKit.
   using namespace ROOT::MacOSX::X11;

   assert(!fPimpl->IsRootWindow(wid) && "DrawStringAux, called for the 'root' window");

   QuartzView *view = fPimpl->GetDrawable(wid).fContentView;   

   const CGStateGuard ctxGuard(view.fContext);//Will reset parameters back.

   CGContextSetTextMatrix(view.fContext, CGAffineTransformIdentity);
   
   //View is flipped, I have to transform for Core Text to work.
   CGContextTranslateCTM(view.fContext, 0.f, view.fHeight);
   CGContextScaleCTM(view.fContext, 1.f, -1.f);   

   //Text must be antialiased.
   CGContextSetAllowsAntialiasing(view.fContext, 1);
      
   assert(gcVals.fMask & kGCFont && "DrawString, font is not set in a context");

   if (len < 0)//Negative length can come from caller.
      len = std::strlen(text);
   const std::string substr(text, len);
      
   //Text can be not black, for example, highlighted label.
   CGFloat textColor[4] = {0., 0., 0., 1.};//black by default.
   //I do not check the results here, it's ok to have a black text.
   if (gcVals.fMask & kGCForeground)
      PixelToRGB(gcVals.fForeground, textColor);

   ROOT::Quartz::TextLine ctLine(substr.c_str(), (CTFontRef)gcVals.fFont, textColor);

   CGContextSetTextPosition(view.fContext, x, LocalYROOTToCocoa(view, y));
   ctLine.DrawLine(view.fContext);
}

//______________________________________________________________________________
void TGCocoa::DrawString(Drawable_t wid, GContext_t gc, Int_t x, Int_t y, const char *text, Int_t len)
{
   //Can be called by ROOT directly, or indirectly by AppKit.

   if (!wid)//from TGX11.
      return;

   assert(!fPimpl->IsRootWindow(wid) && "DrawString, called for the 'root' window");
   assert(gc > 0 && gc <= fX11Contexts.size() && "DrawString, bad GContext_t");

   QuartzView *view = fPimpl->GetDrawable(wid).fContentView;
   const GCValues_t &gcVals = fX11Contexts[gc - 1];
   assert(gcVals.fMask & kGCFont && "DrawString, font is not set in a context");
   if (!view.fContext) {
      fPimpl->fX11CommandBuffer.AddDrawString(wid, gcVals, x, y, text, len);
      return;
   }
   
   DrawStringAux(wid, gcVals, x, y, text, len);
}

//______________________________________________________________________________
void TGCocoa::ClearArea(Window_t wid, Int_t x, Int_t y, UInt_t w, UInt_t h)
{
   //Can be called from drawRect method and also by ROOT's GUI directly.

   assert(!fPimpl->IsRootWindow(wid) && "ClearArea, called for the 'root' window");
   using namespace ROOT::MacOSX::X11;
   
   QuartzView *view = fPimpl->GetDrawable(wid).fContentView;
   if (!view.fContext) {
      fPimpl->fX11CommandBuffer.AddClearArea(wid, x, y, w, h);
      return;
   }

   //TODO: remove this crap and do it right!!!
   const Pixel_t color = view.fBackgroundPixel;
   const CGFloat red   = ((color & 0xFF0000) >> 16) / 255.f;
   const CGFloat green = ((color & 0xFF00) >> 8) / 255.f;
   const CGFloat blue  = (color & 0xFF) / 255.f;
   
   const CGStateGuard ctxGuard(view.fContext);
   CGContextSetRGBFillColor(view.fContext, red, green, blue, 1.f);//alpha can be also used.
   CGContextFillRect(view.fContext, CGRectMake(x, y, w, h));
}

//
//End of GUI rendering part.
//


//______________________________________________________________________________
Bool_t TGCocoa::CheckEvent(Window_t /*wid*/, EGEventType /*type*/, Event_t & /*ev*/)
{
   //No need in this.
   return kFALSE;
}

//______________________________________________________________________________
void TGCocoa::SendEvent(Window_t wid, Event_t *event)
{
   // Specifies the event "ev" is to be sent to the window "wid".
   // This function requires you to pass an event mask.
   assert(!fPimpl->IsRootWindow(wid) && "SendEvent, can not send event to a root window");
   assert(event != nullptr && "SendEvent, event parameter is null");
   
   id<X11Drawable> widget = fPimpl->GetDrawable(wid);
   assert(widget.fID != 0 && "SendEvent, widget.fID is 0");
   
   TGWindow *window = gClient->GetWindowById(wid);
   assert(window != nullptr && "SendEvent, no window was found");
   window->HandleEvent(event);
}

//______________________________________________________________________________
void TGCocoa::WMDeleteNotify(Window_t /*wid*/)
{
   // Tells WM to send message when window is closed via WM.
}

//______________________________________________________________________________
void TGCocoa::SetKeyAutoRepeat(Bool_t /*on = kTRUE*/)
{
   // Turns key auto repeat on (kTRUE) or off (kFALSE).
}

//______________________________________________________________________________
void TGCocoa::GrabKey(Window_t /*wid*/, Int_t /*keycode*/, UInt_t /*modifier*/,
                        Bool_t /*grab = kTRUE*/)
{
   // Establishes a passive grab on the keyboard. In the future, the
   // keyboard is actively grabbed, the last-keyboard-grab time is set
   // to the time at which the key was pressed (as transmitted in the
   // KeyPress event), and the KeyPress event is reported if all of the
   // following conditions are true:
   //    - the keyboard is not grabbed and the specified key (which can
   //      itself be a modifier key) is logically pressed when the
   //      specified modifier keys are logically down, and no other
   //      modifier keys are logically down;
   //    - either the grab window "id" is an ancestor of (or is) the focus
   //      window, or "id" is a descendant of the focus window and contains
   //      the pointer;
   //    - a passive grab on the same key combination does not exist on any
   //      ancestor of grab_window
   //
   // id       - window id
   // keycode  - specifies the KeyCode or AnyKey
   // modifier - specifies the set of keymasks or AnyModifier; the mask is
   //            the bitwise inclusive OR of the valid keymask bits
   // grab     - a switch between grab/ungrab key
   //            grab = kTRUE  grab the key and modifier
   //            grab = kFALSE ungrab the key and modifier
}

//______________________________________________________________________________
void TGCocoa::GrabButton(Window_t wid, EMouseButton button, UInt_t keyModifiers, UInt_t eventMask,
                         Window_t /*confine*/, Cursor_t /*cursor*/, Bool_t grab)
{
   //Emulate "passive grab" feature of X11 (similar to "implicit grab" in Cocoa
   //and implicit grab on X11, the difference is that "implicit grab" works as
   //if owner_events parameter for XGrabButton was False, but in ROOT
   //owner_events for XGrabButton is _always_ True.
   //Confine will never be used - no such feature on MacOSX and
   //I'm not going to emulate it..
   //This function also does ungrab.
   
   assert(!fPimpl->IsRootWindow(wid) && "GrabButton, called for 'root' window");
   
   id<X11Drawable> widget = fPimpl->GetDrawable(wid);
   
   if (grab) {
      widget.fOwnerEvents = YES;   //This is how TGX11 works.
      widget.fGrabButton = button;
      widget.fGrabButtonEventMask = eventMask;
      widget.fGrabKeyModifiers = keyModifiers;
      //Set the cursor.
   } else {
      widget.fOwnerEvents = NO;
      widget.fGrabButton = -1;//0 is kAnyButton.
      widget.fGrabButtonEventMask = 0;
      widget.fGrabKeyModifiers = 0;
   }
}

//______________________________________________________________________________
void TGCocoa::GrabPointer(Window_t wid, UInt_t eventMask, Window_t /*confine*/, Cursor_t /*cursor*/, Bool_t grab, Bool_t ownerEvents)
{
   //Emulate pointer grab from X11.
   //Confine will never be used - no such feature on MacOSX and
   //I'm not going to emulate it..
   //This function also does ungrab.
   if (grab) {
      QuartzView *view = fPimpl->GetDrawable(wid).fContentView;
      assert(!fPimpl->IsRootWindow(wid) && "GrabPointer, called for 'root' window");
      //set the cursor.
      //set active grab.
      fPimpl->fX11EventTranslator.SetPointerGrab(view, eventMask, ownerEvents);
   } else {
      //unset cursor?
      //cancel grab.
      fPimpl->fX11EventTranslator.CancelPointerGrab();
   }
}

//______________________________________________________________________________
void TGCocoa::SetWindowName(Window_t wid, char *name)
{
   if (!wid || !name)//This can happen.
      return;
   
   id<X11Drawable> widget = fPimpl->GetDrawable(wid);
   if ([(NSObject *)widget isKindOfClass : [NSWindow class]]) {
      const ROOT::MacOSX::Util::AutoreleasePool pool;
      NSString *windowTitle = [NSString stringWithCString : name encoding : NSASCIIStringEncoding];
      [(NSWindow *)widget setTitle : windowTitle];
   } else
      Error("SetWindowName", "Window for id %lu is not an NSWindow object", wid);
}

//______________________________________________________________________________
void TGCocoa::SetIconName(Window_t /*wid*/, char * /*name*/)
{
   // Sets the window icon name.
}

//______________________________________________________________________________
void TGCocoa::SetIconPixmap(Window_t /*wid*/, Pixmap_t /*pix*/)
{
   // Sets the icon name pixmap.
}

//______________________________________________________________________________
void TGCocoa::SetClassHints(Window_t /*wid*/, char * /*className*/,
                              char * /*resourceName*/)
{
   // Sets the windows class and resource name.
}

//______________________________________________________________________________
void TGCocoa::SetMWMHints(Window_t /*wid*/, UInt_t /*value*/, UInt_t /*funcs*/,
                            UInt_t /*input*/)
{
   // Sets decoration style.
}

//______________________________________________________________________________
void TGCocoa::SetWMPosition(Window_t /*wid*/, Int_t /*x*/, Int_t /*y*/)
{
   // Tells the window manager the desired position [x,y] of window "wid".
}

//______________________________________________________________________________
void TGCocoa::SetWMSize(Window_t /*wid*/, UInt_t /*w*/, UInt_t /*h*/)
{
   // Tells window manager the desired size of window "wid".
   //
   // w - the width
   // h - the height
}

//______________________________________________________________________________
void TGCocoa::SetWMSizeHints(Window_t /*wid*/, UInt_t /*wmin*/, UInt_t /*hmin*/,
                               UInt_t /*wmax*/, UInt_t /*hmax*/,
                               UInt_t /*winc*/, UInt_t /*hinc*/)
{
   // Gives the window manager minimum and maximum size hints of the window
   // "wid". Also specify via "winc" and "hinc" the resize increments.
   //
   // wmin, hmin - specify the minimum window size
   // wmax, hmax - specify the maximum window size
   // winc, hinc - define an arithmetic progression of sizes into which
   //              the window to be resized (minimum to maximum)
}

//______________________________________________________________________________
void TGCocoa::SetWMState(Window_t /*wid*/, EInitialState /*state*/)
{
   // Sets the initial state of the window "wid": either kNormalState
   // or kIconicState.
}

//______________________________________________________________________________
void TGCocoa::SetWMTransientHint(Window_t /*wid*/, Window_t /*main_id*/)
{
   // Tells window manager that the window "wid" is a transient window
   // of the window "main_id". A window manager may decide not to decorate
   // a transient window or may treat it differently in other ways.
}

//______________________________________________________________________________
Int_t TGCocoa::TextWidth(FontStruct_t font, const char *s, Int_t len)
{
   // Return lenght of the string "s" in pixels. Size depends on font.
   return fPimpl->fFontManager.GetTextWidth(font, s, len) + 1;
}

//______________________________________________________________________________
void TGCocoa::GetFontProperties(FontStruct_t font, Int_t &maxAscent, Int_t &maxDescent)
{
   // Returns the font properties.
   fPimpl->fFontManager.GetFontProperties(font, maxAscent, maxDescent);
}

//______________________________________________________________________________
void TGCocoa::GetGCValues(GContext_t gc, GCValues_t &gval)
{
   // Returns the components specified by the mask in "gval" for the
   // specified GC "gc" (see also the GCValues_t structure)
   const GCValues_t &gcVal = fX11Contexts[gc - 1];
   gval = gcVal;
}

//______________________________________________________________________________
FontStruct_t TGCocoa::GetFontStruct(FontH_t fh)
{
   // Retrieves the associated font structure of the font specified font
   // handle "fh".
   //
   // Free returned FontStruct_t using FreeFontStruct().

   return fh;
}

//______________________________________________________________________________
void TGCocoa::FreeFontStruct(FontStruct_t /*fs*/)
{
   // Frees the font structure "fs". The font itself will be freed when
   // no other resource references it.

   //
   // fFontManager->UnloadFont(fs);
}

//______________________________________________________________________________
void TGCocoa::ClearWindow(Window_t /*wid*/)
{
   // Clears the entire area in the specified window and it is equivalent to
   // ClearArea(id, 0, 0, 0, 0)
}

//______________________________________________________________________________
Int_t TGCocoa::KeysymToKeycode(UInt_t /*keysym*/)
{
   // Converts the "keysym" to the appropriate keycode. For example,
   // keysym is a letter and keycode is the matching keyboard key (which
   // is dependend on the current keyboard mapping). If the specified
   // "keysym" is not defined for any keycode, returns zero.

   return 0;
}

//______________________________________________________________________________
void TGCocoa::DrawSegments(Drawable_t /*wid*/, GContext_t /*gc*/,
                             Segment_t * /*seg*/, Int_t /*nseg*/)
{
   // Draws multiple line segments. Each line is specified by a pair of points.
   // Segment_t *seg - specifies an array of segments
   // Int_t nseg     - specifies the number of segments in the array
   //
   // GC components in use: function, plane-mask, line-width, line-style,
   // cap-style, join-style, fill-style, subwindow-mode, clip-x-origin,
   // clip-y-origin, clip-mask.
   // GC mode-dependent components: foreground, background, tile, stipple,
   // tile-stipple-x-origin, tile-stipple-y-origin, dash-offset, and dash-list.
   // (see also the GCValues_t structure)
}

//______________________________________________________________________________
void TGCocoa::SelectInput(Window_t wid, UInt_t evmask)
{
   // Defines which input events the window is interested in. By default
   // events are propageted up the window stack. This mask can also be
   // set at window creation time via the SetWindowAttributes_t::fEventMask
   // attribute.
   
   assert(!fPimpl->IsRootWindow(wid) && "SelectInput, called for 'root' window");
   
   id<X11Drawable> window = fPimpl->GetDrawable(wid);
   //XSelectInput overrides previous mask.
   window.fEventMask = evmask;
}

//______________________________________________________________________________
Window_t TGCocoa::GetInputFocus()
{
   // Returns the window id of the window having the input focus.

   return kNone;
}

//______________________________________________________________________________
void TGCocoa::SetInputFocus(Window_t /*wid*/)
{
   // Changes the input focus to specified window "wid".
}

//______________________________________________________________________________
Window_t TGCocoa::GetPrimarySelectionOwner()
{
   // Returns the window id of the current owner of the primary selection.
   // That is the window in which, for example some text is selected.

   return kNone;
}

//______________________________________________________________________________
void TGCocoa::SetPrimarySelectionOwner(Window_t /*wid*/)
{
   // Makes the window "wid" the current owner of the primary selection.
   // That is the window in which, for example some text is selected.
}

//______________________________________________________________________________
void TGCocoa::ConvertPrimarySelection(Window_t /*wid*/, Atom_t /*clipboard*/, Time_t /*when*/)
{
   // Causes a SelectionRequest event to be sent to the current primary
   // selection owner. This event specifies the selection property
   // (primary selection), the format into which to convert that data before
   // storing it (target = XA_STRING), the property in which the owner will
   // place the information (sel_property), the window that wants the
   // information (id), and the time of the conversion request (when).
   // The selection owner responds by sending a SelectionNotify event, which
   // confirms the selected atom and type.
}

//______________________________________________________________________________
void TGCocoa::LookupString(Event_t * /*event*/, char * /*buf*/, Int_t /*buflen*/, UInt_t &keysym)
{
   // Converts the keycode from the event structure to a key symbol (according
   // to the modifiers specified in the event structure and the current
   // keyboard mapping). In "buf" a null terminated ASCII string is returned
   // representing the string that is currently mapped to the key code.
   //
   // event  - specifies the event structure to be used
   // buf    - returns the translated characters
   // buflen - the length of the buffer
   // keysym - returns the "keysym" computed from the event
   //          if this argument is not NULL

   keysym = 0;
}

//______________________________________________________________________________
void TGCocoa::TranslateCoordinates(Window_t srcWin, Window_t dstWin, Int_t srcX, Int_t srcY, Int_t &dstX, Int_t &dstY, Window_t &child)
{
   // Translates coordinates in one window to the coordinate space of another
   // window. It takes the "src_x" and "src_y" coordinates relative to the
   // source window's origin and returns these coordinates to "dest_x" and
   // "dest_y" relative to the destination window's origin.

   // child          - returns the child of "dest" if the coordinates
   //                  are contained in a mapped child of the destination
   //                  window; otherwise, child is set to 0

   namespace X11 = ROOT::MacOSX::X11;

   child = 0;   
   if (!srcWin || !dstWin)//This is from TGX11, looks like this can happen.
      return;
   
   const bool srcIsRoot = fPimpl->IsRootWindow(srcWin);
   const bool dstIsRoot = fPimpl->IsRootWindow(dstWin);
   
   if (srcIsRoot && dstIsRoot) {
      //TODO: can this ever happen? If this is ok, I'll simply remove warning.
      Warning("TranslateCoordinates", "both source and destination windows are 'root' window");
      return;
   }
   
   NSPoint srcPoint = {};
   srcPoint.x = srcX;
   srcPoint.y = srcY;

   NSPoint dstPoint = {};


   if (dstIsRoot) {
      QuartzView *srcView = fPimpl->GetDrawable(srcWin).fContentView;
      dstPoint = X11::TranslateToScreen(srcView, srcPoint);
   } else if (srcIsRoot) {
      QuartzView *dstView = fPimpl->GetDrawable(dstWin).fContentView;
      dstPoint = X11::TranslateFromScreen(srcPoint, dstView);

      if ([dstView superview]) {
         //hitTest requires a point in a superview's coordinate system.
         //Even contentView of QuartzWindow has a superview (NSThemeFrame),
         //so this should always work.
         dstPoint = [[dstView superview] convertPoint : dstPoint fromView : dstView];
         if (QuartzView *view = (QuartzView *)[dstView hitTest : dstPoint]) {
            if (view != dstView && view.fMapState == kIsViewable)
               child = view.fID;
         }
      }
   } else {
      QuartzView *srcView = fPimpl->GetDrawable(srcWin).fContentView;
      QuartzView *dstView = fPimpl->GetDrawable(dstWin).fContentView;

      dstPoint = X11::TranslateCoordinates(srcView, dstView, srcPoint);
      if ([dstView superview]) {
         //hitTest requires a point in a view's superview coordinate system.
         //Even contentView of QuartzWindow has a superview (NSThemeFrame),
         //so this should always work.
         dstPoint = [[dstView superview] convertPoint : dstPoint fromView : dstView];
         if (QuartzView *view = (QuartzView *)[dstView hitTest : dstPoint]) {
            if (view != dstView && view.fMapState == kIsViewable)
               child = view.fID;
         }
      }
   }

   dstX = dstPoint.x;
   dstY = dstPoint.y;
}

//______________________________________________________________________________
void TGCocoa::GetWindowSize(Drawable_t wid, Int_t &x, Int_t &y, UInt_t &w, UInt_t &h)
{
   // Returns the location and the size of window "wid"
   //
   // x, y - coordinates of the upper-left outer corner relative to the
   //        parent window's origin
   // w, h - the inside size of the window, not including the border
   //Crappy GUI classes can use rootID and 0?

   if (!wid)//As it is in GX11Gui.cxx.
      return;
   
   if (fPimpl->IsRootWindow(wid)) {
      WindowAttributes_t attr = {};
      ROOT::MacOSX::X11::GetRootWindowAttributes(&attr);
      x = attr.fX;
      y = attr.fY;
      w = attr.fWidth;
      h = attr.fHeight;
   } else {
      id<X11Drawable> window = fPimpl->GetDrawable(wid);
      //ROOT can ask window size for ... non-window drawable.
      if (!window.fIsPixmap) {
         x = window.fX;
         y = window.fY;
      } else {
         x = 0;
         y = 0;
      }

      w = window.fWidth;
      h = window.fHeight;
   }
}

//______________________________________________________________________________
void TGCocoa::FillPolygon(Window_t /*wid*/, GContext_t /*gc*/, Point_t * /*points*/, Int_t /*npnt*/) 
{
   // Fills the region closed by the specified path. The path is closed
   // automatically if the last point in the list does not coincide with the
   // first point.
   //
   // Point_t *points - specifies an array of points
   // Int_t npnt      - specifies the number of points in the array
   //
   // GC components in use: function, plane-mask, fill-style, fill-rule,
   // subwindow-mode, clip-x-origin, clip-y-origin, and clip-mask.  GC
   // mode-dependent components: foreground, background, tile, stipple,
   // tile-stipple-x-origin, and tile-stipple-y-origin.
   // (see also the GCValues_t structure)
}

//______________________________________________________________________________
void TGCocoa::QueryPointer(Window_t /*wid*/, Window_t &/*rootw*/, Window_t &/*childw*/,
                           Int_t &/*root_x*/, Int_t &/*root_y*/, Int_t &/*win_x*/,
                           Int_t &/*win_y*/, UInt_t &/*mask*/)
{
   // Returns the root window the pointer is logically on and the pointer
   // coordinates relative to the root window's origin.
   //
   // id             - specifies the window
   // rotw           - the root window that the pointer is in
   // childw         - the child window that the pointer is located in, if any
   // root_x, root_y - the pointer coordinates relative to the root window's
   //                  origin
   // win_x, win_y   - the pointer coordinates relative to the specified
   //                  window "id"
   // mask           - the current state of the modifier keys and pointer
   //                  buttons
}

//______________________________________________________________________________
void TGCocoa::SetForeground(GContext_t gc, ULong_t foreground)
{
   // Sets the foreground color for the specified GC (shortcut for ChangeGC
   // with only foreground mask set).
   //
   // gc         - specifies the GC
   // foreground - the foreground you want to set
   // (see also the GCValues_t structure)
   
   assert(gc <= fX11Contexts.size() && gc > 0 && "ChangeGC - stange context id");
   
   GCValues_t &x11Context = fX11Contexts[gc - 1];
   x11Context.fMask |= kGCForeground;
   x11Context.fForeground = foreground;
}

//______________________________________________________________________________
void TGCocoa::SetClipRectangles(GContext_t /*gc*/, Int_t /*x*/, Int_t /*y*/,
                                Rectangle_t * /*recs*/, Int_t /*n*/)
{
   // Sets clipping rectangles in graphics context. [x,y] specify the origin
   // of the rectangles. "recs" specifies an array of rectangles that define
   // the clipping mask and "n" is the number of rectangles.
   // (see also the GCValues_t structure)
}

//______________________________________________________________________________
void TGCocoa::Update(Int_t /*mode = 0*/)
{
   // Flushes (mode = 0, default) or synchronizes (mode = 1) X output buffer.
   // Flush flushes output buffer. Sync flushes buffer and waits till all
   // requests have been processed by X server.
   
   gClient->DoRedraw();//Call DoRedraw for all widgets, who need to be updated.
   fPimpl->fX11CommandBuffer.Flush(fPimpl.get());
}

//______________________________________________________________________________
Region_t TGCocoa::CreateRegion()
{
   // Creates a new empty region.

   return 0;
}

//______________________________________________________________________________
void TGCocoa::DestroyRegion(Region_t /*reg*/)
{
   // Destroys the region "reg".
}

//______________________________________________________________________________
void TGCocoa::UnionRectWithRegion(Rectangle_t * /*rect*/, Region_t /*src*/,
                                  Region_t /*dest*/)
{
   // Updates the destination region from a union of the specified rectangle
   // and the specified source region.
   //
   // rect - specifies the rectangle
   // src  - specifies the source region to be used
   // dest - returns the destination region
}

//______________________________________________________________________________
Region_t TGCocoa::PolygonRegion(Point_t * /*points*/, Int_t /*np*/,
                                Bool_t /*winding*/)
{
   // Returns a region for the polygon defined by the points array.
   //
   // points  - specifies an array of points
   // np      - specifies the number of points in the polygon
   // winding - specifies the winding-rule is set (kTRUE) or not(kFALSE)

   return 0;
}

//______________________________________________________________________________
void TGCocoa::UnionRegion(Region_t /*rega*/, Region_t /*regb*/,
                          Region_t /*result*/)
{
   // Computes the union of two regions.
   //
   // rega, regb - specify the two regions with which you want to perform
   //              the computation
   // result     - returns the result of the computation

}

//______________________________________________________________________________
void TGCocoa::IntersectRegion(Region_t /*rega*/, Region_t /*regb*/,
                              Region_t /*result*/)
{
   // Computes the intersection of two regions.
   //
   // rega, regb - specify the two regions with which you want to perform
   //              the computation
   // result     - returns the result of the computation
}

//______________________________________________________________________________
void TGCocoa::SubtractRegion(Region_t /*rega*/, Region_t /*regb*/,
                             Region_t /*result*/)
{
   // Subtracts regb from rega and stores the results in result.
}

//______________________________________________________________________________
void TGCocoa::XorRegion(Region_t /*rega*/, Region_t /*regb*/, Region_t /*result*/)
{
   // Calculates the difference between the union and intersection of
   // two regions.
   //
   // rega, regb - specify the two regions with which you want to perform
   //              the computation
   // result     - returns the result of the computation

}

//______________________________________________________________________________
Bool_t  TGCocoa::EmptyRegion(Region_t /*reg*/)
{
   // Returns kTRUE if the region reg is empty.

   return kFALSE;
}

//______________________________________________________________________________
Bool_t  TGCocoa::PointInRegion(Int_t /*x*/, Int_t /*y*/, Region_t /*reg*/)
{
   // Returns kTRUE if the point [x, y] is contained in the region reg.

   return kFALSE;
}

//______________________________________________________________________________
Bool_t  TGCocoa::EqualRegion(Region_t /*rega*/, Region_t /*regb*/)
{
   // Returns kTRUE if the two regions have the same offset, size, and shape.

   return kFALSE;
}

//______________________________________________________________________________
void TGCocoa::GetRegionBox(Region_t /*reg*/, Rectangle_t * /*rect*/)
{
   // Returns smallest enclosing rectangle.
}

//______________________________________________________________________________
char **TGCocoa::ListFonts(const char * /*fontname*/, Int_t /*max*/, Int_t &count)
{
   // Returns list of font names matching fontname regexp, like "-*-times-*".
   // The pattern string can contain any characters, but each asterisk (*)
   // is a wildcard for any number of characters, and each question mark (?)
   // is a wildcard for a single character. If the pattern string is not in
   // the Host Portable Character Encoding, the result is implementation
   // dependent. Use of uppercase or lowercase does not matter. Each returned
   // string is null-terminated.
   //
   // fontname - specifies the null-terminated pattern string that can
   //            contain wildcard characters
   // max      - specifies the maximum number of names to be returned
   // count    - returns the actual number of font names
   count = 0;

   //Parse XLFD name, take family name and ask CoreText/Quartz?

   return 0;
}

//______________________________________________________________________________
void TGCocoa::FreeFontNames(char ** /*fontlist*/)
{
   // Frees the specified the array of strings "fontlist".
}

//______________________________________________________________________________
Drawable_t TGCocoa::CreateImage(UInt_t /*width*/, UInt_t /*height*/)
{
   // Allocates the memory needed for an drawable.
   //
   // width  - the width of the image, in pixels
   // height - the height of the image, in pixels

   return 0;
}

//______________________________________________________________________________
void TGCocoa::GetImageSize(Drawable_t /*wid*/, UInt_t &/*width*/, UInt_t &/*height*/)
{
   // Returns the width and height of the image wid
}

//______________________________________________________________________________
void TGCocoa::PutPixel(Drawable_t /*wid*/, Int_t /*x*/, Int_t /*y*/, ULong_t /*pixel*/)
{
   // Overwrites the pixel in the image with the specified pixel value.
   // The image must contain the x and y coordinates.
   //
   // wid   - specifies the image
   // x, y  - coordinates
   // pixel - the new pixel value
}

//______________________________________________________________________________
void TGCocoa::PutImage(Drawable_t /*wid*/, GContext_t /*gc*/,
                       Drawable_t /*img*/, Int_t /*dx*/, Int_t /*dy*/,
                       Int_t /*x*/, Int_t /*y*/, UInt_t /*w*/, UInt_t /*h*/)
{
   // Combines an image with a rectangle of the specified drawable. The
   // section of the image defined by the x, y, width, and height arguments
   // is drawn on the specified part of the drawable.
   //
   // wid  - the drawable
   // gc   - the GC
   // img  - the image you want combined with the rectangle
   // dx   - the offset in X from the left edge of the image
   // dy   - the offset in Y from the top edge of the image
   // x, y - coordinates, which are relative to the origin of the
   //        drawable and are the coordinates of the subimage
   // w, h - the width and height of the subimage, which define the
   //        rectangle dimensions
   //
   // GC components in use: function, plane-mask, subwindow-mode,
   // clip-x-origin, clip-y-origin, and clip-mask.
   // GC mode-dependent components: foreground and background.
   // (see also the GCValues_t structure)
}

//______________________________________________________________________________
void TGCocoa::DeleteImage(Drawable_t /*img*/)
{
   // Deallocates the memory associated with the image img
}

//______________________________________________________________________________
Window_t TGCocoa::GetCurrentWindow() const
{
   // pointer to the current internal window used in canvas graphics

   return Window_t();
}

//______________________________________________________________________________
unsigned char *TGCocoa::GetColorBits(Drawable_t /*wid*/, Int_t /*x*/, Int_t /*y*/,
                                     UInt_t /*w*/, UInt_t /*h*/)
{
   // Returns an array of pixels created from a part of drawable (defined by x, y, w, h)
   // in format:
   // b1, g1, r1, 0,  b2, g2, r2, 0 ... bn, gn, rn, 0 ..
   //
   // Pixels are numbered from left to right and from top to bottom.
   // By default all pixels from the whole drawable are returned.
   //
   // Note that return array is 32-bit aligned

   return 0;
}

//______________________________________________________________________________
Pixmap_t TGCocoa::CreatePixmapFromData(unsigned char *bits, UInt_t width, UInt_t height)
{
   // create pixmap from RGB data. RGB data is in format :
   // b1, g1, r1, a1,  b2, g2, r2, a2 ... bn, gn, rn, an ..
   //
   assert(bits != nullptr && "CreatePixmapFromData, data parameter is null");
   assert(width != 0 && "CreatePixmapFromData, width parameter is 0");
   assert(height != 0 && "CreatePixmapFromData, height parameter is 0");

   QuartzImage *image = nil;

   try {
      //I'm not using vector here, since I have to pass this pointer to Obj-C code
      //(and Obj-C object will own this memory later).
      unsigned char *imageData = new unsigned char[width * height * 4];
      std::copy(bits, bits + width * height * 4, imageData);
      BgraToRgba(imageData, width, height);
   
      //Now we can create CGImageRef.
      QuartzImage *mem = [QuartzImage alloc];
      if (!mem) {
         Error("CreatePixmapFromData", "[QuartzImage alloc] failed");
         delete [] imageData;
         return Pixmap_t();
      }
   
      image = [mem initWithW : width H : height data: imageData];
      if (!image) {
         [mem release];
         delete [] imageData;
         Error("CreatePixmapFromData", "[QuartzImage initWithW:H:data:] failed");
         return Pixmap_t();
      }
      
      //Now, imageData is owned by image.

      image.fID = fPimpl->RegisterDrawable(image);//This can throw.
      [image release];
      
      return image.fID;      
   } catch (const std::exception &e) {
      [image release];//Also will delete imageData.
      Error("CreatePixmapFromData", "%s", e.what());
   }

   return Pixmap_t();
}

//______________________________________________________________________________
void TGCocoa::ShapeCombineMask(Window_t, Int_t, Int_t, Pixmap_t)
{
   // The Nonrectangular Window Shape Extension adds nonrectangular
   // windows to the System.
   // This allows for making shaped (partially transparent) windows

}

//______________________________________________________________________________
UInt_t TGCocoa::ScreenWidthMM() const
{
   // Returns the width of the screen in millimeters.

   return 400;
}

//______________________________________________________________________________
void TGCocoa::DeleteProperty(Window_t, Atom_t&)
{
   // Deletes the specified property only if the property was defined on the
   // specified window and causes the X server to generate a PropertyNotify
   // event on the window unless the property does not exist.

}

//______________________________________________________________________________
Int_t TGCocoa::GetProperty(Window_t, Atom_t, Long_t, Long_t, Bool_t, Atom_t,
                           Atom_t*, Int_t*, ULong_t*, ULong_t*, unsigned char**)
{
   // Returns the actual type of the property; the actual format of the property;
   // the number of 8-bit, 16-bit, or 32-bit items transferred; the number of
   // bytes remaining to be read in the property; and a pointer to the data
   // actually returned.

   return 0;
}

//______________________________________________________________________________
void TGCocoa::ChangeActivePointerGrab(Window_t, UInt_t, Cursor_t)
{
   // Changes the specified dynamic parameters if the pointer is actively
   // grabbed by the client and if the specified time is no earlier than the
   // last-pointer-grab time and no later than the current X server time.

}

//______________________________________________________________________________
void TGCocoa::ConvertSelection(Window_t, Atom_t&, Atom_t&, Atom_t&, Time_t&)
{
   // Requests that the specified selection be converted to the specified
   // target type.

}

//______________________________________________________________________________
Bool_t TGCocoa::SetSelectionOwner(Window_t, Atom_t&)
{
   // Changes the owner and last-change time for the specified selection.

   return kFALSE;
}

//______________________________________________________________________________
void TGCocoa::ChangeProperties(Window_t, Atom_t, Atom_t, Int_t, UChar_t *, Int_t)
{
   // Alters the property for the specified window and causes the X server
   // to generate a PropertyNotify event on that window.
}

//______________________________________________________________________________
void TGCocoa::SetDNDAware(Window_t, Atom_t *)
{
   // Add XdndAware property and the list of drag and drop types to the
   // Window win.

}

//______________________________________________________________________________
void TGCocoa::SetTypeList(Window_t, Atom_t, Atom_t *)
{
   // Add the list of drag and drop types to the Window win.

}

//______________________________________________________________________________
Window_t TGCocoa::FindRWindow(Window_t, Window_t, Window_t, int, int, int)
{
   // Recursively search in the children of Window for a Window which is at
   // location x, y and is DND aware, with a maximum depth of maxd.

   return kNone;
}

//______________________________________________________________________________
Bool_t TGCocoa::IsDNDAware(Window_t, Atom_t *)
{
   // Checks if the Window is DND aware, and knows any of the DND formats
   // passed in argument.

   return kFALSE;
}


//______________________________________________________________________________
Int_t TGCocoa::SupportsExtension(const char *) const
{
   // Returns 1 if window system server supports extension given by the
   // argument, returns 0 in case extension is not supported and returns -1
   // in case of error (like server not initialized).

   return -1;
}

//______________________________________________________________________________
ROOT::MacOSX::X11::EventTranslator *TGCocoa::GetEventTranslator()
{
   return &fPimpl->fX11EventTranslator;
}

//______________________________________________________________________________
void *TGCocoa::GetCurrentContext()
{
   assert(fSelectedDrawable > fPimpl->GetRootWindowID() && "GetCurrentContext, no context for 'root' window");
   id<X11Drawable> pixmap = fPimpl->GetDrawable(fSelectedDrawable);
   assert(pixmap.fIsPixmap == YES && "GetCurrentContext, the selected drawable is not a pixmap");
   
   return pixmap.fContext;
}

//______________________________________________________________________________
Bool_t TGCocoa::MakeProcessForeground()
{
   //We start root in a terminal window, so it's considered as a 
   //background process. Background process has a lot of problems
   //if it tries to create and manage windows.
   //So, first time we convert process to foreground, next time
   //we make it front.
   
   if (!fForegroundProcess) {
      ProcessSerialNumber psn = {0, kCurrentProcess};

      const OSStatus res1 = TransformProcessType(&psn, kProcessTransformToForegroundApplication);
      if (res1 != noErr) {
         Error("MakeProcessForeground", "TransformProcessType failed with code %d", res1);
         return kFALSE;
      }
   
      const OSErr res2 = SetFrontProcess(&psn);
      if (res2 != noErr) {
         Error("MakeProcessForeground", "SetFrontProcess failed with code %d", res2);
         return kFALSE;
      }

      fForegroundProcess = kTRUE;
   } else {
      ProcessSerialNumber psn = {};    

      OSErr res = GetCurrentProcess(&psn);
      if (res != noErr) {
         Error("MapProcessForeground", "GetCurrentProcess failed with code %d", res);
         return kFALSE;
      }
      
      res = SetFrontProcess(&psn);
      if (res != noErr) {
         Error("MapProcessForeground", "SetFrontProcess failed with code %d", res);
         return kFALSE;
      }
   }
   
   return kTRUE;
}
