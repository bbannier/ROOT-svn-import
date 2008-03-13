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

   // fDLCache = kFALSE; // Disable display list.
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

   TGLCapabilitySwitch lights_off(GL_LIGHTING, kFALSE);

   for (TQuakeViz::vQData_i i = fM->fData.begin(); i != fM->fData.end(); ++i)
   {
      glPushMatrix();
      glTranslatef(i->fLat, i->fLon, i->fDepth);
      // TGLUtil::Color4f
      gluSphere(rnrCtx.GetGluQuadric(),
                0.005f + (i->fStr - fM->fMinStr)*0.045f/(fM->fMaxStr - fM->fMinStr),
                8, 8);
      glPopMatrix();
   }
}
