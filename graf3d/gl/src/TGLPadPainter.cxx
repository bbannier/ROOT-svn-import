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
#include "KeySymbols.h"
#include "TCanvas.h"
#include "TTimer.h"
#include "TError.h"
#include "TImage.h"
#include "TList.h"
#include "TROOT.h"
#include "TPad.h"

#include "TGLPadPainter.h"
#include "TGLIncludes.h"
#include "TGLUtil.h"
#include "TError.h"

//______________________________________________________________________________
   //"Delegating" part of TGLPadPainter. Line/fill/etc. attributes can be
   //set inside TPad, but not only there:
   //many of them are set by base sub-objects of 2d primitives
   //(2d primitives usually inherit TAttLine or TAttFill etc.).  And these sub-objects
   //call gVirtualX->SetLineWidth ... etc. So, if I save some attributes in my painter,
   //it will be mess - at any moment I do not know, where to take line attribute - from
   //gVirtualX or from my own member. So! All attributed, _ALL_ go to/from gVirtualX.

ClassImp(TGLPadPainter)

//______________________________________________________________________________
TGLPadPainter::TGLPadPainter()
                  : fIsHollowArea(kFALSE),
                    fLocked(kTRUE),
                    fIsCoverFlow(kFALSE),
                    fCoversGenerated(kFALSE),
                    fAnimation(kNoAnimation),
                    fFrame(0),
                    fFrontCover(0),
                    fCanvas(0)
{
   fVp[0] = fVp[1] = fVp[2] = fVp[3] = 0;

   fTimer.SetObject(this);
}


//______________________________________________________________________________
Color_t TGLPadPainter::GetLineColor() const
{
   //Delegate to gVirtualX.
   return gVirtualX->GetLineColor();
}

//______________________________________________________________________________
Style_t TGLPadPainter::GetLineStyle() const
{
   //Delegate to gVirtualX.
   return gVirtualX->GetLineStyle();
}

//______________________________________________________________________________
Width_t TGLPadPainter::GetLineWidth() const
{
   //Delegate to gVirtualX.
   return gVirtualX->GetLineWidth();
}

//______________________________________________________________________________
void TGLPadPainter::SetLineColor(Color_t lcolor)
{
   //Delegate to gVirtualX.
   gVirtualX->SetLineColor(lcolor);
}

//______________________________________________________________________________
void TGLPadPainter::SetLineStyle(Style_t lstyle)
{
   //Delegate to gVirtualX.
   gVirtualX->SetLineStyle(lstyle);
}

//______________________________________________________________________________
void TGLPadPainter::SetLineWidth(Width_t lwidth)
{
   //Delegate to gVirtualX.
   gVirtualX->SetLineWidth(lwidth);
}

//______________________________________________________________________________
Color_t TGLPadPainter::GetFillColor() const
{
   //Delegate to gVirtualX.
   return gVirtualX->GetFillColor();
}

//______________________________________________________________________________
Style_t TGLPadPainter::GetFillStyle() const
{
   //Delegate to gVirtualX.
   return gVirtualX->GetFillStyle();
}

//______________________________________________________________________________
Bool_t TGLPadPainter::IsTransparent() const
{
   //Delegate to gVirtualX.
   //IsTransparent is implemented as inline function in TAttFill.
   return gVirtualX->IsTransparent();
}

//______________________________________________________________________________
void TGLPadPainter::SetFillColor(Color_t fcolor)
{
   //Delegate to gVirtualX.
   gVirtualX->SetFillColor(fcolor);
}

//______________________________________________________________________________
void TGLPadPainter::SetFillStyle(Style_t fstyle)
{
   //Delegate to gVirtualX.
   gVirtualX->SetFillStyle(fstyle);
}

//______________________________________________________________________________
void TGLPadPainter::SetOpacity(Int_t percent)
{
   //Delegate to gVirtualX.
   gVirtualX->SetOpacity(percent);
}

//______________________________________________________________________________
Short_t TGLPadPainter::GetTextAlign() const
{
   //Delegate to gVirtualX.
   return gVirtualX->GetTextAlign();
}

//______________________________________________________________________________
Float_t TGLPadPainter::GetTextAngle() const
{
   //Delegate to gVirtualX.
   return gVirtualX->GetTextAngle();
}

//______________________________________________________________________________
Color_t TGLPadPainter::GetTextColor() const
{
   //Delegate to gVirtualX.
   return gVirtualX->GetTextColor();
}

//______________________________________________________________________________
Font_t TGLPadPainter::GetTextFont() const
{
   //Delegate to gVirtualX.
   return gVirtualX->GetTextFont();
}

//______________________________________________________________________________
Float_t TGLPadPainter::GetTextSize() const
{
   //Delegate to gVirtualX.
   return gVirtualX->GetTextSize();
}

//______________________________________________________________________________
Float_t TGLPadPainter::GetTextMagnitude() const
{
   //Delegate to gVirtualX.
   return gVirtualX->GetTextMagnitude();
}

//______________________________________________________________________________
void TGLPadPainter::SetTextAlign(Short_t align)
{
   //Delegate to gVirtualX.
   gVirtualX->SetTextAlign(align);
}

//______________________________________________________________________________
void TGLPadPainter::SetTextAngle(Float_t tangle)
{
   //Delegate to gVirtualX.
   gVirtualX->SetTextAngle(tangle);
}

//______________________________________________________________________________
void TGLPadPainter::SetTextColor(Color_t tcolor)
{
   //Delegate to gVirtualX.
   gVirtualX->SetTextColor(tcolor);
}

//______________________________________________________________________________
void TGLPadPainter::SetTextFont(Font_t tfont)
{
   //Delegate to gVirtualX.
   gVirtualX->SetTextFont(tfont);
}

//______________________________________________________________________________
void TGLPadPainter::SetTextSize(Float_t tsize)
{
   //Delegate to gVirtualX.
   gVirtualX->SetTextSize(tsize);
}

//______________________________________________________________________________
void TGLPadPainter::SetTextSizePixels(Int_t npixels)
{
   //Delegate to gVirtualX.
   gVirtualX->SetTextSizePixels(npixels);
}

/*
"Pixmap" part of TGLPadPainter.
*/

//______________________________________________________________________________
Int_t TGLPadPainter::CreateDrawable(UInt_t/*w*/, UInt_t/*h*/)
{
   //Not required at the moment.
   return 0;
}

//______________________________________________________________________________
void TGLPadPainter::ClearDrawable()
{
   //Not required at the moment.
}

//______________________________________________________________________________
void TGLPadPainter::CopyDrawable(Int_t /*id*/, Int_t /*px*/, Int_t /*py*/)
{
   //Not required at the moment.
}

//______________________________________________________________________________
void TGLPadPainter::DestroyDrawable()
{
   //Not required at the moment.
}

//______________________________________________________________________________
void TGLPadPainter::SelectDrawable(Int_t /*device*/)
{
   //For gVirtualX this means select pixmap (or window)
   //and all subsequent drawings will go into
   //this pixmap. For OpenGL this means the change of
   //coordinate system and viewport.
   if (fLocked)
      return;

   if (TPad *pad = dynamic_cast<TPad *>(gPad)) {
      Int_t px = 0, py = 0;

      pad->XYtoAbsPixel(pad->GetX1(), pad->GetY1(), px, py);

      py = gPad->GetWh() - py;
      //
      glViewport(px, py, GLsizei(gPad->GetWw() * pad->GetAbsWNDC()), GLsizei(gPad->GetWh() * pad->GetAbsHNDC()));

      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      glOrtho(pad->GetX1(), pad->GetX2(), pad->GetY1(), pad->GetY2(), -10., 10.);

      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();
      glTranslated(0., 0., -1.);
   } else {
      Error("TGLPadPainter::SelectDrawable", "function was called not from TPad or TCanvas code\n");
      throw std::runtime_error("");
   }
}

//______________________________________________________________________________
void TGLPadPainter::InitPainter()
{
   //Init gl-pad painter:
   //1. 2D painter does not use depth test, should not modify
   //   depth-buffer content (except initial cleanup).
   //2. Disable cull face.
   //3. Disable lighting.
   //4. Set viewport to the whole canvas area.
   //5. Set camera.
   //6. Unlock painter.
   glDisable(GL_DEPTH_TEST);
   glDisable(GL_CULL_FACE);
   glDisable(GL_LIGHTING);

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
}

//______________________________________________________________________________
void TGLPadPainter::InvalidateCS()
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
void TGLPadPainter::LockPainter()
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
void TGLPadPainter::DrawLine(Double_t x1, Double_t y1, Double_t x2, Double_t y2)
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
void TGLPadPainter::DrawLineNDC(Double_t u1, Double_t v1, Double_t u2, Double_t v2)
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
void TGLPadPainter::DrawBox(Double_t x1, Double_t y1, Double_t x2, Double_t y2, EBoxMode mode)
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
void TGLPadPainter::DrawFillArea(Int_t n, const Double_t *x, const Double_t *y)
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
void TGLPadPainter::DrawFillArea(Int_t n, const Float_t *x, const Float_t *y)
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
void TGLPadPainter::DrawPolyLine(Int_t n, const Double_t *x, const Double_t *y)
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
void TGLPadPainter::DrawPolyLine(Int_t n, const Float_t *x, const Float_t *y)
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
void TGLPadPainter::DrawPolyLineNDC(Int_t n, const Double_t *u, const Double_t *v)
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

//______________________________________________________________________________
void TGLPadPainter::DrawPolyMarker(Int_t n, const Double_t *x, const Double_t *y)
{
   //Poly-marker.
   if (fLocked)
      return;

   ConvertMarkerPoints(n, x, y, fPoly);
   DrawPolyMarker();
}

//______________________________________________________________________________
void TGLPadPainter::DrawPolyMarker(Int_t n, const Float_t *x, const Float_t *y)
{
   //Poly-marker.
   if (fLocked)
      return;

   ConvertMarkerPoints(n, x, y, fPoly);
   DrawPolyMarker();
}

//______________________________________________________________________________
void TGLPadPainter::DrawPolyMarker()
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
void TGLPadPainter::DrawText(Double_t x, Double_t y, const char *text, ETextMode /*mode*/)
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
   Rgl::Pad::ExtractRGB(gVirtualX->GetTextColor(), rgba);
   glColor3fv(rgba);

   fFM.RegisterFont(Int_t(gVirtualX->GetTextSize()) - 1,
                    TGLFontManager::GetFontNameFromId(gVirtualX->GetTextFont()),
                    TGLFont::kTexture, fF);
   fF.PreRender();

   const UInt_t padH = UInt_t(gPad->GetAbsHNDC() * gPad->GetWh());
   fF.Render(text, gPad->XtoPixel(x), padH - gPad->YtoPixel(y), GetTextAngle(), GetTextMagnitude());

   fF.PostRender();
   RestoreProjectionMatrix();
   glMatrixMode(GL_MODELVIEW);
}

//______________________________________________________________________________
void TGLPadPainter::DrawTextNDC(Double_t u, Double_t v, const char *text, ETextMode mode)
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
void TGLPadPainter::SaveProjectionMatrix()const
{
   //Save the projection matrix.
   //Attention! GL_PROJECTION will become the current matrix
   //after this call!
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
}

//______________________________________________________________________________
void TGLPadPainter::RestoreProjectionMatrix()const
{
   //Restore the projection matrix.
   //Attention! GL_PROJECTION will become the current matrix
   //after this call!
   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
}

//______________________________________________________________________________
void TGLPadPainter::SaveModelviewMatrix()const
{
   //Save the modelview matrix.
   //Attention! GL_MODELVIEW will become the current matrix
   //after this call!
   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
}

//______________________________________________________________________________
void TGLPadPainter::RestoreModelviewMatrix()const
{
   //Restore the modelview matrix.
   //Attention! GL_MODELVIEW will become the current matrix
   //after this call!
   glMatrixMode(GL_MODELVIEW);
   glPopMatrix();
}

//______________________________________________________________________________
void TGLPadPainter::SaveViewport()
{
   //Extract and save the current viewport.
   glGetIntegerv(GL_VIEWPORT, fVp);
}

//______________________________________________________________________________
void TGLPadPainter::RestoreViewport()
{
   //Restore the saved viewport.
   glViewport(fVp[0], fVp[1], fVp[2], fVp[3]);
}

//______________________________________________________________________________
void TGLPadPainter::SaveImage(TVirtualPad *pad, const char *fileName, Int_t type) const
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
      Error("TGLPadPainter::SaveImage", "TImage creation failed");
      return;
   }

   image->DrawRectangle(0, 0, canvas->GetWw(), canvas->GetWh());
   UInt_t *argb = image->GetArgbArray();

   if (!argb) {
      Error("TGLPadPainter::SaveImage", "null argb array in TImage object");
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

//////////////////////////////////////////////////////
// Cover-flow core.
//////////////////////////////////////////////////////

//______________________________________________________________________________
TGLPadCover::TGLPadCover()
               : fFBO(new TGLFBO),
                 fW(0),
                 fH(0)
{
}

//______________________________________________________________________________
TGLPadCover::TGLPadCover(const TGLPadCover &rhs)
               : fFBO(new TGLFBO),
                 fW(0),
                 fH(0),
                 fXLowNDC(0.),
                 fYLowNDC(0.),
                 fXUpNDC(0.),
                 fYUpNDC(0.)
{
}

//______________________________________________________________________________
TGLPadCover &TGLPadCover::operator = (const TGLPadCover &rhs)
{
   return *this;
}

//______________________________________________________________________________
TGLPadCover::~TGLPadCover()
{
   //Do cleanup here.
}

//______________________________________________________________________________
Bool_t TGLPadCover::Init(UInt_t w, UInt_t h, TPad *pad)
{
   try {
      fFBO->Init(w, h);
   } catch (const std::exception &) {
      return kFALSE;
   }

   fPad = pad;
   //Save old pad's dimensions.
   fXLowNDC = fPad->GetXlowNDC();
   fYLowNDC = fPad->GetYlowNDC();
   fXUpNDC  = fXLowNDC + fPad->GetWNDC();
   fYUpNDC  = fYLowNDC + fPad->GetHNDC();
   //Set new dimensions for pad.
   fPad->SetPad(0., 0., 1., 1.);

   return kTRUE;
}

//______________________________________________________________________________
void TGLPadCover::RestorePad()
{
   fPad->SetPad(fXLowNDC, fYLowNDC, fXUpNDC, fYUpNDC);
}

//______________________________________________________________________________
void TGLPadCover::DrawToFBO(TGLPadPainter *painter)
{
   if (!painter)
      return;

   fFBO->Bind();

   painter->InitPainter();

   fPad->Paint();
   fFBO->Unbind();
}

//______________________________________________________________________________
void TGLPadCover::DrawCover(Bool_t reflected)
{
   fFBO->BindTexture();

   glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

   glBegin(GL_QUADS);
   if (!reflected)
      glTexCoord2d(0., 0.);
   else
      glTexCoord2d(0., 1.);
   glVertex2d(-0.25, -0.25);

   if (!reflected)
      glTexCoord2d(1., 0.);
   else
      glTexCoord2d(1., 1.);
   glVertex2d(0.25, -0.25);

   if (!reflected)
      glTexCoord2d(1., 1.);
   else
      glTexCoord2d(1., 0.);
   glVertex2d(0.25, 0.25);

   if (!reflected)
      glTexCoord2d(0., 1.);
   else
      glTexCoord2d(0., 0.);
   glVertex2d(-0.25, 0.25);
   glEnd();

   fFBO->UnbindTexture();
}

//______________________________________________________________________________
void TGLPadPainter::DrawCovers(Bool_t reflection)
{
   switch(fAnimation)
   {
   case kNoAnimation:
      return DrawCoversStatic(reflection);
   case kLeftShift:
      return DrawLeftShift(reflection);
   case kRightShift:
      return DrawRightShift(reflection);
   default:;
   }  
}

namespace {

//Different cover-flow constants and parameters.


//Covers on the left and right are rotated.
const Double_t coverAngle = 60.;
//Number of visible covers is 7, one on the left and one on the right are invisible.
const UInt_t nCovers = 9;
//Covers' positions on the left and on the right.
//1.1 and -1.1 are invisible covers.
const Double_t coverPos[] = {-1.1, -0.5, -0.4, -0.3, 0., 0.3, 0.4, 0.5, 1.1};
//Shifts to move covers from coverPos to the right.
const Double_t coverRShifts[] = {0.6, 0.1, 0.1, 0.3, 0.3, 0.1, 0.1, 0.6, 0.};
//Shifts to move covers from coverPos to the left.
const Double_t coverLShifts[] = {0., -0.6, -0.1, -0.1, -0.3, -0.3, -0.1, -0.1, -0.6};

}

//______________________________________________________________________________
void TGLPadPainter::DrawLeftShift(Bool_t reflection)
{
   const Double_t step      = fFrame / double(kFrames);
   const Double_t rotAngle  = coverAngle * step;
   const size_type oldFront = fFrontCover - 1;

   //First, fix covers on the left.
   const size_type left = oldFront > 3 ? oldFront - 3 : 0;
   size_type posIndex = 4 - (oldFront - left);

   for (size_type coverIndex = left; coverIndex < oldFront; ++coverIndex, ++posIndex) {
      glPushMatrix();
      glTranslated(coverPos[posIndex] + step * coverLShifts[posIndex], 0., -1.5);
      glRotated(coverAngle, 0., 1., 0.);
      fCovers[coverIndex].DrawCover(reflection);
      glPopMatrix();
   }

   //Now, fix covers on the right.
   const size_type right = fCovers.size() - oldFront > 4 ? oldFront + 5 : fCovers.size();
   posIndex = 6;

   for (size_type coverIndex = oldFront + 2; coverIndex < right; ++coverIndex, ++posIndex) {
      glPushMatrix();
      glTranslated(coverPos[posIndex] + coverLShifts[posIndex] * step, 0., -1.5);
      glRotated(-coverAngle, 0., 1., 0.);
      fCovers[coverIndex].DrawCover(reflection);
      glPopMatrix();
   }

   //The final stage: rotate and move the previous front cover and replace it with the new one.
   const Double_t angleStep = coverAngle * step;

   //The new cover.
   glPushMatrix();
   glTranslated(coverPos[5] + coverLShifts[5] * step, 0., -1.5 + 0.4 * step);
   glRotated(-coverAngle + angleStep, 0., 1., 0.);
   fCovers[fFrontCover].DrawCover(reflection);
   glPopMatrix();
   //The old cover.
   glPushMatrix();
   glTranslated(coverPos[4] + coverLShifts[4] * step, 0., -1.1 - 0.4 * step);
   glRotated(angleStep, 0., 1., 0.);
   fCovers[oldFront].DrawCover(reflection);
   glPopMatrix();
}

//______________________________________________________________________________
void TGLPadPainter::DrawRightShift(Bool_t reflection)
{
   const double step        = fFrame / Double_t(kFrames);
   const double rotAngle    = coverAngle * step;
   const size_type oldFront = fFrontCover + 1;

   //First, fix covers on the right.
   const size_type right = fCovers.size() - oldFront > 3 ? oldFront + 4 : fCovers.size();
   size_type posIndex = 5;

   for (size_type coverIndex = oldFront + 1; coverIndex < right; ++coverIndex, ++posIndex) {
      glPushMatrix();
      glTranslated(coverPos[posIndex] + coverRShifts[posIndex] * step, 0., -1.5);
      glRotated(-coverAngle, 0., 1., 0.);
      fCovers[coverIndex].DrawCover(reflection);
      glPopMatrix();
   }
 
   //Now, fix covers on the left.
   const size_type left = oldFront > 4 ? oldFront - 4 : 0;
   posIndex = 4 - (oldFront - left);

   for (size_type coverIndex = left; coverIndex < oldFront - 1; ++coverIndex, ++posIndex) {
      glPushMatrix();
      glTranslated(coverPos[posIndex] + step * coverRShifts[posIndex], 0., -1.5);
      glRotated(coverAngle, 0., 1., 0.);
      fCovers[coverIndex].DrawCover(reflection);
      glPopMatrix();
   }

   //The final stage: rotate and move the previous front cover and replace it with the new one.
   const double angleStep = coverAngle * step;
   //The new cover.
   glPushMatrix();
   glTranslated(coverPos[3] + coverRShifts[3] * step, 0., -1.5 + 0.4 * step);
   glRotated(coverAngle - angleStep, 0., 1., 0.);
   fCovers[fFrontCover].DrawCover(reflection);
   glPopMatrix();
   //The old cover.
   glPushMatrix();
   glTranslated(coverPos[4] + coverRShifts[4] * step, 0., -1.1 - 0.4 * step);
   glRotated(-angleStep, 0., 1., 0.);
   fCovers[oldFront].DrawCover(reflection);
   glPopMatrix();
}

//______________________________________________________________________________
void TGLPadPainter::DrawCoversStatic(Bool_t reflection)
{
   //Static cover-flow (no animation).
	const size_type left = fFrontCover > 3 ? fFrontCover - 3 : 0;
   size_type posIndex   = 4 - (fFrontCover - left);

   for (size_type coverIndex = left; coverIndex < fFrontCover; ++coverIndex, ++posIndex) {
      glPushMatrix();
      glTranslated(coverPos[posIndex], 0., -1.5);
      glRotated(coverAngle, 0., 1., 0.);
      fCovers[coverIndex].DrawCover(reflection);
      glPopMatrix();
   }
   //"Covers" on the right.
   const size_type right = fCovers.size() - fFrontCover > 3 ? fFrontCover + 4 : fCovers.size();
   posIndex = 5;

   for (size_type coverIndex = fFrontCover + 1; coverIndex < right; ++coverIndex, ++posIndex) {
      glPushMatrix();
      glTranslated(coverPos[posIndex], 0., -1.5);
      glRotated(-coverAngle, 0., 1., 0.);
      fCovers[coverIndex].DrawCover(reflection);
      glPopMatrix();
   }

   //Now, the front cover.
   glPushMatrix();
   glTranslated(0., 0., -1.1);
   fCovers[fFrontCover].DrawCover(reflection);
   glPopMatrix();
}

//______________________________________________________________________________
void TGLPadPainter::DrawCoverFlow()
{
   if (!fCoversGenerated) {
      //"Save" pad's image into FBO.
      for (size_type i = 0; i < fCovers.size(); ++i)
         fCovers[i].DrawToFBO(this);
      fCoversGenerated = kTRUE;
   }

   //
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glFrustum(-0.5, 0.5, -0.5, 0.5, 1., 2.);

   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   //

   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   
   //[Draw the reflection first.
   glEnable(GL_DEPTH_TEST);
   glDepthMask(GL_TRUE);

   glPushMatrix();
   glTranslated(0., -0.5, 0.);

   DrawCovers(kTRUE);//kTRUE - draw the relfection of cover.

   glPopMatrix();
   //Reflection is done.]

   //[Draw "water" on top of reflection.
   glDisable(GL_DEPTH_TEST);
   glDepthMask(GL_FALSE);

   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
 
   //Switch to ortho projection and full-window draw rectangle.
   //1. Save old projection matrix.
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   //2. Set new projection.
   glLoadIdentity();
   glOrtho(-1., 1., -1., 1., -10., 10.);
   //3. Save old modelview matrix.
   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
   glLoadIdentity();
   //
   glTranslated(0., 0., -1.2);
  
   glBegin(GL_QUADS);
   glColor4d(0., 0., 0.7, 0.9);
   glVertex2d(-1., -1.);
   glVertex2d(1., -1.);
   glColor4d(0., 0., 0., 0.9);
   glVertex2d(1., 1.);
   glVertex2d(-1., 1.);
   glEnd();

   //Restore old modelview matrix.
   glPopMatrix();
   //Restore old projection matrix.
   glMatrixMode(GL_PROJECTION);
   glPopMatrix();

   glDisable(GL_BLEND);
 
   //
   glMatrixMode(GL_MODELVIEW);
   //"Water" is done.]

   //The final pass - draw fCovers.
   glDepthMask(GL_TRUE);
   glEnable(GL_DEPTH_TEST);

   DrawCovers(kFALSE);//kFALSE - not a reflection.
}

//______________________________________________________________________________
void TGLPadPainter::TurnOnCoverFlow(TCanvas *topPad)
{
   if (!topPad) {
      Error("TGLPadPainter::TurnOnCoverFlow", "TCanvas' pointer is null");
      return;
   }

   fCovers.clear();

   const TList *lp = topPad->GetListOfPrimitives();
   if (!lp || !lp->GetEntries()) {
      return;
   } else {
      for (TObjLink *link = lp->FirstLink(); link; link = link->Next()) {
         const TObject *obj = link->GetObject();
         if (!obj)
            continue;
         if (obj->InheritsFrom("TPad"))
            fCovers.push_back(TGLPadCover());
      }

      if (!fCovers.size()) {
         return;
      } else {
         UInt_t i = 0;
         for (TObjLink *link = lp->FirstLink(); link; link = link->Next()) {
            const TObject *obj = link->GetObject();
            if (!obj)
               continue;
            if (obj->InheritsFrom("TPad")) {
               if (!fCovers.at(i).Init(topPad->GetWw(), topPad->GetWh(), (TPad *)obj)) {
                  Error("TGLPadPainter::TurnOnCoverFlow", "FBO initialization failed");
                  fCovers.clear();
                  return;
               }
               ++i;
            }
         }
      }
   }

   fCanvas = topPad;
   fFrontCover = 0;

   fIsCoverFlow = kTRUE;
}

//______________________________________________________________________________
void TGLPadPainter::TurnOffCoverFlow()
{
   typedef std::vector<TGLPadCover>::size_type size_type;
   for (size_type i = 0; i < fCovers.size(); ++i) {
      fCovers[i].RestorePad();
   }

   fCovers.clear();
   fIsCoverFlow     = kFALSE;
   fCoversGenerated = kFALSE;

   fCanvas = 0;
}

//______________________________________________________________________________
void TGLPadPainter::Animate(Int_t key)
{
   //
   if (key == kKey_Left) {
      if (fFrontCover + 1 == fCovers.size())
         return;

      ++fFrontCover;
      fAnimation = kLeftShift;
      fFrame = 0;
      fTimer.Start(kTimeStep);
   } else if (key == kKey_Right) {
      if (!fFrontCover)
         return;

      --fFrontCover;
      fAnimation = kRightShift;
      fFrame = 0;
      fTimer.Start(kTimeStep);
   }
}

//______________________________________________________________________________
Bool_t TGLPadPainter::HandleTimer(TTimer *timer)
{
   if (fFrame + 1 < kFrames)
   {
      ++fFrame;
      if (fCanvas)
         fCanvas->Update();
   } else {
      fTimer.Stop();
      fAnimation = kNoAnimation;
      if (fCanvas)
         fCanvas->Update();
   }
   return kTRUE;
}
