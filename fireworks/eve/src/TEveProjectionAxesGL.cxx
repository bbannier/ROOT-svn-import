// @(#)root/eve:$Id$
// Author: Matevz Tadel 2007

/*************************************************************************
 * Copyright (C) 1995-2007, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include "TEveProjectionAxesGL.h"
#include "TEveProjectionAxes.h"
#include "TEveProjectionManager.h"

#include "TGLIncludes.h"
#include "TGLRnrCtx.h"
#include "TGLFontManager.h"
#include "TGLCamera.h"

#include "TMath.h"

//______________________________________________________________________________
//
// OpenGL renderer class for TEveProjectionAxes.
//

ClassImp(TEveProjectionAxesGL);

//______________________________________________________________________________
TEveProjectionAxesGL::TEveProjectionAxesGL() :
   TGLObject(),
   fM(0),
   fProjection(0)
{
   // Constructor.

   fDLCache    = kFALSE; // Disable display list.
}

//______________________________________________________________________________
Bool_t TEveProjectionAxesGL::SetModel(TObject* obj, const Option_t* /*opt*/)
{
   // Set model object.
   // Virtual from TGLObject.

   if (SetModelCheckClass(obj, TEveProjectionAxes::Class())) {
      fM = dynamic_cast<TEveProjectionAxes*>(obj);
      return fM->GetManager() ? kTRUE : kFALSE;
   }
   return kFALSE;
}

//______________________________________________________________________________
void TEveProjectionAxesGL::SetBBox()
{
   // Fill the bounding-box data of the logical-shape.
   // Virtual from TGLObject.

   SetAxisAlignedBBox(((TEveProjectionAxes*)fExternalObj)->AssertBBox());
}

//______________________________________________________________________________
void TEveProjectionAxesGL::DrawScales(Bool_t horizontal, TGLFont& font, Float_t tmSize) const
{
   // Draw labels and tick-marks.

   // tick-marks
   Float_t off = 1.5*tmSize;
   glBegin(GL_LINES);
   Int_t cnt = 0;
   Int_t nsd = fM->GetNSecDiv();
   Float_t mh = tmSize*0.5;
   for (TMVec_t::iterator it = fTickMarks.begin(); it != fTickMarks.end(); ++it)
   {
      if (horizontal)
      {
         glVertex2f(*it, 0);
         glVertex2f(*it, (cnt%nsd)? mh:tmSize);
      }
      else
      {
         glVertex2f(0, *it);
         glVertex2f((cnt%nsd)? mh:tmSize, *it);
      }
      cnt++;
   }
   glEnd();

   // labels
   TGLFont::ETextAlign_e align;
   if (horizontal)
      align = (off > 0) ? TGLFont::kCenterUp : TGLFont::kCenterDown;
   else
      align = (off > 0) ? TGLFont::kRight : TGLFont::kLeft;

   // move from center out to be symetric
   Int_t minIdx = TMath::BinarySearch(fTickMarks.size(), &fTickMarks[0], 0.f);
   minIdx /= fM->GetNSecDiv();
   Int_t nl = fLabVec.size();
   const char* txt;

   // right
   Float_t prev = fLabVec[minIdx-1].first;
   Float_t minDist = TMath::Abs(tmSize*5);
   for (Int_t i=minIdx; i<nl; ++i)
   {
      txt =TEveUtil::FormAxisValue(fLabVec[i].second);
      if (i > (minIdx) && (fLabVec[i].first - prev) < minDist)
         continue;

      if (horizontal) 
         font.RenderBitmap(txt, fLabVec[i].first, off, 0, align);
      else
         font.RenderBitmap(txt, off, fLabVec[i].first, 0, align);

      prev = fLabVec[i].first;
   }


   prev = fLabVec[minIdx].first;  
   minIdx -= 1;
   // left
   for (Int_t i=minIdx; i>=0; --i)
   {
      if ((prev - fLabVec[i].first) < minDist)
        continue;
 
      txt =TEveUtil::FormAxisValue(fLabVec[i].second);
      if (horizontal) 
         font.RenderBitmap(txt, fLabVec[i].first, off, 0, align);
      else
         font.RenderBitmap(txt, off, fLabVec[i].first, 0, align);
      prev = fLabVec[i].first;
   }
}

//______________________________________________________________________________
void TEveProjectionAxesGL::SplitInterval(Float_t p1, Float_t p2, Int_t ax) const
{
   // Build an array of tick-mark position-value pairs.

   fLabVec.clear();
   fTickMarks.clear();

   if (fM->GetLabMode() == TEveProjectionAxes::kValue)
   {
      SplitIntervalByVal(fProjection->GetValForScreenPos(ax, p1), fProjection->GetValForScreenPos(ax, p2), ax);
   }
   else if (fM->GetLabMode() == TEveProjectionAxes::kPosition)
   {
      SplitIntervalByPos(p1, p2, ax);
   }
}

//______________________________________________________________________________
void TEveProjectionAxesGL::SplitIntervalByPos(Float_t p1, Float_t p2, Int_t ax) const
{
   // Add tick-marks at equidistant position.

   Float_t step = fProjection->GetScreenVal(ax, fM->GetStep());
   Int_t nsd = fM->GetNSecDiv();
   Float_t ss = step/nsd;

   Int_t n1=Int_t(p1/step);
   Int_t n2=Int_t(p2/step);
   Float_t p = n1*step;

   for (Int_t l=n1; l<=n2; l++)
   {
      fLabVec.push_back(Lab_t(p , fProjection->GetValForScreenPos(ax, p)));
      for (Int_t i=0; i<nsd; i++)
      {
         fTickMarks.push_back(p + i*ss);
      }
      p += step;
   }
}

//______________________________________________________________________________
void TEveProjectionAxesGL::SplitIntervalByVal(Float_t v1, Float_t v2, Int_t ax) const
{
   // Add tick-marks on fixed value step.

   Float_t step = fM->GetStep();
   Int_t nsd = fM->GetNSecDiv();
   Float_t ss = step/nsd;

   Int_t n1 = Int_t (v1/step);
   Int_t n2 = Int_t (v2/step);
   Float_t v = n1*step;

   for (Int_t l=n1; l<=n2; l++)
   {
      fLabVec.push_back(Lab_t(fProjection->GetScreenVal(ax, v) , v));
      for (Int_t i=0; i<nsd; i++)
      {
         fTickMarks.push_back(fProjection->GetScreenVal(ax, v+i*ss));
      }
      v += step;
   }
}

//______________________________________________________________________________
void TEveProjectionAxesGL::DirectDraw(TGLRnrCtx& rnrCtx) const
{
   // Actual rendering code.
   // Virtual from TGLLogicalShape.

   if (rnrCtx.Selection() || rnrCtx.Highlight()) return;

   fProjection = fM->GetManager()->GetProjection();
   Float_t bf = 1.2; // epxand to see scales over bounding box
   Float_t bbox[6];
   for(Int_t i=0; i<6; i++)
      bbox[i] = bf*fM->GetManager()->GetBBox()[i];

   // horizontal font setup

   Float_t l =  -rnrCtx.GetCamera()->FrustumPlane(TGLCamera::kLeft).D();
   Float_t r =   rnrCtx.GetCamera()->FrustumPlane(TGLCamera::kRight).D();
   Float_t t =   rnrCtx.GetCamera()->FrustumPlane(TGLCamera::kTop).D();
   Float_t b =  -rnrCtx.GetCamera()->FrustumPlane(TGLCamera::kBottom).D();

   TGLFont font;
   Int_t rng;
   GLint    vp[4];
   glGetIntegerv(GL_VIEWPORT, vp);

   if (fM->fAxesMode == TEveProjectionAxes::kAll || TEveProjectionAxes::kVertical)
      rng = vp[3];
   else
      rng = vp[2];

   Int_t fs =  TGLFontManager::GetFontSize(rng*fM->GetLabelSize(), 8, 36);
   rnrCtx.RegisterFont(fs, "arial", TGLFont::kPixmap, font);
   font.PreRender();

   glPushMatrix();
   TGLMatrix modview;
   glGetDoublev(GL_MODELVIEW_MATRIX, modview.Arr());
   TGLVertex3 worldRef;
   TGLVector3 diff = rnrCtx.RefCamera().ViewportDeltaToWorld(worldRef, 1, 1, &modview);
   Float_t tms = diff.X()*rng*0.02;
   Float_t uLim, dLim;
   Float_t start, end;
   Float_t limFac = 0.98;
   //______________________________________________________________________________
   // X-axis

   if (fM->fAxesMode == TEveProjectionAxes::kAll
       || (fM->fAxesMode == TEveProjectionAxes::kHorizontal))
   {
      dLim = fProjection->GetLimit(0, 0)*limFac;
      uLim = fProjection->GetLimit(0, 1)*limFac;
      start =  (bbox[0] > dLim) ?  bbox[0] : dLim;
      end =    (bbox[1] < uLim) ?  bbox[1] : uLim;
      SplitInterval(start, end, 0);
      {
         // bottom
         glPushMatrix();
         glTranslatef((bbox[1]+bbox[0])*0.5, b, 0);
         DrawScales(kTRUE, font, tms);
         glPopMatrix();
      }
      {
         // top
         glPushMatrix();
         glTranslatef((bbox[1]+bbox[0])*0.5, t, 0);
         DrawScales(kTRUE, font, -tms);
         glPopMatrix();
      }
   }

   // Y-axis
   if (fM->fAxesMode == TEveProjectionAxes::kAll
       || (fM->fAxesMode == TEveProjectionAxes::kVertical))
   {
      dLim = fProjection->GetLimit(1, 0)*limFac;
      uLim = fProjection->GetLimit(1, 1)*limFac;
      start =  (bbox[2] > dLim) ? bbox[2] : dLim;
      end =    (bbox[3] < uLim) ? bbox[3] : uLim;
      // left
      SplitInterval(start, end, 1);
      {
         glPushMatrix();
         glTranslatef(l, (bbox[3]+bbox[2])*0.5, 0);
         DrawScales(kFALSE, font, tms);
         glPopMatrix();
      }
      // right
      {
         glPushMatrix();
         glTranslatef(r, (bbox[3]+bbox[2])*0.5, 0);
         DrawScales(kFALSE, font, -tms);
         glPopMatrix();
      }
   }

   glPopMatrix();
   font.PostRender();
}
