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
#include "TGLSelectRecord.h"
#include "TGLIncludes.h"

//______________________________________________________________________________
// OpenGL renderer class for TQuakeViz.
//

ClassImp(TQuakeVizGL);

//______________________________________________________________________________
TQuakeVizGL::TQuakeVizGL() :
   TGLObject(), fM(0)
{
   // Constructor.

   fDLCache = kFALSE; // Disable display list.
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
void TQuakeVizGL::DirectDraw(TGLRnrCtx & rnrCtx) const
{
   // Render with OpenGL.

   // printf("TQuakeVizGL::DirectDraw LOD %d\n", rnrCtx.CombiLOD());

   if (rnrCtx.Highlight()) return;

   TEveRGBAPalette* pal = fM->AssertPalette();

   Long64_t minTime = fM->GetLimitTimeMin().GetSec();
   Long64_t maxTime = fM->GetLimitTimeMax().GetSec();

   // For easier palette scaling.
   // Palette initialized to 100 values, need 2*delta.
   Double_t pfac = 100.0 / (maxTime - minTime);

   TGLCapabilitySwitch light_switch(GL_LIGHTING, fM->fLighting);
   TGLCapabilitySwitch auto_norm(GL_NORMALIZE, kTRUE);

   UChar_t c[4], alpha = UChar_t(255 * (1.0 - 0.01*fM->fTransparency));
   UChar_t mc[4];
   TEveUtil::ColorFromIdx(fM->GetMainColor(), mc);
   mc[3] = alpha;

   glPushName(0);
   Int_t idx = 0;
   if (fM->fLimitRange)
   {
      for (TQuakeViz::vQData_i i = fM->fData.begin(); i != fM->fData.end(); ++i, ++idx)
      {
         if (fM->AcceptForDraw(*i) &&
             i->fTime.GetSec() >= minTime && i->fTime.GetSec() <= maxTime)
         {
            Float_t relStr  = (i->fStr  - fM->fMinStr)  / (fM->fMaxStr  - fM->fMinStr);
            Float_t relDist = (i->fDist - fM->fMinDist) / (fM->fMaxDist - fM->fMinDist);
            Float_t colFac  = 1.0f - 0.6f*relDist;

            glPushMatrix();
            glTranslatef(i->fX, i->fY, i->fDepth);

            Int_t val = TMath::Nint(pfac*(i->fTime.GetSec() - minTime));
            pal->ColorFromValue(val, -1, c);
            c[3] = alpha;
            c[0] *= colFac; c[1] *= colFac; c[2] *= colFac;
            TGLUtil::Color4ubv(c);

            glLoadName(idx);
            // Draw sphere: void gluSphere(GLUquadric* quad, double radius, int slices, int stacks)
            gluSphere(rnrCtx.GetGluQuadric(), 0.5f + 4.5f*relStr, 8, 8);
            glPopMatrix();
         }
      }
   }
   else
   {
      for (TQuakeViz::vQData_i i = fM->fData.begin(); i != fM->fData.end(); ++i, ++idx)
      {
         if (fM->AcceptForDraw(*i))
         {
            glPushMatrix();
            glTranslatef(i->fX, i->fY, i->fDepth);

            Float_t relStr  = (i->fStr  - fM->fMinStr)  / (fM->fMaxStr  - fM->fMinStr);
            Float_t relDist = (i->fDist - fM->fMinDist) / (fM->fMaxDist - fM->fMinDist);
            Float_t colFac  = 1.0f - 0.6*relDist;

            c[0] = mc[0]*colFac; c[1] = mc[0]*colFac; c[2] = mc[0]*colFac;
            TGLUtil::Color4ubv(c);

            glLoadName(idx);
            // Draw sphere: void gluSphere(GLUquadric* quad, double radius, int slices, int stacks)
            gluSphere(rnrCtx.GetGluQuadric(), 0.5f + 4.5f*relStr, 8, 8);
            glPopMatrix();
         }
      }
   }
   glPopName();
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
