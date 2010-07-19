// @(#)root/eve:$Id$
// Author: Matevz Tadel 2007

/*************************************************************************
 * Copyright (C) 1995-2007, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include "TEvePadFrame.h"
#include "TPad.h"


//______________________________________________________________________________
// Description of TEvePadFrame
//

ClassImp(TEvePadFrame);

//______________________________________________________________________________
TEvePadFrame::TEvePadFrame(const char* n, const char* t) :
   TEveElementList(n, t),
   TAttBBox(),
   fPad(0),
   fSizeX(1)
{
   // Constructor.
}

//______________________________________________________________________________
TEvePadFrame::TEvePadFrame(TPad* pad, const char* n, const char* t) :
   TEveElementList(n, t),
   TAttBBox(),
   fPad(pad),
   fSizeX(1)
{
   // Constructor.
}

//==============================================================================

//______________________________________________________________________________
void TEvePadFrame::ComputeBBox()
{
   // Compute bounding-box of the data.

   if (fPad == 0)
   {
      BBoxZero();
      return;
   }

   BBoxInit();
   BBoxCheckPoint(0, 0, 0);
   BBoxCheckPoint(fSizeX, (fSizeX * fPad->GetWh()) / fPad->GetWw(), 0.1*fSizeX);
}
