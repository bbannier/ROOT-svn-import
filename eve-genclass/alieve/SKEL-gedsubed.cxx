// $Id$
// Author: Matevz Tadel 2009

/**************************************************************************
 * Copyright(c) 1998-2009, ALICE Experiment at CERN, all rights reserved. *
 * See http://aliceinfo.cern.ch/Offline/AliRoot/License.html for          *
 * full copyright notice.                                                 *
 **************************************************************************/

#include "CLASS.h"
#include "STEM.h"

#include "TVirtualPad.h"
#include "TColor.h"

// Cleanup these includes:
#include "TGLabel.h"
#include "TGButton.h"
#include "TGNumberEntry.h"
#include "TGColorSelect.h"
#include "TGDoubleSlider.h"

//______________________________________________________________________________
// GUI sub-editor for STEM.
//

ClassImp(XXCLASS)

//______________________________________________________________________________

XXCLASS::XXCLASS(const TGWindow *p) :
  TGVerticalFrame(p),
  fM             (0)
{
  // Constructor.
}

//______________________________________________________________________________

void XXCLASS::SetModel(STEM* m)
{
  // Set model object.

  fM = m;
}

//______________________________________________________________________________
void XXCLASS::Changed()
{
  // Emit "Changed()" signal.

  Emit("Changed()");
}

//______________________________________________________________________________
//void XXCLASS::DoABCD()
//{
//   // Slot for ABCD.
//
//   // Set values from widgets.
//   Changed();
//}


//______________________________________________________________________________
// GUI editor for STEM.
//

ClassImp(CLASS)

//______________________________________________________________________________
CLASS::CLASS(const TGWindow *p, Int_t width, Int_t height,
	     UInt_t options, Pixel_t back) :
  TGedFrame(p, width, height, options | kVerticalFrame, back),
  fM  (0),
  fSE (0)
{
  // Constructor.

  MakeTitle("STEM");

  fSE = new XXCLASS(this);
  AddFrame(fSE, new TGLayoutHints(kLHintsTop, 2, 0, 2, 2));
  fSE->Connect("Changed()", "CLASS", this, "Update()");
}

/******************************************************************************/

//______________________________________________________________________________
void CLASS::SetModel(TObject* obj)
{
  // Set model object.

  fM = dynamic_cast<STEM*>(obj);
  fSE->SetModel(fM);
}

/******************************************************************************/

// Implement callback/slot methods

//______________________________________________________________________________
// void CLASS::DoXYZZ()
// {
//    // Slot for XYZZ.
//
//    fM->SetXYZZ(fXYZZ->GetValue());
//    Update();
// }
