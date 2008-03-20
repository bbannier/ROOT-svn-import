// @(#)root/eve:$Id$
// Author: Matevz Tadel 2007

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
#include "TEveCaloData.h"
#include "TGLIncludes.h"
#include "TGLFontManager.h"
// #include <list>

class TEveCaloLego;

class TEveCaloLegoGL : public TGLObject
{
private:
   TEveCaloLegoGL(const TEveCaloLegoGL&);            // Not implemented
   TEveCaloLegoGL& operator=(const TEveCaloLegoGL&); // Not implemented

protected:
   struct Box_t
   {
     Float_t pnts[24];
     UChar_t color[4];
   };

   void    RnrText(const char* txt, Float_t x, Float_t y, const GLdouble *pm, Bool_t isNum) const;

   Float_t RenderCell(const TEveCaloData::CellData_t &cell, Float_t towerH, Float_t offset) const;

   void    DrawAxis(TGLRnrCtx &rnrCtx, Float_t x0, Float_t x1, Float_t y0, Float_t y1) const;
   void    DrawTitle(TGLRnrCtx &rnrCtx) const;

   Int_t   GetGridStep(Int_t axId, const TAxis* ax, TGLRnrCtx &rnrCtx) const;

   TEveCaloLego            *fM;  // Model object.

   //   std::list<Box_t>         fBoxList;

   mutable TGLFont          fNumFont;
   mutable TGLFont          fSymbolFont;
   mutable TGLFont          fTitleFont;

   Float_t                  fMinBinWidth;
   Int_t                    fNBinSteps;
   Int_t*                   fBinSteps;

public:
   TEveCaloLegoGL();
   virtual ~TEveCaloLegoGL();

   virtual Bool_t SetModel(TObject* obj, const Option_t* opt=0);
   virtual void   SetBBox();

   virtual void DirectDraw(TGLRnrCtx & rnrCtx) const;

   // To support two-level selection
   virtual Bool_t SupportsSecondarySelect() const { return kTRUE; }
   virtual void ProcessSelection(TGLRnrCtx & rnrCtx, TGLSelectRecord & rec);

   ClassDef(TEveCaloLegoGL, 0); // GL renderer class for TEveCaloLego.
};

#endif
