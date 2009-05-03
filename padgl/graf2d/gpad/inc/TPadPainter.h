#ifndef ROOT_TGLPadPainter
#define ROOT_TGLPadPainter

#include <vector>

#ifndef ROOT_TVirtualPadPainter
#include "TVirtualPadPainter.h"
#endif

/*
TVirtualPadPainter is an attempt to abstract
painting operation furthermore. gVirtualX can
be X11 or GDI, but pad painter can be gVirtualX (X11 or GDI),
or gl pad painter.
This class does not require any data (except point buffer). 
It's quite trivial.
Just delegates _everything_ to TVirtualX, that enough 
(or ignore unsupported operation like InitPainter).
*/
class TPadPainter : public TVirtualPadPainter {
public:
   TPadPainter();
   //Final overriders for TVirtualPadPainter pure virtual functions.
   //1. Part, which simply delegates to TVirtualX.
   //Line attributes.
   Color_t  GetLineColor() const;
   Style_t  GetLineStyle() const;
   Width_t  GetLineWidth() const;
   
   void     SetLineColor(Color_t lcolor);
   void     SetLineStyle(Style_t lstyle);
   void     SetLineWidth(Width_t lwidth);
   //Fill attributes.
   Color_t  GetFillColor() const;
   Style_t  GetFillStyle() const;
   Bool_t   IsTransparent() const;

   void     SetFillColor(Color_t fcolor);
   void     SetFillStyle(Style_t fstyle);
   void     SetOpacity(Int_t percent);
   //Text attributes.
   Short_t  GetTextAlign() const;
   Float_t  GetTextAngle() const;
   Color_t  GetTextColor() const;
   Font_t   GetTextFont()  const;
   Float_t  GetTextSize()  const;
   Float_t  GetTextMagnitude() const;
   
   void     SetTextAlign(Short_t align);
   void     SetTextAngle(Float_t tangle);
   void     SetTextColor(Color_t tcolor);
   void     SetTextFont(Font_t tfont);
   void     SetTextSize(Float_t tsize);
   void     SetTextSizePixels(Int_t npixels);
   //2. "Off-screen management" part.
   Int_t    CreateDrawable(UInt_t w, UInt_t h);
   void     ClearDrawable();
   void     CopyDrawable(Int_t id, Int_t px, Int_t py);
   void     DestroyDrawable();
   void     SelectDrawable(Int_t device);

   //Does nothing.
   void     InitPainter();
   
   void     DrawLine(Double_t x1, Double_t y1, Double_t x2, Double_t y2);
   void     DrawBox(Double_t x1, Double_t y1, Double_t x2, Double_t y2, EBoxMode mode);
   //Double versions.
   void     DrawFillArea(UInt_t n, const Double_t *x, const Double_t *y);
   void     DrawPolyLine(UInt_t n, const Double_t *x, const Double_t *y);
   void     DrawPolyMarker(UInt_t n, const Double_t *x, const Double_t *y);
   //Obsolete float versions.
   void     DrawFillArea(UInt_t n, const Float_t *x, const Float_t *y);
   void     DrawPolyLine(UInt_t n, const Float_t *x, const Float_t *y);
   void     DrawPolyMarker(UInt_t n, const Float_t *x, const Float_t *y);
   
   void     DrawText(Double_t x, Double_t y, Double_t angle, Double_t mgn, 
                     const char *text, ETextMode mode);

   void     InvalidateCS();                  
private:
   TPadPainter(const TPadPainter &rhs);
   TPadPainter & operator = (const TPadPainter &rhs);
   
   ClassDef(TPadPainter, 0)
};

#endif
