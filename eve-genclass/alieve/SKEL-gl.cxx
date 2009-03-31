// $Id$
// Author: Matevz Tadel 2009

/**************************************************************************
 * Copyright(c) 1998-2009, ALICE Experiment at CERN, all rights reserved. *
 * See http://aliceinfo.cern.ch/Offline/AliRoot/License.html for          *
 * full copyright notice.                                                 *
 **************************************************************************/

#include "CLASS.h"
#include "STEM.h"

#include "TGLRnrCtx.h"
#include "TGLIncludes.h"

//______________________________________________________________________________
// OpenGL renderer class for STEM.
//

ClassImp(CLASS)

//______________________________________________________________________________
CLASS::CLASS() :
  TGLObject(), fM(0)
{
  // Constructor.

  // fDLCache = kFALSE; // Disable display list.
}

/******************************************************************************/

//______________________________________________________________________________
Bool_t CLASS::SetModel(TObject* obj, const Option_t* /*opt*/)
{
  // Set model object.

  if (SetModelCheckClass(obj, STEM::Class())) {
    fM = dynamic_cast<STEM*>(obj);
    return kTRUE;
  }
  return kFALSE;
}

//______________________________________________________________________________
void CLASS::SetBBox()
{
  // Set bounding box.

  // !! This ok if master sub-classed from TAttBBox
  SetAxisAlignedBBox(((STEM*)fExternalObj)->AssertBBox());
}

/******************************************************************************/

//______________________________________________________________________________
void CLASS::DirectDraw(TGLRnrCtx & rnrCtx) const
{
  // Render with OpenGL.

  // printf("CLASS::DirectDraw LOD %d\n", flags.CombiLOD());
}
