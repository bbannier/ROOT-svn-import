// @(#)root/eve:$Id$
// Author: Matevz Tadel 2007

/*************************************************************************
 * Copyright (C) 1995-2007, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include "TQuakeVizEditor.h"
#include "TQuakeViz.h"

#include "TVirtualPad.h"
#include "TColor.h"

// Cleanup these includes:
#include "TGLabel.h"
#include "TGButton.h"
#include "TGNumberEntry.h"
#include "TGColorSelect.h"
#include "TGDoubleSlider.h"


//______________________________________________________________________________
// GUI editor for TQuakeViz.
//

ClassImp(TQuakeVizEditor);

//______________________________________________________________________________
TQuakeVizEditor::TQuakeVizEditor(const TGWindow *p, Int_t width, Int_t height,
             UInt_t options, Pixel_t back) :
   TGedFrame(p, width, height, options | kVerticalFrame, back),
   fM(0)
   // Initialize widget pointers to 0
{
   // Constructor.

   MakeTitle("TQuakeViz");

   // Create widgets
   // fXYZZ = new TGSomeWidget(this, ...);
   // AddFrame(fXYZZ, new TGLayoutHints(...));
   // fXYZZ->Connect("SignalName()", "Reve::TQuakeVizEditor", this, "DoXYZZ()");
}

/******************************************************************************/

//______________________________________________________________________________
void TQuakeVizEditor::SetModel(TObject* obj)
{
   // Set model object.

   fM = dynamic_cast<TQuakeViz*>(obj);

   // Set values of widgets
   // fXYZZ->SetValue(fM->GetXYZZ());
}

/******************************************************************************/

// Implements callback/slot methods

//______________________________________________________________________________
// void TQuakeVizEditor::DoXYZZ()
// {
//    // Slot for XYZZ.
//
//    fM->SetXYZZ(fXYZZ->GetValue());
//    Update();
// }
