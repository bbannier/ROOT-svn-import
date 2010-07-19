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


//______________________________________________________________________________
// Description of TEvePadFrame
//

ClassImp(TEvePadFrame);

//______________________________________________________________________________
TEvePadFrame::TEvePadFrame(const char* n, const char* t) :
   TEveElementList(n, t),
   TAttBBox(),
   fPad(0)
{
   // Constructor.
}

//______________________________________________________________________________
TEvePadFrame::TEvePadFrame(TPad* pad, const char* n, const char* t) :
   TEveElementList(n, t),
   TAttBBox(),
   fPad(pad)
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

   // Should scale in some better way??? Probably ...
   BBoxInit();
   BBoxCheckPoint(-1, -1, 0);
   BBoxCheckPoint( 1,  1, 0);
}
