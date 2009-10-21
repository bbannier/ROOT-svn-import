// @(#)root/eve:$Id$
// Author: Alja Mrak-Tadel 2007

/*************************************************************************
 * Copyright (C) 1995-2007, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TEveCaloLegoGL
#define ROOT_TEveCaloLegoGL

#include "TGLObject.h"
#include "TGLAxisPainter.h"

#include "TEveCaloData.h"
#include "TEveVSDStructs.h"
#include "TEveCalo.h"

#include <map>

class TEveCaloLego;

class TEveCaloLegoGL : public TGLObject
{
private:
   struct Cell2D_t
   {
      Int_t   fId;
      Float_t fSumVal;
      Int_t   fMaxSlice;
      Float_t fGeom[4];

      Cell2D_t(Int_t id, Float_t sumVal, Int_t maxSlice)
      {
         fId       = id;
         fSumVal   = sumVal;
         fMaxSlice = maxSlice;
      }

      void SetGeom(Float_t etaMin, Float_t phiMin, Float_t etaMax, Float_t phiMax)
      {
         fGeom[0] = etaMin;
         fGeom[1] = phiMin;
         fGeom[2] = etaMax;
         fGeom[3] = phiMax;
      }

      Float_t MinSize() { return TMath::Min(fGeom[2] - fGeom[0], fGeom[3] - fGeom[1]); }
      Float_t X()       { return 0.5*(fGeom[2] + fGeom[0]); }
      Float_t Y()       { return 0.5*(fGeom[1] + fGeom[3]); }
   };

   typedef std::vector<Cell2D_t>             vCell2D_t;
   typedef std::vector<Cell2D_t>::iterator   vCell2D_i;

   typedef std::map<Int_t, UInt_t>           SliceDLMap_t;
   typedef std::map<Int_t, UInt_t>::iterator SliceDLMap_i;

   mutable Float_t   fDataMax;   // cached
   mutable Color_t   fGridColor; // cached
   mutable Color_t   fFontColor; // cached

   // axis
   mutable TAxis      *fEtaAxis;
   mutable TAxis      *fPhiAxis;
   mutable TAxis      *fZAxis;
   mutable TEveVector  fXAxisTitlePos;
   mutable TEveVector  fYAxisTitlePos;
   mutable TEveVector  fZAxisTitlePos;
   mutable TEveVector  fBackPlaneXConst[2];
   mutable TEveVector  fBackPlaneYConst[2];

   mutable TGLAxisPainter fAxisPainter;

   TEveCaloLego                     *fM;
   mutable Bool_t                    fDLCacheOK;

   // cached
   mutable TEveCaloData::RebinData_t fRebinData;
   mutable Float_t                   fMaxValRebin;
   mutable Float_t                   fValToPixel; // top logaritmic viewview
   
   mutable SliceDLMap_t              fDLMap;
   mutable SliceDLMap_t              fDLMapSelected;
   mutable Bool_t                    fCells3D;

   TEveCaloLegoGL(const TEveCaloLegoGL&);            // Stop default
   TEveCaloLegoGL& operator=(const TEveCaloLegoGL&); // Stop default

protected:
   Int_t   GetGridStep(TGLRnrCtx &rnrCtx) const;
   void    RebinAxis(TAxis *orig, TAxis *curr) const;

   void    SetAxis3DTitlePos(TGLRnrCtx &rnrCtx, Float_t x0, Float_t x1, Float_t y0, Float_t y1) const;
   void    DrawAxis3D(TGLRnrCtx &rnrCtx) const;
   void    DrawAxis2D(TGLRnrCtx &rnrCtx) const;
   void    DrawHistBase(TGLRnrCtx &rnrCtx) const;

   // top view
   void    PrepareCell2DData(TEveCaloData::vCellId_t& cellList, vCell2D_t& cells2D) const;
   void    PrepareCell2DDataRebin(TEveCaloData::RebinData_t& rebinData, vCell2D_t& cells2D) const;
   void    DrawCells2D(TGLRnrCtx & rnrCtx, vCell2D_t& cells2D) const;

   // 3D view
   void    DrawCells3D(TGLRnrCtx & rnrCtx) const;
   void    MakeQuad(Float_t x, Float_t y, Float_t z, Float_t xw, Float_t yw, Float_t zh) const;
   void    Make3DDisplayList(TEveCaloData::vCellId_t& cellList, SliceDLMap_t& map, Bool_t select) const;
   void    Make3DDisplayListRebin(TEveCaloData::RebinData_t& rebinData, SliceDLMap_t& map, Bool_t select) const;

   void    WrapTwoPi(Float_t &min, Float_t &max) const;

public:
   TEveCaloLegoGL();
   virtual ~TEveCaloLegoGL();

   virtual Bool_t SetModel(TObject* obj, const Option_t* opt = 0);

   virtual void   SetBBox();

   virtual void   DLCacheDrop();
   virtual void   DLCachePurge();

   virtual void   DirectDraw(TGLRnrCtx & rnrCtx) const;
   virtual void   DrawHighlight(TGLRnrCtx& rnrCtx, const TGLPhysicalShape* ps) const;

   virtual Bool_t SupportsSecondarySelect() const { return kTRUE; }
   virtual Bool_t AlwaysSecondarySelect()   const { return kTRUE; }
   virtual void   ProcessSelection(TGLRnrCtx & rnrCtx, TGLSelectRecord & rec);

   ClassDef(TEveCaloLegoGL, 0); // GL renderer class for TEveCaloLego.
};

//______________________________________________________________________________
inline void TEveCaloLegoGL::WrapTwoPi(Float_t &min, Float_t &max) const
{
   if (fM->GetData()->GetWrapTwoPi())
   {
      if (fM->GetPhiMax()>TMath::Pi() && max<=fM->GetPhiMin())
      {
         min += TMath::TwoPi();
         max += TMath::TwoPi();
      }
      else if (fM->GetPhiMin()<-TMath::Pi() && min>=fM->GetPhiMax())
      {
         min -= TMath::TwoPi();
         max -= TMath::TwoPi();
      }
   }
}
#endif
