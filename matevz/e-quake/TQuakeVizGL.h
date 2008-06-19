// @(#)root/eve:$Id$
// Author: Matevz Tadel 2007

/*************************************************************************
 * Copyright (C) 1995-2007, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TQuakeVizGL
#define ROOT_TQuakeVizGL

#include "TGLObject.h"

class TGLViewer;
class TGLScene;

class TQuakeViz;

class TQuakeVizGL : public TGLObject
{
private:
   TQuakeVizGL(const TQuakeVizGL&);            // Not implemented
   TQuakeVizGL& operator=(const TQuakeVizGL&); // Not implemented

protected:
   TQuakeViz             *fM;        // Model object.
   mutable UInt_t         fSphereDL; // Display-list for sphere.

public:
   TQuakeVizGL();
   virtual ~TQuakeVizGL() {}

   virtual Bool_t ShouldDLCache(const TGLRnrCtx & rnrCtx) const;
   virtual void   DLCacheDrop();
   virtual void   DLCachePurge();

   virtual Bool_t SetModel(TObject* obj, const Option_t* opt=0);
   virtual void   SetBBox();

   void         MakeSphereDL(TGLRnrCtx& rnrCtx) const;
   virtual void DirectDraw(TGLRnrCtx & rnrCtx) const;

   // To support two-level selection
   virtual Bool_t SupportsSecondarySelect() const { return kTRUE; }
   virtual void   ProcessSelection(TGLRnrCtx & rnrCtx, TGLSelectRecord & rec);

   ClassDef(TQuakeVizGL, 0); // GL renderer class for TQuakeViz.
};

#endif
