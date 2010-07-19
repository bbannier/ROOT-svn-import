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
private:
   TEvePadFrame(const TEvePadFrame&);            // Not implemented
   TEvePadFrame& operator=(const TEvePadFrame&); // Not implemented

protected:
   TPad    *fPad;

public:
   TEvePadFrame(const char* n="TEvePadFrame", const char* t="");
   TEvePadFrame(TPad* pad, const char* n="TEvePadFrame", const char* t="");
   virtual ~TEvePadFrame() {}

   // For TAttBBox:
   virtual void ComputeBBox();

   virtual void Paint(Option_t*) { PaintStandard(this); }

   TPad* GetPad() const  { return fPad; }
   void  SetPad(TPad* p) { fPad = p; }

   ClassDef(TEvePadFrame, 0); // Short description.
};

#endif
