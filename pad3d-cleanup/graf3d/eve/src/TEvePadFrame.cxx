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
#include "TEveManager.h"

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
   fSizeX(1),
   fUseFBO(kFALSE), fSizeFBO(0),
   fRTS(1)
{
   // Constructor.
}

//______________________________________________________________________________
TEvePadFrame::TEvePadFrame(TPad* pad, const char* n, const char* t) :
   TEveElementList(n, t),
   TAttBBox(),
   fPad(0),
   fSizeX(1),
   fUseFBO(kFALSE), fSizeFBO(0),
   fRTS(1)
{
   // Constructor.

   SetPad(pad);
}

TEvePadFrame::~TEvePadFrame()
{
   // Destructor.

   SetPad(0);
}

//==============================================================================

void TEvePadFrame::SetPad(TPad* p)
{
   // Set the current pad. It disconnects signals from the old pad and
   // connects to the new one.

   if (fPad)
   {
      fPad->Disconnect("Closed()",           this, "PadClosed()");
      fPad->Disconnect("Modified()",         this, "PadModified()");
      fPad->Disconnect("RangeChanged()",     this, "PadRangeChanged()");
      fPad->Disconnect("RangeAxisChanged()", this, "PadRangeAxisChanged()");
   }
   fPad = p;
   if (fPad)
   {
      fPad->Connect("Closed()",           "TEvePadFrame", this, "PadClosed()");
      fPad->Connect("Modified()",         "TEvePadFrame", this, "PadModified()");
      fPad->Connect("RangeChanged()",     "TEvePadFrame", this, "PadRangeChanged()");
      fPad->Connect("RangeAxisChanged()", "TEvePadFrame", this, "PadRangeAxisChanged()");
   }

   IncRTS();
   StampObjProps();
   gEve->Redraw3D();
}

//______________________________________________________________________________
void TEvePadFrame::SetUseFBO(Bool_t u)
{
   // Set usage of frame-buffer object. This enforces dropping of
   // display lists and an eve redraw.

   fUseFBO = u;

   IncRTS();
   StampObjProps();
   gEve->Redraw3D();
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

//==============================================================================

//______________________________________________________________________________
void TEvePadFrame::PadClosed()
{
   // Slot for Closed signal from pad.

   SetPad(0);

   IncRTS();
   StampObjProps();
   gEve->Redraw3D();
}

//______________________________________________________________________________
void TEvePadFrame::PadModified()
{
   // Slot for Modified signal from pad.

   IncRTS();
   StampObjProps();
   gEve->Redraw3D();
}

//______________________________________________________________________________
void TEvePadFrame::PadRangeChanged()
{
   // Slot for RangeChanged signal from pad.

   IncRTS();
   StampObjProps();
   gEve->Redraw3D();
}

//______________________________________________________________________________
void TEvePadFrame::PadRangeAxisChanged()
{
   // Slot for RangeAxisChanged signal from pad.

   IncRTS();
   StampObjProps();
   gEve->Redraw3D();
}
