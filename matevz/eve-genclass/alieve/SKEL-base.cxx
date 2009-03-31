// $Id$
// Author: Matevz Tadel 2009

/**************************************************************************
 * Copyright(c) 1998-2009, ALICE Experiment at CERN, all rights reserved. *
 * See http://aliceinfo.cern.ch/Offline/AliRoot/License.html for          *
 * full copyright notice.                                                 *
 **************************************************************************/

#include "CLASS.h"


//______________________________________________________________________________
// Full description of CLASS
//

ClassImp(CLASS)

//______________________________________________________________________________
CLASS::CLASS() :
  TObject()
{
  // Constructor.
}


/******************************************************************************/

/*

//______________________________________________________________________________
void CLASS::ComputeBBox()
{
  // Compute bounding-box of the data.

  if (<no-data>)
  {
    BBoxZero();
    return;
  }

  BBoxInit();
  // loop over data, collect bbox info.
}

//______________________________________________________________________________
void CLASS::Paint(Option_t* option)
{
  // Paint object.
  // This is for direct rendering (using CLASSGL class).

  static const TEveException eh("CLASS::Paint ");

  if (fRnrSelf == kFALSE) return;

  TBuffer3D buff(TBuffer3DTypes::kGeneric);

  // Section kCore
  buff.fID           = this;
  buff.fColor        = GetMainColor();
  buff.fTransparency = GetMainTransparency();
  if (HasMainTrans())
    RefMainTrans().SetBuffer3D(buff);
  buff.SetSectionsValid(TBuffer3D::kCore);

  Int_t reqSections = gPad->GetViewer3D()->AddObject(buff);
  if (reqSections != TBuffer3D::kNone)
    Error(eh, "only direct GL rendering supported.");
}
*/
