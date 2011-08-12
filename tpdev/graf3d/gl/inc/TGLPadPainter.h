// @(#)root/gl:$Id$
// Author:  Timur Pocheptsov  06/05/2009

/*************************************************************************
 * Copyright (C) 1995-2009, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

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

/*
The _main_ purpose of TGLPadPainter is to enable 2d gl raphics
inside standard TPad/TCanvas.
*/
class TGLPadPainter : public TVirtualPadPainter {
private:
   Rgl::Pad::PolygonStippleSet fSSet;
   Rgl::Pad::Tesselator        fTess;
   Rgl::Pad::MarkerPainter     fMarker;
   Rgl::Pad::GLLimits          fLimits;
   
   std::vector<Double_t>       fVs;//Vertex buffer for tesselator.
   
   TGLFontManager              fFM;
   TGLFont                     fF;
   
   Int_t                       fVp[4];

   std::vector<TPoint>         fPoly;
   Bool_t                      fIsHollowArea;
   
   Bool_t                      fLocked;
public:
   TGLPadPainter();
   
   //Final overriders for TVirtualPadPainter pure virtual functions.
   //"Off-screen management" part.
   Int_t    CreateDrawable(UInt_t w, UInt_t h);
   void     ClearDrawable();
   void     CopyDrawable(Int_t id, Int_t px, Int_t py);
   void     DestroyDrawable();
   void     SelectDrawable(Int_t device);

   void     InitPainter();
   void     InvalidateCS();
   void     LockPainter();
   
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
   void     DrawTextNDC(Double_t x, Double_t y, const char *text, ETextMode mode);
                     
   //jpg, png, gif and bmp output.
   void     SaveImage(TVirtualPad *pad, const char *fileName, Int_t type) const;

private:
   
   //Attention! GL_PROJECTION will become 
   //the current matrix after these calls.
   void     SaveProjectionMatrix()const;
   void     RestoreProjectionMatrix()const;
   
   //Attention! GL_MODELVIEW will become the 
   //current matrix after these calls.
   void     SaveModelviewMatrix()const;
   void     RestoreModelviewMatrix()const;
   
   void     SaveViewport();
   void     RestoreViewport();
   
   void     DrawPolyMarker();
   
   TGLPadPainter(const TGLPadPainter &rhs);
   TGLPadPainter & operator = (const TGLPadPainter &rhs);
   
   ClassDef(TGLPadPainter, 0)
};

#endif
