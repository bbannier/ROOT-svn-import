#include "TAttMarker.h"
#include "TVirtualX.h"

#include "IOSGraphicUtils.h"
#include "IOSMarkers.h"

namespace ROOT_iOS {
namespace GraphicUtils {

namespace {

typedef std::vector<TPoint>::size_type size_type;

//______________________________________________________________________________
void SetMarkerFillColor(CGContextRef ctx)
{
   Float_t r = 0.f, g = 0.f, b = 0.f, a = 1.f;
   GetColorForIndex(gVirtualX->GetMarkerColor(), r, g, b);
   CGContextSetRGBFillColor(ctx, r, g, b, a);
}

//______________________________________________________________________________
void SetMarkerStrokeColor(CGContextRef ctx)
{
   Float_t r = 0.f, g = 0.f, b = 0.f, a = 1.f;
   GetColorForIndex(gVirtualX->GetMarkerColor(), r, g, b);
   CGContextSetRGBStrokeColor(ctx, r, g, b, a);
}

}

//______________________________________________________________________________
void DrawMarkerDot(CGContextRef /*ctx*/, const std::vector<TPoint> &/*xy*/)
{
}

//______________________________________________________________________________
void DrawMarkerPlus(CGContextRef /*ctx*/, const std::vector<TPoint> &/*xy*/)
{
}

//______________________________________________________________________________
void DrawMarkerStar(CGContextRef /*ctx*/, const std::vector<TPoint> &/*xy*/)
{
}

//______________________________________________________________________________
void DrawMarkerOpenCircle(CGContextRef ctx, const std::vector<TPoint> &xy)
{
   SetMarkerStrokeColor(ctx);
   Double_t r = 4 * gVirtualX->GetMarkerSize() + 0.5;
   if (r > 100.)
      r = 100.;//as in TGX11.
   
   const Double_t d = 2 * r;
   
   for (size_type i = 0, e = xy.size(); i < e; ++i) {
      const Double_t x = xy[i].fX;
      const Double_t y = xy[i].fY;

      const CGRect rect = CGRectMake(x - r, y - r, d, d);
      CGContextStrokeEllipseInRect(ctx, rect);
   }
}

//______________________________________________________________________________
void DrawMarkerX(CGContextRef ctx, const std::vector<TPoint> &xy)
{
   SetMarkerStrokeColor(ctx);

   const Double_t im = 0.707 * (4 * gVirtualX->GetMarkerSize() + 0.5) + 0.5;
   for (size_type i = 0, e = xy.size(); i < e; ++i) {
      const Double_t x = xy[i].fX;
      const Double_t y = xy[i].fY;

      CGContextBeginPath(ctx);      
      CGContextMoveToPoint(ctx, -im + x, -im + y);
      CGContextAddLineToPoint(ctx, im + x, im + y);
      CGContextDrawPath(ctx, kCGPathStroke);
      
      CGContextBeginPath(ctx);
      CGContextMoveToPoint(ctx, -im + x, im + y);
      CGContextAddLineToPoint(ctx, im + x, -im + y);
      CGContextDrawPath(ctx, kCGPathStroke);
   }
}

//______________________________________________________________________________
void DrawMarkerFullDotSmall(CGContextRef ctx, const std::vector<TPoint> &xy)
{
   SetMarkerStrokeColor(ctx);

   for (size_type i = 0, e = xy.size(); i < e; ++i) {
      const Double_t x = xy[i].fX;
      const Double_t y = xy[i].fY;

      CGContextBeginPath(ctx);
      CGContextMoveToPoint(ctx, -1. + x, y);
      CGContextAddLineToPoint(ctx, x + 1., y);
      CGContextDrawPath(ctx, kCGPathStroke);
      
      CGContextBeginPath(ctx);
      CGContextMoveToPoint(ctx, x, -1. + y);
      CGContextAddLineToPoint(ctx, x, 1. + y);
      CGContextDrawPath(ctx, kCGPathStroke);
   }
}

//______________________________________________________________________________
void DrawMarkerFullDotMedium(CGContextRef ctx, const std::vector<TPoint> &xy)
{
   SetMarkerFillColor(ctx);

   for (size_type i = 0, e = xy.size(); i < e; ++i)
      CGContextFillRect(ctx, CGRectMake(xy[i].fX - 1, xy[i].fY - 1, 3.f, 3.f));
}

//______________________________________________________________________________
void DrawMarkerFullDotLarge(CGContextRef ctx, const std::vector<TPoint> &xy)
{
   SetMarkerFillColor(ctx);

   Double_t radius = 4 * gVirtualX->GetMarkerSize() + 0.5;
   if (radius > 100.)
      radius = 100;//as in TGX11.

   const Double_t d = 2 * radius;

   for (size_type i = 0, e = xy.size(); i < e; ++i) {
      const Double_t x = xy[i].fX;
      const Double_t y = xy[i].fY;
      
      const CGRect rect = CGRectMake(x - radius, y - radius, d, d);
      CGContextFillEllipseInRect(ctx, rect);
   }
}

//______________________________________________________________________________
void DrawMarkerFullSquare(CGContextRef ctx, const std::vector<TPoint> &xy)
{
   SetMarkerFillColor(ctx);

   const Double_t im = 4 * gVirtualX->GetMarkerSize() + 0.5;
   for (size_type i = 0, e = xy.size(); i < e; ++i) {
      const CGRect rect = CGRectMake(xy[i].fX - im, xy[i].fY - im, im * 2, im * 2);
      CGContextFillRect(ctx, rect);
   }
}

//______________________________________________________________________________
void DrawMarkerOpenSquare(CGContextRef ctx, const std::vector<TPoint> &xy)
{
   SetMarkerStrokeColor(ctx);

   const Double_t im = 4 * gVirtualX->GetMarkerSize() + 0.5;
   for (size_type i = 0, e = xy.size(); i < e; ++i) {
      const CGRect rect = CGRectMake(xy[i].fX - im, xy[i].fY - im, im * 2, im * 2);
      CGContextStrokeRect(ctx, rect);
   }
}


//______________________________________________________________________________
void DrawMarkerFullTrianlgeUp(CGContextRef ctx, const std::vector<TPoint> &xy)
{
   SetMarkerFillColor(ctx);

   const Double_t im = 4 * gVirtualX->GetMarkerSize() + 0.5;
   for (size_type i = 0, e = xy.size(); i < e; ++i) {
      const Double_t x = xy[i].fX;
      const Double_t y = xy[i].fY;
      CGContextBeginPath(ctx);
      CGContextMoveToPoint(ctx, x - im, y - im);
      CGContextAddLineToPoint(ctx, x + im, y - im);
      CGContextAddLineToPoint(ctx, x, im + y);
      CGContextDrawPath(ctx, kCGPathFillStroke);
   }
}

//______________________________________________________________________________
void DrawMarkerOpenTrianlgeUp(CGContextRef ctx, const std::vector<TPoint> &xy)
{
   SetMarkerStrokeColor(ctx);

   const Double_t im = 4 * gVirtualX->GetMarkerSize() + 0.5;
   for (size_type i = 0, e = xy.size(); i < e; ++i) {
      const Double_t x = xy[i].fX;
      const Double_t y = xy[i].fY;
      CGContextBeginPath(ctx);
      CGContextMoveToPoint(ctx, x - im, y - im);
      CGContextAddLineToPoint(ctx, x + im, y - im);
      CGContextAddLineToPoint(ctx, x, im + y);
      CGContextAddLineToPoint(ctx, x - im, y - im);
      CGContextDrawPath(ctx, kCGPathStroke);
   }
}

//______________________________________________________________________________
void DrawMarkerFullTrianlgeDown(CGContextRef ctx, const std::vector<TPoint> &xy)
{
   SetMarkerFillColor(ctx);

   const Int_t im = Int_t(4 * gVirtualX->GetMarkerSize() + 0.5);   
   for (size_type i = 0, e = xy.size(); i < e; ++i) {
      const Double_t x = xy[i].fX;
      const Double_t y = xy[i].fY;

      CGContextBeginPath(ctx);
      CGContextMoveToPoint(ctx, x - im, y + im);
      CGContextAddLineToPoint(ctx, x, y - im);
      CGContextAddLineToPoint(ctx, im + x, y + im);
      CGContextDrawPath(ctx, kCGPathFillStroke);
   }
}

//______________________________________________________________________________
void DrawMarkerDiamond(CGContextRef ctx, const std::vector<TPoint> &xy)
{
   SetMarkerFillColor(ctx);

   const Int_t im  = Int_t(4 * gVirtualX->GetMarkerSize() + 0.5);
   const Int_t imx = Int_t(2.66 * gVirtualX->GetMarkerSize() + 0.5);
   
   for (size_type i = 0, e = xy.size(); i < e; ++i) {
      const Double_t x = xy[i].fX;
      const Double_t y = xy[i].fY;
      
      CGContextBeginPath(ctx);
      CGContextMoveToPoint(ctx, x - imx,  y);
      CGContextAddLineToPoint(ctx, x, y - im);
      CGContextAddLineToPoint(ctx, x + imx, y);
      CGContextAddLineToPoint(ctx, x, y + im);
      CGContextDrawPath(ctx, kCGPathFillStroke);
   }
}

//______________________________________________________________________________
void DrawMarkerOpenDiamond(CGContextRef ctx, const std::vector<TPoint> &xy)
{
   SetMarkerStrokeColor(ctx);

   const Int_t im  = Int_t(4 * gVirtualX->GetMarkerSize() + 0.5);
   const Int_t imx = Int_t(2.66 * gVirtualX->GetMarkerSize() + 0.5);
   
   for (size_type i = 0, e = xy.size(); i < e; ++i) {
      const Double_t x = xy[i].fX;
      const Double_t y = xy[i].fY;
      
      CGContextBeginPath(ctx);
      CGContextMoveToPoint(ctx, x - imx,  y);
      CGContextAddLineToPoint(ctx, x, y - im);
      CGContextAddLineToPoint(ctx, x + imx, y);
      CGContextAddLineToPoint(ctx, x, y + im);
      CGContextAddLineToPoint(ctx, x - imx,  y);
      CGContextDrawPath(ctx, kCGPathStroke);
   }
}

//______________________________________________________________________________
void DrawMarkerCross(CGContextRef ctx, const std::vector<TPoint> &xy)
{
   Float_t r = 0.f, g = 0.f, b = 0.f, a = 1.f;
   GetColorForIndex(gVirtualX->GetMarkerColor(), r, g, b);
   CGContextSetRGBStrokeColor(ctx, r, g, b, a);

   const Int_t im  = Int_t(4 * gVirtualX->GetMarkerSize() + 0.5);
   const Int_t imx = Int_t(1.33 * gVirtualX->GetMarkerSize() + 0.5);

   for (size_type i = 0, e = xy.size(); i < e; ++i) {
      const Double_t x = xy[i].fX;
      const Double_t y = xy[i].fY;
   
      CGContextBeginPath(ctx);
      CGContextMoveToPoint(ctx, x - im, y - imx);
      CGContextAddLineToPoint(ctx, x - imx, y - imx);
      CGContextAddLineToPoint(ctx, x - imx, y - im);
      CGContextAddLineToPoint(ctx, x + imx, y - im);
      CGContextAddLineToPoint(ctx, x + imx, y - imx);
      CGContextAddLineToPoint(ctx, x + im, y - imx);
      CGContextAddLineToPoint(ctx, x + im, y + imx);
      CGContextAddLineToPoint(ctx, x + imx, y + imx);
      CGContextAddLineToPoint(ctx, x + imx, y + im);
      CGContextAddLineToPoint(ctx, x - imx, y + im);
      CGContextAddLineToPoint(ctx, x - imx, y + imx);
      CGContextAddLineToPoint(ctx, x - im, y + imx);
      CGContextAddLineToPoint(ctx, x - im, y - imx);
      CGContextDrawPath(ctx, kCGPathStroke);
   }
}

//______________________________________________________________________________
void DrawMarkerFullStar(CGContextRef ctx, const std::vector<TPoint> &xy)
{
   // HIGZ full star pentagone
   const Size_t markerSize = gVirtualX->GetMarkerSize();
   const Int_t im  = Int_t(4 * markerSize + 0.5);
   const Int_t im1 = Int_t(0.66 * markerSize + 0.5);
   const Int_t im2 = Int_t(2.00 * markerSize + 0.5);
   const Int_t im3 = Int_t(2.66 * markerSize + 0.5);
   const Int_t im4 = Int_t(1.33 * markerSize + 0.5);
   
   Float_t r = 0.f, g = 0.f, b = 0.f, a = 1.f;
   GetColorForIndex(gVirtualX->GetMarkerColor(), r, g, b);
   CGContextSetRGBFillColor(ctx, r, g, b, a);
   
   for (size_type i = 0, e = xy.size(); i < e; ++i) {
      const Double_t x = xy[i].fX;
      const Double_t y = xy[i].fY;

      CGContextBeginPath(ctx);
      CGContextMoveToPoint(ctx, x - im, y - im4);
      CGContextAddLineToPoint(ctx, x - im2, y + im1);
      CGContextAddLineToPoint(ctx, x - im4, y - im4);
      CGContextDrawPath(ctx, kCGPathFillStroke);
 
      CGContextBeginPath(ctx);
      CGContextMoveToPoint(ctx, x - im2, y + im1);//1
      CGContextAddLineToPoint(ctx, x - im3, y + im);//2
      CGContextAddLineToPoint(ctx, x, y + im2);//3
      CGContextDrawPath(ctx, kCGPathFillStroke);
      
      CGContextBeginPath(ctx);
      CGContextMoveToPoint(ctx, x, y + im2);//3
      CGContextAddLineToPoint(ctx, x + im3, y + im);//4
      CGContextAddLineToPoint(ctx, x + im2, y + im1);//5
      CGContextDrawPath(ctx, kCGPathFillStroke);
      
      CGContextBeginPath(ctx);
      CGContextMoveToPoint(ctx, x + im2, y + im1);//5
      CGContextAddLineToPoint(ctx, x + im, y - im4);//6
      CGContextAddLineToPoint(ctx,x + im4, y - im4);//7
      CGContextDrawPath(ctx, kCGPathFillStroke);
      
      CGContextBeginPath(ctx);
      CGContextMoveToPoint(ctx, x + im4, y - im4);//7
      CGContextAddLineToPoint(ctx, x, y - im);//8
      CGContextAddLineToPoint(ctx, x - im4, y - im4);//9
      CGContextDrawPath(ctx, kCGPathFillStroke);
            
      CGContextBeginPath(ctx);
      CGContextMoveToPoint(ctx, x - im4, y - im4);//9
      CGContextAddLineToPoint(ctx, x - im2, y + im1);//1
      CGContextAddLineToPoint(ctx, x, y + im2);//3
      CGContextDrawPath(ctx, kCGPathFillStroke);
            
      CGContextBeginPath(ctx);
      CGContextMoveToPoint(ctx, x - im4, y - im4);//9
      CGContextAddLineToPoint(ctx, x, y + im2);//3
      CGContextAddLineToPoint(ctx, x + im2, y + im1);//5
      CGContextDrawPath(ctx, kCGPathFillStroke);
            
      CGContextBeginPath(ctx);
      CGContextMoveToPoint(ctx, x - im4, y - im4);//9
      CGContextAddLineToPoint(ctx, x + im2, y + im1);//5
      CGContextAddLineToPoint(ctx, x + im4, y - im4);//7
      CGContextDrawPath(ctx, kCGPathFillStroke);   
   }
}

//______________________________________________________________________________
void DrawMarkerOpenStar(CGContextRef ctx, const std::vector<TPoint> &xy)
{
   SetMarkerStrokeColor(ctx);

   const Int_t im  = Int_t(4 * gVirtualX->GetMarkerSize() + 0.5);
   const Int_t im1 = Int_t(0.66 * gVirtualX->GetMarkerSize() + 0.5);
   const Int_t im2 = Int_t(2.00 * gVirtualX->GetMarkerSize() + 0.5);
   const Int_t im3 = Int_t(2.66 * gVirtualX->GetMarkerSize() + 0.5);
   const Int_t im4 = Int_t(1.33 * gVirtualX->GetMarkerSize() + 0.5);
   
   for (size_type i = 0, e = xy.size(); i < e; ++i) {
      const Double_t x = xy[i].fX;
      const Double_t y = xy[i].fY;

      CGContextBeginPath(ctx);
      CGContextMoveToPoint(ctx, x - im, y - im4);
      CGContextAddLineToPoint(ctx, x - im2, y + im1);
      CGContextAddLineToPoint(ctx, x - im3, y + im);
      CGContextAddLineToPoint(ctx, x, y + im2);
      CGContextAddLineToPoint(ctx, x + im3, y + im);
      CGContextAddLineToPoint(ctx, x + im2, y + im1);
      CGContextAddLineToPoint(ctx, x + im, y - im4);
      CGContextAddLineToPoint(ctx, x + im4, y - im4);
      CGContextAddLineToPoint(ctx, x, y - im);
      CGContextAddLineToPoint(ctx, x - im4, y - im4);
      CGContextAddLineToPoint(ctx, x - im, y - im4);
      CGContextDrawPath(ctx, kCGPathStroke);
   }
}

//______________________________________________________________________________
void DrawPolyMarker(CGContextRef ctx, const std::vector<TPoint> &xy)
{
   const Style_t markerStyle = gVirtualX->GetMarkerStyle();
   
   switch (markerStyle) {
   case kDot:
      DrawMarkerDot(ctx, xy);
      break;
   case kPlus:
      DrawMarkerPlus(ctx, xy);
      break;
   case kStar:
      DrawMarkerStar(ctx, xy);
      break;
   case kCircle:
   case kOpenCircle:
      DrawMarkerOpenCircle(ctx, xy);
      break;
   case kMultiply:
      DrawMarkerX(ctx, xy);
      break;
   case kFullDotSmall:
      DrawMarkerFullDotSmall(ctx, xy);
      break;
   case kFullDotMedium:
      DrawMarkerFullDotMedium(ctx, xy);
      break;
   case kFullDotLarge:
   case kFullCircle:
      DrawMarkerFullDotLarge(ctx, xy);
      break;
   case kFullSquare:
      DrawMarkerFullSquare(ctx, xy);
      break;
   case kFullTriangleUp:
      DrawMarkerFullTrianlgeUp(ctx, xy);
      break;
   case kFullTriangleDown:
      DrawMarkerFullTrianlgeDown(ctx, xy);
      break;
   case kOpenSquare:
      DrawMarkerOpenSquare(ctx, xy);
      break;
   case kOpenTriangleUp:
      DrawMarkerOpenTrianlgeUp(ctx, xy);
      break;
   case kOpenDiamond:
      DrawMarkerOpenDiamond(ctx, xy);
      break;
   case kOpenCross:
      DrawMarkerCross(ctx, xy);
      break;
   case kFullStar:
      DrawMarkerFullStar(ctx, xy);
      break;
   case kOpenStar:
      DrawMarkerOpenStar(ctx, xy);
      break;
   }   
}

}//namespace GraphicUtils
}//namespace ROOT_iOS
