// @(#)root/gl:$Name$:$Id$
// Author:  Matevz Tadel, Feb 2007

/*************************************************************************
 * Copyright (C) 1995-2004, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TGLLightSet_H
#define ROOT_TGLLightSet_H

#include <TObject.h>

class TGLBoundingBox;
class TGLCamera;


class TGLLightSet : public TObject
{
public:
   enum ELight { kLightFront    = 0x0001,
                 kLightTop      = 0x0002,
                 kLightBottom   = 0x0004,
                 kLightLeft     = 0x0008,
                 kLightRight    = 0x0010,
                 kLightMask     = 0x001f,
                 kLightSpecular = 0x0100 };
private:
   TGLLightSet(const TGLLightSet&);            // Not implemented
   TGLLightSet& operator=(const TGLLightSet&); // Not implemented

protected:
   UInt_t        fLightState;   //! light states (on/off) mask
   Bool_t        fUseSpecular;  //!

public:
   TGLLightSet();
   virtual ~TGLLightSet() {}

   void    ToggleLight(ELight light);
   void    SetLight(ELight light, Bool_t on);
   UInt_t  GetLightState() { return fLightState; }

   Bool_t GetUseSpecular() const   { return fUseSpecular; }
   void   SetUseSpecular(Bool_t s) { fUseSpecular = s; }

   void StdSetupLights(const TGLBoundingBox& bbox, const TGLCamera& camera,
                       Bool_t debug=kFALSE);

   ClassDef(TGLLightSet, 0) // A set of OpenGL lights.
}; // endclass TGLLightSet


#endif
