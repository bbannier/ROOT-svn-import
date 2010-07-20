// @(#)root/eve:$Id$
// Author: Matevz Tadel 2007

/*************************************************************************
 * Copyright (C) 1995-2007, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TEvePadFrame
#define ROOT_TEvePadFrame

#include "TEveElement.h"
#include "TAttBBox.h"

class TPad;

class TEvePadFrame : public TEveElementList,
                     public TAttBBox
{
   friend class TEvePadFrameGL;

private:
   TEvePadFrame(const TEvePadFrame&);            // Not implemented
   TEvePadFrame& operator=(const TEvePadFrame&); // Not implemented

protected:
   TPad    *fPad;
   Double_t fSizeX;
   Bool_t   fUseFBO;
   Int_t    fSizeFBO;  // Width of FBO buffer. If 0, pad pixel width is used.

   Int_t    fRTS;      //! Rendering TimeStamp
   void     IncRTS() { ++fRTS; }

public:
   TEvePadFrame(const char* n="TEvePadFrame", const char* t="");
   TEvePadFrame(TPad* pad, const char* n="TEvePadFrame", const char* t="");
   virtual ~TEvePadFrame();

   // For TAttBBox:
   virtual void ComputeBBox();

   // Override TObject:
   virtual void Paint(Option_t*) { PaintStandard(this); }

   // Getters & Setters
   TPad*    GetPad() const { return fPad; }
   void     SetPad(TPad* p);
   Double_t GetSizeX() const     { return fSizeX;  }
   void     SetSizeX(Double_t x) { fSizeX = x;     }
   Bool_t   GetUseFBO() const    { return fUseFBO; }
   void     SetUseFBO(Bool_t u);
   Int_t    GetSizeFBO() const   { return fSizeFBO; }
   void     SetSizeFBO(Int_t s)  { fSizeFBO = s;    }

   // Slots for pad-change notifications
   void PadClosed();
   void PadModified();
   void PadRangeChanged();
   void PadRangeAxisChanged();

   ClassDef(TEvePadFrame, 0); // Short description.
};

#endif
