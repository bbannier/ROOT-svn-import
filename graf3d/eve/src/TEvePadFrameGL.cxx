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

//______________________________________________________________________________
// OpenGL renderer class for TEvePadFrame.
//

ClassImp(TEvePadFrameGL);

TGLPadPainter *TEvePadFrameGL::fgPainter = new TGLPadPainter;

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

   TPad *pad = fM->GetPad();

   //printf("TEvePadFrameGL::DirectDraw, pad=%p\n", pad);

   if (fM->GetPad() == 0)
      return;

   TVirtualPad *opad = gPad;
   gPad = fM->GetPad();
   TVirtualPadPainter *opainter = gPad->GetCanvas()->SwitchCanvasPainter(fgPainter);

   glPushAttrib(GL_ENABLE_BIT);
   fgPainter->InitPainterForGLViewer();

   TObjOptLink *lnk = (TObjOptLink*) gPad->GetListOfPrimitives()->FirstLink();
   while (lnk)
   {
      lnk->GetObject()->Paint(lnk->GetOption());
      lnk = (TObjOptLink*) lnk->Next();
   }

   fgPainter->LockPainter();
   glPopAttrib();

   gPad->GetCanvas()->SwitchCanvasPainter(opainter);
   gPad = opad;
}
