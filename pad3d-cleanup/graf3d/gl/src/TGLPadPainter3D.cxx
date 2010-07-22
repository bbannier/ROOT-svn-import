// @(#)root/gl:$Id$
// Author:  Timur Pocheptsov  06/05/2009

/*************************************************************************
 * Copyright (C) 1995-2009, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include <stdexcept>
#include <memory>
#include <vector>

#include "TAttMarker.h"
#include "TVirtualX.h"
#include "TError.h"
#include "TImage.h"
#include "TROOT.h"
#include "TMath.h"
#include "TPad.h"

#include "TGLPadPainter3D.h"
#include "TGLIncludes.h"
#include "TGLUtil.h"
#include "TError.h"

//______________________________________________________________________________
   //"Delegating" part of TGLPadPainter3D. Line/fill/etc. attributes can be
   //set inside TPad, but not only there:
   //many of them are set by base sub-objects of 2d primitives
   //(2d primitives usually inherit TAttLine or TAttFill etc.).  And these sub-objects
   //call gVirtualX->SetLineWidth ... etc. So, if I save some attributes in my painter,
   //it will be mess - at any moment I do not know, where to take line attribute - from
   //gVirtualX or from my own member. So! All attributed, _ALL_ go to/from gVirtualX.

ClassImp(TGLPadPainter3D)

//______________________________________________________________________________
TGLPadPainter3D::TGLPadPainter3D()
                  : fIsHollowArea(kFALSE),
                    fLocked(kTRUE)
{
   fVp[0] = fVp[1] = fVp[2] = fVp[3] = 0;
}


//______________________________________________________________________________
Color_t TGLPadPainter3D::GetLineColor() const
{
   //Delegate to gVirtualX.
   return gVirtualX->GetLineColor();
}

//______________________________________________________________________________
Style_t TGLPadPainter3D::GetLineStyle() const
{
   //Delegate to gVirtualX.
   return gVirtualX->GetLineStyle();
}

//______________________________________________________________________________
Width_t TGLPadPainter3D::GetLineWidth() const
{
   //Delegate to gVirtualX.
   return gVirtualX->GetLineWidth();
}

//______________________________________________________________________________
void TGLPadPainter3D::SetLineColor(Color_t lcolor)
{
   //Delegate to gVirtualX.
   gVirtualX->SetLineColor(lcolor);
}

//______________________________________________________________________________
void TGLPadPainter3D::SetLineStyle(Style_t lstyle)
{
   //Delegate to gVirtualX.
   gVirtualX->SetLineStyle(lstyle);
}

//______________________________________________________________________________
void TGLPadPainter3D::SetLineWidth(Width_t lwidth)
{
   //Delegate to gVirtualX.
   gVirtualX->SetLineWidth(lwidth);
}

//______________________________________________________________________________
Color_t TGLPadPainter3D::GetFillColor() const
{
   //Delegate to gVirtualX.
   return gVirtualX->GetFillColor();
}

//______________________________________________________________________________
Style_t TGLPadPainter3D::GetFillStyle() const
{
   //Delegate to gVirtualX.
   return gVirtualX->GetFillStyle();
}

//______________________________________________________________________________
Bool_t TGLPadPainter3D::IsTransparent() const
{
   //Delegate to gVirtualX.
   //IsTransparent is implemented as inline function in TAttFill.
   return gVirtualX->IsTransparent();
}

//______________________________________________________________________________
void TGLPadPainter3D::SetFillColor(Color_t fcolor)
{
   //Delegate to gVirtualX.
   gVirtualX->SetFillColor(fcolor);
}

//______________________________________________________________________________
void TGLPadPainter3D::SetFillStyle(Style_t fstyle)
{
   //Delegate to gVirtualX.
   gVirtualX->SetFillStyle(fstyle);
}

//______________________________________________________________________________
void TGLPadPainter3D::SetOpacity(Int_t percent)
{
   //Delegate to gVirtualX.
   gVirtualX->SetOpacity(percent);
}

//______________________________________________________________________________
Short_t TGLPadPainter3D::GetTextAlign() const
{
   //Delegate to gVirtualX.
   return gVirtualX->GetTextAlign();
}

//______________________________________________________________________________
Float_t TGLPadPainter3D::GetTextAngle() const
{
   //Delegate to gVirtualX.
   return gVirtualX->GetTextAngle();
}

//______________________________________________________________________________
Color_t TGLPadPainter3D::GetTextColor() const
{
   //Delegate to gVirtualX.
   return gVirtualX->GetTextColor();
}

//______________________________________________________________________________
Font_t TGLPadPainter3D::GetTextFont() const
{
   //Delegate to gVirtualX.
   return gVirtualX->GetTextFont();
}

//______________________________________________________________________________
Float_t TGLPadPainter3D::GetTextSize() const
{
   //Delegate to gVirtualX.
   return gVirtualX->GetTextSize();
}

//______________________________________________________________________________
Float_t TGLPadPainter3D::GetTextMagnitude() const
{
   //Delegate to gVirtualX.
   return gVirtualX->GetTextMagnitude();
}

//______________________________________________________________________________
void TGLPadPainter3D::SetTextAlign(Short_t align)
{
   //Delegate to gVirtualX.
   gVirtualX->SetTextAlign(align);
}

//______________________________________________________________________________
void TGLPadPainter3D::SetTextAngle(Float_t tangle)
{
   //Delegate to gVirtualX.
   gVirtualX->SetTextAngle(tangle);
}

//______________________________________________________________________________
void TGLPadPainter3D::SetTextColor(Color_t tcolor)
{
   //Delegate to gVirtualX.
   gVirtualX->SetTextColor(tcolor);
}

//______________________________________________________________________________
void TGLPadPainter3D::SetTextFont(Font_t tfont)
{
   //Delegate to gVirtualX.
   gVirtualX->SetTextFont(tfont);
}

//______________________________________________________________________________
void TGLPadPainter3D::SetTextSize(Float_t tsize)
{
   //Delegate to gVirtualX.
   gVirtualX->SetTextSize(tsize);
}

//______________________________________________________________________________
void TGLPadPainter3D::SetTextSizePixels(Int_t npixels)
{
   //Delegate to gVirtualX.
   gVirtualX->SetTextSizePixels(npixels);
}

/*
"Pixmap" part of TGLPadPainter3D.
*/

//______________________________________________________________________________
Int_t TGLPadPainter3D::CreateDrawable(UInt_t/*w*/, UInt_t/*h*/)
{
   //Not required at the moment.
   return 0;
}

//______________________________________________________________________________
void TGLPadPainter3D::ClearDrawable()
{
   //Not required at the moment.
}

//______________________________________________________________________________
void TGLPadPainter3D::CopyDrawable(Int_t /*id*/, Int_t /*px*/, Int_t /*py*/)
{
   //Not required at the moment.
}

//______________________________________________________________________________
void TGLPadPainter3D::DestroyDrawable()
{
   //Not required at the moment.
}

//______________________________________________________________________________
void TGLPadPainter3D::SelectDrawable(Int_t /*device*/)
{
   //For gVirtualX this means select pixmap (or window)
   //and all subsequent drawings will go into
   //this pixmap. For OpenGL this means the change of
   //coordinate system and viewport.
   if (fLocked)
      return;

   if (TPad *pad = dynamic_cast<TPad*>(gPad))
   {
      if (fForGLViewer)
      {
         return;
      }
      else
      {
         glViewport(TMath::Nint(fVp[0]*pad->GetAbsXlowNDC()), TMath::Nint(fVp[1]*(1.0 - pad->GetAbsYlowNDC())),
                    TMath::Nint(fVp[2]*pad->GetAbsWNDC()),    TMath::Nint(fVp[3]*pad->GetAbsHNDC()));

         glMatrixMode(GL_PROJECTION);
         glLoadIdentity();
         glOrtho(pad->GetX1(), pad->GetX2(), pad->GetY1(), pad->GetY2(), -10., 10.);

         // XXXX WTF???
         // glMatrixMode(GL_MODELVIEW);
         // glLoadIdentity();
         // glTranslated(0., 0., -1.);
      }
   }
   else
   {
      Error("TGLPadPainter3D::SelectDrawable", "function was called not from TPad or TCanvas code\n");
      throw std::runtime_error("");
   }
}

//______________________________________________________________________________
void TGLPadPainter3D::InitPainter()
{
   //Init gl-pad painter:
   //1. 2D painter does not use depth test, should not modify
   //   depth-buffer content (except initial cleanup).
   //2. Disable cull face.
   //3. Disable lighting.
   //4. Set viewport (to the whole canvas area).
   //5. Set camera.
   //6. Unlock painter.
   glDisable(GL_DEPTH_TEST);
   glDisable(GL_CULL_FACE);
   glDisable(GL_LIGHTING);

   //Clear the buffer
   glViewport(0, 0, GLsizei(gPad->GetWw()), GLsizei(gPad->GetWh()));

   glDepthMask(GL_TRUE);
   glClearColor(1.,1.,1.,1.);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glDepthMask(GL_FALSE);

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();

   glOrtho(gPad->GetX1(), gPad->GetX2(), gPad->GetY1(), gPad->GetY2(), -10., 10.);

   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glTranslated(0., 0., -1.);

   fLocked = kFALSE;
}

//______________________________________________________________________________
void TGLPadPainter3D::InitPainterForFBO(Double_t vpsx, Double_t vpsy,
                                        Bool_t set_viewport,
                                        Bool_t dl_capture_active)
{
   //Init gl-pad painter:
   //1. 2D painter does not use depth test, should not modify
   //   depth-buffer content (except initial cleanup).
   //2. Disable cull face.
   //3. Disable lighting.
   //4. Set viewport to the whole canvas area -- if set_viewport is true.
   //5. Set camera.
   //6. Unlock painter.

   glDisable(GL_DEPTH_TEST);
   glDisable(GL_CULL_FACE);
   glDisable(GL_LIGHTING);

   if (set_viewport)
      glViewport(0, 0, GLsizei(gPad->GetWw()), GLsizei(gPad->GetWh()));

   //Clear the buffer
   glDepthMask(GL_TRUE);
   glClearColor(1.,1.,1.,1.);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glDepthMask(GL_FALSE);

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();

   glOrtho(gPad->GetX1(), gPad->GetX2(), gPad->GetY1(), gPad->GetY2(), -10., 10.);

   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glTranslated(0., 0., -1.);

   fLocked = kFALSE;

   fForGLViewer = kFALSE;
   fVPScaleX = vpsx;
   fVPScaleY = vpsy;
   SaveViewport();

   fBeingEmbeddedInDisplayList = dl_capture_active;
}

//______________________________________________________________________________
void TGLPadPainter3D::InitPainterForGLViewer(Bool_t dl_capture_active)
{
   //Init gl-pad painter:
   //2. Disable cull face.
   //3. Disable lighting.
   //6. Unlock painter.

   glDisable(GL_CULL_FACE);
   glDisable(GL_LIGHTING);

   fLocked = kFALSE;

   fForGLViewer = kTRUE;

   fBeingEmbeddedInDisplayList = dl_capture_active;
}

//______________________________________________________________________________
void TGLPadPainter3D::InvalidateCS()
{
   //When TPad::Range for gPad is called, projection
   //must be changed in OpenGL.
   if (fLocked)
      return;

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();

   glOrtho(gPad->GetX1(), gPad->GetX2(), gPad->GetY1(), gPad->GetY2(), -10., 10.);

   glMatrixMode(GL_MODELVIEW);
}

//______________________________________________________________________________
void TGLPadPainter3D::LockPainter()
{
   //Locked state of painter means, that
   //GL context can be invalid, so no GL calls
   //can be executed.
   if (fLocked)
      return;

   glFinish();
   fLocked = kTRUE;
}

/*
2D primitives.
*/

const Double_t lineWidthTS = 3.;

//______________________________________________________________________________
void TGLPadPainter3D::DrawLine(Double_t x1, Double_t y1, Double_t x2, Double_t y2)
{
   //Draw line segment.
   if (fLocked) {
      //GL pad painter can be called in non-standard situation:
      //not from TPad::Paint, but
      //from TView3D::ExecuteRotateView. This means in fact,
      //that TView3D wants to draw itself in a XOR mode, via
      //gVirtualX.
      if (gVirtualX->GetDrawMode() == TVirtualX::kInvert) {
         gVirtualX->DrawLine(gPad->XtoAbsPixel(x1), gPad->YtoAbsPixel(y1),
                             gPad->XtoAbsPixel(x2), gPad->YtoAbsPixel(y2));
      }

      return;
   }

   const Rgl::Pad::LineAttribSet lineAttribs(kTRUE, gVirtualX->GetLineStyle(), fLimits.GetMaxLineWidth(), kTRUE);

   glBegin(GL_LINES);
   glVertex2d(x1, y1);
   glVertex2d(x2, y2);
   glEnd();

   if (gVirtualX->GetLineWidth() > lineWidthTS) {
      Double_t pointSize = gVirtualX->GetLineWidth();
      if (pointSize > fLimits.GetMaxPointSize())
         pointSize = fLimits.GetMaxPointSize();
      glPointSize((GLfloat)pointSize);
      const TGLEnableGuard pointSmooth(GL_POINT_SMOOTH);
      glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
      glBegin(GL_POINTS);

      glVertex2d(x1, y1);
      glVertex2d(x2, y2);

      glEnd();
      glPointSize(1.f);
   }

}

//______________________________________________________________________________
void TGLPadPainter3D::DrawLineNDC(Double_t u1, Double_t v1, Double_t u2, Double_t v2)
{
   //Draw line segment in NDC coordinates.
   if (fLocked)
      return;

   const Rgl::Pad::LineAttribSet lineAttribs(kTRUE, gVirtualX->GetLineStyle(), fLimits.GetMaxLineWidth(), kTRUE);
   const Double_t xRange = gPad->GetX2() - gPad->GetX1();
   const Double_t yRange = gPad->GetY2() - gPad->GetY1();

   glBegin(GL_LINES);
   glVertex2d(gPad->GetX1() + u1 * xRange, gPad->GetY1() + v1 * yRange);
   glVertex2d(gPad->GetX1() + u2 * xRange, gPad->GetY1() + v2 * yRange);
   glEnd();
}

//______________________________________________________________________________
void TGLPadPainter3D::DrawBox(Double_t x1, Double_t y1, Double_t x2, Double_t y2, EBoxMode mode)
{
   //Draw filled or hollow box.
   if (fLocked)
      return;

   if (mode == kHollow) {
      const Rgl::Pad::LineAttribSet lineAttribs(kTRUE, 0, fLimits.GetMaxLineWidth(), kTRUE);
      //
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      glRectd(x1, y1, x2, y2);
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      glLineWidth(1.f);
   } else {
      const Rgl::Pad::FillAttribSet fillAttribs(fSSet, kFALSE);//Set filling parameters.
      glRectd(x1, y1, x2, y2);
   }
}

//______________________________________________________________________________
void TGLPadPainter3D::DrawFillArea(Int_t n, const Double_t *x, const Double_t *y)
{
   //Draw tesselated polygon (probably, outline only).
   if (fLocked)
      return;

   if (!gVirtualX->GetFillStyle()) {
      fIsHollowArea = kTRUE;
      return DrawPolyLine(n, x, y);
   }

   fVs.resize(n * 3);

   for (Int_t i = 0; i < n; ++i) {
      fVs[i * 3]     = x[i];
      fVs[i * 3 + 1] = y[i];
      fVs[i * 3 + 2] = 0.;
   }

   const Rgl::Pad::FillAttribSet fillAttribs(fSSet, kFALSE);

   GLUtesselator *t = (GLUtesselator *)fTess.GetTess();
   gluBeginPolygon(t);
   gluNextContour(t, (GLenum)GLU_UNKNOWN);

   for (Int_t i = 0; i < n; ++i)
      gluTessVertex(t, &fVs[i * 3], &fVs[i * 3]);


   gluEndPolygon(t);
}

//______________________________________________________________________________
void TGLPadPainter3D::DrawFillArea(Int_t n, const Float_t *x, const Float_t *y)
{
   //Draw tesselated polygon (never called, probably, since TPad::PaintFillArea for floats
   //is deprecated).
   if (fLocked)
      return;

   if (!gVirtualX->GetFillStyle()) {
      fIsHollowArea = kTRUE;
      return DrawPolyLine(n, x, y);
   }

   fVs.resize(n * 3);

   for (Int_t i = 0; i < n; ++i) {
      fVs[i * 3]     = x[i];
      fVs[i * 3 + 1] = y[i];
   }

   const Rgl::Pad::FillAttribSet fillAttribs(fSSet, kFALSE);

   GLUtesselator *t = (GLUtesselator *)fTess.GetTess();
   gluBeginPolygon(t);
   gluNextContour(t, (GLenum)GLU_UNKNOWN);

   for (Int_t i = 0; i < n; ++i)
      gluTessVertex(t, &fVs[i * 3], &fVs[i * 3]);


   gluEndPolygon(t);
}

//______________________________________________________________________________
void TGLPadPainter3D::DrawPolyLine(Int_t n, const Double_t *x, const Double_t *y)
{
   //Draw poly-line in user coordinates.
   if (fLocked)
      return;

   const Rgl::Pad::LineAttribSet lineAttribs(kTRUE, gVirtualX->GetLineStyle(), fLimits.GetMaxLineWidth(), kTRUE);

   glBegin(GL_LINE_STRIP);

   for (Int_t i = 0; i < n; ++i)
      glVertex2d(x[i], y[i]);

   if (fIsHollowArea) {
      glVertex2d(x[0], y[0]);
      fIsHollowArea = kFALSE;
   }
   glEnd();

   if (gVirtualX->GetLineWidth() > lineWidthTS) {
      Double_t pointSize = gVirtualX->GetLineWidth();
      if (pointSize > fLimits.GetMaxPointSize())
         pointSize = fLimits.GetMaxPointSize();
      glPointSize((GLfloat)pointSize);
      const TGLEnableGuard pointSmooth(GL_POINT_SMOOTH);
      glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
      glBegin(GL_POINTS);

      for (Int_t i = 0; i < n; ++i)
         glVertex2d(x[i], y[i]);

      glEnd();
      glPointSize(1.f);
   }
}

//______________________________________________________________________________
void TGLPadPainter3D::DrawPolyLine(Int_t n, const Float_t *x, const Float_t *y)
{
   //Never called?
   if (fLocked)
      return;

   const Rgl::Pad::LineAttribSet lineAttribs(kTRUE, gVirtualX->GetLineStyle(), fLimits.GetMaxLineWidth(), kTRUE);

   glBegin(GL_LINE_STRIP);

   for (Int_t i = 0; i < n; ++i)
      glVertex2f(x[i], y[i]);

   if (fIsHollowArea) {
      glVertex2f(x[0], y[0]);
      fIsHollowArea = kFALSE;
   }

   glEnd();
}

//______________________________________________________________________________
void TGLPadPainter3D::DrawPolyLineNDC(Int_t n, const Double_t *u, const Double_t *v)
{
   //Poly line in NDC.
   if (fLocked)
      return;

   const Rgl::Pad::LineAttribSet lineAttribs(kTRUE, gVirtualX->GetLineStyle(), fLimits.GetMaxLineWidth(), kTRUE);
   const Double_t xRange = gPad->GetX2() - gPad->GetX1();
   const Double_t yRange = gPad->GetY2() - gPad->GetY1();
   const Double_t x1 = gPad->GetX1(), y1 = gPad->GetY1();

   glBegin(GL_LINE_STRIP);

   for (Int_t i = 0; i < n; ++i)
      glVertex2d(x1 + u[i] * xRange, y1 + v[i] * yRange);

   glEnd();
}

namespace {

//Aux. function.
template<class ValueType>
void ConvertMarkerPoints(Int_t n, const ValueType *x, const ValueType *y, std::vector<TPoint> & dst);

}

//______________________________________________________________________________
void TGLPadPainter3D::DrawPolyMarker(Int_t n, const Double_t *x, const Double_t *y)
{
   //Poly-marker.
   if (fLocked)
      return;

   ConvertMarkerPoints(n, x, y, fPoly);
   DrawPolyMarker();
}

//______________________________________________________________________________
void TGLPadPainter3D::DrawPolyMarker(Int_t n, const Float_t *x, const Float_t *y)
{
   //Poly-marker.
   if (fLocked)
      return;

   ConvertMarkerPoints(n, x, y, fPoly);
   DrawPolyMarker();
}

//______________________________________________________________________________
void TGLPadPainter3D::DrawPolyMarker()
{
   //Poly-marker.
   if (fLocked)
      return;

   SaveProjectionMatrix();
   glLoadIdentity();
   //
   glOrtho(0, gPad->GetAbsWNDC() * gPad->GetWw(), 0, gPad->GetAbsHNDC() * gPad->GetWh(), -10., 10.);
   //
   glMatrixMode(GL_MODELVIEW);
   //
   Float_t rgba[3] = {};
   Rgl::Pad::ExtractRGB(gVirtualX->GetMarkerColor(), rgba);
   glColor3fv(rgba);

   const TPoint *xy = &fPoly[0];
   const Style_t markerStyle = gVirtualX->GetMarkerStyle();
   const UInt_t n = UInt_t(fPoly.size());
   switch (markerStyle) {
   case kDot:
      fMarker.DrawDot(n, xy);
      break;
   case kPlus:
      fMarker.DrawPlus(n, xy);
      break;
   case kStar:
      fMarker.DrawStar(n, xy);
      break;
   case kCircle:
   case kOpenCircle:
      fMarker.DrawCircle(n, xy);
      break;
   case kMultiply:
      fMarker.DrawX(n, xy);
      break;
   case kFullDotSmall://"Full dot small"
      fMarker.DrawFullDotSmall(n, xy);
      break;
   case kFullDotMedium:
      fMarker.DrawFullDotMedium(n, xy);
      break;
   case kFullDotLarge:
   case kFullCircle:
      fMarker.DrawFullDotLarge(n, xy);
      break;
   case kFullSquare:
      fMarker.DrawFullSquare(n, xy);
      break;
   case kFullTriangleUp:
      fMarker.DrawFullTrianlgeUp(n, xy);
      break;
   case kFullTriangleDown:
      fMarker.DrawFullTrianlgeDown(n, xy);
      break;
   case kOpenSquare:
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      fMarker.DrawFullSquare(n, xy);
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      break;
   case kOpenTriangleUp:
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      fMarker.DrawFullTrianlgeUp(n, xy);
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      break;
   case kOpenDiamond:
      fMarker.DrawDiamond(n, xy);
      break;
   case kOpenCross:
      fMarker.DrawCross(n, xy);
      break;
   case kFullStar:
      fMarker.DrawFullStar(n, xy);
      break;
   case kOpenStar:
      fMarker.DrawOpenStar(n, xy);
   }

   RestoreProjectionMatrix();
   glMatrixMode(GL_MODELVIEW);
}

//______________________________________________________________________________
void TGLPadPainter3D::DrawText(Double_t x, Double_t y, const char *text, ETextMode /*mode*/)
{
   //Draw text. This operation is especially
   //dangerous if in locked state -
   //ftgl will assert on zero texture size
   //(which is result of bad GL context).
   if (fLocked)
      return;

   SaveProjectionMatrix();
   glLoadIdentity();
   //
   glOrtho(0, gPad->GetAbsWNDC() * gPad->GetWw(), 0, gPad->GetAbsHNDC() * gPad->GetWh(), -10., 10.);
   //
   glMatrixMode(GL_MODELVIEW);

   Float_t rgba[3] = {};
   Rgl::Pad::ExtractRGB(GetTextColor(), rgba);
   glColor3fv(rgba);

   // XXXX The font size can get TOO LARGE, esp. for large FBOs.
   // Also get GL-errors for polygons fonts --- why??
   Int_t size = TMath::Max(2, Int_t(GetTextSize() * fVPScaleY) - 1);

   // printf("Rendering at %d, '%s' \n", size, text);

   if (size >= 10 && size <= 64)
   {
      fFM.RegisterFont(size, TGLFontManager::GetFontNameFromId(GetTextFont()),
                       TGLFont::kTexture, fF);
   }
   else
   {
      fFM.RegisterFont(size, TGLFontManager::GetFontNameFromId(GetTextFont()),
                       TGLFont::kPolygon, fF);
      if (fBeingEmbeddedInDisplayList)
         fF.SetUseDisplayLists(kFALSE);
   }
   fF.PreRender();

   const UInt_t padH = UInt_t(gPad->GetAbsHNDC() * gPad->GetWh());
   fF.Render(text, TMath::Nint(fVPScaleX * gPad->XtoPixel(x)), padH - TMath::Nint(fVPScaleY * gPad->YtoPixel(y)), GetTextAngle(), GetTextMagnitude());

   fF.PostRender();
   RestoreProjectionMatrix();
   glMatrixMode(GL_MODELVIEW);

   TGLUtil::CheckError("TGLPadPainter3D::DrawText post rendering");
}

//______________________________________________________________________________
void TGLPadPainter3D::DrawTextNDC(Double_t u, Double_t v, const char *text, ETextMode mode)
{
   //Draw text in NDC. This operation is especially
   //dangerous if in locked state -
   //ftgl will assert on zero texture size
   //(which is result of bad GL context).
   if (fLocked)
      return;

   const Double_t xRange = gPad->GetX2() - gPad->GetX1();
   const Double_t yRange = gPad->GetY2() - gPad->GetY1();
   DrawText(gPad->GetX1() + u * xRange, gPad->GetY1() + v * yRange, text, mode);
}

//______________________________________________________________________________
void TGLPadPainter3D::SaveProjectionMatrix()const
{
   //Save the projection matrix.
   //Attention! GL_PROJECTION will become the current matrix
   //after this call!
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
}

//______________________________________________________________________________
void TGLPadPainter3D::RestoreProjectionMatrix()const
{
   //Restore the projection matrix.
   //Attention! GL_PROJECTION will become the current matrix
   //after this call!
   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
}

//______________________________________________________________________________
void TGLPadPainter3D::SaveModelviewMatrix()const
{
   //Save the modelview matrix.
   //Attention! GL_MODELVIEW will become the current matrix
   //after this call!
   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
}

//______________________________________________________________________________
void TGLPadPainter3D::RestoreModelviewMatrix()const
{
   //Restore the modelview matrix.
   //Attention! GL_MODELVIEW will become the current matrix
   //after this call!
   glMatrixMode(GL_MODELVIEW);
   glPopMatrix();
}

//______________________________________________________________________________
void TGLPadPainter3D::SaveViewport()
{
   //Extract and save the current viewport.
   glGetIntegerv(GL_VIEWPORT, fVp);
}

//______________________________________________________________________________
void TGLPadPainter3D::RestoreViewport()
{
   //Restore the saved viewport.
   glViewport(fVp[0], fVp[1], fVp[2], fVp[3]);
}

//______________________________________________________________________________
void TGLPadPainter3D::SaveImage(TVirtualPad *pad, const char *fileName, Int_t type) const
{
   // Using TImage save frame-buffer contents as a picture.

   TVirtualPad *canvas = (TVirtualPad *)pad->GetCanvas();
   if (!canvas) return;
   gROOT->ProcessLine(Form("((TCanvas *)0x%lx)->Flush();", (ULong_t)canvas));

   std::vector<unsigned> buff(canvas->GetWw() * canvas->GetWh());
   glPixelStorei(GL_PACK_ALIGNMENT, 1);
   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
   //In case GL_BGRA is not in gl.h (old windows' gl) - comment/uncomment lines.
   //glReadPixels(0, 0, canvas->GetWw(), canvas->GetWh(), GL_BGRA, GL_UNSIGNED_BYTE, (char *)&buff[0]);
   glReadPixels(0, 0, canvas->GetWw(), canvas->GetWh(), GL_RGBA, GL_UNSIGNED_BYTE, (char *)&buff[0]);

   std::auto_ptr<TImage> image(TImage::Create());
   if (!image.get()) {
      Error("TGLPadPainter3D::SaveImage", "TImage creation failed");
      return;
   }

   image->DrawRectangle(0, 0, canvas->GetWw(), canvas->GetWh());
   UInt_t *argb = image->GetArgbArray();

   if (!argb) {
      Error("TGLPadPainter3D::SaveImage", "null argb array in TImage object");
      return;
   }

   const Int_t nLines  = canvas->GetWh();
   const Int_t nPixels = canvas->GetWw();

   for (Int_t i = 0; i < nLines; ++i) {
     Int_t base = (nLines - 1 - i) * nPixels;
     for (Int_t j = 0; j < nPixels; ++j, ++base) {
        //Uncomment/comment if you don't have GL_BGRA.

        const UInt_t pix  = buff[base];
        const UInt_t bgra = ((pix & 0xff) << 16) | (pix & 0xff00) |
                            ((pix & 0xff0000) >> 16) | (pix & 0xff000000);

        //argb[i * nPixels + j] = buff[base];
        argb[i * nPixels + j] = bgra;
     }
   }

   image->WriteImage(fileName, (TImage::EImageFileTypes)type);
}


//Aux. functions.
namespace {

template<class ValueType>
void ConvertMarkerPoints(Int_t n, const ValueType *x, const ValueType *y, std::vector<TPoint> & dst)
{
   const UInt_t padH = UInt_t(gPad->GetAbsHNDC() * gPad->GetWh());

   dst.resize(n);
   for (Int_t i = 0; i < n; ++i) {
      dst[i].fX = gPad->XtoPixel(x[i]);
      dst[i].fY = padH - gPad->YtoPixel(y[i]);
   }
}

}
