// @(#)root/eve:$Id$
// Author: Matevz Tadel 2007

/*************************************************************************
 * Copyright (C) 1995-2007, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include "CLASS.h"


//______________________________________________________________________________
// Description of CLASS
//

ClassImp(CLASS);

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
