// @(#)root/gl:$Id$
// Author: Matevz Tadel, Aug 2009

/*************************************************************************
 * Copyright (C) 1995-2007, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TGLFBO
#define ROOT_TGLFBO

#include "Rtypes.h"

class TGLFBO
{
private:
   TGLFBO(const TGLFBO&);            // Not implemented
   TGLFBO& operator=(const TGLFBO&); // Not implemented

protected:
   UInt_t  fFrameBuffer;
   UInt_t  fColorTexture;
   UInt_t  fDepthBuffer;
   // UInt_t  fStencilBuffer;

   Int_t   fRequestedW, fRequestedH;
   Int_t   fW, fH;

   Float_t fWScale, fHScale;
   Bool_t  fIsRescaled;

   Bool_t  fIsTextureBound;

   static Bool_t fgRescaleToPow2;

public:
   TGLFBO();
   virtual ~TGLFBO();

   void Init(int w, int h);
   void Release();

   void Bind();
   void Unbind();

   void BindTexture();
   void UnbindTexture();

   // ----------------------------------------------------------------

   Int_t GetW() const { return fW; }
   Int_t GetH() const { return fH; }

   Int_t GetRequestedW() const { return fRequestedW; }
   Int_t GetRequestedH() const { return fRequestedH; }

   ClassDef(TGLFBO, 0); // Frame-buffer object.
};

#endif
