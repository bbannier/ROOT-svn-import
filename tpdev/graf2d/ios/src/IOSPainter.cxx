#include <iostream>
#include <cstring>

#include <CoreText/CTStringAttributes.h>
#include <CoreText/CTFont.h>
#include <CoreText/CTLine.h>

#include "TVirtualX.h"
#include "TMath.h"

#include "IOSResourceManagement.h"
#include "IOSTextOperations.h"
#include "IOSGraphicUtils.h"
#include "IOSFillPatterns.h"
#include "IOSLineStyles.h"
#include "IOSPainter.h"
#include "IOSMarkers.h"

namespace ROOT_iOS {

//_________________________________________________________________
SpaceConverter::SpaceConverter()
                  : fXMin(0.),
                    fXConv(1.),
                    fYMin(0.),
                    fYConv(1.)
{
   //Default ctor.
}

//_________________________________________________________________
SpaceConverter::SpaceConverter(UInt_t w, Double_t xMin, Double_t xMax, UInt_t h, Double_t yMin, Double_t yMax)
{
   //Construct for conversion.
   SetConverter(w, xMin, xMax, h, yMin, yMax);
}

//_________________________________________________________________
void SpaceConverter::SetConverter(UInt_t w, Double_t xMin, Double_t xMax, UInt_t h, Double_t yMin, Double_t yMax)
{
   //Set conversion coefficients.
   fXMin = xMin;
   fXConv = w / (xMax - xMin);
   
   fYMin = yMin;
   fYConv = h / (yMax - yMin);
}

//_________________________________________________________________
inline Double_t SpaceConverter::XToView(Double_t x)const
{
   //From pad's user space to view's user space.
   return (x - fXMin) * fXConv;
}
   
//_________________________________________________________________
inline Double_t SpaceConverter::YToView(Double_t y)const
{
   //From pad's user space to view's user space.
   return (y - fYMin) * fYConv;
}

//_________________________________________________________________
Painter::Painter()
            : fCtx(0),
              fRootOpacity(100),
              fPainterMode(kPaintToView),
              fCurrentObjectID(0),
              fEncoder(10, 255) //radix is 10, color channel value is 255.
{
   //Default ctor.
}

//_________________________________________________________________
void Painter::SetOpacity(Int_t percent)
{
   //Opacity is strange at the moment - there is no line or polygon opacity,
   //only one opacity value.
   percent > 100 || percent < 0 ? fRootOpacity = 100 : fRootOpacity = percent;
}

//_________________________________________________________________      
void Painter::SetStrokeParameters()const
{
   //Painter can work in two modes: draw objects into a view or
   //draw them into a bitmap file, in a "selection mode".
   //In the latter case, line color is quite a special thing, it's used as 
   //object's identity. Line width is also different: to be selectable by tap gesture,
   //line has to be very thick. All these staff must be done externally, but
   //unfortunately ROOT specifies line parameters everywhere via gVirtualX,
   //what makes my life hard.

   //FIX: better algorithm to select line cap and line join.
   CGContextSetLineCap(fCtx, kCGLineCapRound);
   CGContextSetLineJoin(fCtx, kCGLineJoinMiter);
   
   if (fPainterMode == kPaintToSelectionBuffer) {
      SetLineColorForCurrentObjectID();
      CGContextSetLineWidth(fCtx, 40.f);//Ohh yeahhh, really thiiick line!
      return;
   }
   
   if (fPainterMode == kPaintSelected) {
      CGContextSetRGBStrokeColor(fCtx, 1.f, 0.f, 0.4f, 0.2f);
      CGContextSetLineWidth(fCtx, 5.f);
      return;
   }

   if (fPainterMode == kPaintShadow) {
      CGContextSetRGBStrokeColor(fCtx, 0.1f, 0.1f, 0.1f, 0.2f);
      CGContextSetLineWidth(fCtx, 5.f);
      return;
   }

   if (gVirtualX->GetLineWidth() > 1.f)
      CGContextSetLineWidth(fCtx, gVirtualX->GetLineWidth());

   const Float_t alpha = fRootOpacity / 100;
   Float_t red = 0.f, green = 0.f, blue = 0.f;//Black line by default.
   
   GraphicUtils::GetColorForIndex(gVirtualX->GetLineColor(), red, green, blue);
   CGContextSetRGBStrokeColor(fCtx, red, green, blue, alpha);
   
   const Style_t lineStyle = gVirtualX->GetLineStyle();
   if (lineStyle > 1 && lineStyle <= 10)
      CGContextSetLineDash(fCtx, 0., GraphicUtils::dashLinePatterns[lineStyle - 1], GraphicUtils::linePatternLengths[lineStyle - 1]);
   else
      CGContextSetLineDash(fCtx, 0., 0, 0);
}

//_________________________________________________________________      
void Painter::DrawLine(Double_t x1, Double_t y1, Double_t x2, Double_t y2)
{
   //In principle, stroke parameters (line style, width, color) must
   //be specified externally, before DrawLine is called.
   //Unfortunately, in ROOT gVirtualX->SetLineXXXX is called in every
   //possible place and not only inside gPad, so I simply can not 
   //controll all places, there line parameters are set. So
   //they are specified here.
   SetStrokeParameters();
   
   const Double_t x1p = fConverter.XToView(x1);
   const Double_t x2p = fConverter.XToView(x2);

   const Double_t y1p = fConverter.YToView(y1);
   const Double_t y2p = fConverter.YToView(y2);
   
   CGContextBeginPath(fCtx);
   CGContextMoveToPoint(fCtx, x1p, y1p);
   CGContextAddLineToPoint(fCtx, x2p, y2p);
   CGContextStrokePath(fCtx);
   
   //TODO: place stroke parameters in a guard object or save/restore graphics' state.
   //May be, Util::CGStateGuard?
   if (fPainterMode != kPaintToView || gVirtualX->GetLineWidth()  > 1.f)
      CGContextSetLineWidth(fCtx, 1.f);
}

//_________________________________________________________________
void Painter::DrawLineNDC(Double_t, Double_t, Double_t, Double_t)
{
   //Just an empty overrider. PadProxy does conversions required and
   //calls DrawLine instead of this.
}
   
//_________________________________________________________________
void Painter::SetPolygonParameters()const
{
   //TODO: check, if stroke parameters also should
   //be specified for polygon.
   
   if (fPainterMode == kPaintToSelectionBuffer)
      return SetPolygonColorForCurrentObjectID();

   if (fPainterMode == kPaintSelected) {
      CGContextSetRGBFillColor(fCtx, 1.f, 0.f, 0.4f, 0.2f);
      return;
   }

   if (fPainterMode == kPaintShadow) {
      CGContextSetRGBFillColor(fCtx, 0.1f, 0.1f, 0.1f, 0.2f);
      return;
   }

   const Float_t alpha = fRootOpacity / 100.f;
   Float_t red = 1.f, green = 1.f, blue = 1.f;//White by default.
 
   GraphicUtils::GetColorForIndex(gVirtualX->GetFillColor(), red, green, blue);

   CGContextSetRGBFillColor(fCtx, red, green, blue, alpha);
   CGContextSetRGBStrokeColor(fCtx, red, green, blue, alpha);
   CGContextSetLineWidth(fCtx, 1.f);
}

//_________________________________________________________________
Bool_t Painter::PolygonHasStipple()const
{
   const Style_t fillStyle = gVirtualX->GetFillStyle() / 1000;
   const Style_t pattern = gVirtualX->GetFillStyle() % 1000;

   return fillStyle == 3 && pattern >= 1 && pattern <= GraphicUtils::kPredefinedFillPatterns;
}

//_________________________________________________________________
void Painter::FillBoxWithPattern(Double_t x1, Double_t y1, Double_t x2, Double_t y2)const
{
   const Util::CGStateGuard state(fCtx);
   //
   Util::RefGuardGeneric<CGColorSpaceRef, CGColorSpaceRelease> patternColorSpace(CGColorSpaceCreatePattern(0));
   CGContextSetFillColorSpace(fCtx, patternColorSpace.Get());
   
   //patternIndex < kPredefinedFillPatterns, this is assumed by previous call
   //to PolygonHasStipples.
   Float_t rgb[3] = {};
   GraphicUtils::GetColorForIndex(gVirtualX->GetFillColor(), rgb[0], rgb[1], rgb[2]);
   const Style_t patternIndex = gVirtualX->GetFillStyle() % 1000 - 1;
   const Util::SmartRef<CGPatternRef, CGPatternRelease> pattern(GraphicUtils::gPatternGenerators[patternIndex](rgb));
   
   const float alpha = 1.f;
   CGContextSetFillPattern(fCtx, pattern.Get(), &alpha);
   
   CGContextBeginPath(fCtx);
   CGContextFillRect(fCtx, CGRectMake(x1, y1, x2 - x1, y2 - y1));
}

//_________________________________________________________________
void Painter::FillBox(Double_t x1, Double_t y1, Double_t x2, Double_t y2)const
{
   //Box with solid fill style.
   SetPolygonParameters();

   CGContextBeginPath(fCtx);
   CGContextAddRect(fCtx, CGRectMake(x1, y1, x2 - x1, y2 - y1));
   CGContextDrawPath(fCtx, kCGPathFillStroke);
}

//_________________________________________________________________
void Painter::DrawBoxOutline(Double_t x1, Double_t y1, Double_t x2, Double_t y2)const
{
   //Hollow box.
   SetStrokeParameters();

   CGContextBeginPath(fCtx);
   CGContextAddRect(fCtx, CGRectMake(x1, y1, x2 - x1, y2 - y1));
   CGContextStrokePath(fCtx);
}

//_________________________________________________________________
void Painter::DrawBox(Double_t x1, Double_t y1, Double_t x2, Double_t y2, EBoxMode mode)
{
   const Double_t x1p = fConverter.XToView(x1);
   const Double_t y1p = fConverter.YToView(y1);
   const Double_t x2p = fConverter.XToView(x2);
   const Double_t y2p = fConverter.YToView(y2);
   
   if (/*fPainterMode == kPaintToSelectionBuffer ||*/ fPainterMode == kPaintSelected)
      return DrawBoxOutline(x1p, y1p, x2p, y2p);
      
   if (fPainterMode == kPaintToSelectionBuffer && PolygonHasStipple())
      return DrawBoxOutline(x1p, y1p, x2p, y2p);
      
   if (mode == kFilled) 
      PolygonHasStipple() ? FillBoxWithPattern(x1p, y1p, x2p, y2p) : FillBox(x1p, y1p, x2p, y2p);
   else
      DrawBoxOutline(x1p, y1p, x2p, y2p);
}

namespace 
{

//_________________________________________________________________
template<class PointCoordinate>
void draw_polygon(CGContextRef ctx, UInt_t n, const PointCoordinate *x, const PointCoordinate *y, const SpaceConverter & sc, Bool_t withStroke = kTRUE)
{
   CGContextBeginPath(ctx);
   CGContextMoveToPoint(ctx, sc.XToView(x[0]), sc.YToView(y[0]));
   
   for (UInt_t i = 1; i < n; ++i)
      CGContextAddLineToPoint(ctx, sc.XToView(x[i]), sc.YToView(y[i]));

   CGContextClosePath(ctx);
   
   if (!withStroke)
      CGContextDrawPath(ctx, kCGPathFill);
   else
      CGContextDrawPath(ctx, kCGPathFillStroke);
}

//_________________________________________________________________
template<class PointCoordinate>
void draw_polyline(CGContextRef ctx, UInt_t n, const PointCoordinate *x, const PointCoordinate *y, const SpaceConverter & sc)
{
   CGContextBeginPath(ctx);
   
   CGContextMoveToPoint(ctx, sc.XToView(x[0]), sc.YToView(y[0]));

   for (UInt_t i = 1; i < n; ++i)
      CGContextAddLineToPoint(ctx, sc.XToView(x[i]), sc.YToView(y[i]));

   CGContextStrokePath(ctx);
}

}

//_________________________________________________________________
void Painter::FillAreaWithPattern(Int_t n, const Double_t *x, const Double_t *y)const
{
   const Util::CGStateGuard state(fCtx);
   //
   Util::RefGuardGeneric<CGColorSpaceRef, CGColorSpaceRelease> patternColorSpace(CGColorSpaceCreatePattern(0));
   CGContextSetFillColorSpace(fCtx, patternColorSpace.Get());
   
   Float_t rgb[3] = {};
   GraphicUtils::GetColorForIndex(gVirtualX->GetFillColor(), rgb[0], rgb[1], rgb[2]);
   const Style_t patternIndex = gVirtualX->GetFillStyle() % 1000 - 1;
   const Util::SmartRef<CGPatternRef, CGPatternRelease> pattern(GraphicUtils::gPatternGenerators[patternIndex](rgb));

   const float alpha = 1.f;
   CGContextSetFillPattern(fCtx, pattern.Get(), &alpha);

   draw_polygon(fCtx, n, x, y, fConverter, kFALSE);   
}

//_________________________________________________________________
void Painter::FillArea(Int_t n, const Double_t *x, const Double_t *y)const
{
   SetPolygonParameters();

   draw_polygon(fCtx, n, x, y, fConverter);
}

//_________________________________________________________________   
void Painter::DrawFillArea(Int_t n, const Double_t *x, const Double_t *y)
{
   //Check, may be, that's a hollow area, if so, call DrawPolyline instead.
   
   if (!gVirtualX->GetFillStyle())
      return DrawPolyLine(n, x, y);
      
   if (fPainterMode == kPaintSelected || fPainterMode == kPaintShadow || fPainterMode == kPaintThumbnail)
      return FillArea(n, x, y);
      
   if (fPainterMode == kPaintToSelectionBuffer && PolygonHasStipple())
      return DrawPolyLine(n, x, y);
         
   if (PolygonHasStipple())
      return FillAreaWithPattern(n, x, y);

   FillArea(n, x, y);
}

//_________________________________________________________________
void Painter::DrawFillArea(Int_t, const Float_t *, const Float_t *)
{
/*   if (!gVirtualX->GetFillStyle())
      return DrawPolyLine(n, x, y);

   SetPolygonParameters();
   draw_polygon(fCtx, n, x, y, fConverter);*/
}

//_________________________________________________________________
void Painter::DrawPolyLine(Int_t n, const Double_t *x, const Double_t *y)
{
   SetStrokeParameters();

   draw_polyline(fCtx, n, x, y, fConverter);

   if (gVirtualX->GetLineWidth() > 1.f || fPainterMode == kPaintToSelectionBuffer)
      CGContextSetLineWidth(fCtx, 1.f);
}

//_________________________________________________________________
void Painter::DrawPolyLine(Int_t n, const Float_t *x, const Float_t *y)
{
   SetStrokeParameters();

   draw_polyline(fCtx, n, x, y, fConverter);

   if (gVirtualX->GetLineWidth() > 1.f || fPainterMode == kPaintToSelectionBuffer)
      CGContextSetLineWidth(fCtx, 1.f);
}

//_________________________________________________________________
void Painter::DrawPolyLineNDC(Int_t, const Double_t *, const Double_t *)
{
}

//_________________________________________________________________
void Painter::DrawPolyMarker(Int_t n, const Double_t *x, const Double_t *y)
{
   fPolyMarker.resize(n);
   for (Int_t i = 0; i < n; ++i) {
      TPoint &p = fPolyMarker[i];
      p.SetX(fConverter.XToView(x[i]));
      p.SetY(fConverter.YToView(y[i]));
   }
   
   GraphicUtils::DrawPolyMarker(fCtx, fPolyMarker);
}

//_________________________________________________________________
void Painter::DrawPolyMarker(Int_t, const Float_t *, const Float_t *)
{

}

//_________________________________________________________________
void Painter::DrawText(Double_t x, Double_t y, const CTLineGuard &ctLine)
{
   UInt_t w = 0, h = 0;
   ctLine.GetBounds(w, h);
   
   x = fConverter.XToView(x);
   y = fConverter.YToView(y);
   
   Double_t xc = 0., yc = 0.;
   
   const UInt_t hAlign = UInt_t(gVirtualX->GetTextAlign() / 10);   
   switch (hAlign) {
   case 1:
      xc = 0.5 * w;
      break;
   case 2:
      break;
   case 3:
      xc = -0.5 * w;
      break;
   }

   const UInt_t vAlign = UInt_t(gVirtualX->GetTextAlign() % 10);
   switch (vAlign) {
   case 1:
      yc = 0.5 * h;
      break;
   case 2:
      break;
   case 3:
      yc = -0.5 * h;
      break;
   }
   
   CGContextSaveGState(fCtx);

   CGContextSetTextPosition(fCtx, 0.f, 0.f);
   CGContextTranslateCTM(fCtx, x, y);  
   CGContextRotateCTM(fCtx, gVirtualX->GetTextAngle() * TMath::DegToRad());
   CGContextTranslateCTM(fCtx, xc, yc);
   CGContextTranslateCTM(fCtx, -0.5 * w, -0.5 * h);

   CTLineDraw(ctLine.fCTLine, fCtx);
   CGContextRestoreGState(fCtx);
}

//_________________________________________________________________
void Painter::DrawText(Double_t x, Double_t y, const char *text, ETextMode /*mode*/)
{
   //TODO: mode parameter.
   if (fPainterMode == kPaintThumbnail) {
      CGContextSetRGBFillColor(fCtx, 0.f, 0.f, 0.f, 1.f);
      CGContextFillRect(fCtx, CGRectMake(fConverter.XToView(x), fConverter.YToView(y), 5.f, 2.f));
   }
   
   if (fPainterMode != kPaintToView)//TEXT is not selectable, no text in thumbnail either.
      return;
     
   CTFontRef currentFont = fFontManager.SelectFont(gVirtualX->GetTextFont(), gVirtualX->GetTextSize());
   if (gVirtualX->GetTextFont() / 10 - 1 == 11) {
      CTLineGuard ctLine(text, currentFont, fFontManager.GetSymbolMap());
      DrawText(x, y, ctLine);
   } else {
      CTLineGuard ctLine(text, currentFont, gVirtualX->GetTextColor());
      DrawText(x, y, ctLine);
   }
}

//_________________________________________________________________
void Painter::DrawTextNDC(Double_t, Double_t, const char *, ETextMode)
{
}

//_________________________________________________________________
void Painter::SetContext(CGContextRef ctx)
{
   fCtx = ctx;
}

//_________________________________________________________________
void Painter::SetTransform(UInt_t w, Double_t xMin, Double_t xMax, UInt_t h, Double_t yMin, Double_t yMax)
{
   fConverter.SetConverter(w, xMin, xMax, h, yMin, yMax);
}

//_________________________________________________________________
void Painter::GetTextExtent(UInt_t &w, UInt_t &h, const char *text)
{
   if (fPainterMode == kPaintThumbnail) {
      h = 1;
      w = std::strlen(text);
      return;
   }
   
   fFontManager.SelectFont(gVirtualX->GetTextFont(), gVirtualX->GetTextSize());
   fFontManager.GetTextBounds(w, h, text);
}

//_________________________________________________________________
void Painter::SetLineColorForCurrentObjectID() const
{
   Float_t rgb[3] = {};
   fEncoder.IdToColor(fCurrentObjectID, rgb);
   CGContextSetRGBStrokeColor(fCtx, rgb[0], rgb[1], rgb[2], 1.f);
}

//_________________________________________________________________
void Painter::SetPolygonColorForCurrentObjectID() const
{
   //std::cout<<"ID is "<<fCurrentObjectID<<std::endl;
   Float_t rgb[3] = {};
   fEncoder.IdToColor(fCurrentObjectID, rgb);
   CGContextSetRGBFillColor(fCtx, rgb[0], rgb[1], rgb[2], 1.f);
}

//_________________________________________________________________
void Painter::SetLineColorHighlighted() const
{
   CGContextSetRGBStrokeColor(fCtx, 1.f, 0.f, 0.5f, 0.5f);
}

}
