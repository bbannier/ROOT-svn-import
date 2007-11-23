// @(#)root/reve:$Id$
// Authors: Matevz Tadel & Alja Mrak-Tadel: 2006, 2007

/*************************************************************************
 * Copyright (C) 1995-2007, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include <TEveTrackPropagator.h>
#include <TEveTrack.h>

#include <TMath.h>

#include <cassert>

//______________________________________________________________________________
void TEveTrackPropagator::Helix::Step(Vertex4D& v, TEveVector& p)
{
   v.x += (p.x*fSin - p.y*(1 - fCos))/fA + fXoff;
   v.y += (p.y*fSin + p.x*(1 - fCos))/fA + fYoff;
   v.z += fLam*TMath::Abs(fR*fPhiStep);
   v.t += fTimeStep;

   Float_t pxt = p.x*fCos  - p.y*fSin;
   Float_t pyt = p.y*fCos  + p.x*fSin;
   p.x = pxt;
   p.y = pyt;

}

//______________________________________________________________________________
void TEveTrackPropagator::Helix::StepVertex(Vertex4D& v, TEveVector& p, Vertex4D& forw)
{
   forw.x = v.x + (p.x*fSin - p.y*(1 - fCos))/fA + fXoff;
   forw.y = v.y + (p.y*fSin + p.x*(1 - fCos))/fA + fYoff;
   forw.z = v.z + fLam*TMath::Abs(fR*fPhiStep);
   forw.t = v.t + fTimeStep;
}


//______________________________________________________________________________
// TEveTrackPropagator
//
// Calculates path of a particle taking into account special
// path-marks and imposed boundaries.

ClassImp(TEveTrackPropagator)

//______________________________________________________________________________
TEveTrackPropagator::TEveTrackPropagator(TEveTrackRnrStyle* rs, Int_t charge,
                                         TEveVector& v, TEveVector& p, Float_t beta) :
   fRnrMod   (rs),
   fCharge   (charge),
   fVelocity (0.0f),
   fV        (v.x, v.y, v.z),
   fN        (0),
   fNLast    (-1),
   fNMax     (4096)
{
   fVelocity = TMath::C()*beta;

   fPoints.push_back(fV);

   if (fCharge)
   {
      // initialise helix
      using namespace TMath;
      Float_t pT = p.Perp();
      fH.fA      = fRnrMod->fgkB2C *fRnrMod->fMagField * charge;
      fH.fLam    = p.z/pT;
      fH.fR      = pT/fH.fA;

      fH.fPhiStep = fRnrMod->fMinAng * DegToRad();
      if (fRnrMod->fDelta < Abs(fH.fR))
      {
         Float_t ang  = 2*ACos(1 - fRnrMod->fDelta/Abs(fH.fR));
         if (ang < fH.fPhiStep) fH.fPhiStep = ang;
      }
      if (fH.fA < 0) fH.fPhiStep *= -1;
      //printf("PHI STEP %f \n", fH.fPhiStep);

      fH.fTimeStep = 0.01* Abs(fH.fR*fH.fPhiStep)*Sqrt(1+(fH.fLam*fH.fLam))/fVelocity;//cm->m
      fH.fSin = Sin(fH.fPhiStep);
      fH.fCos = Cos(fH.fPhiStep);
   }
}

//______________________________________________________________________________
Bool_t TEveTrackPropagator::GoToVertex(TEveVector& v, TEveVector& p)
{
   Bool_t hit;
   if (fCharge != 0 && TMath::Abs(fRnrMod->fMagField) > 1e-5 && p.Perp2() > 1e-12)
      hit = HelixToVertex(v, p);
   else
      hit = LineToVertex(v);
   return hit;
}

//______________________________________________________________________________
void TEveTrackPropagator::GoToBounds(TEveVector& p)
{
   if(fCharge != 0 && TMath::Abs(fRnrMod->fMagField) > 1e-5 && p.Perp2() > 1e-12)
      HelixToBounds(p);
   else
      LineToBounds(p);
}

//______________________________________________________________________________
void TEveTrackPropagator::SetNumOfSteps()
{
   using namespace TMath;
   // max orbits
   fNLast = Int_t(fRnrMod->fMaxOrbs*TwoPi()/Abs(fH.fPhiStep));
   // Z boundaries
   Float_t nz;
   if (fH.fLam > 0) {
      nz = ( fRnrMod->fMaxZ - fV.z)/( fH.fLam*Abs(fH.fR*fH.fPhiStep) );
   } else {
      nz = (-fRnrMod->fMaxZ - fV.z)/( fH.fLam*Abs(fH.fR*fH.fPhiStep) );
   }
   if (nz < fNLast) fNLast = Int_t(nz + 1);
   // printf("end steps in helix line %d \n", fNLast);
}


//______________________________________________________________________________
void TEveTrackPropagator::HelixToBounds(TEveVector& p)
{
   // printf("HelixToBounds\n");
   SetNumOfSteps();
   if (fNLast > 0)
   {
      Bool_t crosR = kFALSE;
      if (fV.Perp() < fRnrMod->fMaxR + TMath::Abs(fH.fR))
         crosR = true;

      Float_t maxR2 = fRnrMod->fMaxR * fRnrMod->fMaxR;
      Vertex4D forw;
      while (fN < fNLast)
      {
         fH.StepVertex(fV, p, forw);
         if (crosR && forw.Perp2() > maxR2)
         {
            Float_t t = (fRnrMod->fMaxR - fV.R()) / (forw.R() - fV.R());
            assert(t >= 0 && t <= 1);
            fPoints.push_back(fV + (forw-fV)*t);fN++;
            return;
         }
         if (TMath::Abs(forw.z) > fRnrMod->fMaxZ)
         {
            Float_t t = (fRnrMod->fMaxZ - TMath::Abs(fV.z)) / TMath::Abs((forw.z - fV.z));
            assert(t >= 0 && t <= 1);
            fPoints.push_back(fV + (forw-fV)*t);fN++;
            return;
         }
         fH.Step(fV, p); fPoints.push_back(fV); fN++;
      }
      return;
   }
}

//______________________________________________________________________________
Bool_t TEveTrackPropagator::HelixToVertex(TEveVector& v, TEveVector& p)
{
   Float_t p0x = p.x, p0y = p.y;
   Float_t zs = fH.fLam*TMath::Abs(fH.fR*fH.fPhiStep);
   Float_t maxrsq  = fRnrMod->fMaxR * fRnrMod->fMaxR;
   Float_t fnsteps = (v.z - fV.z)/zs;
   Int_t   nsteps  = Int_t((v.z - fV.z)/zs);
   Float_t sinf = TMath::Sin(fnsteps*fH.fPhiStep); // final sin
   Float_t cosf = TMath::Cos(fnsteps*fH.fPhiStep); // final cos

   // check max orbits
   nsteps = TMath::Min(nsteps, fNLast -fN);
   {
      if (nsteps > 0)
      {
         // check offset and distribute it over all steps
         Float_t xf  = fV.x + (p.x*sinf - p.y*(1 - cosf))/fH.fA;
         Float_t yf =  fV.y + (p.y*sinf + p.x*(1 - cosf))/fH.fA;
         fH.fXoff =  (v.x - xf)/fnsteps;
         fH.fYoff =  (v.y - yf)/fnsteps;
         Vertex4D forw;
         for (Int_t l=0; l<nsteps; l++)
         {
            fH.StepVertex(fV, p, forw);
            if (fV.Perp2() > maxrsq || TMath::Abs(fV.z) > fRnrMod->fMaxZ)
               return kFALSE;
            fH.Step(fV, p); fPoints.push_back(fV); fN++;
         }
      }
      // set time to the end point
      fV.t += TMath::Sqrt((fV.x-v.x)*(fV.x-v.x)+(fV.y-v.y)*(fV.y-v.y) +(fV.z-v.z)*(fV.z-v.z))/fVelocity;
      fV.x = v.x; fV.y = v.y; fV.z = v.z;
      fPoints.push_back(fV); fN++;
   }
   { // rotate momentum for residuum
      Float_t cosr = TMath::Cos((fnsteps-nsteps)*fH.fPhiStep);
      Float_t sinr = TMath::Sin((fnsteps-nsteps)*fH.fPhiStep);
      Float_t pxt = p.x*cosr - p.y*sinr;
      Float_t pyt = p.y*cosr + p.x*sinr;
      p.x = pxt;
      p.y = pyt;
   }
   { // calculate size of faked p.x,py
      Float_t pxf = (p0x*cosf - p0y*sinf)/TMath::Abs(fH.fA) + fH.fXoff/fH.fPhiStep;
      Float_t pyf = (p0y*cosf + p0x*sinf)/TMath::Abs(fH.fA) + fH.fYoff/fH.fPhiStep;
      Float_t fac = TMath::Sqrt((p0x*p0x + p0y*p0y) / (pxf*pxf + pyf*pyf));
      p.x = fac*pxf;
      p.y = fac*pyf;
   }
   return kTRUE;
}

//______________________________________________________________________________
Bool_t TEveTrackPropagator::LineToVertex(TEveVector& v)
{
   fV.t += TMath::Sqrt((fV.x-v.x)*(fV.x-v.x)+(fV.y-v.y)*(fV.y-v.y)+(fV.z-v.z)*(fV.z-v.z))/fVelocity;
   fV.x = v.x;
   fV.y = v.y;
   fV.z = v.z;
   fPoints.push_back(fV);

   return kTRUE;
}

//______________________________________________________________________________
void TEveTrackPropagator::LineToBounds(TEveVector& p)
{
   Float_t tZ = 0, Tb = 0;
   // time where particle intersect +/- fMaxZ
   if (p.z > 0) {
      tZ = (fRnrMod->fMaxZ - fV.z)/p.z;
   }
   else  if (p.z < 0 ) {
      tZ = (-1)*(fRnrMod->fMaxZ + fV.z)/p.z;
   }
   // time where particle intersects cylinder
   Float_t tR = 0;
   Double_t a = p.x*p.x + p.y*p.y;
   Double_t b = 2*(fV.x*p.x + fV.y*p.y);
   Double_t c = fV.x*fV.x + fV.y*fV.y - fRnrMod->fMaxR*fRnrMod->fMaxR;
   Double_t D = b*b - 4*a*c;
   if (D >= 0) {
      Double_t D_sqrt=TMath::Sqrt(D);
      tR = ( -b - D_sqrt )/(2*a);
      if (tR < 0) {
         tR = ( -b + D_sqrt )/(2*a);
      }
      Tb = tR < tZ ? tR : tZ; // compare the two times
   } else {
      Tb = tZ;
   }
   TEveVector nv(fV.x + p.x*Tb, fV.y + p.y*Tb, fV.z+ p.z*Tb);
   LineToVertex(nv);
}

//______________________________________________________________________________
void TEveTrackPropagator::FillPointSet(TEvePointSet* ps) const
{
   Int_t size = TMath::Min(fNMax, (Int_t) fPoints.size());
   ps->Reset(size);
   for (Int_t i=0; i<size; ++i)
   {
      const Vertex4D& v = fPoints[i];
      ps->SetNextPoint(v.x, v.y, v.z);
   }
}
