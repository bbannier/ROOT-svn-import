// $Id$
// Author: Matevz Tadel 2009

/**************************************************************************
 * Copyright(c) 1998-2009, ALICE Experiment at CERN, all rights reserved. *
 * See http://aliceinfo.cern.ch/Offline/AliRoot/License.html for          *
 * full copyright notice.                                                 *
 **************************************************************************/

#ifndef CLASS_H
#define CLASS_H

#include "TGLObject.h"

class TGLViewer;
class TGLScene;

class STEM;

//______________________________________________________________________________
// Short description of CLASS
//

class CLASS : public TGLObject
{
public:
   CLASS();
   virtual ~CLASS() {}

   virtual Bool_t SetModel(TObject* obj, const Option_t* opt=0);
   virtual void   SetBBox();

   virtual void DirectDraw(TGLRnrCtx & rnrCtx) const;

   // To support two-level selection
   // virtual Bool_t SupportsSecondarySelect() const { return kTRUE; }
   // virtual void ProcessSelection(TGLRnrCtx & rnrCtx, TGLSelectRecord & rec);

protected:
   STEM             *fM;  // Model object.

private:
   CLASS(const CLASS&);            // Not implemented
   CLASS& operator=(const CLASS&); // Not implemented

   ClassDef(CLASS, 0); // GL renderer class for STEM.
};

#endif
