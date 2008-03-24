// @(#)root/eve:$Id$
// Author: Matevz Tadel 2007

/*************************************************************************
 * Copyright (C) 1995-2007, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include "TEveCaloLegoEditor.h"
#include "TEveCalo.h"

#include "TColor.h"

#include "TGColorSelect.h"
#include "TGLabel.h"

//______________________________________________________________________________
// GUI editor for TEveCaloLego.
//

ClassImp(TEveCaloLegoEditor);

//______________________________________________________________________________
TEveCaloLegoEditor::TEveCaloLegoEditor(const TGWindow *p, Int_t width, Int_t height,
                                       UInt_t options, Pixel_t back) :
   TGedFrame(p, width, height, options | kVerticalFrame, back),
   fM(0),
   fFontColor(0),
   fGridColor(0)
{
   // Constructor.

   MakeTitle("TEveCaloLego");

   {
      // font color
      TGHorizontalFrame* f = new TGHorizontalFrame(this);
      TGLabel* lab = new TGLabel(f, "FontColor:");   
      f->AddFrame(lab, new TGLayoutHints(kLHintsLeft|kLHintsBottom, 1, 8, 1, 2));
      
      fFontColor = new TGColorSelect(f, 0, -1);
      f->AddFrame(fFontColor, new TGLayoutHints(kLHintsLeft|kLHintsTop, 3, 1, 0, 2));
      fFontColor->Connect("ColorSelected(Pixel_t)", "TEveCaloLegoEditor", this, "DoFontColor(Pixel_t)");

      AddFrame(f, new TGLayoutHints(kLHintsTop, 1, 1, 1, 0));
   }

   {
      // grid color
      TGHorizontalFrame* f = new TGHorizontalFrame(this);
      TGLabel* lab = new TGLabel(f, "GridColor:");   
      f->AddFrame(lab, new TGLayoutHints(kLHintsLeft|kLHintsBottom, 1, 10, 1, 2));
    
      fGridColor = new TGColorSelect(f, 0, -1);
      f->AddFrame(fGridColor, new TGLayoutHints(kLHintsLeft|kLHintsTop, 3, 1, 0, 2));
      fGridColor->Connect("ColorSelected(Pixel_t)", "TEveCaloLegoEditor", this, "DoGridColor(Pixel_t)");

      AddFrame(f, new TGLayoutHints(kLHintsTop, 1, 1, 1, 0));
   }
}

//______________________________________________________________________________
void TEveCaloLegoEditor::SetModel(TObject* obj)
{
   // Set model object.

   fM = dynamic_cast<TEveCaloLego*>(obj); 
   fFontColor->SetColor(TColor::Number2Pixel(fM->GetFontColor()), kFALSE);
   fGridColor->SetColor(TColor::Number2Pixel(fM->GetGridColor()), kFALSE);
}

//______________________________________________________________________________
void TEveCaloLegoEditor::DoFontColor(Pixel_t pixel)
{
   // Slot for FontColor.

   fM->SetFontColor(Color_t(TColor::GetColor(pixel)));
   Update();
}

//______________________________________________________________________________
void TEveCaloLegoEditor::DoGridColor(Pixel_t pixel)
{
   // Slot for GridColor.

   fM->SetGridColor(Color_t(TColor::GetColor(pixel)));
   Update();
}
