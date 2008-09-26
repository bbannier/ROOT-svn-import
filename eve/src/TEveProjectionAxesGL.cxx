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
#include "THLimitsFinder.h"

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
void TEveProjectionAxesGL::DrawScales(Bool_t horizontal, TGLFont& font, Float_t tmSize, Float_t dtw) const
{
   // Draw labels and tick-marks.

   // tick-marks
   Float_t off = 1.5*tmSize;
   glBegin(GL_LINES);
   Int_t cnt = 0;
   Int_t nsd = fTickMarks.size()/fLabVec.size();
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
   Float_t llx, lly, llz, urx, ury, urz;

   // move from center out to be symetric
   Int_t minIdx = TMath::BinarySearch(fTickMarks.size(), &fTickMarks[0], 0.f);
   minIdx /= nsd;
   Int_t nl = fLabVec.size();
   const char* txt;

   // right
   Float_t prev = fLabVec[minIdx-1].first;
   for (Int_t i=minIdx; i<nl; ++i)
   {
      txt =TEveUtil::FormAxisValue(fLabVec[i].second);
      font.BBox(txt, llx, lly, llz, urx, ury, urz);
      if (i > (minIdx) && prev > (fLabVec[i].first - (urx-llx)*0.5*dtw))
         continue;

      if (horizontal)
         font.RenderBitmap(txt, fLabVec[i].first, off, 0, align);
      else
         font.RenderBitmap(txt, off, fLabVec[i].first, 0, align);

      prev = fLabVec[i].first + (urx-llx)*0.5*dtw;
   }

   // left
   txt =TEveUtil::FormAxisValue(fLabVec[minIdx].second);
   font.BBox(txt, llx, lly, llz, urx, ury, urz);
   prev = fLabVec[minIdx].first -(urx-llx)*0.5 *dtw;
   minIdx -= 1;
   for (Int_t i=minIdx; i>=0; --i)
   {
      txt =TEveUtil::FormAxisValue(fLabVec[i].second);
      font.BBox(txt, llx, lly, llz, urx, ury, urz);
      if ( prev < (fLabVec[i].first + (urx-llx)*0.5*dtw ))
         continue;

      if (horizontal)
         font.RenderBitmap(txt, fLabVec[i].first, off, 0, align);
      else
         font.RenderBitmap(txt, off, fLabVec[i].first, 0, align);

      prev = fLabVec[i].first -(urx-llx)*0.5 *dtw;
   }
}

//______________________________________________________________________________
void TEveProjectionAxesGL::SplitInterval(Float_t p1, Float_t p2, Int_t ax) const
{
   // Build an array of tick-mark position-value pairs.

   fLabVec.clear();
   fTickMarks.clear();

    // optimise linits
   Int_t ndiv = fM->GetNdivisions();
   Int_t n1a = TMath::FloorNint(ndiv/100);
   Int_t n2a = ndiv-n1a*100;
   Int_t bn1, bn2;
   Double_t bw1, bw2; // bin with first second order
   Double_t bl1, bh1, bl2, bh2; // bin low, high first second order

   THLimitsFinder::Optimize(p1, p2, n1a, bl1, bh1, bn1, bw1);
   THLimitsFinder::Optimize(bl1, bl1+bw1, n2a, bl2, bh2, bn2, bw2);
   //    printf("%f labels %d sec tick \n", bw1, bn2);

   if (fM->GetLabMode() == TEveProjectionAxes::kValue)
   {
      SplitIntervalByVal(fProjection->GetValForScreenPos(ax, p1),
                         fProjection->GetValForScreenPos(ax, p2),
                         ax, bw1, bn2);
   }
   else if (fM->GetLabMode() == TEveProjectionAxes::kPosition)
   {
      SplitIntervalByPos(p1, p2, ax, bw1, bn2);
   }
}

//______________________________________________________________________________
void TEveProjectionAxesGL::SplitIntervalByPos(Float_t p1, Float_t p2, Int_t ax, Float_t step, Int_t nsd) const
{
   // Add tick-marks at equidistant position.

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
void TEveProjectionAxesGL::SplitIntervalByVal(Float_t v1, Float_t v2, Int_t ax, Float_t step, Int_t nsd) const
{
   // Add tick-marks on fixed value step.

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
   // draw axis orund bbox with offset
   Float_t bsf = 1.2;
   Float_t bbox[6];
   for(Int_t i=0; i<6; i++)
      bbox[i] = bsf*fM->GetManager()->GetBBox()[i];

   // horizontal font setup
   Float_t l =  -rnrCtx.GetCamera()->FrustumPlane(TGLCamera::kLeft).D();
   Float_t r =   rnrCtx.GetCamera()->FrustumPlane(TGLCamera::kRight).D();
   Float_t t =   rnrCtx.GetCamera()->FrustumPlane(TGLCamera::kTop).D();
   Float_t b =  -rnrCtx.GetCamera()->FrustumPlane(TGLCamera::kBottom).D();

   // font size relative to wiewport width
   TGLFont font;
   GLint    vp[4];
   glGetIntegerv(GL_VIEWPORT, vp);
   Int_t fs =  TGLFontManager::GetFontSize(TMath::Min(vp[2], vp[3])*fM->GetLabelSize(), 8, 36);
   rnrCtx.RegisterFont(fs, "arial", TGLFont::kPixmap, font);
   font.PreRender();

   glPushMatrix();
   //______________________________________________________________________________
   // X-axis

   Float_t limFac = 0.98; // set limit factor in case of divergence
   Float_t frOff  = 0.8; // draw frustum axis with offset not to cross X and Y axis
   if (fM->fAxesMode == TEveProjectionAxes::kAll
       || (fM->fAxesMode == TEveProjectionAxes::kHorizontal))
   {
      Float_t p0 = bbox[0] > l*frOff ? bbox[0] : l*frOff;
      Float_t p1=  bbox[1] < r*frOff ? bbox[1] : r*frOff;
      Float_t dLim = fProjection->GetLimit(0, 0)*limFac;
      Float_t uLim = fProjection->GetLimit(0, 1)*limFac;
      Float_t start =  (p0 > dLim) ?  p0 :dLim;
      Float_t end =    (p1 < uLim) ?  p1 :uLim;
      Float_t tms = (t-b)*0.02;
      Float_t dtw = (r-l)/vp[2]; // delta to viewport

      SplitInterval(start, end, 0);
      {
         // bottom
         glPushMatrix();
         glTranslatef((bbox[1]+bbox[0])*0.5, b, 0);
         DrawScales(kTRUE, font, tms, dtw);
         glPopMatrix();
      }
      {
         // top
         glPushMatrix();
         glTranslatef((bbox[1]+bbox[0])*0.5, t, 0);
         DrawScales(kTRUE, font, -tms, dtw);
         glPopMatrix();
      }
   }

   // Y-axis
   if (fM->fAxesMode == TEveProjectionAxes::kAll
       || (fM->fAxesMode == TEveProjectionAxes::kVertical))
   {
      Float_t p0 = bbox[2] > b*frOff ? bbox[2] : b*frOff;
      Float_t p1=  bbox[3] < t*frOff ? bbox[3] : t*frOff;

      Float_t dLim = fProjection->GetLimit(1, 0)*limFac;
      Float_t uLim = fProjection->GetLimit(1, 1)*limFac;
      Float_t start =  (p0 > dLim) ? p0 : dLim;
      Float_t end =    (p1 < uLim) ? p1 : uLim;
      Float_t tms = (r-l)*0.015; 
      Float_t dtw = (t-b)/vp[3];// delta to viewport

      // left
      SplitInterval(start, end, 1);
      {
         glPushMatrix();
         glTranslatef(l, (bbox[3]+bbox[2])*0.5, 0);
         DrawScales(kFALSE, font, tms, dtw);
         glPopMatrix();
      }
      // right
      {
         glPushMatrix();
         glTranslatef(r, (bbox[3]+bbox[2])*0.5, 0);
         DrawScales(kFALSE, font, -tms, dtw);
         glPopMatrix();
      }
   }

   glPopMatrix();
   font.PostRender();
}
