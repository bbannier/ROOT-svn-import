// @(#)root/base:$Id$
// Author: Valery Fine(fine@vxcern.cern.ch)   05/03/97

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TVirtualGL
#define ROOT_TVirtualGL

#include <utility>


#ifndef ROOT_Rtypes
#include "Rtypes.h"
#endif

class TGLContext;
class TGLFormat;

class TGLPaintDevice {
   friend class TGLContext;
public:
   virtual ~TGLPaintDevice(){}

   virtual Bool_t MakeCurrent() = 0;
   virtual void   SwapBuffers() = 0;
   virtual const  TGLFormat *GetPixelFormat()const = 0;
   virtual const  TGLContext *GetContext()const = 0;
   virtual void   ExtractViewport(Int_t *vp)const = 0;

   //Function for TRootCanvas.
   virtual Int_t  GetWindowIndex()const
   {
      return -1;//By default, gl device is not required to be a windows.
   }

private:
   virtual void   AddContext(TGLContext *ctx) = 0;
   virtual void   RemoveContext(TGLContext *ctx) = 0;

   ClassDef(TGLPaintDevice, 0) // Base class for GL widgets and GL off-screen rendering
};

#endif
