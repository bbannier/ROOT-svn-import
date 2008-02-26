// @(#)root/eve:$Id$
// Author: Matevz Tadel 2007

/*************************************************************************
 * Copyright (C) 1995-2007, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TEveCalo
#define ROOT_TEveCalo

#include "TEveElement.h"
#include "TEveProjectionBases.h"
#include "TEveProjectionManager.h"

#include "TAtt3D.h"
#include "TAttBBox.h"

class TClass;
class TEveCaloData;
class TEveRGBAPalette;

class TEveCaloViz : public TEveElement,
                    public TNamed,
                    public TAtt3D,
                    public TAttBBox,
                    public TEveProjectable
{
   friend class TEveCaloVizEditor;

private:
   TEveCaloViz(const TEveCaloViz&);        // Not implemented
   TEveCaloViz& operator=(const TEveCaloViz&); // Not implemented

protected:
   TEveCaloData* fData;

   Float_t      fEtaMin;
   Float_t      fEtaMax;
   Float_t      fPhi;
   Float_t      fPhiRng;

   Float_t      fThreshold;

   Float_t      fBarrelRadius;
   Float_t      fEndCapPos;

   Float_t      fTowerHeight;;

   Int_t             fDefaultValue;   // Default signal value.
   Bool_t            fValueIsColor;   // Interpret signal value as RGBA color.
   TEveRGBAPalette*  fPalette;        // Pointer to signal-color palette.

public:
   TEveCaloViz(const Text_t* n="TEveCaloViz", const Text_t* t="");
   TEveCaloViz(TEveCaloData* data, const Text_t* n="TEveCaloViz", const Text_t* t="");

   virtual ~TEveCaloViz();

   TEveCaloData* GetData() const { return fData; }
   Float_t GetBarrelRadius() const { return fBarrelRadius; }
   Float_t GetEndCapPos   () const { return fEndCapPos; }
   Float_t GetTransitionEta() const;
   Float_t GetTransitionTheta() const;

   void SetEta(Float_t l, Float_t u){ fEtaMin=l; fEtaMax =u;}
   void SetPhi(Float_t x){ fPhi= x; }
   void SetPhiRng(Float_t r){ fPhiRng = r;}

   void SetBarrelRadius(Float_t r) { fBarrelRadius = r; }
   void SetEndCapPos   (Float_t z) { fEndCapPos = z; }
   void SetTowerHeight (Float_t h) { fTowerHeight = h;}

   void AssignCaloVizParameters(TEveCaloViz* cv);

   TEveRGBAPalette* GetPalette() const { return fPalette; }
   void             SetPalette(TEveRGBAPalette* p);
   TEveRGBAPalette* AssertPalette();

   virtual void Paint(Option_t* option="");

   virtual void ComputeBBox();
   virtual TClass* ProjectedClass() const;

   ClassDef(TEveCaloViz, 0); // Calorimeter class.
};

/**************************************************************************/
/**************************************************************************/

class TEveCalo3D : public TEveCaloViz
{
   friend class TEveCalo3DGL;
private:
   TEveCalo3D(const TEveCalo3D&);            // Not implemented
   TEveCalo3D& operator=(const TEveCalo3D&); // Not implemented

public:
   TEveCalo3D(const Text_t* n="TEveCalo3D", const Text_t* t="");
   TEveCalo3D(TEveCaloData* data, const Text_t* n="TEveCalo3D", const Text_t* t="");
   virtual ~TEveCalo3D() {}


   ClassDef(TEveCalo3D, 0); // 3D calorimeter class.
};

/**************************************************************************/
 /**************************************************************************/

class TEveCalo2D : public TEveCaloViz,
                   public TEveProjected
{
   friend class TEveCalo2DGL;
private:
   TEveCalo2D(const TEveCalo2D&);            // Not implemented
   TEveCalo2D& operator=(const TEveCalo2D&); // Not implemented

public:
   TEveCalo2D(const Text_t* n="TEveCalo2D", const Text_t* t="");
   virtual ~TEveCalo2D(){}

   virtual void SetProjection(TEveProjectionManager* proj, TEveProjectable* model);
   virtual void UpdateProjection();
   virtual void SetDepth(Float_t x){fDepth = x;}

   virtual void ComputeBBox();

   ClassDef(TEveCalo2D, 0); // 2D projected calorimeter class.
};
#endif
