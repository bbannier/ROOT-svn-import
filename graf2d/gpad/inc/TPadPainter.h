#ifndef ROOT_TGLPadPainter
#define ROOT_TGLPadPainter

#ifndef ROOT_TVirtualPadPainter
#include "TVirtualPadPainter.h"
#endif

/*
TVirtualPadPainter is an attempt to abstract
painting operation furthermore. gVirtualX can
be X11 or GDI, but pad painter can be gVirtualX (X11 or GDI),
or gl pad painter.
*/

class TVirtualPad;

class TPadPainter : public TVirtualPadPainter {
public:
   TPadPainter();
   //Final overriders for TVirtualPadPainter pure virtual functions.
   //"Off-screen management" part.
   Int_t    CreateDrawable(UInt_t w, UInt_t h);
   void     ClearDrawable();
   void     CopyDrawable(Int_t id, Int_t px, Int_t py);
   void     DestroyDrawable();
   void     SelectDrawable(Int_t device);

   void     DrawLine(Double_t x1, Double_t y1, Double_t x2, Double_t y2);
   void     DrawLineNDC(Double_t u1, Double_t v1, Double_t u2, Double_t v2);
   
   void     DrawBox(Double_t x1, Double_t y1, Double_t x2, Double_t y2, EBoxMode mode);
   //TPad needs double and float versions.
   void     DrawFillArea(Int_t n, const Double_t *x, const Double_t *y);
   void     DrawFillArea(Int_t n, const Float_t *x, const Float_t *y);
   
   //TPad needs both double and float versions of DrawPolyLine.
   void     DrawPolyLine(Int_t n, const Double_t *x, const Double_t *y);
   void     DrawPolyLine(Int_t n, const Float_t *x, const Float_t *y);
   void     DrawPolyLineNDC(Int_t n, const Double_t *u, const Double_t *v);
   
   //TPad needs both versions.
   void     DrawPolyMarker(Int_t n, const Double_t *x, const Double_t *y);
   void     DrawPolyMarker(Int_t n, const Float_t *x, const Float_t *y);
   
   void     DrawText(Double_t x, Double_t y, const char *text, ETextMode mode);
   void     DrawTextNDC(Double_t u, Double_t v, const char *text, ETextMode mode);
   
   //jpg, png, bmp, gif output.
   void     SaveImage(TVirtualPad *pad, const char *fileName, Int_t type) const;

private:
   TPadPainter(const TPadPainter &rhs);
   TPadPainter & operator = (const TPadPainter &rhs);
   
   ClassDef(TPadPainter, 0) //Abstract interface for painting in TPad
};

#endif
