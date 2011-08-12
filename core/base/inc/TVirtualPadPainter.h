#ifndef ROOT_TVirtualPadPainter
#define ROOT_TVirtualPadPainter

#ifndef ROOT_Rtypes
#include "Rtypes.h"
#endif

/*
To make it possible to use gl for 2D graphic in a TPad/TCanvas,
TVirtualPadPainter interface must be used instead of TVirtualX.
Internally, non-gl implementation _should_ delegate all calls
to gVirtualX, gl implementation will delegate part of calls
to gVirtualX, and has to implement some of the calls from the scratch.
*/

class TVirtualPad;

class TVirtualPadPainter {
public:
   enum EBoxMode  {kHollow, kFilled};
   enum ETextMode {kClear,  kOpaque};

   virtual ~TVirtualPadPainter();
   
   //This part is an interface to X11 pixmap management and to save sub-pads off-screens for OpenGL.
   //Currently, must be implemented only for X11/GDI
   virtual Int_t    CreateDrawable(UInt_t w, UInt_t h) = 0;//gVirtualX->OpenPixmap
   virtual void     ClearDrawable() = 0;//gVirtualX->Clear()
   virtual void     CopyDrawable(Int_t device, Int_t px, Int_t py) = 0;
   virtual void     DestroyDrawable() = 0;//gVirtualX->CloseWindow
   virtual void     SelectDrawable(Int_t device) = 0;//gVirtualX->SelectWindow
   //
   //These functions are not required by X11/GDI.
   virtual void     InitPainter();
   virtual void     InvalidateCS();
   virtual void     LockPainter();
      
   //Now, drawing primitives.
   virtual void     DrawLine(Double_t x1, Double_t y1, Double_t x2, Double_t y2) = 0;
   virtual void     DrawLineNDC(Double_t u1, Double_t v1, Double_t u2, Double_t v2) = 0;
   
   virtual void     DrawBox(Double_t x1, Double_t y1, Double_t x2, Double_t y2, EBoxMode mode) = 0;
   
   virtual void     DrawFillArea(Int_t n, const Double_t *x, const Double_t *y) = 0;
   virtual void     DrawFillArea(Int_t n, const Float_t *x, const Float_t *y) = 0;
      
   virtual void     DrawPolyLine(Int_t n, const Double_t *x, const Double_t *y) = 0;
   virtual void     DrawPolyLine(Int_t n, const Float_t *x, const Float_t *y) = 0;
   virtual void     DrawPolyLineNDC(Int_t n, const Double_t *u, const Double_t *v) = 0;
   
   virtual void     DrawPolyMarker(Int_t n, const Double_t *x, const Double_t *y) = 0;
   virtual void     DrawPolyMarker(Int_t n, const Float_t *x, const Float_t *y) = 0;
   
   virtual void     DrawText(Double_t x, Double_t y, const char *text, ETextMode mode) = 0;
   virtual void     DrawTextNDC(Double_t u, Double_t v, const char *text, ETextMode mode) = 0;
   
   //gif, jpg, png, bmp output.
   virtual void     SaveImage(TVirtualPad *pad, const char *fileName, Int_t type) const = 0;

   
   static TVirtualPadPainter *PadPainter(Option_t *opt = "");

   ClassDef(TVirtualPadPainter, 0)//Painter interface for pad.
};

#endif
