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

#include "TEveGValuators.h"

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
   fM(0),
   fLighting(0),
   fLimitRange(0),
   fYear(0),
   fMonth(0),
   fDay(0),
   fHour(0),
   fDayHalfRange(0)
{
   // Constructor.

   MakeTitle("TQuakeViz");

   UInt_t ww = 160, hh = 20;
   Int_t  labelW = 40;

   fLighting = new TGCheckButton(this, "Lightning");
   AddFrame(fLighting); // new TGLayoutHints());
   fLighting->Connect("Clicked()", "TQuakeVizEditor", this,
                      "DoLighting()");

   fLimitRange = new TGCheckButton(this, "Limit range");
   AddFrame(fLimitRange); // new TGLayoutHints());
   fLimitRange->Connect("Clicked()", "TQuakeVizEditor", this,
                        "DoLimitRange()");

   fYear = new TEveGValuator(this, "Year", ww, hh);
   fYear->SetLabelWidth(labelW);
   fYear->SetNELength(5);
   fYear->Build();
   AddFrame(fYear); // new TGLayoutHints());
   fYear->Connect("ValueSet(Double_t)", "TQuakeVizEditor", this,
                  "DoYear()");

   fMonth = new TEveGValuator(this, "Month", ww, hh);
   fMonth->SetLabelWidth(labelW);
   fMonth->SetNELength(5);
   fMonth->Build();
   fMonth->SetLimits(1, 12, 12, TGNumberFormat::kNESInteger);
   AddFrame(fMonth); // new TGLayoutHints());
   fMonth->Connect("ValueSet(Double_t)", "TQuakeVizEditor", this,
                   "DoMonth()");

   fDay = new TEveGValuator(this, "Day", ww, hh);
   fDay->SetLabelWidth(labelW);
   fDay->SetNELength(5);
   fDay->Build();
   fDay->SetLimits(1, 31, 31, TGNumberFormat::kNESInteger);
   AddFrame(fDay); // new TGLayoutHints());
   fDay->Connect("ValueSet(Double_t)", "TQuakeVizEditor", this,
                 "DoDay()");

   fHour = new TEveGValuator(this, "Hour", ww, hh);
   fHour->SetLabelWidth(labelW);
   fHour->SetNELength(5);
   fHour->Build();
   fHour->SetLimits(0, 23, 24, TGNumberFormat::kNESInteger);
   AddFrame(fHour); // new TGLayoutHints());
   fHour->Connect("ValueSet(Double_t)", "TQuakeVizEditor", this,
                  "DoHour()");

   fDayHalfRange = new TEveGValuator(this, "Days shown", 130, hh);
   fDayHalfRange->SetLabelWidth(labelW + 30);
   fDayHalfRange->SetNELength(5);
   fDayHalfRange->Build();
   fDayHalfRange->SetLimits(0, 1000, 101, TGNumberFormat::kNESInteger);
   AddFrame(fDayHalfRange); // new TGLayoutHints());
   fDayHalfRange->Connect("ValueSet(Double_t)", "TQuakeVizEditor", this,
                          "DoDayHalfRange()");
}

/******************************************************************************/

//______________________________________________________________________________
void TQuakeVizEditor::SetModel(TObject* obj)
{
   // Set model object.

   fM = dynamic_cast<TQuakeViz*>(obj);

   fLighting  ->SetState(fM->GetLighting() ? kButtonDown : kButtonUp);
   fLimitRange->SetState(fM->GetLimitRange() ? kButtonDown : kButtonUp);

   UInt_t minY, maxY;
   fM->fMinTime.GetDate(kFALSE, 0, &minY);
   fM->fMaxTime.GetDate(kFALSE, 0, &maxY);

   fYear->SetValue(fM->GetYear());
   fYear->SetLimits(minY, maxY, maxY - minY + 1, TGNumberFormat::kNESInteger);

   fMonth->SetValue(fM->GetMonth());
   fDay->SetValue(fM->GetDay());
   fHour->SetValue(fM->GetHour());
   fDayHalfRange->SetValue(fM->GetDayHalfRange());
}

/******************************************************************************/

// Implement callback/slot methods

//______________________________________________________________________________
void TQuakeVizEditor::DoLighting()
{
   // Slot for Lighting.

   fM->SetLighting(fLighting->IsOn());
   Update();
}

//______________________________________________________________________________
void TQuakeVizEditor::DoLimitRange()
{
   // Slot for LimitRange.

   fM->SetLimitRange(fLimitRange->IsOn());
   Update();
}

//______________________________________________________________________________
void TQuakeVizEditor::DoYear()
{
   // Slot for Year.

   fM->SetYear((Int_t) fYear->GetValue());
   Update();
}

//______________________________________________________________________________
void TQuakeVizEditor::DoMonth()
{
   // Slot for Month.

   fM->SetMonth((Int_t) fMonth->GetValue());
   Update();
}

//______________________________________________________________________________
void TQuakeVizEditor::DoDay()
{
   // Slot for Day.

   fM->SetDay((Int_t) fDay->GetValue());
   Update();
}

//______________________________________________________________________________
void TQuakeVizEditor::DoHour()
{
   // Slot for Hour.

   fM->SetHour((Int_t) fHour->GetValue());
   Update();
}

//______________________________________________________________________________
void TQuakeVizEditor::DoDayHalfRange()
{
   // Slot for DayHalfRange.

   fM->SetDayHalfRange((Int_t) fDayHalfRange->GetValue());
   Update();
}
