// @(#)root/eve:$Id$
// Author: Matevz Tadel 2007

/*************************************************************************
 * Copyright (C) 1995-2007, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include "TEveCalo.h"
#include "TEveCaloData.h"
#include "TEveProjections.h"
#include "TEveProjectionManager.h"
#include "TEveRGBAPalette.h"

#include "TClass.h"
#include "TMathBase.h"
#include "TMath.h"
#include "TBuffer3D.h"
#include "TBuffer3DTypes.h"
#include "TVirtualPad.h"
#include "TVirtualViewer3D.h"

#include "TGLUtil.h"

//______________________________________________________________________________
// Description of calorimeter.
//

ClassImp(TEveCaloViz);

//______________________________________________________________________________
TEveCaloViz::TEveCaloViz(const Text_t* n, const Text_t* t) :
   TEveElement(),
   TNamed(n, t),
   TEveProjectable(),

   fData(0),

   fEtaMin(-5.),
   fEtaMax(5.),
   fPhi(0.),
   fPhiRng(TMath::Pi()),

   fThreshold(0.001f),

   fBarrelRadius(-1.f),
   fEndCapPos(-1.f),

   fTowerHeight(0.2),

   fDefaultValue(5),
   fValueIsColor(kTRUE),
   fPalette(0)
{
   // Constructor.

   SetElementName("TEveCaloViz");
}

//______________________________________________________________________________
TEveCaloViz::TEveCaloViz(TEveCaloData* data, const Text_t* n, const Text_t* t) :
   TEveElement(),
   TNamed(n, t),

   fData(data),

   fEtaMin(-5.),
   fEtaMax(5.),
   fPhi(0.),
   fPhiRng(TMath::Pi()),

   fThreshold(0.001f),

   fBarrelRadius(-1.f),
   fEndCapPos(-1.f),

   fTowerHeight(0.2),

   fDefaultValue(5),
   fValueIsColor(kTRUE),
   fPalette(0)
{
   // Constructor.

}

//______________________________________________________________________________
TEveCaloViz::~TEveCaloViz()
{
   // Destructor.

   SetPalette(0);
}

//______________________________________________________________________________
Float_t TEveCaloViz::GetTransitionTheta() const
{
  return TMath::ATan(fBarrelRadius/fEndCapPos);
}

//______________________________________________________________________________
Float_t TEveCaloViz::GetTransitionEta() const
{
   using namespace TMath;
   Float_t t = GetTransitionTheta()*0.5f;
   return -Log(Tan(t));
}

//______________________________________________________________________________
void TEveCaloViz::AssignCaloVizParameters(TEveCaloViz* m)
{
   fData = m->fData;

   fEtaMin    = m->fEtaMin;
   fEtaMax    = m->fEtaMax;
   fPhi       = m->fPhi;
   fPhiRng    = m->fPhiRng;
   fThreshold = m->fThreshold;

   fBarrelRadius = m->fBarrelRadius;
   fEndCapPos    = m->fEndCapPos;
   // fTowerHeight  = m->fTowerHeight;

   TEveRGBAPalette& mp = * m->fPalette;
   TEveRGBAPalette* p = new TEveRGBAPalette(mp.GetMinVal(), mp.GetMaxVal(),
                                          mp.GetInterpolate());
   p->SetDefaultColor(mp.GetDefaultColor());
   SetPalette(p);
}


//______________________________________________________________________________
void TEveCaloViz::SetPalette(TEveRGBAPalette* p)
{
   // Set TEveRGBAPalette pointer.

   if ( fPalette == p) return;
   if (fPalette) fPalette->DecRefCount();
   fPalette = p;
   if (fPalette) fPalette->IncRefCount();
}

//______________________________________________________________________________
TEveRGBAPalette* TEveCaloViz::AssertPalette()
{
   // Make sure the TEveRGBAPalette pointer is not null.
   // If it is not set, a new one is instantiated and the range is set
   // to current min/max signal values.

   if (fPalette == 0) {
      fPalette = new TEveRGBAPalette;
   }
   return fPalette;
}
//______________________________________________________________________________
void TEveCaloViz::Paint(Option_t* /*option*/)
{
   // Paint this object. Only direct rendering is supported.

   static const TEveException eH("TEvecaloViz::Paint ");

   TBuffer3D buff(TBuffer3DTypes::kGeneric);

   // Section kCore
   buff.fID           = this;
   buff.fTransparency = 0;
   buff.SetSectionsValid(TBuffer3D::kCore);

   Int_t reqSections = gPad->GetViewer3D()->AddObject(buff);
   if (reqSections != TBuffer3D::kNone)
      Error(eH, "only direct GL rendering supported.");
}

//______________________________________________________________________________
TClass* TEveCaloViz::ProjectedClass() const
{
   // Virtual from TEveProjectable, returns TEveCalo2D class.

   return TEveCalo2D::Class();
}

//______________________________________________________________________________
void TEveCaloViz::SetTowerHeight(Float_t x)
{

   fTowerHeight = x;
   ComputeBBox();
}

//______________________________________________________________________________
void TEveCaloViz::ComputeBBox()
{
   // Fill bounding-box information of the base-class TAttBBox (virtual method).
   // If member 'TEveFrameBox* fFrame' is set, frame's corners are used as bbox.

   BBoxInit();

   Float_t th = fBarrelRadius*fTowerHeight*fData->GetNSlices();
   
   fBBox[0] = -fBarrelRadius - th;
   fBBox[1] =  fBarrelRadius + th;
   fBBox[2] =  fBBox[0];
   fBBox[3] =  fBBox[1];
   fBBox[4] = -fEndCapPos - th;
   fBBox[5] =  fEndCapPos + th;

   // printf("EveCaloVIZ ComputeBBox (%f, %f, %f) (%f, %f, %f)\n",
   //        fBBox[0],  fBBox[1],  fBBox[2], fBBox[3], fBBox[4], fBBox[5]);
}


//______________________________________________________________________________
inline Bool_t TEveCaloViz::SetupColorHeight(Float_t value, Int_t slice, Float_t &out) const
{
   Int_t val =  (Int_t)value;
   out = fBarrelRadius*fTowerHeight;

   if(fPalette->GetShowDefValue())
   {
      if( value >=fPalette->GetMinVal() && value < fPalette->GetMaxVal())
      {
         TGLUtil::Color(fPalette->GetDefaultColor()+slice);
         out *= ((value -fPalette->GetMinVal())
                 /(fPalette->GetMaxVal() -fPalette->GetMinVal()));
         return kTRUE;
      }
   }
   else if (fPalette->WithinVisibleRange(val)) 
   {
      UChar_t c[4]; //c[4] = 255;
      fPalette->ColorFromValue(val, c);
      TGLUtil::Color4ubv(c);
      return kTRUE;
   }

   return kFALSE;
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

ClassImp(TEveCalo3D);

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
//______________________________________________________________________________
TEveCalo3D::TEveCalo3D(const Text_t* n, const Text_t* t):
   TEveCaloViz(n, t)
{
}

//______________________________________________________________________________
TEveCalo3D::TEveCalo3D(TEveCaloData* data, const Text_t* n, const Text_t* t):
   TEveCaloViz(data, n, t)
{
}


ClassImp(TEveCalo2D);

//______________________________________________________________________________
TEveCalo2D::TEveCalo2D(const Text_t* n, const Text_t* t):
   TEveCaloViz(n, t),
   TEveProjected()
{
}

//______________________________________________________________________________
void TEveCalo2D::UpdateProjection()
{
   ComputeBBox();
   ElementChanged(kTRUE, kTRUE);
}

//______________________________________________________________________________
void TEveCalo2D::SetProjection(TEveProjectionManager* mng, TEveProjectable* model)
{
   // Set projection manager and model object.

   TEveProjected::SetProjection(mng, model);
   TEveCaloViz* viz = dynamic_cast<TEveCaloViz*>(model);
   AssignCaloVizParameters(viz);
}

//______________________________________________________________________________
void TEveCalo2D::ComputeBBox()
{
   // Fill bounding-box information of the base-class TAttBBox (virtual method).
   // If member 'TEveFrameBox* fFrame' is set, frame's corners are used as bbox.

   BBoxZero();
   TEveProjection& proj = *fManager->GetProjection();

   Float_t th = fTowerHeight*fData->GetNSlices()*fBarrelRadius; 

   Float_t x1, y1, z1, x2, y2, z2;
   x1 = y1 = -fBarrelRadius - th;
   x2 = y2  = fBarrelRadius + th;
   z1 = -fEndCapPos - th;
   z2 =  fEndCapPos + th;

   proj.ProjectPoint(x1, y1, z1);
   proj.ProjectPoint(x2, y2, z2);

   fBBox[0] = x1;
   fBBox[1] = x2;
   fBBox[2] = y1;
   fBBox[3] = y2;
   fBBox[4] = fDepth;
   fBBox[5] = fDepth;

   AssertBBoxExtents(0.1);
   // printf("EveCalo2D ComputeBBox (%f, %f, %f) (%f, %f, %f)\n",
   //      fBBox[0],  fBBox[2],  fBBox[4], fBBox[1], fBBox[3], fBBox[5]);
}
