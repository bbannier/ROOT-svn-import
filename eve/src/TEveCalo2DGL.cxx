// @(#)root/eve:$Id$
// Author: Matevz Tadel 2007

/*************************************************************************
 * Copyright (C) 1995-2007, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include "TEveCalo2DGL.h"
#include "TEveCalo.h"
#include "TEveProjections.h"
#include "TEveProjectionManager.h"
#include "TEveRGBAPalette.h"

#include "TGLRnrCtx.h"
#include "TGLIncludes.h"
#include "TGLUtil.h"
#include "TAxis.h"

using namespace TMath;

//______________________________________________________________________________
// OpenGL renderer class for TEveCalo2D.
//

ClassImp(TEveCalo2DGL);

//______________________________________________________________________________
TEveCalo2DGL::TEveCalo2DGL() :
   TGLObject(),
   fM(0),
   fProjection(0)
{
   // Constructor.

   // fDLCache = kFALSE; // Disable display list.
}

/******************************************************************************/

//______________________________________________________________________________
Bool_t TEveCalo2DGL::SetModel(TObject* obj, const Option_t* /*opt*/)
{
   // Set model object.

   if (SetModelCheckClass(obj, TEveCalo2D::Class())) {
      fM = dynamic_cast<TEveCalo2D*>(obj);
      fProjection = fM->fManager->GetProjection();
      return kTRUE;
   }
   return kFALSE;
}

//______________________________________________________________________________
void TEveCalo2DGL::SetBBox()
{
   // Set bounding box.

   // !! This ok if master sub-classed from TAttBBox
   SetAxisAlignedBBox(((TEveCalo2D*)fExternalObj)->AssertBBox());
}

/******************************************************************************/



//______________________________________________________________________________
Float_t TEveCalo2DGL::MakeRPhiCell(Float_t phiMin, Float_t phiMax, Float_t towerH, Float_t offset) const
{
   // Float_t towerH =  fM->fBarrelRadius*fM->fTowerHeight*(value -fMinVal)/(fMaxVal-fMinVal);
   Float_t r1 = fM->fBarrelRadius + offset;
   Float_t r2 = r1 + towerH;

   Float_t pnts[8];
   Float_t *p = pnts;

   p[0] = r1*Cos(phiMin);
   p[1] = r1*Sin(phiMin);
   p +=2;
   p[0] = r2*Cos(phiMin);
   p[1] = r2*Sin(phiMin);
   p +=2;
   p[0] = r2*Cos(phiMax);
   p[1] = r2*Sin(phiMax);
   p +=2;
   p[0] = r1*Cos(phiMax);
   p[1] = r1*Sin(phiMax);


   Float_t x, y, z;
   for (Int_t i=0; i<4; i++){
      x = pnts[2*i];
      y = pnts[2*i+1];
      z = 0.f;
      fProjection->ProjectPoint(x, y, z);
      glVertex3f(x, y, fM->fDepth);
   }
   return offset+towerH;
}

//______________________________________________________________________________
void TEveCalo2DGL::DrawRPhi() const
{
   TEveCaloData* data = fM->GetData();

   TEveCaloData::vCellId_t  cids;
   TEveCaloData::CellData_t cellData;
   Float_t eta = (fM->fEtaMax+fM->fEtaMin)*0.5f;
   Float_t etaRng = fM->fEtaMax-fM->fEtaMin;
   Float_t pr[4];
   // calculate the two intervals when circle is cut
   Float_t phi1 = fM->fPhi - fM->fPhiRng;
   Float_t phi2 = fM->fPhi + fM->fPhiRng;
   if (phi2 >TMath::Pi() && phi1<-Pi()) {
      pr[0] =  phi1;
      pr[1] =  Pi();
      pr[2] =  -Pi();
      pr[3] =  -TwoPi()+phi2;
   }
   else if (phi1<-TMath::Pi() && phi2<=Pi()) {
      pr[0] = -Pi();
      pr[1] =  phi2;
      pr[2] =  TwoPi()+phi1;
      pr[3] =  Pi();
   } else {
      pr[0] = pr[2] = phi1;
      pr[1] = pr[3] = phi2;
   }

   Int_t nSlices = data->GetNSlices();
   Float_t *sliceVal = new Float_t[nSlices];

   if (data->SupportsPhiBinning()) {
      const TAxis* ax = data->GetPhiBins();
      Int_t nBins = ax->GetNbins();
      Float_t ht;
      for (Int_t ibin=0; ibin<nBins; ibin++) {
         if ( (   ax->GetBinLowEdge(ibin)>=pr[0] && ax->GetBinUpEdge(ibin)<pr[1])
              || (ax->GetBinLowEdge(ibin)>=pr[2] && ax->GetBinUpEdge(ibin)<pr[3])) {

            if (data->GetCellList(eta, etaRng, ax->GetBinCenter(ibin), ax->GetBinWidth(ibin), fM->fThreshold, cids)) {

               for(Int_t i=0 ; i<nSlices; i++)
                  sliceVal[i] = 0.f;

               for (TEveCaloData::vCellId_i it = cids.begin(); it != cids.end(); it++) {
                  data->GetCellData(*it, cellData);
                  sliceVal[(*it).fSlice]  += cellData.Value();
               }
               Float_t off = 0;
               for (Int_t s=0; s<nSlices; s++) {
                  if (fM->SetupColorHeight(sliceVal[s], s, ht))
                     off = MakeRPhiCell(ax->GetBinLowEdge(ibin), ax->GetBinUpEdge(ibin), ht, off);
               }
               cids.clear();
            }
         }
      }
   }
   delete [] sliceVal;
}
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

//______________________________________________________________________________
Float_t TEveCalo2DGL::MakeRhoZBarrelCell(Float_t thetaMin, Float_t thetaMax, Int_t phiSign, Float_t towerH, Float_t offset) const
{
   //   Float_t towerH = fM->fBarrelRadius*fM->fTowerHeight*(value -fMinVal)/(fMaxVal-fMinVal);
   Float_t theta  = (thetaMin+thetaMax)*0.5;
   Float_t r1 = fM->fBarrelRadius/Sin(theta) + offset;
   Float_t r2 = r1 + towerH;

   Float_t pnts[12];
   Float_t *p = pnts;

   p[0] = 0.f;
   p[1] = r1*Sin(thetaMin);
   p[2] = r1*Cos(thetaMin);
   p +=3;

   p[0] = 0.f;
   p[1] = r2*Sin(thetaMin);
   p[2] = r2*Cos(thetaMin);
   p +=3;

   p[0] = 0.f;
   p[1] = r2*Sin(thetaMax);
   p[2] = r2*Cos(thetaMax);

   p +=3;
   p[0] = 0.f;
   p[1] = r1*Sin(thetaMax);
   p[2] = r1*Cos(thetaMax);

   Float_t x, y, z;
   for (Int_t i=0; i<4; i++) {
      x = pnts[3*i];
      y = pnts[3*i+1]*phiSign;
      z = pnts[3*i+2];
      fProjection->ProjectPoint(x, y, z);
      //printf("MakeRhoZBarrelCell (%f, %f, %f)-> (%f, %f, %f)\n", pnts[3*i], pnts[3*i+1]*phiSign,  pnts[3*i+2], x, y, z);
      glVertex3f(x, y, fM->fDepth);
   }
   return offset+towerH;
}

//______________________________________________________________________________
Float_t TEveCalo2DGL::MakeRhoZEndCapCell(Float_t thetaMin, Float_t thetaMax, Int_t phiSign, Float_t towerH, Float_t offset) const
{
   //   Float_t towerH =  fM->fEndCapPos*fM->fTowerHeight*(value - fMinVal)/(fMaxVal-fMinVal);
   Float_t theta  = (thetaMin+thetaMax)*0.5;
   Float_t r1 =Abs( fM->GetEndCapPos()/Cos(theta)) + offset;
   Float_t r2 = r1 + towerH;
   Float_t pnts[12];
   Float_t *p = pnts;

   p[0] = 0.f;
   p[1] = r1*Sin(thetaMin);
   p[2] = r1*Cos(thetaMin);
   p +=3;

   p[0] = 0.f;
   p[1] = r2*Sin(thetaMin);
   p[2] = r2*Cos(thetaMin);
   p +=3;

   p[0] = 0.f;
   p[1] = r2*Sin(thetaMax);
   p[2] = r2*Cos(thetaMax);

   p +=3;
   p[0] = 0.f;
   p[1] = r1*Sin(thetaMax);
   p[2] = r1*Cos(thetaMax);

   Float_t x, y, z;
   for (Int_t i=0; i<4; i++) {
      x = pnts[3*i];
      y = pnts[3*i+1]*phiSign;
      z = pnts[3*i+2];
      fProjection->ProjectPoint(x, y, z);
      glVertex3f(x, y, fM->fDepth);
   }
   return offset+towerH;
}

//______________________________________________________________________________
void TEveCalo2DGL::DrawRhoZ() const
{
   TEveCaloData* data = fM->GetData();
   Int_t nSlices = data->GetNSlices();
   Float_t *sliceValsUp  = new Float_t[nSlices];
   Float_t *sliceValsLow = new Float_t[nSlices];

   Float_t transTheta = fM->GetTransitionTheta();
   TEveCaloData::vCellId_t  cids;
   TEveCaloData::CellData_t cellData;

   if (data->SupportsEtaBinning()) {
      const TAxis* ax = data->GetEtaBins();
      Int_t nBins = ax->GetNbins();
      for (Int_t ibin=0; ibin<nBins; ibin++) {
         if (ax->GetBinLowEdge(ibin)<=fM->fEtaMin || ax->GetBinUpEdge(ibin)>fM->fEtaMax) continue;
         Float_t eta = ax->GetBinCenter(ibin);
         Float_t etaW = ax->GetBinWidth(ibin);

         if (data->GetCellList(eta, etaW, fM->fPhi, fM->fPhiRng, fM->fThreshold, cids)) {
            for (TEveCaloData::vCellId_i it = cids.begin(); it != cids.end(); it++) {
               data->GetCellData(*it, cellData);
               if (cellData.PhiMin() >= 0)
                  sliceValsUp[(*it).fSlice]  += cellData.Value();
               else
                  sliceValsLow[(*it).fSlice] += cellData.Value();
            }
            // pick first in the array as geo model
            Float_t offUp  = 0;
            Float_t offLow = 0;
            data->GetCellData(cids.front(), cellData);
            Float_t ht;
            for (Int_t s=0; s<nSlices; s++) {
               if (cellData.Theta()>transTheta) {
                  if (fM->SetupColorHeight(sliceValsUp[s], s, ht))
                     offUp  = MakeRhoZBarrelCell(cellData.ThetaMin(kTRUE), cellData.ThetaMax(kTRUE),  1, ht,  offUp);
                  if (fM->SetupColorHeight(sliceValsLow[s], s, ht))
                     offLow = MakeRhoZBarrelCell(cellData.ThetaMin(kTRUE), cellData.ThetaMax(kTRUE), -1, ht,  offLow);
               }
               else {
                  if (fM->SetupColorHeight(sliceValsUp[s], s, ht))
                     offUp  = MakeRhoZEndCapCell(cellData.ThetaMin(kTRUE), cellData.ThetaMax(kTRUE),  1, ht,  offUp);
                  if (fM->SetupColorHeight(sliceValsLow[s], s, ht))
                     offLow = MakeRhoZEndCapCell(cellData.ThetaMin(kTRUE), cellData.ThetaMax(kTRUE), -1, ht, offLow);
               }
            }
            // clear
            cids.clear();
            for (Int_t s=0; s<nSlices; s++) {
               sliceValsUp[s]  = 0;
               sliceValsLow[s] = 0;
            }
         }
      }
      delete [] sliceValsUp;
      delete [] sliceValsLow;
   }
}

//______________________________________________________________________________
void TEveCalo2DGL::DirectDraw(TGLRnrCtx & /*rnrCtx*/) const
{
   // Render with OpenGL.
   //  printf("TEveCalo2DGL::DirectDraw \n");

   glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT | GL_POINT_BIT | GL_POLYGON_BIT);
   glDisable(GL_LIGHTING);
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   glDisable(GL_CULL_FACE);
   glPointSize(3);
   //glBegin(GL_POINTS);
   glBegin(GL_QUADS);

   fM->AssertPalette();
   if (fProjection->GetType() == TEveProjection::kPT_RhoZ) {
      DrawRhoZ();
   }
   else if (fProjection->GetType() == TEveProjection::kPT_RPhi) {
      DrawRPhi();
   }

   glEnd();
   glPopAttrib();
}


