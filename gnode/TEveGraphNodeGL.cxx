// @(#)root/eve:$Id$
// Author: Matevz Tadel 2007

/*************************************************************************
 * Copyright (C) 1995-2007, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include "TEveGraphNodeGL.h"
#include "TEveGraphNode.h"

#include "TGeoShape.h"
#include "TEveGeoShape.h"
#include "TEveUtil.h"
#include "TEveTrans.h"

#include "TGLRnrCtx.h"
#include "TGLIncludes.h"
#include "TGLFaceSet.h"

#include "TBuffer3D.h"

//______________________________________________________________________________
// OpenGL renderer class for TEveGraphNode.
//

ClassImp(TEveGraphNodeGL);

//______________________________________________________________________________
TEveGraphNodeGL::TEveGraphNodeGL() :
   TGLObject(), fM(0), fFaceSet(0)
{
   // Constructor.

   fDLCache = kFALSE; // Disable display list.
}
//______________________________________________________________________________
TEveGraphNodeGL::~TEveGraphNodeGL()
{
   // Destructor.

   delete fFaceSet;
}

/******************************************************************************/

//______________________________________________________________________________
Bool_t TEveGraphNodeGL::SetModel(TObject* obj, const Option_t* /*opt*/)
{
   // Set model object.

   if (SetModelCheckClass(obj, TEveGraphNode::Class())) {
      fM = dynamic_cast<TEveGraphNode*>(obj);

      TGeoShape* s = fM->GetShape();
      if (s) {
         TEveGeoManagerHolder gmgr(TEveGeoShape::GetGeoMangeur(), fM->GetNSegments());

         TBuffer3D& buff = (TBuffer3D&) s->GetBuffer3D
            (TBuffer3D::kCore, kFALSE);

         buff.fID           = s;
         // Irrelevant, taken from graph-node.
         // buff.fColor        = fM->GetMainColor();
         // buff.fTransparency = fM->GetMainTransparency();
         buff.fLocalFrame   = kTRUE; // Always enforce local frame (no geo manager).

         Int_t sections = TBuffer3D::kBoundingBox | TBuffer3D::kShapeSpecific |
                          TBuffer3D::kRawSizes    | TBuffer3D::kRaw;
         s->GetBuffer3D(sections, kTRUE);

         fFaceSet = new TGLFaceSet(buff);
      }
      return kTRUE;
   }
   return kFALSE;
}

//______________________________________________________________________________
void TEveGraphNodeGL::SetBBox()
{
   // Set bounding box.

   if (fFaceSet)
      fBoundingBox = fFaceSet->BoundingBox();
}

/******************************************************************************/

//______________________________________________________________________________
void TEveGraphNodeGL::DirectDraw(TGLRnrCtx & rnrCtx) const
{
   // Render with OpenGL.

   if (fFaceSet)
      fFaceSet->DirectDraw(rnrCtx);

   // Draw the pixel ...
   TGLUtil::PointSize(1);
   glBegin(GL_POINTS);
   glVertex3d(0, 0, 0);
   glEnd();

   // Draw the line
   const Double_t *t = fM->RefMainTrans().ArrT();
   TGLUtil::LineWidth(1);
   glBegin(GL_LINES);
   glVertex3d(0, 0, 0);
   glVertex3d(-t[0], -t[1], -t[2]);
   glEnd();
}
