//#define DEBUG_ROOT_COCOA

#ifdef DEBUG_ROOT_COCOA
#include <iostream>
#endif

#include <algorithm>
#include <stdexcept>

#include  <Cocoa/Cocoa.h>

#include "RootQuartzWindow.h"
#include "RootQuartzView.h"
#include "CocoaPrivate.h"
#include "QuartzFonts.h"
#include "CocoaUtils.h"
#include "TGCocoa.h"

ClassImp(TGCocoa)

namespace Details = ROOT::MacOSX::Details;
namespace Quartz = ROOT::MacOSX::Quartz;

//______________________________________________________________________________
TGCocoa::TGCocoa()
            : fForegroundProcess(false)
{
   try {
      fPimpl.reset(new Details::CocoaPrivate);
      fFontManager.reset(new Quartz::FontManager);
   } catch (const std::exception &) {
      //let's kill ROOT! without gui, no reason to live at all!
      throw;
   }
}

//______________________________________________________________________________
TGCocoa::TGCocoa(const char *name, const char *title)
            : TVirtualX(name, title)              
{
   try {
      fPimpl.reset(new Details::CocoaPrivate);
      fFontManager.reset(new Quartz::FontManager);
   } catch (const std::exception &) {
      //let's kill ROOT! without gui, no reason to live at all!
      throw;   
   }
}

/*

// Window attributes that can be inquired
struct WindowAttributes_t {
   Int_t      fX, fY;                 // location of window
   Int_t      fWidth, fHeight;        // width and height of window
   Int_t      fBorderWidth;           // border width of window
   Int_t      fDepth;                 // depth of window
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
TGCocoa::~TGCocoa()
{
   //
}

//______________________________________________________________________________
void TGCocoa::GetWindowAttributes(Window_t wid, WindowAttributes_t & attr)
{
#ifdef DEBUG_ROOT_COCOA
   std::cout<<"TGCocoa::GetWindowAttributes, wid == "<<wid<<std::endl;
#endif

   if (wid < fPimpl->fWindows.size()) {
      attr = fPimpl->fWindows[wid].fROOTWindowAttribs;
   } else {
#ifdef DEBUG_ROOT_COCOA
      std::cout<<"TGCocoa::GetWindowAttributes, wid is "<<wid<<" but number of windows: "<<fPimpl->fWindows.size()<<std::endl;
      throw std::runtime_error("Bad index for GetWindowAttributes");
#endif
   }
}

//______________________________________________________________________________
Bool_t TGCocoa::ParseColor(Colormap_t /*cmap*/, const char *colorName, ColorStruct_t &color)
{
   //"Color" passed as colorName, can be one of the names, defined in X11/rgb.txt,
   //or rgb triplet, which looks like: #rgb #rrggbb #rrrgggbbb #rrrrggggbbbb,
   //where r, g, and b - are hex digits.
#ifdef DEBUG_ROOT_COCOA
   std::cout<<"TGCocoa::ParseColor\n";
#endif
   return fPimpl->fX11ColorParser.ParseColor(colorName, color);
}

//______________________________________________________________________________
Bool_t TGCocoa::AllocColor(Colormap_t /*cmap*/, ColorStruct_t &color)
{
   //   
#ifdef DEBUG_ROOT_COCOA
   std::cout<<"TGCocoa::AllocColor\n";
#endif
   color.fPixel = ((color.fRed >> 8) & 0xFF) << 16 | ((color.fGreen >> 8) & 0xFF) << 8 | ((color.fGreen >> 8) & 0xFF);
   return kTRUE;
}

//______________________________________________________________________________
void TGCocoa::QueryColor(Colormap_t /*cmap*/, ColorStruct_t & color)
{
   // Returns the current RGB value for the pixel in the "color" structure
#ifdef DEBUG_ROOT_COCOA
   NSLog(@"TGCocoa::QueryColor");
#endif
   color.fRed = color.fPixel >> 16 & 0xFF;
   color.fGreen = color.fPixel >> 8 & 0xFF;
   color.fBlue = color.fPixel & 0xFF;
}

//______________________________________________________________________________
void TGCocoa::NextEvent(Event_t &event)
{
#ifdef DEBUG_ROOT_COCOA
   NSLog(@"TGCocoa::NextEvent");
#endif
   if (fEventQueue.size()) {
      event = fEventQueue.back();
      fEventQueue.pop_back();
   }
#ifdef DEBUG_ROOT_COCOA
   else {
      NSLog(@"TGCocoa::NextEvent: NextEvent called, but event queue is empty!");
      throw std::runtime_error("NextEvent called on empty event queue");
   }
#endif
   
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
#ifdef DEBUG_ROOT_COCOA
   std::cout<<"TGCocoa::Init\n";
#endif
   return kFALSE;
}

//______________________________________________________________________________
void TGCocoa::ClearWindow()
{
#ifdef DEBUG_ROOT_COCOA
   std::cout<<"TGCocoa::ClearWindow";
#endif
   // Clears the entire area of the current window.
}

//______________________________________________________________________________
void TGCocoa::CloseWindow()
{
#ifdef DEBUG_ROOT_COCOA
   std::cout<<"TGCocoa::CloseWindow";
#endif
   // Deletes current window.
}

//______________________________________________________________________________
void TGCocoa::ClosePixmap()
{
#ifdef DEBUG_ROOT_COCOA
   std::cout<<"TGCocoa::ClosePixmap";
#endif
   // Deletes current pixmap.
}

//______________________________________________________________________________
void TGCocoa::CopyPixmap(Int_t /*wid*/, Int_t /*xpos*/, Int_t /*ypos*/)
{
#ifdef DEBUG_ROOT_COCOA
   std::cout<<"TGCocoa::CopyPixmap";
#endif
   // Copies the pixmap "wid" at the position [xpos,ypos] in the current window.
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
#ifdef DEBUG_ROOT_COCOA
   std::cout<<"TGCocoa::ExecCommand\n";
#endif
   // Executes the command "code" coming from the other threads (Win32)
   return 0;
}

//______________________________________________________________________________
Int_t TGCocoa::GetDoubleBuffer(Int_t /*wid*/)
{
   // Queries the double buffer value for the window "wid".
#ifdef DEBUG_ROOT_COCOA
   std::cout<<"TGCocoa::GetDoubleBuffer\n";
#endif
   return 0;
}

//______________________________________________________________________________
void TGCocoa::GetCharacterUp(Float_t &chupx, Float_t &chupy)
{
   // Returns character up vector.
#ifdef DEBUG_ROOT_COCOA
   std::cout<<"TGCocoa::GetCharacterUp\n";
#endif
   chupx = chupy = 0;
}

//______________________________________________________________________________
void TGCocoa::GetGeometry(Int_t /*wid*/, Int_t & /*x*/, Int_t & /*y*/, UInt_t & /*w*/, UInt_t & /*h*/)
{
   // Returns position and size of window "wid".
   //
   // wid  - window identifier
   //        if wid < 0 the size of the display is returned
   // x, y - returned window position
   // w, h - returned window size
#ifdef DEBUG_ROOT_COCOA
   std::cout<<"TGCocoa::GetGeometry\n";
#endif
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
#ifdef DEBUG_ROOT_COCOA
   std::cout<<"TGCocoa::GetNativeEvent\n";
#endif
   return 0;
}

//______________________________________________________________________________
ULong_t TGCocoa::GetPixel(Color_t /*cindex*/)
{
   // Returns pixel value associated to specified ROOT color number "cindex".
#ifdef DEBUG_ROOT_COCOA
   std::cout<<"TGCocoa::GetPixel\n";
#endif
   return 0;
}

//______________________________________________________________________________
void TGCocoa::GetPlanes(Int_t & /*nplanes*/)
{
   // Returns the maximum number of planes.
#ifdef DEBUG_ROOT_COCOA
//   std::cout<<"TGCocoa::GetPlanes\n";
#endif
}

//______________________________________________________________________________
void TGCocoa::GetRGB(Int_t /*index*/, Float_t &/*r*/, Float_t &/*g*/, Float_t &/*b*/)
{
   // Returns RGB values for color "index".
#ifdef DEBUG_ROOT_COCOA
   std::cout<<"TGCocoa::GetRGB\n";
#endif
}

//______________________________________________________________________________
void TGCocoa::GetTextExtent(UInt_t &/*w*/, UInt_t &/*h*/, char *text)
{
   // Returns the size of the specified character string "mess".
   //
   // w    - the text width
   // h    - the text height
   // mess - the string
//#ifdef DEBUG_ROOT_COCOA
//   NSLog(@"GetTextExtent for text %s", text);
//#endif
}

//______________________________________________________________________________
Int_t   TGCocoa::GetFontAscent() const
{
   // Returns the ascent of the current font (in pixels).
   // The ascent of a font is the distance from the baseline
   // to the highest position characters extend to
#ifdef DEBUG_ROOT_COCOA
   std::cout<<"TGCocoa::GetFontAscent\n";
#endif
   return 0;
}
//______________________________________________________________________________
Int_t   TGCocoa::GetFontDescent() const
{
  // Returns the descent of the current font (in pixels.
  // The descent is the distance from the base line
  // to the lowest point characters extend to.
#ifdef DEBUG_ROOT_COCOA
   std::cout<<"TGCocoa::GetFontDescent\n";
#endif
   return 0;
}

//______________________________________________________________________________
Float_t TGCocoa::GetTextMagnitude()
{
   // Returns the current font magnification factor
#ifdef DEBUG_ROOT_COCOA
   std::cout<<"TGCocoa::GetTextMagnitude\n";
#endif
   return 0;
}

//______________________________________________________________________________
Bool_t TGCocoa::HasTTFonts() const
{
   // Returns True when TrueType fonts are used
#ifdef DEBUG_ROOT_COCOA
   std::cout<<"TGCocoa::HasTTFonts\n";
#endif

   return kFALSE;
}

//______________________________________________________________________________
Window_t TGCocoa::GetWindowID(Int_t /*wid*/)
{
   // Returns the Cocoa window identifier.
   //
   // wid - workstation identifier (input)
#ifdef DEBUG_ROOT_COCOA
   std::cout<<"TGCocoa::GetWindowID\n";
#endif

   return Window_t();
}

//______________________________________________________________________________
Int_t TGCocoa::InitWindow(ULong_t /*window*/)
{
   // Creates a new window and return window number.
   // Returns -1 if window initialization fails.
#ifdef DEBUG_ROOT_COCOA
   std::cout<<"TGCocoa::InitWindow\n";
#endif

   return 0;
}

//______________________________________________________________________________
Int_t TGCocoa::AddWindow(ULong_t /*qwid*/, UInt_t /*w*/, UInt_t /*h*/)
{
   // Registers a window created by Qt as a ROOT window
   //
   // w, h - the width and height, which define the window size
#ifdef DEBUG_ROOT_COCOA
   std::cout<<"TGCocoa::AddWindow\n";
#endif

   return 0;
}

//______________________________________________________________________________
Int_t TGCocoa::AddPixmap(ULong_t /*pixind*/, UInt_t /*w*/, UInt_t /*h*/)
{
   // Registers a pixmap created by TGLManager as a ROOT pixmap
   //
   // w, h - the width and height, which define the pixmap size
#ifdef DEBUG_ROOT_COCOA
   std::cout<<"TGCocoa::AddPixmap\n";
#endif


   return 0;
}


//______________________________________________________________________________
void TGCocoa::RemoveWindow(ULong_t /*qwid*/)
{
   // Removes the created by Qt window "qwid".
#ifdef DEBUG_ROOT_COCOA
   std::cout<<"TGCocoa::RemoveWindow\n";
#endif

}

//______________________________________________________________________________
void TGCocoa::MoveWindow(Int_t /*wid*/, Int_t /*x*/, Int_t /*y*/)
{
   // Moves the window "wid" to the specified x and y coordinates.
   // It does not change the window's size, raise the window, or change
   // the mapping state of the window.
   //
   // x, y - coordinates, which define the new position of the window
   //        relative to its parent.
#ifdef DEBUG_ROOT_COCOA
   std::cout<<"TGCocoa::MoveWindow\n";
#endif

}

//______________________________________________________________________________
Int_t TGCocoa::OpenPixmap(UInt_t /*w*/, UInt_t /*h*/)
{
   // Creates a pixmap of the width "w" and height "h" you specified.

   return 0;
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
Int_t TGCocoa::ResizePixmap(Int_t /*wid*/, UInt_t /*w*/, UInt_t /*h*/)
{
   // Resizes the specified pixmap "wid".
   //
   // w, h - the width and height which define the pixmap dimensions

   return 0;
}

//______________________________________________________________________________
void TGCocoa::ResizeWindow(Int_t /*wid*/)
{
   // Resizes the window "wid" if necessary.
}

//______________________________________________________________________________
void TGCocoa::SelectWindow(Int_t /*wid*/)
{
   // Selects the window "wid" to which subsequent output is directed.
}

//______________________________________________________________________________
void TGCocoa::SelectPixmap(Int_t /*qpixid*/)
{
   // Selects the pixmap "qpixid".
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
void TGCocoa::SetDoubleBuffer(Int_t /*wid*/, Int_t /*mode*/)
{
   // Sets the double buffer on/off on the window "wid".
   // wid  - window identifier.
   //        999 means all opened windows.
   // mode - the on/off switch
   //        mode = 1 double buffer is on
   //        mode = 0 double buffer is off

}

//______________________________________________________________________________
void TGCocoa::SetDoubleBufferOFF()
{
   // Turns double buffer mode off.
}

//______________________________________________________________________________
void TGCocoa::SetDoubleBufferON()
{
   // Turns double buffer mode on.
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

/*
//______________________________________________________________________________
void TGCocoa::SetLineType(Int_t n, Int_t * dash)
{
   // Sets the line type.
   //
   // n       - length of the dash list
   //           n <= 0 use solid lines
   //           n >  0 use dashed lines described by dash(n)
   //                 e.g. n = 4,dash = (6,3,1,3) gives a dashed-dotted line
   //                 with dash length 6 and a gap of 7 between dashes
   // dash(n) - dash segment lengths
}

//______________________________________________________________________________
void TGCocoa::SetOpacity(Int_t percent)
{
   // Sets opacity of the current window. This image manipulation routine
   // works by adding to a percent amount of neutral to each pixels RGB.
   // Since it requires quite some additional color map entries is it
   // only supported on displays with more than > 8 color planes (> 256
   // colors).
}
*/
//______________________________________________________________________________
void TGCocoa::SetRGB(Int_t /*cindex*/, Float_t /*r*/, Float_t /*g*/, Float_t /*b*/)
{
   // Sets color intensities the specified color index "cindex".
   //
   // cindex  - color index
   // r, g, b - the red, green, blue intensities between 0.0 and 1.0
}
/*
//______________________________________________________________________________
Int_t TGCocoa::SetTextFont(char * fontname, ETextSetMode mode)
{
   // Sets text font to specified name "fontname".This function returns 0 if
   // the specified font is found, 1 if it is not.
   //
   // mode - loading flag
   //        mode = 0 search if the font exist (kCheck)
   //        mode = 1 search the font and load it if it exists (kLoad)

   return 0;
}
*/
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


//---- Methods used for GUI -----
//______________________________________________________________________________
void TGCocoa::MapWindow(Window_t /*wid*/)
{
   // Maps the window "wid" and all of its subwindows that have had map
   // requests. This function has no effect if the window is already mapped.
//#ifdef DEBUG_ROOT_COCOA
//   std::cout<<"TGCocoa::MapWindow, wid == "<<wid<<std::endl;
//#endif
}

//______________________________________________________________________________
void TGCocoa::MapSubwindows(Window_t /*wid*/)
{
   // Maps all subwindows for the specified window "wid" in top-to-bottom
   // stacking order.
   
}

//______________________________________________________________________________
void TGCocoa::MapRaised(Window_t wid)
{
   // Maps the window "wid" and all of its subwindows that have had map
   // requests on the screen and put this window on the top of of the
   // stack of all windows.
   
   //Just a sketch code.
#ifdef DEBUG_ROOT_COCOA
   std::cout<<"TGCocoa::MapRaised, wid == "<<wid<<std::endl;
#endif

   ROOT::MacOSX::Util::AutoreleasePool pool;

   auto winIter = fPimpl->fWindows.find(wid);

#ifdef DEBUG_ROOT_COCOA
   if (winIter == fPimpl->fWindows.end()) {
      std::cout<<"TGCocoa::MapRaised, bad window id "<<wid<<std::endl;
      throw std::runtime_error("TGCocoa::MapRaised, bad window id");
   }
#endif

   if (!fForegroundProcess) {
      ProcessSerialNumber psn = {0, kCurrentProcess}; //GetCurrentProcess(&psn);
      TransformProcessType(&psn, kProcessTransformToForegroundApplication);
      SetFrontProcess(&psn);
      fForegroundProcess = true;
   } else {
      ProcessSerialNumber psn = {};    
      if (GetCurrentProcess(&psn) == noErr)
         SetFrontProcess(&psn);
#ifdef DEBUG_ROOT_COCOA
      else
         NSLog(@"SetFrontProcess failed");
#endif
   }

   NSWindow *cocoaWin = (NSWindow *)fPimpl->fWindows[wid].fCocoaWindow.Get();
   [cocoaWin makeKeyAndOrderFront : cocoaWin];
//   [cocoaWin orderFront : nil];
}

//______________________________________________________________________________
void TGCocoa::UnmapWindow(Window_t /*wid*/)
{
   // Unmaps the specified window "wid". If the specified window is already
   // unmapped, this function has no effect. Any child window will no longer
   // be visible (but they are still mapped) until another map call is made
   // on the parent.
}

//______________________________________________________________________________
void TGCocoa::DestroyWindow(Window_t /*wid*/)
{
   // Destroys the window "wid" as well as all of its subwindows.
   // The window should never be referenced again. If the window specified
   // by the "id" argument is mapped, it is unmapped automatically.
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
//#ifdef DEBUG_ROOT_COCOA
//   std::cout<<"TGCocoa::RaiseWindow was called with wid == "<<wid<<std::endl;
//#endif
}

//______________________________________________________________________________
void TGCocoa::LowerWindow(Window_t /*wid*/)
{
   // Lowers the specified window "wid" to the bottom of the stack so
   // that it does not obscure any sibling windows.
}

//______________________________________________________________________________
void TGCocoa::MoveWindow(Window_t /*wid*/, Int_t /*x*/, Int_t /*y*/)
{
   // Moves the specified window to the specified x and y coordinates.
   // It does not change the window's size, raise the window, or change
   // the mapping state of the window.
   //
   // x, y - coordinates, which define the new position of the window
   //        relative to its parent.
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
//   NSLog(@"move/resize for window %d, x == %d, y == %d, w == %d, h == %d", wid, x, y, w, h);
   //y = RootToCocoaY(y);
   NSRect newFrame;
   newFrame.origin.x = x;
   newFrame.origin.y = y;

   newFrame.size.height = h;
   newFrame.size.width = w;
   

   id<RootGUIElement> widget = fPimpl->GetWindow(wid);
   if (![widget parentView]) {
      //
   } else {
      NSView *contentView = [widget contentView];
      contentView.frame = newFrame;
   }
}

//______________________________________________________________________________
void TGCocoa::ResizeWindow(Window_t /*wid*/, UInt_t /*w*/, UInt_t /*h*/)
{
   // Changes the width and height of the specified window "wid", not
   // including its borders. This function does not change the window's
   // upper-left coordinate.
   //
   // w, h - the width and height, which are the interior dimensions of
   //        the window after the call completes.
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
void TGCocoa::ReparentWindow(Window_t /*wid*/, Window_t /*pid*/, Int_t /*x*/, Int_t /*y*/)
{
   // If the specified window is mapped, ReparentWindow automatically
   // performs an UnmapWindow request on it, removes it from its current
   // position in the hierarchy, and inserts it as the child of the specified
   // parent. The window is placed in the stacking order on top with respect
   // to sibling windows.
}

//______________________________________________________________________________
void TGCocoa::SetWindowBackground(Window_t wid, ULong_t color)
{
   // Sets the background of the window "wid" to the specified color value
   // "color". Changing the background does not cause the window contents
   // to be changed.
//   NSLog(@"SetWindowBackground called for wid %lu, color is %xu", wid, (UInt_t)color);
}

//______________________________________________________________________________
void TGCocoa::SetWindowBackgroundPixmap(Window_t /*wid*/, Pixmap_t /*pxm*/)
{
   // Sets the background pixmap of the window "wid" to the specified
   // pixmap "pxm".
}

namespace {

//______________________________________________________________________________
RootQuartzWindow *CreateTopLevelWindow(Int_t x, Int_t y, UInt_t w, UInt_t h, UInt_t /*border*/, Int_t /*depth*/,
                                       UInt_t /*clss*/, void * /*visual*/, SetWindowAttributes_t * /*attr*/, UInt_t /*wtype*/)
{
   NSRect contentRect = {};
   contentRect.origin.x = x;
   contentRect.origin.y = y;
   contentRect.size.width = w;
   contentRect.size.height = h;
   NSUInteger styleMask = NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask;
   RootQuartzWindow *newWindow = [[RootQuartzWindow alloc] initWithContentRect : contentRect styleMask : styleMask backing : NSBackingStoreBuffered defer : NO];
   [newWindow setAcceptsMouseMovedEvents : YES];


   //Adjust view rect to exclude title bar????

   contentRect.origin = CGPointZero;
   RootQuartzView *view = [[RootQuartzView alloc] initWithFrame : contentRect];
   newWindow.fTopLevelView = view;   
   [view release];

   return newWindow;
}

//______________________________________________________________________________
RootQuartzView *CreateChildView(Int_t /*x*/, Int_t /*y*/, UInt_t w, UInt_t h, UInt_t /*border*/, Int_t /*depth*/,
                                UInt_t /*clss*/, void * /*visual*/, SetWindowAttributes_t * /*attr*/, UInt_t /*wtype*/)
{
   NSRect contentRect = {};
   contentRect.origin = CGPointZero;
   contentRect.size.width = w;
   contentRect.size.height = h;
   
   RootQuartzView *view = [[RootQuartzView alloc] initWithFrame : contentRect];
   
   return view;
}

//______________________________________________________________________________
void SetWindowAttributes(const SetWindowAttributes_t * /*src*/, WindowAttributes_t * /*dst*/)
{
 /*  dst->fX = x;
   dst->fY = y;
   dst->fWidth = w;
   dst->fHeight = h;*/
}

}

//______________________________________________________________________________
Window_t TGCocoa::CreateWindow(Window_t parent, Int_t x, Int_t y, UInt_t w, UInt_t h, UInt_t border, Int_t depth,
                               UInt_t clss, void * visual, SetWindowAttributes_t * attr, UInt_t wtype)
{
   //Do not know at the moment, what to do with ALL these possible X11 parameters, which
   //means nothing for Cocoa. TODO: create window correctly to emulate what ROOT wants from TGCocoa.
   //This implementation is just a sketch to try.
   //
#ifdef DEBUG_ROOT_COCOA
   NSLog(@"CreateWindow was called %d %d %d %u %u %u %d", parent, x, y, w, h, border, depth);
#endif   
   //Check if really need this.
   ROOT::MacOSX::Util::AutoreleasePool pool;

   WindowAttributes_t winAttr = {};
   //
   winAttr.fX = x;
   winAttr.fY = y;
   winAttr.fWidth = w;
   winAttr.fHeight = h;
   //
   
   SetWindowAttributes(attr, &winAttr);
   
   if (!parent) {//parent == root window.
      RootQuartzWindow *newWindow = CreateTopLevelWindow(x, y, w, h, border, depth, clss, visual, attr, wtype);
      const Window_t result = fPimpl->RegisterWindow(newWindow, winAttr);

      newWindow.fWinID = result;

      [newWindow release];//Owned by fPimpl now.
      return result;
   } else {
      id<RootGUIElement> parentWin = fPimpl->GetWindow(parent);
      RootQuartzView *childView = CreateChildView(x, y, w, h, border, depth, clss, visual, attr, wtype);
      const Window_t result = fPimpl->RegisterWindow(childView, winAttr);
      
      childView.fWinID = result;

      [parentWin addChildView : childView];
      [childView release];
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

   return 0;
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
   // Provides the most common way for accessing a font: opens (loads) the
   // specified font and returns a pointer to the appropriate FontStruct_t
   // structure. If the font does not exist, it returns NULL.
   ROOT::MacOSX::Quartz::XLFDName xlfd = {};
   if (ParseXLFDName(fontName, xlfd))
      return fFontManager->LoadFont(xlfd);

   return 0;
}

//______________________________________________________________________________
FontH_t TGCocoa::GetFontHandle(FontStruct_t fs)
{
   // Returns the font handle of the specified font structure "fs".
   NSLog(@"GetFontHandle for %lu", fs);
   return (FontH_t)fs;
}

//______________________________________________________________________________
void TGCocoa::DeleteFont(FontStruct_t /*fs*/)
{
   // Explicitely deletes the font structure "fs" obtained via LoadQueryFont().
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
   NSLog(@"changing GC %lu", gc);
   // Changes the components specified by the mask in gval for the specified GC.
   //
   // GContext_t gc   - specifies the GC to be changed
   // GCValues_t gval - specifies the mask and the values to be set
   // (see also the GCValues_t structure)
}

//______________________________________________________________________________
void TGCocoa::CopyGC(GContext_t /*org*/, GContext_t /*dest*/, Mask_t /*mask*/)
{
   // Copies the specified components from the source GC "org" to the
   // destination GC "dest". The "mask" defines which component to copy
   // and it is a data member of GCValues_t.
   NSLog(@"CopyGC");
}

//______________________________________________________________________________
void TGCocoa::DeleteGC(GContext_t /*gc*/)
{
   // Deletes the specified GC "gc".
   NSLog(@"DeleteGC");
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
   return Int_t(fEventQueue.size());
}

//______________________________________________________________________________
void TGCocoa::Bell(Int_t /*percent*/)
{
   // Sets the sound bell. Percent is loudness from -100% .. 100%.
}

//______________________________________________________________________________
void TGCocoa::CopyArea(Drawable_t /*src*/, Drawable_t /*dest*/,
                       GContext_t /*gc*/, Int_t /*src_x*/, Int_t /*src_y*/,
                       UInt_t /*width*/, UInt_t /*height*/,
                       Int_t /*dest_x*/, Int_t /*dest_y*/)
{
   // Combines the specified rectangle of "src" with the specified rectangle
   // of "dest" according to the "gc".
   //
   // src_x, src_y   - specify the x and y coordinates, which are relative
   //                  to the origin of the source rectangle and specify
   //                  upper-left corner.
   // width, height  - the width and height, which are the dimensions of both
   //                  the source and destination rectangles                                                                   //
   // dest_x, dest_y - specify the upper-left corner of the destination
   //                  rectangle
   //
   // GC components in use: function, plane-mask, subwindow-mode,
   // graphics-exposure, clip-x-origin, clip-y-origin, and clip-mask.
   // (see also the GCValues_t structure)
}

//______________________________________________________________________________
void TGCocoa::ChangeWindowAttributes(Window_t wid, SetWindowAttributes_t *attr)
{
   if (!wid)//Should never happen, this is 'root' window.
      return;
   
   id<RootGUIElement> widget = fPimpl->GetWindow(wid);
   RootQuartzView *view = (RootQuartzView *)[widget contentView];
   
   //There are a lot of X11 attribs. in SetWindowAttributes_t struct.
   //Now set only background color.
   if (attr->fMask & kWABackPixel)
      view.fBackgroundColor = attr->fBackgroundPixel;
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

//______________________________________________________________________________
void TGCocoa::DrawLine(Drawable_t wid, GContext_t gc, Int_t x1, Int_t y1, Int_t x2, Int_t y2)
{
   //This code is just a hack to show button.
   
   if (!wid) {
      NSLog(@"DrawLine was called for 'root' window");
      throw std::runtime_error("DrawLine was called for 'root' window");
   }
   
   CGContextRef ctx = (CGContextRef)fCtx;
   if (!ctx) {
      NSLog(@"DrawLine called outside of drawRect function");
      throw std::runtime_error("DrawLine called outside of drawRect function");
   }
   
   const GCValues_t &gcVals = fX11Contexts[gc - 1];

   
   
//   CGContextSetRGBStrokeColor(ctx, 0.f, 0.f, 0.f, 1.f);
//   CGContextBeginPath(ctx);
//   CGContextMoveToPoint(ctx, x1, y1);
//   CGContextAddLineToPoint(ctx, x2, y2);
//   CGContextStrokePath(ctx);
}

//______________________________________________________________________________
void TGCocoa::ClearArea(Window_t wid, Int_t x, Int_t y, UInt_t w, UInt_t h)
{
   // Paints a rectangular area in the specified window "id" according to
   // the specified dimensions with the window's background pixel or pixmap.
   //
   // wid - specifies the window
   // x, y - coordinates, which are relative to the origin
   // w, h - the width and height which define the rectangle dimensions

   if (!wid) {//Should never happen ('root' window).
      NSLog(@"clear area for 'root' window was called");
      throw std::runtime_error("clear area for 'root' window was called");
   }

   if (!fCtx) {
      NSLog(@"clear area called outside of drawRect function");
      throw std::runtime_error("clear area called outside of drawRect function");
   }
   
   //NSLog(@"clear area for %d", wid);

   id<RootGUIElement> widget = fPimpl->GetWindow(wid);
   RootQuartzView *view = (RootQuartzView *)[widget contentView];
   const Pixel_t color = view.fBackgroundColor;
   const CGFloat red = ((color & 0xff0000) >> 16) / 255.f;
   const CGFloat green = ((color & 0xff00) >> 8) / 255.f;
   const CGFloat blue = (color & 0xff) / 255.f;
   
   CGContextRef ctx = static_cast<CGContextRef>(fCtx);
   CGContextSetRGBFillColor(ctx, red, green, blue, 1.f);//alpha can be also used.
   CGContextFillRect(ctx, CGRectMake(x, y, w, h));
}

namespace {

class EventRemovalPredicate {
public:
   EventRemovalPredicate(Window_t wid, EGEventType type) : fWid(wid), fType(type)
   {
   }
   Bool_t operator () (const Event_t &ev) const
   {
      if (fWid == ev.fWindow && ev.fType == fType)
         return kTRUE;
      return kFALSE;
   }
private:
   Window_t fWid;
   EGEventType fType;
};

}

//______________________________________________________________________________
Bool_t TGCocoa::CheckEvent(Window_t /*wid*/, EGEventType /*type*/, Event_t & /*ev*/)
{
   //No need in this.
   return kFALSE;
}

//______________________________________________________________________________
void TGCocoa::SendEvent(Window_t /*wid*/, Event_t * /*ev*/)
{
   // Specifies the event "ev" is to be sent to the window "wid".
   // This function requires you to pass an event mask.
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
void TGCocoa::GrabButton(Window_t /*wid*/, EMouseButton /*button*/,
                           UInt_t /*modifier*/, UInt_t /*evmask*/,
                           Window_t /*confine*/, Cursor_t /*cursor*/,
                           Bool_t /*grab = kTRUE*/)
{
   // Establishes a passive grab on a certain mouse button. That is, when a
   // certain mouse button is hit while certain modifier's (Shift, Control,
   // Meta, Alt) are active then the mouse will be grabed for window id.
   // When grab is false, ungrab the mouse button for this button and modifier.
}

//______________________________________________________________________________
void TGCocoa::GrabPointer(Window_t /*wid*/, UInt_t /*evmask*/,
                            Window_t /*confine*/, Cursor_t /*cursor*/,
                            Bool_t /*grab = kTRUE*/,
                            Bool_t /*owner_events = kTRUE*/)
{
   // Establishes an active pointer grab. While an active pointer grab is in
   // effect, further pointer events are only reported to the grabbing
   // client window.
}

//______________________________________________________________________________
void TGCocoa::SetWindowName(Window_t /*wid*/, char * /*name*/)
{
   // Sets the window name.
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
void TGCocoa::DrawString(Drawable_t wID, GContext_t gc, Int_t x, Int_t y, const char *text, Int_t len)
{
   // wid  - the drawable
   // gc   - the GC
   // x, y - coordinates, which are relative to the origin of the specified
   //        drawable and define the origin of the first character
   // s    - the character string
   // len  - the number of characters in the string argument
   if (!wID) {
      NSLog(@"TGCocoa::DrawString was called for 'root' window");
      throw std::runtime_error("TGCocoa::DrawString was called for 'root' window");
   }
   
   CGContextRef ctx = (CGContextRef)fCtx;
   if (!ctx) {
      NSLog(@"TGCocoa::DrawString: function was called, but context not set");
      throw std::runtime_error("TGCocoa::DrawString: function was called, but context not set");
   }
   
   id<RootGUIElement> widget = fPimpl->GetWindow(wID);
   NSView *view = [widget contentView];
   y = view.frame.size.height - y;
   
   const GCValues_t &gcVals = fX11Contexts[gc - 1];
   ROOT::MacOSX::Quartz::CTLineGuard ctLine(text, (CTFontRef)gcVals.fFont);

   CGContextSaveGState(ctx);
   CGContextSetTextPosition(ctx, x, y);
   CTLineDraw(ctLine.fCTLine, ctx);
   CGContextRestoreGState(ctx);
}

//______________________________________________________________________________
Int_t TGCocoa::TextWidth(FontStruct_t font, const char *s, Int_t len)
{
   // Return lenght of the string "s" in pixels. Size depends on font.
//   NSLog(@"text width requested for string %s, font is %p", s, (void*)font);
   return fFontManager->GetTextWidth(font, s, len);
}

//______________________________________________________________________________
void TGCocoa::GetFontProperties(FontStruct_t font, Int_t &maxAscent, Int_t &maxDescent)
{
   // Returns the font properties.
//   NSLog(@"GetFontProperties!!!");
   fFontManager->GetFontProperties(font, maxAscent, maxDescent);
}

//______________________________________________________________________________
void TGCocoa::GetGCValues(GContext_t /*gc*/, GCValues_t &/*gval*/)
{
   // Returns the components specified by the mask in "gval" for the
   // specified GC "gc" (see also the GCValues_t structure)
}

//______________________________________________________________________________
FontStruct_t TGCocoa::GetFontStruct(FontH_t /*fh*/)
{
   // Retrieves the associated font structure of the font specified font
   // handle "fh".
   //
   // Free returned FontStruct_t using FreeFontStruct().

   return 0;
}

//______________________________________________________________________________
void TGCocoa::FreeFontStruct(FontStruct_t fs)
{
   // Frees the font structure "fs". The font itself will be freed when
   // no other resource references it.
   fFontManager->UnloadFont(fs);
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
void TGCocoa::FillRectangle(Drawable_t wid, GContext_t /*gc*/,
                            Int_t /*x*/, Int_t /*y*/,
                            UInt_t /*w*/, UInt_t /*h*/)
{
   // Fills the specified rectangle defined by [x,y] [x+w,y] [x+w,y+h] [x,y+h].
   // using the GC you specify.
   //
   // GC components in use are: function, plane-mask, fill-style,
   // subwindow-mode, clip-x-origin, clip-y-origin, clip-mask.
   // GC mode-dependent components: foreground, background, tile, stipple,
   // tile-stipple-x-origin, and tile-stipple-y-origin.
   // (see also the GCValues_t structure)
   NSLog(@"Fill rectangle for widget %d", wid);
}

//______________________________________________________________________________
void TGCocoa::DrawRectangle(Drawable_t /*wid*/, GContext_t /*gc*/,
                            Int_t /*x*/, Int_t /*y*/,
                            UInt_t /*w*/, UInt_t /*h*/)
{
   // Draws rectangle outlines of [x,y] [x+w,y] [x+w,y+h] [x,y+h]
   //
   // GC components in use: function, plane-mask, line-width, line-style,
   // cap-style, join-style, fill-style, subwindow-mode, clip-x-origin,
   // clip-y-origin, clip-mask.
   // GC mode-dependent components: foreground, background, tile, stipple,
   // tile-stipple-x-origin, tile-stipple-y-origin, dash-offset, dash-list.
   // (see also the GCValues_t structure)
   NSLog(@"DrawRectangle");
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
void TGCocoa::SelectInput(Window_t /*wid*/, UInt_t /*evmask*/)
{
   // Defines which input events the window is interested in. By default
   // events are propageted up the window stack. This mask can also be
   // set at window creation time via the SetWindowAttributes_t::fEventMask
   // attribute.
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
void TGCocoa::TranslateCoordinates(Window_t /*src*/, Window_t /*dest*/, Int_t /*src_x*/, Int_t /*src_y*/, 
                                   Int_t &/*dest_x*/, Int_t &/*dest_y*/, Window_t &/*child*/)
{
   // Translates coordinates in one window to the coordinate space of another
   // window. It takes the "src_x" and "src_y" coordinates relative to the
   // source window's origin and returns these coordinates to "dest_x" and
   // "dest_y" relative to the destination window's origin.
   //
   // src            - the source window
   // dest           - the destination window
   // src_x, src_y   - coordinates within the source window
   // dest_x, dest_y - coordinates within the destination window
   // child          - returns the child of "dest" if the coordinates
   //                  are contained in a mapped child of the destination
   //                  window; otherwise, child is set to 0
}

//______________________________________________________________________________
void TGCocoa::GetWindowSize(Drawable_t /*wid*/, Int_t &/*x*/, Int_t &/*y*/, UInt_t &/*w*/, UInt_t &/*h*/)
{
   // Returns the location and the size of window "wid"
   //
   // x, y - coordinates of the upper-left outer corner relative to the
   //        parent window's origin
   // w, h - the inside size of the window, not including the border
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
void TGCocoa::SetForeground(GContext_t /*gc*/, ULong_t /*foreground*/)
{
   // Sets the foreground color for the specified GC (shortcut for ChangeGC
   // with only foreground mask set).
   //
   // gc         - specifies the GC
   // foreground - the foreground you want to set
   // (see also the GCValues_t structure)
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
#ifdef DEBUG_ROOT_COCOA
   std::cout<<"TGCocoa::ListFonts: Font name is "<<fontname<<std::endl;
#endif
   count = 0;

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
Pixmap_t TGCocoa::CreatePixmapFromData(unsigned char * /*bits*/, UInt_t /*width*/,
                                       UInt_t /*height*/)
{
   // create pixmap from RGB data. RGB data is in format :
   // b1, g1, r1, 0,  b2, g2, r2, 0 ... bn, gn, rn, 0 ..
   //
   // Pixels are numbered from left to right and from top to bottom.
   // Note that data must be 32-bit aligned
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
void TGCocoa::QueueEvent(const Event_t &event)
{
   //Window delegate (and views?) calls this function
   //and adds ROOT's events into the queue (which later
   //will be processed by TGClient and ROOT's GUI).
   fEventQueue.push_front(event);
}

//______________________________________________________________________________
Int_t TGCocoa::CocoaToRootY(Window_t /*wid*/, Int_t y)const
{
 //  id<RootGUIElement> widget = fPimpl->GetWindow(wid);
/*   if (fPimpl->fWindows.size()) {
      //"Window" with index 0 is a fake "root" window,
      //it has the sizes of a screen.
      const WindowAttributes_t &attr = fPimpl->fWindows[0].fROOTWindowAttribs;
      return attr.fHeight - y;
   } else {
#ifdef DEBUG_ROOT_COCOA
      NSLog(@"CocoaToRootY: No root window found");
      throw std::runtime_error("CocoaToRootY: No root window found");
#endif
      return y;//Should never happen.
   }*/
   return y;
}

//______________________________________________________________________________
Int_t TGCocoa::RootToCocoaY(Window_t /*wid*/, Int_t y)const
{
/*   if (fPimpl->fWindows.size()) {
      //"Window" with index 0 is a fake "root" window,
      //it has the sizes of a screen.
      const WindowAttributes_t &attr = fPimpl->fWindows[0].fROOTWindowAttribs;
      return attr.fHeight - y;
   } else {
#ifdef DEBUG_ROOT_COCOA
      NSLog(@"RootToCocoaY: No root window found");
      throw std::runtime_error("RootToCocoaY: No root window found");
#endif*/
      return y;//Should never happen.
//   }
}

//______________________________________________________________________________
void TGCocoa::SetContext(void *ctx)
{
   fCtx = ctx;
}
