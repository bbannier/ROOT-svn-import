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

   TEveRGBAPalette* pal = fM->AssertPalette();

   TTimeStamp midTime(fM->fYear, fM->fMonth, fM->fDay, fM->fHour, 0, 0);

   Long64_t delta   = 3600*24*fM->fDayHalfRange;

   Long64_t minTime = midTime.GetSec() - delta;
   Long64_t maxTime = midTime.GetSec() + delta;

   // For easier palette scaling.
   // Palette initialized to 100 values, need 2*delta.
   Double_t pfac = 50.0 / delta;

   TGLCapabilitySwitch lights_off(GL_LIGHTING, fM->fLighting);

   UChar_t c[4], alpha = UChar_t(255 * (1.0 - 0.01*fM->fTransparency));

   if (fM->fLimitRange)
   {
      for (TQuakeViz::vQData_i i = fM->fData.begin(); i != fM->fData.end(); ++i)
      {
         if (i->fTime.GetSec() >= minTime && i->fTime.GetSec() <= maxTime)
         {
            glPushMatrix();
            glTranslatef(i->fLat, i->fLon, i->fDepth);

            Int_t val = TMath::Nint(pfac*(i->fTime.GetSec() - minTime));
            pal->ColorFromValue(val, -1, c);
            c[3] = alpha;
            TGLUtil::Color4ubv(c);

            gluSphere(rnrCtx.GetGluQuadric(),
                      0.005f + (i->fStr - fM->fMinStr)*0.045f/(fM->fMaxStr - fM->fMinStr),
                      8, 8);
            glPopMatrix();
         }
      }
   }
   else
   {
      for (TQuakeViz::vQData_i i = fM->fData.begin(); i != fM->fData.end(); ++i)
      {
         glPushMatrix();
         glTranslatef(i->fLat, i->fLon, i->fDepth);

         gluSphere(rnrCtx.GetGluQuadric(),
                   0.005f + (i->fStr - fM->fMinStr)*0.045f/(fM->fMaxStr - fM->fMinStr),
                   8, 8);
         glPopMatrix();
      }
   }
}
