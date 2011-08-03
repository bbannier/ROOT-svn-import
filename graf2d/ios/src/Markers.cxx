#include "TAttMarker.h"
#include "TVirtualX.h"

#include "GraphicUtils.h"
#include "Markers.h"

namespace ROOT_iOS {
namespace GraphicUtils {

namespace {

typedef std::vector<TPoint>::size_type size_type;

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
void DrawMarkerCircle(CGContextRef /*ctx*/, const std::vector<TPoint> &/*xy*/)
{
}

//______________________________________________________________________________
void DrawMarkerX(CGContextRef /*ctx*/, const std::vector<TPoint> &/*xy*/)
{
}

//______________________________________________________________________________
void DrawMarkerFullDotSmall(CGContextRef /*ctx*/, const std::vector<TPoint> &/*xy*/)
{
}

//______________________________________________________________________________
void DrawMarkerFullDotMedium(CGContextRef /*ctx*/, const std::vector<TPoint> &/*xy*/)
{
}

//______________________________________________________________________________
void DrawMarkerFullDotLarge(CGContextRef /*ctx*/, const std::vector<TPoint> &/*xy*/)
{
}

//______________________________________________________________________________
void DrawMarkerFullSquare(CGContextRef /*ctx*/, const std::vector<TPoint> &/*xy*/)
{
}

//______________________________________________________________________________
void DrawMarkerFullTrianlgeUp(CGContextRef /*ctx*/, const std::vector<TPoint> &/*xy*/)
{
}

//______________________________________________________________________________
void DrawMarkerFullTrianlgeDown(CGContextRef /*ctx*/, const std::vector<TPoint> &/*xy*/)
{
}

//______________________________________________________________________________
void DrawMarkerDiamond(CGContextRef /*ctx*/, const std::vector<TPoint> &/*xy*/)
{
}

//______________________________________________________________________________
void DrawMarkerCross(CGContextRef /*ctx*/, const std::vector<TPoint> &/*xy*/)
{
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
void DrawMarkerOpenStar(CGContextRef /*ctx*/, const std::vector<TPoint> &/*xy*/)
{
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
      DrawMarkerCircle(ctx, xy);
      break;
   case kMultiply:
      DrawMarkerX(ctx, xy);
      break;
   case kFullDotSmall://"Full dot small"
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
      DrawMarkerFullSquare(ctx, xy);
      break;
   case kOpenTriangleUp:
      DrawMarkerFullTrianlgeUp(ctx, xy);
      break;
   case kOpenDiamond:
      DrawMarkerDiamond(ctx, xy);
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
