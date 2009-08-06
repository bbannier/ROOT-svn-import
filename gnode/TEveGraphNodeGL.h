// @(#)root/eve:$Id: SKEL-gl.h 23035 2008-04-08 09:17:02Z matevz $
// Author: Matevz Tadel 2007

/*************************************************************************
 * Copyright (C) 1995-2007, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TEveGraphNodeGL
#define ROOT_TEveGraphNodeGL

#include "TGLObject.h"

class TGLViewer;
class TGLScene;
class TGLFaceSet;

class TEveGraphNode;

class TEveGraphNodeGL : public TGLObject
{
private:
   TEveGraphNodeGL(const TEveGraphNodeGL&);            // Not implemented
   TEveGraphNodeGL& operator=(const TEveGraphNodeGL&); // Not implemented

protected:
   TEveGraphNode             *fM;  // Model object.
   TGLFaceSet                *fFaceSet;

public:
   TEveGraphNodeGL();
   virtual ~TEveGraphNodeGL();

   virtual Bool_t SetModel(TObject* obj, const Option_t* opt=0);
   virtual void   SetBBox();

   virtual void DirectDraw(TGLRnrCtx & rnrCtx) const;

   // To support two-level selection
   // virtual Bool_t SupportsSecondarySelect() const { return kTRUE; }
   // virtual void ProcessSelection(TGLRnrCtx & rnrCtx, TGLSelectRecord & rec);

   ClassDef(TEveGraphNodeGL, 0); // GL renderer class for TEveGraphNode.
};

#endif
