#ifndef ROOT_TGLPadPainter
#define ROOT_TGLPadPainter

#include <deque>

#ifndef ROOT_TVirtualPadPainter
#include "TVirtualPadPainter.h"
#endif
#ifndef ROOT_TGLFontmanager
#include "TGLFontManager.h"
#endif
#ifndef ROOT_TGLPadUtils
#include "TGLPadUtils.h"
#endif
#ifndef ROOT_TPoint
#include "TPoint.h"
#endif

class TCanvas;

/*
The _only_ purpose of TGLPadPainter is to enable 2d gl raphics
inside standard TPad/TCanvas.
So, all TPad/TCanvas dependencies _ARE_ required,
and this is _VERY_ specific class, designed and 
implemented for TPad/TCanvas exclusively.

I'm not interested at all, how can it work/crush 
outside of TPad/TCanvas.
*/
class TGLPadPainter : public TVirtualPadPainter {
private:
   //std::deque<Rgl::Pad::OffScreenDevice> fOffs;
   Rgl::Pad::PolygonStippleSet fSSet;
   Rgl::Pad::Tesselator        fTess;
   Rgl::Pad::MarkerPainter     fMarker;
   Rgl::Pad::GLLimits          fLimits;
   //
   TCanvas                    *fCanvas;
   //
   Double_t fX;
   Double_t fY;
   Double_t fW;
   Double_t fH;
   
   std::vector<Double_t>       fVs;//Vertex buffer for tesselator.
   
   TGLFontManager              fFM;
   TGLFont                     fF;
   
   Int_t                       fVp[4];

   std::vector<TPoint>         fPoly;
public:
   TGLPadPainter(TCanvas *cnv);
   //~TGLPadPainter();
   
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

   void     InitPainter();
   
   void     DrawLine(Double_t x1, Double_t y1, Double_t x2, Double_t y2);
   void     DrawBox(Double_t x1, Double_t y1, Double_t x2, Double_t y2, EBoxMode mode);
   
   void     DrawFillArea(UInt_t n, const Double_t *x, const Double_t *y);
   void     DrawPolyLine(UInt_t n, const Double_t *x, const Double_t *y);
   void     DrawPolyMarker(UInt_t n, const Double_t *x, const Double_t *y);
   //
   void     DrawFillArea(UInt_t n, const Float_t *x, const Float_t *y);
   void     DrawPolyLine(UInt_t n, const Float_t *x, const Float_t *y);
   void     DrawPolyMarker(UInt_t n, const Float_t *x, const Float_t *y);
   
   void     DrawText(Double_t x, Double_t y, Double_t angle, Double_t mgn, 
                     const char *text, ETextMode mode);
                     
   void     InvalidateCS();
private:
   Double_t GetX(Double_t x)const;
   Double_t GetY(Double_t y)const;
   
   void SaveProjectionMatrix()const;
   void RestoreProjectionMatrix()const;
   
   void SaveModelviewMatrix()const;
   void RestoreModelviewMatrix()const;
   
   void SaveViewport();
   void RestoreViewport();

   void ConvertMarkerPoints(UInt_t n, const Double_t *x, const Double_t *y);
   
   TGLPadPainter(const TGLPadPainter &rhs);
   TGLPadPainter & operator = (const TGLPadPainter &rhs);
   
   ClassDef(TGLPadPainter, 0)
};

#endif
