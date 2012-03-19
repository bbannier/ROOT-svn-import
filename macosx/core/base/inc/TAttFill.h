// @(#)root/base:$Id$
// Author: Rene Brun   12/12/94

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TAttFill
#define ROOT_TAttFill


//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TAttFill                                                             //
//                                                                      //
// Fill area attributes.                                                //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_Rtypes
#include "Rtypes.h"
#endif
#ifndef ROOT_Riosfwd
#include "Riosfwd.h"
#endif


class TAttFill {
public:
   enum EFillGradient {
      kNoGradientFill,//0
      kGradientVertical,//1
      kGradientHorizontal//2
   };

protected:
   Color_t    fFillColor;           //fill area color
   Style_t    fFillStyle;           //fill area style

   //fExtension == gradient * 1000 + alpha (0-100).
   UInt_t     fExtension;           //transparency + gradient fill
   
public:

   TAttFill();
   TAttFill(Color_t fcolor,Style_t fstyle);
   virtual ~TAttFill();
   void             Copy(TAttFill &attfill) const;
   virtual Color_t  GetFillColor() const { return fFillColor; }
   virtual Style_t  GetFillStyle() const { return fFillStyle; }
   virtual Bool_t   IsTransparent() const;
   virtual void     Modify();
   virtual void     ResetAttFill(Option_t *option="");
   virtual void     SaveFillAttributes(ostream &out, const char *name, Int_t coldef=1, Int_t stydef=1001);
   virtual void     SetFillAttributes(); // *MENU*
   virtual void     SetFillColor(Color_t fcolor) { fFillColor = fcolor; }
   virtual void     SetFillStyle(Style_t fstyle) { fFillStyle = fstyle; }
   
   virtual void     SetFillAlpha(UInt_t alpha);
   virtual UInt_t   GetFillAlpha()const;
   virtual void     SetFillGradient(EFillGradient direction);
   virtual EFillGradient GetFillGradient()const;
   virtual void     SetExtendedFill(UInt_t ext);
   virtual UInt_t   GetExtendedFill()const;

   ClassDef(TAttFill,2)  //Fill area attributes
};

inline Bool_t TAttFill::IsTransparent() const
{ return fFillStyle >= 4000 && fFillStyle <= 4100 ? kTRUE : kFALSE; }

#endif

