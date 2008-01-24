// @(#)root/gui:$Id$
// Author: Bertrand Bellenot 23/01/2008

/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include "TGFrame.h"
#include "TGLayout.h"
#include "TString.h"
#include "TGPicture.h"
#include "TImage.h"
#include "TRootEmbeddedCanvas.h"
#include "TGShapedFrame.h"
#include "TH1.h"
#include "TEnv.h"
#include "TCanvas.h"
#include "Riostream.h"

ClassImp(TGShapedFrame)

//______________________________________________________________________________
TGShapedFrame::TGShapedFrame(const char *pname, const TGWindow *p, UInt_t w, 
                             UInt_t h, UInt_t options) : 
      TGCompositeFrame(p, w, h, options), fBgnd(0), fImage(0)
{
   // Shaped window default constructor

   TString picName;
   // set a few attributes
   if (options & kTempFrame) {
      SetWindowAttributes_t attr;
      attr.fMask             = kWAOverrideRedirect | kWASaveUnder;
      attr.fOverrideRedirect = kTRUE;
      attr.fSaveUnder        = kTRUE;
      gVirtualX->ChangeWindowAttributes(fId, &attr);
   }
   // open the image file used as shape & background
   if (pname)
      picName = pname;
   else
      picName = "Default.png";
   fImage = TImage::Open(picName.Data());
   if (!fImage || !fImage->IsValid())
      Error("TGShapedFrame::TGShapedFrame", Form("%s not found", 
            picName.Data()));
   fBgnd = fClient->GetPicturePool()->GetPicture(picName.Data(),
           fImage->GetPixmap(), fImage->GetMask());
   // shape the window with the picture mask
   gVirtualX->ShapeCombineMask(fId, 0, 0, fBgnd->GetMask());
   // and finally set the background picture
   SetBackgroundPixmap(fBgnd->GetPicture());

   MapSubwindows();
   Resize();
   Resize(fBgnd->GetWidth(), fBgnd->GetHeight());
}

//______________________________________________________________________________
TGShapedFrame::~TGShapedFrame() 
{
   // Destructor.

   delete fImage;
   fClient->FreePicture(fBgnd);
}

//______________________________________________________________________________
void TGShapedFrame::SavePrimitive(ostream &out, Option_t *option /*= ""*/)
{
   // Save a shaped frame as a C++ statement(s) on output stream out.

   if (fBackground != GetDefaultFrameBackground()) SaveUserColor(out, option);

   out << endl << "   // shaped frame" << endl;
   out << "   TGShapedFrame *";
   out << GetName() << " = new TGShapedFrame(" << fImage->GetName()
       << "," << fParent->GetName() << "," << GetWidth() << "," 
       << GetHeight();

   if (fBackground == GetDefaultFrameBackground()) {
      if (!GetOptions()) {
         out << ");" << endl;
      } else {
         out << "," << GetOptionString() <<");" << endl;
      }
   } else {
      out << "," << GetOptionString() << ",ucolor);" << endl;
   }

   // setting layout manager if it differs from the main frame type
   TGLayoutManager * lm = GetLayoutManager();
   if ((GetOptions() & kHorizontalFrame) &&
       (lm->InheritsFrom(TGHorizontalLayout::Class()))) {
      ;
   } else if ((GetOptions() & kVerticalFrame) &&
              (lm->InheritsFrom(TGVerticalLayout::Class()))) {
      ;
   } else {
      out << "   " << GetName() <<"->SetLayoutManager(";
      lm->SavePrimitive(out, option);
      out << ");"<< endl;
   }

   SavePrimitiveSubframes(out, option);
}

ClassImp(TGShapedToolTip)

//______________________________________________________________________________
TGShapedToolTip::TGShapedToolTip(const char *pname, Int_t cx, Int_t cy, Int_t cw, 
                             Int_t ch, Int_t tx, Int_t ty, Int_t th, 
                             const char *col) : 
   TGShapedFrame(pname, gClient->GetDefaultRoot(), 400, 300, kTempFrame | 
                 kHorizontalFrame), fEc(0), fHist(0)
{
   // Shaped window constructor

   fTextX = tx; fTextY = ty; fTextH = th;
   if (col)
      fTextCol = col;
   else
      fTextCol = "0x000000";

   // create the embedded canvas
   if ((cx > 0) && (cy > 0) && (cw > 0) && (ch > 0)) {
      Int_t lhRight  = fWidth-cx-cw;
      Int_t lhBottom = fHeight-cy-ch;
      fEc = new TRootEmbeddedCanvas("ec", this, cw, ch, 0);
      AddFrame(fEc, new TGLayoutHints(kLHintsTop | kLHintsLeft, cx, 
                                      lhRight, cy, lhBottom));
   }
   MapSubwindows();
   Resize();
   Resize(fBgnd->GetWidth(), fBgnd->GetHeight());
}

//______________________________________________________________________________
TGShapedToolTip::~TGShapedToolTip() 
{
   // Destructor.

   if (fHist)
      delete fHist;
   if (fEc)
      delete fEc;
}

//______________________________________________________________________________
void TGShapedToolTip::CloseWindow() 
{
   // Close shaped window.
   
   DeleteWindow();
}

//______________________________________________________________________________
void TGShapedToolTip::Refresh()
{
   // Redraw the window with current attributes.

   const char *str = fText.Data();
   char *string = strdup(str);
   Int_t nlines = 0, size = fTextH;
   TString fp = gEnv->GetValue("Root.TTFontPath", "");
   TString ar = fp + "/arial.ttf";
   char *s = strtok((char *)string, "\n");
   TImage *img = (TImage*)fImage->Clone("img");
   img->DrawText(fTextX, fTextY+(nlines*size), s, size, fTextCol, ar);
   while ((s = strtok(0, "\n"))) {
      nlines++;
      img->DrawText(fTextX, fTextY+(nlines*size), s, size, fTextCol, ar);
   }
   img->PaintImage(fId, 0, 0, 0, 0, 0, 0, "opaque");
   free(string);
   delete img;
   gVirtualX->Update();
}

//______________________________________________________________________________
void TGShapedToolTip::CreateCanvas(Int_t cx, Int_t cy, Int_t cw, Int_t ch)
{

   // create the embedded canvas
   Int_t lhRight  = fWidth-cx-cw;
   Int_t lhBottom = fHeight-cy-ch;
   fEc = new TRootEmbeddedCanvas("ec", this, cw, ch, 0);
   AddFrame(fEc, new TGLayoutHints(kLHintsTop | kLHintsLeft, cx, 
                                   lhRight, cy, lhBottom));
   MapSubwindows();
   Resize();
   Resize(fBgnd->GetWidth(), fBgnd->GetHeight());
   if (IsMapped()) {
      Refresh();
   }
}

//______________________________________________________________________________
void TGShapedToolTip::CreateCanvas(Int_t cw, Int_t ch, TGLayoutHints *hints)
{
   // Create the embedded canvas.

   fEc = new TRootEmbeddedCanvas("ec", this, cw, ch, 0);
   AddFrame(fEc, hints);
   MapSubwindows();
   Resize();
   Resize(fBgnd->GetWidth(), fBgnd->GetHeight());
   if (IsMapped()) {
      Refresh();
   }
}

//______________________________________________________________________________
void TGShapedToolTip::SetHisto(TH1 *hist)
{
   // Set which histogram has to be displayed in the embedded canvas.

   if (hist) {
      if (fHist) {
         delete fHist;
         if (fEc)
            fEc->GetCanvas()->Clear();
      }
      fHist = (TH1 *)hist->Clone();
      if (fEc) {
         fEc->GetCanvas()->SetBorderMode(0);
         fEc->GetCanvas()->SetFillColor(10);
         fEc->GetCanvas()->cd();
         fHist->Draw();
         fEc->GetCanvas()->Update();
      }
   }
}

//______________________________________________________________________________
void TGShapedToolTip::SetText(const char *text)
{
   // Set which text has to be displayed.

   if (text) {
      fText = text;
   }
   if (IsMapped())
      Refresh();
}

//______________________________________________________________________________
void TGShapedToolTip::SetTextColor(const char *col)
{
   // Set text color.

   fTextCol = col;
   if (IsMapped())
      Refresh();
}

//______________________________________________________________________________
void TGShapedToolTip::SetTextAttributes(Int_t tx, Int_t ty, Int_t th, 
                                        const char *col)
{
   // Set text attributes (position, size and color).

   fTextX = tx; fTextY = ty; fTextH = th;
   if (col)
      fTextCol = col;
   if (IsMapped())
      Refresh();
}

//______________________________________________________________________________
void TGShapedToolTip::Show(Int_t x, Int_t y, const char *text, TH1 *hist)
{
   // Show (popup) the shaped window at location x,y and possibly
   // set the text and histogram to be displayed.

   Move(x, y);
   MapWindow();

   if (text)
      SetText(text);
   if (hist)
      SetHisto(hist);
   // end of demo code -------------------------------------------
   if (fHist) {
      fEc->GetCanvas()->SetBorderMode(0);
      fEc->GetCanvas()->SetFillColor(10);
      fEc->GetCanvas()->cd();
      fHist->Draw();
      fEc->GetCanvas()->Update();
   }
   Refresh();
}

