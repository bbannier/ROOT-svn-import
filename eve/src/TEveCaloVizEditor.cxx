// @(#)root/eve:$Id$
// Author: Matevz Tadel 2007

/*************************************************************************
 * Copyright (C) 1995-2007, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include "TEveCaloVizEditor.h"
#include "TEveCalo.h"
#include "TEveGValuators.h"
#include "TEveRGBAPaletteEditor.h"

#include "TGLabel.h"
#include "TGNumberEntry.h"
#include "TGDoubleSlider.h"
#include "TGNumberEntry.h"
#include "TG3DLine.h"

#include "TMathBase.h"
#include "TMath.h"
//______________________________________________________________________________
// GUI editor for TEveCaloEditor.
//

ClassImp(TEveCaloVizEditor);

//______________________________________________________________________________
TEveCaloVizEditor::TEveCaloVizEditor(const TGWindow *p, Int_t width, Int_t height,
                                     UInt_t options, Pixel_t back) :
   TGedFrame(p, width, height, options | kVerticalFrame, back),
   fM(0),

   fEtaRng(0),
   fPhi(0),
   fPhiRng(0),
   fTower(0),
   fPalette(0)
{
   // Constructor.

   MakeTitle("TEveCaloVizEditor");

   Int_t  labelW = 45;

   // eta
   fEtaRng = new TEveGDoubleValuator(this,"Eta rng:", 40, 0);
   fEtaRng->SetNELength(6);
   fEtaRng->SetLabelWidth(labelW);
   fEtaRng->Build();
   fEtaRng->GetSlider()->SetWidth(195);
   fEtaRng->SetLimits(-5, 5, TGNumberFormat::kNESRealTwo);
   fEtaRng->Connect("ValueSet()", "TEveCaloVizEditor", this, "DoEtaRange()");
   AddFrame(fEtaRng, new TGLayoutHints(kLHintsTop, 1, 1, 4, 5));

   // phi
   fPhi = new TEveGValuator(this, "Phi:", 90, 0);
   fPhi->SetLabelWidth(labelW);
   fPhi->SetNELength(6);
   fPhi->Build();
   fPhi->SetLimits(-180, 180);
   fPhi->Connect("ValueSet(Double_t)", "TEveCaloVizEditor", this, "DoPhi()");
   AddFrame(fPhi, new TGLayoutHints(kLHintsTop, 1, 1, 1, 1));

   fPhiRng = new TEveGValuator(this, "PhiRng:", 90, 0);
   fPhiRng->SetLabelWidth(labelW);
   fPhiRng->SetNELength(6);
   fPhiRng->Build();
   fPhiRng->SetLimits(0, 180);
   fPhiRng->Connect("ValueSet(Double_t)", "TEveCaloVizEditor", this, "DoPhi()");
   AddFrame(fPhiRng, new TGLayoutHints(kLHintsTop, 1, 1, 1, 1));

   CreateTowerTab();
}

//______________________________________________________________________________
void TEveCaloVizEditor::CreateTowerTab()
{
  fTower = CreateEditorTabSubFrame("Tower");

  TGCompositeFrame *title1 = new TGCompositeFrame(fTower, 145, 10,
						  kHorizontalFrame |
						  kLHintsExpandX   |
						  kFixedWidth      |
						  kOwnBackground);
  title1->AddFrame(new TGLabel(title1, "Tower"),
		   new TGLayoutHints(kLHintsLeft, 1, 1, 0, 0));
  title1->AddFrame(new TGHorizontal3DLine(title1),
		   new TGLayoutHints(kLHintsExpandX, 5, 5, 7, 7));
  fTower->AddFrame(title1, new TGLayoutHints(kLHintsTop, 0, 0, 2, 0));

  fPalette = new TEveRGBAPaletteSubEditor(fTower);
  fTower->AddFrame(fPalette, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 2, 0, 0, 0));
  fPalette->Connect("Changed()", "TEveCaloVizEditor", this, "Update()");

}

//______________________________________________________________________________
void TEveCaloVizEditor::SetModel(TObject* obj)
{
   // Set model object.

   fM = dynamic_cast<TEveCaloViz*>(obj);

   fEtaRng->SetValues(fM->fEtaMin, fM->fEtaMax);

   fPhi->SetValue(fM->fPhi*TMath::RadToDeg());
   fPhiRng->SetValue(fM->fPhiRng*TMath::RadToDeg());

   fPalette->SetModel(fM->fPalette);
}

//______________________________________________________________________________
void TEveCaloVizEditor::DoEtaRange()
{
   fM->fEtaMin = fEtaRng->GetMin();
   fM->fEtaMax = fEtaRng->GetMax();
   Update();
}

//______________________________________________________________________________
void TEveCaloVizEditor::DoPhi()
{
   fM->fPhi    = fPhi->GetValue()*TMath::DegToRad();
   fM->fPhiRng = fPhiRng->GetValue()*TMath::DegToRad();
   Update();
}

