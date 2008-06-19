// @(#)root/eve:$Id$
// Author: Matevz Tadel 2007

/*************************************************************************
 * Copyright (C) 1995-2007, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include "TQuakeVizGL.h"
#include "TQuakeViz.h"

#include "TEveRGBAPalette.h"

#include "TMath.h"

#include "TGLRnrCtx.h"
#include "TGLContext.h"
#include "TGLSelectRecord.h"
#include "TGLScene.h"
#include "TGLIncludes.h"

//______________________________________________________________________________
// OpenGL renderer class for TQuakeViz.
//

ClassImp(TQuakeVizGL);

//______________________________________________________________________________
TQuakeVizGL::TQuakeVizGL() :
   TGLObject(), fM(0), fSphereDL(0)
{
   // Constructor.

   // We lie here so that DLCacheDrop/Purge() will get called.
   // In ShouldDLCache() we always return false.
   fDLCache    = kTRUE;

   fMultiColor = kTRUE;
}

/******************************************************************************/

//______________________________________________________________________________
Bool_t TQuakeVizGL::ShouldDLCache(const TGLRnrCtx& rnrCtx) const
{
   // Don't use display-lists ... we use one internally for sphere-rendering.
   // Virtual from TGLLogicalShape.

   return kFALSE;
}

//______________________________________________________________________________
void TQuakeVizGL::DLCacheDrop()
{
   // Called when display lists have been destroyed externally and the
   // internal display-list data needs to be cleare.
   // Virtual from TGLLogicalShape.

   fSphereDL = 0;
}

//______________________________________________________________________________
void TQuakeVizGL::DLCachePurge()
{
   // Called when display-lists need to be returned to the system.
   // Virtual from TGLLogicalShape.

   static const TEveException eH("TQuakeVizGL::DLCachePurge ");

   if (fSphereDL == 0) return;
   if (fScene)
   {
      fScene->GetGLCtxIdentity()->RegisterDLNameRangeToWipe(fSphereDL, 1);
   }
   else
   {
      Warning(eH, "TEveScene unknown, attempting direct deletion.");
      glDeleteLists(fSphereDL, 1);
   }
}

/******************************************************************************/

//______________________________________________________________________________
Bool_t TQuakeVizGL::SetModel(TObject* obj, const Option_t* /*opt*/)
{
   // Set model object.

   if (SetModelCheckClass(obj, TQuakeViz::Class())) {
      fM = dynamic_cast<TQuakeViz*>(obj);
      return kTRUE;
   }
   return kFALSE;
}

//______________________________________________________________________________
void TQuakeVizGL::SetBBox()
{
   // Set bounding box.

   // !! This ok if master sub-classed from TAttBBox
   SetAxisAlignedBBox(((TQuakeViz*)fExternalObj)->AssertBBox());
}

/******************************************************************************/

//______________________________________________________________________________
void TQuakeVizGL::MakeSphereDL(TGLRnrCtx& rnrCtx) const
{
   // Create display-list for unit-sphere.
   // Only called when fSphereDL is undefined.

   fSphereDL = glGenLists(1);
   glNewList(fSphereDL, GL_COMPILE);
   gluSphere(rnrCtx.GetGluQuadric(), 1, 18, 18);
   glEndList();
}

//______________________________________________________________________________
void TQuakeVizGL::DirectDraw(TGLRnrCtx & rnrCtx) const
{
   // Render with OpenGL.

   // printf("TQuakeVizGL::DirectDraw LOD %d\n", rnrCtx.CombiLOD());

   if (rnrCtx.Highlight()) return;

   if (fSphereDL == 0)
      MakeSphereDL(rnrCtx);

   TEveRGBAPalette* pal = fM->AssertPalette();

   Long64_t minTime = fM->GetLimitTimeMin().GetSec();
   Long64_t maxTime = fM->GetLimitTimeMax().GetSec();

   // For easier palette scaling.
   // Palette initialized to 100 values, need 2*delta.
   Double_t pfac = 100.0 / (maxTime - minTime);

   TGLCapabilitySwitch lights(GL_LIGHTING,  fM->fLighting);
   TGLCapabilitySwitch norms (GL_NORMALIZE, kTRUE);

   UChar_t c[4], alpha = UChar_t(255 * (1.0 - 0.01*fM->fTransparency));
   UChar_t mc[4];
   TEveUtil::ColorFromIdx(fM->GetMainColor(), mc);
   mc[3] = alpha;

   if (rnrCtx.Selection()) glPushName(0);
   if (fM->fLimitRange)
   {
      Int_t idx = 0;
      for (TQuakeViz::vQData_i i = fM->fData.begin(); i != fM->fData.end(); ++i, ++idx)
      {
         if (fM->AcceptForDraw(*i) &&
             i->fTime.GetSec() >= minTime && i->fTime.GetSec() <= maxTime)
         {
            Float_t relStr  = (i->fStr  - fM->fMinStr)  / (fM->fMaxStr  - fM->fMinStr);
            Float_t relDist = (i->fDist - fM->fMinDist) / (fM->fMaxDist - fM->fMinDist);
            // Float_t colFac  = 1.0f - 0.6f*relDist;
            Float_t colFac  = 0.2 + 0.8*relDist;

            glPushMatrix();
            glTranslatef(i->fX, i->fY, i->fDepth);

            Int_t val = TMath::Nint(pfac*(i->fTime.GetSec() - minTime));
            pal->ColorFromValue(val, -1, c);
            c[0] = TMath::Nint(colFac*c[0]);
            c[1] = TMath::Nint(colFac*c[1]);
            c[2] = TMath::Nint(colFac*c[2]);
            c[3] = alpha;
            TGLUtil::Color4ubv(c);

            Float_t radius = 0.5f + 4.5f*relStr;
            glScalef(radius, radius, radius);
            if (rnrCtx.Selection()) glLoadName(idx);
            glCallList(fSphereDL);
            glPopMatrix();
         }
      }
   }
   else
   {
      Int_t idx = 0;
      for (TQuakeViz::vQData_i i = fM->fData.begin(); i != fM->fData.end(); ++i, ++idx)
      {
         if (fM->AcceptForDraw(*i))
         {
            glPushMatrix();
            glTranslatef(i->fX, i->fY, i->fDepth);

            Float_t relStr  = (i->fStr  - fM->fMinStr)  / (fM->fMaxStr  - fM->fMinStr);
            Float_t relDist = (i->fDist - fM->fMinDist) / (fM->fMaxDist - fM->fMinDist);
            // Float_t colFac  = 1.0f - 0.6*relDist;
            Float_t colFac  = 0.2 + 0.8*relDist;

            c[0] = TMath::Nint(mc[0]*colFac);
            c[1] = TMath::Nint(mc[1]*colFac);
            c[2] = TMath::Nint(mc[2]*colFac);
            c[3] = alpha;
            // c[3] = TMath::Nint(alpha*(1.0f - colFac)); // ? without factor?
            TGLUtil::Color4ubv(c);

            Float_t radius = 0.5f + 4.5f*relStr;
            glScalef(radius, radius, radius);
            if (rnrCtx.Selection()) glLoadName(idx);
            glCallList(fSphereDL);
            glPopMatrix();
         }
      }
   }
   if (rnrCtx.Selection()) glPopName();
}

//______________________________________________________________________________
void TQuakeVizGL::ProcessSelection(TGLRnrCtx & /*rnrCtx*/, TGLSelectRecord & rec)
{
   // Processes secondary selection from TGLViewer.
   // Prints-out the dtails of the e-quake clicked on my Alt-Mouse-1.

   if (rec.GetN() < 2) return;
   printf("----------------------------------------------------------------\n");
   fM->fData[rec.GetItem(1)].Print();
}
