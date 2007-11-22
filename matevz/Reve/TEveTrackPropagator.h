// @(#)root/reve:$Id$
// Authors: Matevz Tadel & Alja Mrak-Tadel: 2006, 2007

/*************************************************************************
 * Copyright (C) 1995-2007, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TEveTrackPropagator
#define ROOT_TEveTrackPropagator

#include <TEveVSDStructs.h>

#include <vector>

class TEvePointSet;
class TEveTrackRnrStyle;

class TEveTrackPropagator
{
protected:
   struct Vertex4D
   {
      Float_t x, y, z, t;

      Vertex4D() : x(0), y(0), z(0), t(0) {}
      Vertex4D(Float_t _x, Float_t _y, Float_t _z, Float_t _t=0) :
         x(_x), y(_y), z(_z), t(_t) {}

      Float_t Mag()  const { return TMath::Sqrt(x*x+y*y+z*z);}
      Float_t Mag2() const { return x*x+y*y+z*z;}

      Float_t Perp()  const { return TMath::Sqrt(x*x+y*y);}
      Float_t Perp2() const { return x*x+y*y;}
      Float_t R()     const { return Perp(); }

      Vertex4D operator + (const Vertex4D & b)
      { return Vertex4D(x + b.x, y + b.y, z + b.z, t + b.t); }

      Vertex4D operator - (const Vertex4D & b)
      { return Vertex4D(x - b.x, y - b.y, z - b.z, t - b.t); }

      Vertex4D operator * (Float_t a)
      { return Vertex4D(a*x, a*y, a*z, a*t); }

      Vertex4D& operator += (const Vertex4D & b)
      { x += b.x; y += b.y; z += b.z; t += b.t; return *this; }
   };

   struct Helix
   {
      Float_t fA;           // contains charge and magnetic field data
      Float_t fLam;         // momentum ratio pT/pZ
      Float_t fR;           // a/pT
      Float_t fPhiStep;     // step size in xy projection, dependent of RnrMode and momentum
      Float_t fTimeStep;    // time step
      Float_t fSin,  fCos;  // current sin, cos
      Float_t fXoff, fYoff; // offset for fitting daughters

      Helix() :
         fLam  (0), fR    (0), fPhiStep (0), fTimeStep (0) ,
         fSin  (0), fCos  (0),
         fXoff (0), fYoff (0)
      {}

      void Step(Vertex4D& v, TEveVector& p);
      void StepVertex(Vertex4D& v, TEveVector& p, Vertex4D& forw);
   };

private:
   TEveTrackPropagator(const TEveTrackPropagator&);            // Not implemented
   TEveTrackPropagator& operator=(const TEveTrackPropagator&); // Not implemented

protected:
   TEveTrackRnrStyle     *fRnrMod;
   Int_t                  fCharge;
   Float_t                fVelocity;
   std::vector<Vertex4D>  fPoints;
   Vertex4D               fV;

   Int_t   fN;           // current step number;
   Int_t   fNLast;       // last step
   Int_t   fNMax;        // max steps
   Helix   fH;           // helix

   void    InitHelix();
   Bool_t  HelixToVertex(TEveVector& v, TEveVector& p);
   void    HelixToBounds(TEveVector& p);
   Bool_t  LineToVertex (TEveVector& v);
   void    LineToBounds (TEveVector& p);

   void    SetNumOfSteps();

public:
   TEveTrackPropagator(TEveTrackRnrStyle* rs, Int_t charge,
                       TEveVector &v, TEveVector &p, Float_t beta);
   virtual ~TEveTrackPropagator() {}

   Int_t    GetSize() { return fPoints.size(); }

   void     GoToBounds(TEveVector& p);
   Bool_t   GoToVertex(TEveVector& v, TEveVector& p);

   void     FillPointSet(TEvePointSet* ps) const;

   ClassDef(TEveTrackPropagator, 0); // Calculates path of a particle taking into account special path-marks and imposed boundaries.
};

#endif
