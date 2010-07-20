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
#include "TGLFBO.h"
#include "TGLIncludes.h"

#include "TList.h"
#include "TCanvas.h"
#include "TPad.h"
#include "TGLPadPainter.h"
#include "TGLPadPainter3D.h"

#include "TMath.h"

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

//______________________________________________________________________________
TEvePadFrameGL::~TEvePadFrameGL()
{
   // Destructor.
}

//==============================================================================

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
void TEvePadFrameGL::DirectDrawIntoFBO(const TGLRnrCtx& /*rnrCtx*/) const
{
   // Draw the pad in 2D mode into the FBO.

   TPad        *pad  = fM->fPad;
   TVirtualPad *opad = gPad;
   gPad = pad;

   TCanvas *canvas = pad->GetCanvas();
   Int_t  reqw = fFBO->GetRequestedW();
   Int_t  reqh = fFBO->GetRequestedH();
   UInt_t ocw, och;
   canvas->SetCanvasSizeForOffscreenRendering(reqw, reqh, ocw, och);
   TVirtualPadPainter *opainter = canvas->SwitchCanvasPainter(fgPainter);

   glPushAttrib(GL_ALL_ATTRIB_BITS);
   glMatrixMode(GL_PROJECTION); glPushMatrix();
   glMatrixMode(GL_MODELVIEW);  glPushMatrix();

   glViewport(0, 0, reqw, reqh);

   fgPainter->InitPainterWithOptions(kFALSE);

   TObjOptLink *lnk = (TObjOptLink*) pad->GetListOfPrimitives()->FirstLink();
   while (lnk)
   {
      lnk->GetObject()->Paint(lnk->GetOption());
      lnk = (TObjOptLink*) lnk->Next();
   }

   fgPainter->LockPainter();

   glMatrixMode(GL_PROJECTION); glPopMatrix();
   glMatrixMode(GL_MODELVIEW);  glPopMatrix();
   glPopAttrib();

   canvas->RestoreCanvasSizeAfterOffscreenRendering(ocw, och);

   canvas->SwitchCanvasPainter(opainter);
   gPad = opad;
}

//______________________________________________________________________________
void TEvePadFrameGL::DirectDrawFBO(const TGLRnrCtx& /*rnrCtx*/) const
{
   // Draw the pad in 2D mode using existing FBO.

   TPad *pad  = fM->fPad;

   glPushAttrib(GL_ENABLE_BIT);
   glDisable(GL_CULL_FACE);
   glDisable(GL_LIGHTING);

   const Double_t sx = fM->fSizeX;
   const Double_t sy = (sx * pad->GetWh()) / pad->GetWw();

   fFBO->BindTexture();
   glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
   // glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, env-rgba);

   glBegin(GL_QUADS);
   glTexCoord2d(0, 0); glVertex2d( 0,  0);
   glTexCoord2d(1, 0); glVertex2d(sx,  0); 
   glTexCoord2d(1, 1); glVertex2d(sx, sy);
   glTexCoord2d(0, 1); glVertex2d( 0, sy); 
   glEnd();

   fFBO->UnbindTexture();

   glPopAttrib();
}

//______________________________________________________________________________
void TEvePadFrameGL::DirectDraw3D(const TGLRnrCtx& /*rnrCtx*/) const
{
   // Draw the pad in 3D mode.

   TPad        *pad  = fM->fPad;
   TVirtualPad *opad = gPad;
   gPad = pad;

   TVirtualPadPainter *opainter = pad->GetCanvas()->SwitchCanvasPainter(fgPainter3D);

   glPushAttrib(GL_ENABLE_BIT);

   const Double_t z_step = 0.1 * 0.2 * fM->fSizeX / pad->GetListOfPrimitives()->GetSize();
   Double_t x1, y1, x2, y2;
   pad->GetRange(x1, y1, x2, y2);

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

//==============================================================================

//______________________________________________________________________________
Bool_t TEvePadFrameGL::ShouldDLCache(const TGLRnrCtx& rnrCtx) const
{
   // Override from TGLObject -- we really just need to check rendering
   // time-stamp here.

   // Check if FBO needs to be repainted.
   if (fM->fPad && fM->fUseFBO)
   {
      if (fM->fRTS > fRTS)
      {
         if (fFBO == 0) fFBO = new TGLFBO;         
      }
      TPad *pad = fM->fPad;
      Int_t w, h;
      if (fM->fSizeFBO > 0) {
         w = fM->fSizeFBO;
         h = TMath::Nint(((Double_t)w * pad->GetWh()) / pad->GetWw());
      } else {
         w = pad->GetWw();
         h = pad->GetWh();
      }
      fFBO->Init(w, h);
      fFBO->Bind();
      DirectDrawIntoFBO(rnrCtx);
      fFBO->Unbind();

      if (fM->fUseMipmaps)
      {
         fFBO->BindTexture();
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
         glGenerateMipmapEXT(GL_TEXTURE_2D);
         fFBO->UnbindTexture();
      }
   }
   else if (fFBO != 0)
   {
      delete fFBO;
      fFBO = 0;
   }

   return TGLObject::ShouldDLCache(rnrCtx);
}

//______________________________________________________________________________
void TEvePadFrameGL::DirectDraw(TGLRnrCtx& rnrCtx) const
{
   // Render with OpenGL.

   if (fM->fPad == 0)
      return;

   // printf("TEvePadFrameGL::DirectDraw ... UseFBO=%d, ww=%d, wh=%d\n", fM->fUseFBO, fM->fPad->GetWw(), fM->fPad->GetWh());

   if (fM->fUseFBO)
   {
      DirectDrawFBO(rnrCtx);
   }
   else
   {
      DirectDraw3D(rnrCtx);
   }
}
