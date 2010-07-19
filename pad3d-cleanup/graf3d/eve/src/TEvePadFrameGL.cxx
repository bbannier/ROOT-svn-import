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
#include "TGLPadPainter3D.h"

//______________________________________________________________________________
// OpenGL renderer class for TEvePadFrame.
//

ClassImp(TEvePadFrameGL);

TGLPadPainter3D *TEvePadFrameGL::fgPainter = new TGLPadPainter3D;

//______________________________________________________________________________
TEvePadFrameGL::TEvePadFrameGL() :
   TGLObject(), fM(0)
{
   // Constructor.

   fDLCache = kFALSE; // Disable display list.
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

/******************************************************************************/

//______________________________________________________________________________
void TEvePadFrameGL::DirectDraw(TGLRnrCtx& /*rnrCtx*/) const
{
   // Render with OpenGL.

   if (fM->GetPad() == 0)
      return;

   //printf("TEvePadFrameGL::DirectDraw, pad=%p\n", pad);

   TPad           *pad    = fM->GetPad();
   const Double_t  z_step = 0.1 * 0.2 * fM->GetSizeX() / pad->GetListOfPrimitives()->GetSize();

   TVirtualPad *opad = gPad;
   gPad = pad;
   TVirtualPadPainter *opainter = pad->GetCanvas()->SwitchCanvasPainter(fgPainter);

   glPushAttrib(GL_ENABLE_BIT);

   Double_t x1, y1, x2, y2;  pad->GetRange(x1, y1, x2, y2);

   glPushMatrix();
   glScaled(fM->GetSizeX() / (x2 - x1),
            (fM->GetSizeX() * pad->GetWh()) / pad->GetWw() / (y2 - y1),
            1.0);
   glTranslated(-x1, -y1, -z_step);

   glDisable(GL_CULL_FACE);

   TGLUtil::Color(pad->GetFrameFillColor());
   glBegin(GL_QUADS);
   glVertex2d(x1, y1); glVertex2d(x2, y1); 
   glVertex2d(x2, y2); glVertex2d(x1, y2); 
   glEnd();

   fgPainter->InitPainterForGLViewer();

   TObjOptLink *lnk = (TObjOptLink*) pad->GetListOfPrimitives()->FirstLink();
   while (lnk)
   {
      glTranslated(0.0, 0.0, z_step);

      lnk->GetObject()->Paint(lnk->GetOption());
      lnk = (TObjOptLink*) lnk->Next();
   }

   glPopMatrix();

   fgPainter->LockPainter();
   glPopAttrib();

   gPad->GetCanvas()->SwitchCanvasPainter(opainter);
   gPad = opad;
}
