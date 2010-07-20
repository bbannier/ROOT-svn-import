// @(#)root/eve:$Id$
// Author: Matevz Tadel 2007

/*************************************************************************
 * Copyright (C) 1995-2007, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include "TEvePadFrameGL.h"
#include "TEvePadFrame.h"

#include "TGLRnrCtx.h"
#include "TGLIncludes.h"

#include "TList.h"
#include "TCanvas.h"
#include "TPad.h"
#include "TGLPadPainter.h"
#include "TGLPadPainter3D.h"

//______________________________________________________________________________
// OpenGL renderer class for TEvePadFrame.
//

ClassImp(TEvePadFrameGL);

TGLPadPainter   *TEvePadFrameGL::fgPainter   = new TGLPadPainter;
TGLPadPainter3D *TEvePadFrameGL::fgPainter3D = new TGLPadPainter3D;

//______________________________________________________________________________
TEvePadFrameGL::TEvePadFrameGL() :
   TGLObject(), fM(0), fFBO(0), fRTS(0)
{
   // Constructor.

   // fDLCache = kFALSE; // Disable display list.
}

/******************************************************************************/

//______________________________________________________________________________
Bool_t TEvePadFrameGL::SetModel(TObject* obj, const Option_t* /*opt*/)
{
   // Set model object.

   if (SetModelCheckClass(obj, TEvePadFrame::Class())) {
      fM = dynamic_cast<TEvePadFrame*>(obj);
      return kTRUE;
   }
   return kFALSE;
}

//______________________________________________________________________________
void TEvePadFrameGL::SetBBox()
{
   // Set bounding box.

   SetAxisAlignedBBox(((TEvePadFrame*)fExternalObj)->AssertBBox());
}

//==============================================================================

//______________________________________________________________________________
void TEvePadFrameGL::DirectDrawIntoFBO(const TGLRnrCtx& rnrCtx) const
{
   // Draw the pad in 2D mode into the FBO.

}

//______________________________________________________________________________
void TEvePadFrameGL::DirectDrawFBO(const TGLRnrCtx& rnrCtx) const
{
   // Draw the pad in 2D mode using existing FBO.

}

//______________________________________________________________________________
void TEvePadFrameGL::DirectDraw3D(const TGLRnrCtx& rnrCtx) const
{
   // Draw the pad in 3D mode.

}

//==============================================================================

//______________________________________________________________________________
Bool_t TEvePadFrameGL::ShouldDLCache(const TGLRnrCtx& rnrCtx) const
{

}

//______________________________________________________________________________
void TEvePadFrameGL::DirectDraw(TGLRnrCtx& /*rnrCtx*/) const
{
   // Render with OpenGL.

   // printf("TEvePadFrameGL::DirectDraw ... UseFBO=%d\n", fM->fUseFBO);

   if (fM->fPad == 0)
      return;

   TPad           *pad    = fM->fPad;
   const Double_t  z_step = 0.1 * 0.2 * fM->fSizeX / pad->GetListOfPrimitives()->GetSize();

   TVirtualPad *opad = gPad;
   gPad = pad;
   TVirtualPadPainter *opainter = pad->GetCanvas()->SwitchCanvasPainter(fgPainter3D);

   glPushAttrib(GL_ENABLE_BIT);

   Double_t x1, y1, x2, y2;  pad->GetRange(x1, y1, x2, y2);

   glPushMatrix();
   glScaled(fM->fSizeX / (x2 - x1),
            (fM->fSizeX * pad->GetWh()) / pad->GetWw() / (y2 - y1),
            1.0);
   glTranslated(-x1, -y1, -z_step);

   glDisable(GL_CULL_FACE);

   TGLUtil::Color(pad->GetFrameFillColor());
   glBegin(GL_QUADS);
   glVertex2d(x1, y1); glVertex2d(x2, y1); 
   glVertex2d(x2, y2); glVertex2d(x1, y2); 
   glEnd();

   fgPainter3D->InitPainterForGLViewer();

   TObjOptLink *lnk = (TObjOptLink*) pad->GetListOfPrimitives()->FirstLink();
   while (lnk)
   {
      glTranslated(0.0, 0.0, z_step);

      lnk->GetObject()->Paint(lnk->GetOption());
      lnk = (TObjOptLink*) lnk->Next();
   }

   glPopMatrix();

   fgPainter3D->LockPainter();
   glPopAttrib();

   gPad->GetCanvas()->SwitchCanvasPainter(opainter);
   gPad = opad;
}
