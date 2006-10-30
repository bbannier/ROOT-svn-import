// @(#)root/gui:$Name:  $:$Id: TGScrollBar.cxx,v 1.22 2006/10/13 07:52:00 antcheva Exp $
// Author: Fons Rademakers   10/01/98

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/
/**************************************************************************

    This source is based on Xclass95, a Win95-looking GUI toolkit.
    Copyright (C) 1996, 1997 David Barth, Ricky Ralston, Hector Peraza.

    Xclass95 is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

**************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TGScrollBar and TGScrollBarElement                                   //
//                                                                      //
// The classes in this file implement scrollbars. Scrollbars can be     //
// either placed horizontal or vertical. A scrollbar contains three     //
// TGScrollBarElements: The "head", "tail" and "slider". The head and   //
// tail are fixed at either end and have the typical arrows in them.    //
//                                                                      //
// The TGHScrollBar will generate the following event messages:         //
// kC_HSCROLL, kSB_SLIDERPOS, position, 0                               //
// kC_HSCROLL, kSB_SLIDERTRACK, position, 0                             //
//                                                                      //
// The TGVScrollBar will generate the following event messages:         //
// kC_VSCROLL, kSB_SLIDERPOS, position, 0                               //
// kC_VSCROLL, kSB_SLIDERTRACK, position, 0                             //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "TGScrollBar.h"
#include "TGResourcePool.h"
#include "TGPicture.h"
#include "TImage.h"
#include "TSystem.h"
#include "TTimer.h"
#include "TMath.h"
#include "TEnv.h"
#include "Riostream.h"


Pixmap_t TGScrollBar::fgBckgndPixmap = 0;
Int_t    TGScrollBar::fgScrollBarWidth = kDefaultScrollBarWidth;

ClassImp(TGScrollBarElement)
ClassImp(TGScrollBar)
ClassImp(TGHScrollBar)
ClassImp(TGVScrollBar)


//______________________________________________________________________________
class TSBRepeatTimer : public TTimer {
private:
   TGScrollBar   *fScrollBar;   // scroll bar
public:
   TSBRepeatTimer(TGScrollBar *s, Long_t ms) : TTimer(ms, kTRUE) { fScrollBar = s; }
   Bool_t Notify();
};

//______________________________________________________________________________
Bool_t TSBRepeatTimer::Notify()
{
   // Notify when timer times out and reset the timer.

   fScrollBar->HandleTimer(this);
   Reset();
   return kFALSE;
}

//______________________________________________________________________________
TGScrollBarElement::TGScrollBarElement(const TGWindow *p, const TGPicture *pic,
                                       UInt_t w, UInt_t h, UInt_t options, Pixel_t back) :
                                       TGFrame(p, w, h, options | kOwnBackground, back)
{
   // Constructor.

   fPic = fPicN = pic;
   fState = kButtonUp;
   fPicD = 0;
}

//______________________________________________________________________________
TGScrollBarElement::~TGScrollBarElement()
{
   // destructor

   if (fPicD) {
      fClient->FreePicture(fPicD);
   }
}

//______________________________________________________________________________
void TGScrollBarElement::SetState(Int_t state)
{
   // Change state of scrollbar element (either up or down).

   if (state != fState) {
      switch (state) {
         case kButtonDown:
            fOptions &= ~kRaisedFrame;
            fOptions |= kSunkenFrame;
            break;
         case kButtonUp:
         case kButtonDisabled:
            fOptions &= ~kSunkenFrame;
            fOptions |= kRaisedFrame;
            break;
      }
      fState = state;
      fClient->NeedRedraw(this);
   }
}

//______________________________________________________________________________
void TGScrollBarElement::SetEnabled(Bool_t on)
{
   // Enable/Disable scroll bar button chaging the state.

   if (on) {
      if (fState == kButtonUp) {
         return;
      }
      SetState(kButtonUp);
      fPic = fPicN;
   } else {
      if (fState == kButtonDisabled) {
         return;
      }
      SetState(kButtonDisabled);

      if (!fPicD) {
         TImage *img = TImage::Create();
         TImage *img2 = TImage::Create();

         TString back = gEnv->GetValue("Gui.BackgroundColor", "#c0c0c0");
         img2->FillRectangle(back.Data(), 0, 0, fPic->GetWidth(), fPic->GetHeight());
         img->SetImage(fPicN->GetPicture(), fPicN->GetMask());
         Pixmap_t mask = img->GetMask();
         img2->Merge(img, "overlay");

         TString name = "disbl_";
         name += fPic->GetName();
         fPicD = fClient->GetPicturePool()->GetPicture(name.Data(), img2->GetPixmap(),
                                                       mask);
         delete img;
         delete img2;
      }
      fPic = fPicD;
   }
   fClient->NeedRedraw(this);
}

//______________________________________________________________________________
void TGScrollBarElement::DrawBorder()
{
   // Draw border around scollbar element.

   switch (fOptions & (kSunkenFrame | kRaisedFrame)) {
      case kSunkenFrame: // pressed
         gVirtualX->DrawLine(fId, GetBlackGC()(), 0, 0, fWidth-2, 0);
         gVirtualX->DrawLine(fId, GetBlackGC()(), 0, 0, 0, fHeight-2);
         gVirtualX->DrawLine(fId, GetShadowGC()(),  1, 1, fWidth-3, 1);
         gVirtualX->DrawLine(fId, GetShadowGC()(),  1, 1, 1, fHeight-3);

         gVirtualX->DrawLine(fId, GetWhiteGC()(),  0, fHeight-1, fWidth-1, fHeight-1);
         gVirtualX->DrawLine(fId, GetWhiteGC()(),  fWidth-1, fHeight-1, fWidth-1, 1);
         gVirtualX->DrawLine(fId, GetBckgndGC()(),  1, fHeight-2, fWidth-2, fHeight-2);
         gVirtualX->DrawLine(fId, GetBckgndGC()(),  fWidth-2, fHeight-2, fWidth-2, 2);

         if (fPic) {
            int x = (fWidth - fPic->GetWidth()) >> 1;
            int y = (fHeight - fPic->GetHeight()) >> 1;
            fPic->Draw(fId, GetBckgndGC()(), x+1, y+1); // 3, 3
         }
         break;

      case kRaisedFrame: // normal
      case kButtonDisabled:
         gVirtualX->DrawLine(fId, GetBckgndGC()(), 0, 0, fWidth-2, 0);
         gVirtualX->DrawLine(fId, GetBckgndGC()(), 0, 0, 0, fHeight-2);
         gVirtualX->DrawLine(fId, GetHilightGC()(), 1, 1, fWidth-3, 1);
         gVirtualX->DrawLine(fId, GetHilightGC()(), 1, 1, 1, fHeight-3);

         gVirtualX->DrawLine(fId, GetShadowGC()(),  1, fHeight-2, fWidth-2, fHeight-2);
         gVirtualX->DrawLine(fId, GetShadowGC()(),  fWidth-2, fHeight-2, fWidth-2, 1);
         gVirtualX->DrawLine(fId, GetBlackGC()(), 0, fHeight-1, fWidth-1, fHeight-1);
         gVirtualX->DrawLine(fId, GetBlackGC()(), fWidth-1, fHeight-1, fWidth-1, 0);

         if (fPic) {
            int x = (fWidth - fPic->GetWidth()) >> 1;
            int y = (fHeight - fPic->GetHeight()) >> 1;
            fPic->Draw(fId, GetBckgndGC()(), x, y); // 2, 2
         }
         break;

      default:
         break;
   }
}

//______________________________________________________________________________
TGScrollBar::TGScrollBar(const TGWindow *p, UInt_t w, UInt_t h,
                         UInt_t options, Pixel_t back) :
   TGFrame(p, w, h, options | kOwnBackground, back),
   fX0(0), fY0(0), fXp(0), fYp(0), fDragging(kFALSE), fGrabPointer(kTRUE),
   fRange(0), fPsize(0), fPos(0), fSliderSize(0), fSliderRange(0),
   fSmallInc(1), fHead(0), fTail(0), fSlider(0), fHeadPic(0),
   fTailPic(0), fRepeat(0), fSubw()
{
   // Constructor.

   fMsgWindow = p;
   SetBackgroundPixmap(GetBckgndPixmap());
   SetWindowName();
}

//______________________________________________________________________________
TGScrollBar::~TGScrollBar()
{
   // Delete a scrollbar (either horizontal or vertical).

   delete fHead;
   delete fTail;
   delete fSlider;
   if (fHeadPic) fClient->FreePicture(fHeadPic);
   if (fTailPic) fClient->FreePicture(fTailPic);
   if (fRepeat) { delete fRepeat; fRepeat = 0; }
}

//______________________________________________________________________________
Bool_t TGScrollBar::HandleTimer(TTimer *t)
{
   // Handle repeat timer for horizontal or vertical scrollbar. Every time
   // timer times out we move slider.

   // shorten time out time to 50 milli seconds
   t->SetTime(50);

   Window_t  dum1, dum2;
   Event_t   ev;

   ev.fType    = kButtonPress;
   ev.fUser[0] = fSubw;

   gVirtualX->QueryPointer(fId, dum1, dum2, ev.fXRoot, ev.fYRoot, ev.fX, ev.fY,
                      ev.fState);

   HandleButton(&ev);

   return kTRUE;
}

//______________________________________________________________________________
Pixmap_t TGScrollBar::GetBckgndPixmap()
{
   // Static method returning scrollbar background pixmap.

   static Bool_t init = kFALSE;
   if (!init) {
      fgBckgndPixmap = gClient->GetResourcePool()->GetCheckeredPixmap();
      init = kTRUE;
   }
   return fgBckgndPixmap;
}

//______________________________________________________________________________
Int_t TGScrollBar::GetScrollBarWidth()
{
   // Static method returning the scrollbar width.

   return fgScrollBarWidth;
}

//______________________________________________________________________________
void TGScrollBar::ChangeBackground(Pixel_t back)
{
   // Change background color

   TGFrame::ChangeBackground(back);
   fHead->ChangeBackground(back);
   fTail->ChangeBackground(back);
   fSlider->ChangeBackground(back);
}

//______________________________________________________________________________
TGHScrollBar::TGHScrollBar(const TGWindow *p, UInt_t w, UInt_t h,
                           UInt_t options, ULong_t back) :
    TGScrollBar(p, w, h, options, back)
{
   // Create a horizontal scrollbar widget.

   fHeadPic = fClient->GetPicture("arrow_left.xpm");
   fTailPic = fClient->GetPicture("arrow_right.xpm");

   if (!fHeadPic || !fTailPic) {
      Error("TGHScrollBar", "arrow_*.xpm not found");
      return;
   }
   fHead   = new TGScrollBarElement(this, fHeadPic, fgScrollBarWidth, fgScrollBarWidth,
                                    kRaisedFrame);
   fTail   = new TGScrollBarElement(this, fTailPic, fgScrollBarWidth, fgScrollBarWidth,
                                    kRaisedFrame);
   fSlider = new TGScrollBarElement(this, 0, fgScrollBarWidth, 50,
                                    kRaisedFrame);

   gVirtualX->GrabButton(fId, kAnyButton, kAnyModifier, kButtonPressMask |
                         kButtonReleaseMask | kPointerMotionMask, kNone, kNone);

   fDragging = kFALSE;
   fX0 = fY0 = (fgScrollBarWidth = TMath::Max(fgScrollBarWidth, 5));
   fPos = 0;

   fRange = TMath::Max((Int_t) w - (fgScrollBarWidth << 1), 1);
   fPsize = fRange >> 1;

   fSliderSize  = 50;
   fSliderRange = 1;

   fHead->SetEditDisabled(kEditDisable | kEditDisableGrab);
   fTail->SetEditDisabled(kEditDisable | kEditDisableGrab);
   fSlider->SetEditDisabled(kEditDisable | kEditDisableGrab);
   fEditDisabled = kEditDisableLayout | kEditDisableHeight | kEditDisableBtnEnable;
}

//______________________________________________________________________________
void TGHScrollBar::Layout()
{
   // Layout and move horizontal scrollbar components.

   // Should also recalculate the slider size and range, etc.
   fHead->MoveResize(0, 0, fgScrollBarWidth, fgScrollBarWidth);
   fTail->MoveResize(fWidth-fgScrollBarWidth, 0, fgScrollBarWidth, fgScrollBarWidth);
   fSlider->MoveResize(fX0, 0, 50, fgScrollBarWidth);
}

//______________________________________________________________________________
Bool_t TGHScrollBar::HandleButton(Event_t *event)
{
   // Handle a mouse button event in a horizontal scrolbar.

   Int_t newpos;

   if (event->fCode == kButton4) {
      if (!fHead->IsEnabled()) {
         return kFALSE;
      }
      //scroll left
      newpos = fPos - fPsize;
      if (newpos<0) newpos = 0;
      SetPosition(newpos);
      return kTRUE;
   }
   if (event->fCode == kButton5) {
      if (!fTail->IsEnabled()) {
         return kFALSE;
      }
      // scroll right
      newpos = fPos + fPsize;
      SetPosition(newpos);
      return kTRUE;
   }

   if (event->fType == kButtonPress) {
      if (event->fCode == kButton3) {
         fX0 = event->fX - fSliderSize/2;
         fX0 = TMath::Max(fX0, fgScrollBarWidth);
         fX0 = TMath::Min(fX0, fgScrollBarWidth + fSliderRange);
         fPos = (fX0 - fgScrollBarWidth) * (fRange-fPsize) / fSliderRange;

         fPos = TMath::Max(fPos, 0);
         fPos = TMath::Min(fPos, fRange-fPsize);
         fSlider->Move(fX0, 0);

         SendMessage(fMsgWindow, MK_MSG(kC_HSCROLL, kSB_SLIDERTRACK), fPos, 0);
         PositionChanged(fPos);
         return kTRUE;
      }

      // fUser[0] contains the subwindow (child) in which the event occured
      // (see GX11Gui.cxx)
      Window_t subw = (Window_t)event->fUser[0];

      if (subw == fSlider->GetId()) {
         fXp = event->fX - fX0;
         fYp = event->fY - fY0;
         fDragging = kTRUE;

      } else {

         if (!fRepeat)
            fRepeat = new TSBRepeatTimer(this, 400); // 500
         fRepeat->Reset();
         gSystem->AddTimer(fRepeat);
         fSubw = subw;

         if (subw == fHead->GetId()) {
            //if (!fHead->IsEnabled()) {
             //  return kFALSE;
            //}
            fHead->SetState(kButtonDown);
            fPos -= fSmallInc;
         } else if (subw == fTail->GetId()) {
            //if (!fTail->IsEnabled()) {
           //    return kFALSE;
           // }
            fTail->SetState(kButtonDown);
            fPos += fSmallInc;
         } else if (event->fX > fgScrollBarWidth && event->fX < fX0)
            fPos -= fPsize;
         else if (event->fX > fX0+fSliderSize && event->fX < (Int_t)fWidth-fgScrollBarWidth)
            fPos += fPsize;

         fPos = TMath::Max(fPos, 0);
         fPos = TMath::Min(fPos, fRange-fPsize);

         fX0 = fgScrollBarWidth + fPos * fSliderRange / TMath::Max(fRange-fPsize, 1);

         fX0 = TMath::Max(fX0, fgScrollBarWidth);
         fX0 = TMath::Min(fX0, fgScrollBarWidth + fSliderRange);

         fSlider->Move(fX0, 0);

         SendMessage(fMsgWindow, MK_MSG(kC_HSCROLL, kSB_SLIDERTRACK), fPos, 0);
         PositionChanged(fPos);
      }

      // last argument kFALSE forces all specified events to this window
      if (fGrabPointer && !fClient->IsEditable())
         gVirtualX->GrabPointer(fId, kButtonPressMask | kButtonReleaseMask |
                                kPointerMotionMask, kNone, kNone,
                                kTRUE, kFALSE);
   } else {
      fHead->SetState(kButtonUp);
      fTail->SetState(kButtonUp);

      if (fRepeat) {
         fRepeat->Remove();
         fRepeat->SetTime(400);  // might have been shortened in HandleTimer()
      }

      fDragging = kFALSE;

      fPos = TMath::Max(fPos, 0);
      fPos = TMath::Min(fPos, fRange-fPsize);

      SendMessage(fMsgWindow, MK_MSG(kC_HSCROLL, kSB_SLIDERPOS), fPos, 0);
      PositionChanged(fPos);

      if (fGrabPointer)
         gVirtualX->GrabPointer(0, 0, 0, 0, kFALSE);  // ungrab pointer
   }
   return kTRUE;
}

//______________________________________________________________________________
Bool_t TGHScrollBar::HandleMotion(Event_t *event)
{
   // Handle mouse motion event in a horizontal scrollbar.

   if (fDragging) {
      fX0 = event->fX - fXp;
      fY0 = event->fY - fYp;

      fX0 = TMath::Max(fX0, fgScrollBarWidth);
      fX0 = TMath::Min(fX0, fgScrollBarWidth + fSliderRange);
      fSlider->Move(fX0, 0);
      fPos = (fX0 - fgScrollBarWidth) * (fRange-fPsize) / fSliderRange;

      fPos = TMath::Max(fPos, 0);
      fPos = TMath::Min(fPos, fRange-fPsize);

      SendMessage(fMsgWindow, MK_MSG(kC_HSCROLL, kSB_SLIDERTRACK), fPos, 0);
      PositionChanged(fPos);
   }
   return kTRUE;
}

//______________________________________________________________________________
void TGHScrollBar::SetRange(Int_t range, Int_t page_size)
{
   // Set range of horizontal scrollbar.

   fRange = TMath::Max(range, 1);
   fPsize = TMath::Max(page_size, 0);

   fSliderSize = TMath::Max(fPsize * (fWidth - (fgScrollBarWidth << 1)) /
                            fRange, (UInt_t) 6);
   fSliderSize = TMath::Min((UInt_t)fSliderSize, fWidth - (fgScrollBarWidth << 1));

   fSliderRange = TMath::Max(fWidth - (fgScrollBarWidth << 1) - fSliderSize,
                             (UInt_t) 1);

   fX0 = fgScrollBarWidth + fPos * fSliderRange / TMath::Max(fRange-fPsize, 1);
   fX0 = TMath::Max(fX0, fgScrollBarWidth);
   fX0 = TMath::Min(fX0, fgScrollBarWidth + fSliderRange);

   fSlider->MoveResize(fX0, 0, fSliderSize, fgScrollBarWidth);

   //  fPos = (fX0 - fgScrollBarWidth) * (fRange-fPsize) / fSliderRange;

   fPos = TMath::Max(fPos, 0);
   fPos = TMath::Min(fPos, fRange-fPsize);

   SendMessage(fMsgWindow, MK_MSG(kC_HSCROLL, kSB_SLIDERPOS), fPos, 0);
   PositionChanged(fPos);
   RangeChanged(fRange);
   PageSizeChanged(fPsize);
}

//______________________________________________________________________________
void TGHScrollBar::SetPosition(Int_t pos)
{
   // Set logical slider position of horizontal scrollbar.

   fPos = TMath::Max(pos, 0);
   fPos = TMath::Min(pos, fRange-fPsize);

   fX0 = fgScrollBarWidth + fPos * fSliderRange / TMath::Max(fRange-fPsize, 1);
   fX0 = TMath::Max(fX0, fgScrollBarWidth);
   fX0 = TMath::Min(fX0, fgScrollBarWidth + fSliderRange);

   fSlider->MoveResize(fX0, 0, fSliderSize, fgScrollBarWidth);

   SendMessage(fMsgWindow, MK_MSG(kC_HSCROLL, kSB_SLIDERPOS), fPos, 0);
   PositionChanged(fPos);
}


//______________________________________________________________________________
TGVScrollBar::TGVScrollBar(const TGWindow *p, UInt_t w, UInt_t h,
                           UInt_t options, ULong_t back) :
    TGScrollBar(p, w, h, options, back)
{
   // Create a vertical scrollbar.

   fHeadPic = fClient->GetPicture("arrow_up.xpm");
   fTailPic = fClient->GetPicture("arrow_down.xpm");

   if (!fHeadPic || !fTailPic) {
      Error("TGVScrollBar", "arrow_*.xpm not found");
      return;
   }
   fHead   = new TGScrollBarElement(this, fHeadPic, fgScrollBarWidth, fgScrollBarWidth,
                                    kRaisedFrame);
   fTail   = new TGScrollBarElement(this, fTailPic, fgScrollBarWidth, fgScrollBarWidth,
                                    kRaisedFrame);
   fSlider = new TGScrollBarElement(this, 0, fgScrollBarWidth, 50,
                                    kRaisedFrame);

   gVirtualX->GrabButton(fId, kAnyButton, kAnyModifier, kButtonPressMask |
                         kButtonReleaseMask | kPointerMotionMask, kNone, kNone);

   fDragging = kFALSE;
   fX0 = fY0 = (fgScrollBarWidth = TMath::Max(fgScrollBarWidth, 5));
   fPos = 0;

   fRange = TMath::Max((Int_t) h - (fgScrollBarWidth << 1), 1);
   fPsize = fRange >> 1;

   fSliderSize  = 50;
   fSliderRange = 1;

   fHead->SetEditDisabled(kEditDisable | kEditDisableGrab);
   fTail->SetEditDisabled(kEditDisable | kEditDisableGrab);
   fSlider->SetEditDisabled(kEditDisable | kEditDisableGrab);
   fEditDisabled = kEditDisableLayout | kEditDisableWidth | kEditDisableBtnEnable;
}

//______________________________________________________________________________
void TGVScrollBar::Layout()
{
   // Layout and move vertical scrollbar components.

   // Should recalculate fSliderSize, fSliderRange, fX0, fY0, etc. too...

   fHead->MoveResize(0, 0, fgScrollBarWidth, fgScrollBarWidth);
   fTail->MoveResize(0, fHeight-fgScrollBarWidth, fgScrollBarWidth, fgScrollBarWidth);
   fSlider->MoveResize(0, fY0, fgScrollBarWidth, 50);
}

//______________________________________________________________________________
Bool_t TGVScrollBar::HandleButton(Event_t *event)
{
   // Handle mouse button event in vertical scrollbar.

   Int_t newpos;

   if (event->fCode == kButton4) {
      if (!fHead->IsEnabled()) {
         return kFALSE;
      }
      //scroll up
      newpos = fPos - fPsize;
      if (newpos<0) newpos = 0;
      SetPosition(newpos);
      return kTRUE;
   }
   if (event->fCode == kButton5) {
      if (!fTail->IsEnabled()) {
         return kFALSE;
      }

      // scroll down
      newpos = fPos + fPsize;
      SetPosition(newpos);
      return kTRUE;
   }

   if (event->fType == kButtonPress) {
      if (event->fCode == kButton3) {
         fY0 = event->fY - fSliderSize/2;
         fY0 = TMath::Max(fY0, fgScrollBarWidth);
         fY0 = TMath::Min(fY0, fgScrollBarWidth + fSliderRange);
         fPos = (fY0 - fgScrollBarWidth) * (fRange-fPsize) / fSliderRange;

         fPos = TMath::Max(fPos, 0);
         fPos = TMath::Min(fPos, fRange-fPsize);
         fSlider->Move(0, fY0);

         SendMessage(fMsgWindow, MK_MSG(kC_VSCROLL, kSB_SLIDERTRACK), fPos, 0);
         PositionChanged(fPos);
         return kTRUE;
      }

      // fUser[0] contains the subwindow (child) in which the event occured
      // (see GX11Gui.cxx)
      Window_t subw = (Window_t)event->fUser[0];

      if (subw == fSlider->GetId()) {
         fXp = event->fX - fX0;
         fYp = event->fY - fY0;
         fDragging = kTRUE;

      } else {

         if (!fRepeat)
            fRepeat = new TSBRepeatTimer(this, 400); // 500
         fRepeat->Reset();
         gSystem->AddTimer(fRepeat);
         fSubw = subw;

         if (subw == fHead->GetId()) {
            //if (!fHead->IsEnabled()) {
            //   return kFALSE;
           // }
            fHead->SetState(kButtonDown);
            fPos -= fSmallInc;
         } else if (subw == fTail->GetId()) {
            //if (!fTail->IsEnabled()) {
            //   return kFALSE;
            //}
            fTail->SetState(kButtonDown);
            fPos += fSmallInc;
         } else if (event->fY > fgScrollBarWidth && event->fY < fY0)
            fPos -= fPsize;
         else if (event->fY > fY0+fSliderSize && event->fY < (Int_t)fHeight-fgScrollBarWidth)
            fPos += fPsize;

         fPos = TMath::Max(fPos, 0);
         fPos = TMath::Min(fPos, fRange-fPsize);

         fY0 = fgScrollBarWidth + fPos * fSliderRange / TMath::Max(fRange-fPsize, 1);

         fY0 = TMath::Max(fY0, fgScrollBarWidth);
         fY0 = TMath::Min(fY0, fgScrollBarWidth + fSliderRange);

         fSlider->Move(0, fY0);

         SendMessage(fMsgWindow, MK_MSG(kC_VSCROLL, kSB_SLIDERTRACK), fPos, 0);
         PositionChanged(fPos);
      }

      // last argument kFALSE forces all specified events to this window
      if (fGrabPointer && !fClient->IsEditable())
         gVirtualX->GrabPointer(fId, kButtonPressMask | kButtonReleaseMask |
                                kPointerMotionMask, kNone, kNone,
                                kTRUE, kFALSE);
   } else {
      fHead->SetState(kButtonUp);
      fTail->SetState(kButtonUp);

      if (fRepeat) {
         fRepeat->Remove();
         fRepeat->SetTime(400);  // might have been shortened in HandleTimer()
      }

      fDragging = kFALSE;

      fPos = TMath::Max(fPos, 0);
      fPos = TMath::Min(fPos, fRange-fPsize);

      SendMessage(fMsgWindow, MK_MSG(kC_VSCROLL, kSB_SLIDERPOS), fPos, 0);
      PositionChanged(fPos);

      if (fGrabPointer)
         gVirtualX->GrabPointer(0, 0, 0, 0, kFALSE);  // ungrab pointer
   }
   return kTRUE;
}

//______________________________________________________________________________
Bool_t TGVScrollBar::HandleMotion(Event_t *event)
{
   // Handle mouse motion in a vertical scrollbar.

   if (fDragging) {
      fX0 = event->fX - fXp;
      fY0 = event->fY - fYp;

      fY0 = TMath::Max(fY0, fgScrollBarWidth);
      fY0 = TMath::Min(fY0, fgScrollBarWidth + fSliderRange);
      fSlider->Move(0, fY0);
      fPos = (fY0 - fgScrollBarWidth) * (fRange-fPsize) / fSliderRange;

      fPos = TMath::Max(fPos, 0);
      fPos = TMath::Min(fPos, fRange-fPsize);

      SendMessage(fMsgWindow, MK_MSG(kC_VSCROLL, kSB_SLIDERTRACK), fPos, 0);
      PositionChanged(fPos);
   }
   return kTRUE;
}

//______________________________________________________________________________
void TGVScrollBar::SetRange(Int_t range, Int_t page_size)
{
   // Set range of vertical scrollbar.

   fRange = TMath::Max(range, 1);
   fPsize = TMath::Max(page_size, 0);

   fSliderSize = TMath::Max(fPsize * (fHeight - (fgScrollBarWidth << 1)) /
                            fRange, (UInt_t) 6);
   fSliderSize = TMath::Min((UInt_t)fSliderSize, fHeight - (fgScrollBarWidth << 1));

   fSliderRange = TMath::Max(fHeight - (fgScrollBarWidth << 1) - fSliderSize,
                             (UInt_t)1);

   fY0 = fgScrollBarWidth + fPos * fSliderRange / TMath::Max(fRange-fPsize, 1);
   fY0 = TMath::Max(fY0, fgScrollBarWidth);
   fY0 = TMath::Min(fY0, fgScrollBarWidth + fSliderRange);

   fSlider->MoveResize(0, fY0, fgScrollBarWidth, fSliderSize);

   //  fPos = (fY0 - fgScrollBarWidth) * (fRange-fPsize) / fSliderRange;

   fPos = TMath::Max(fPos, 0);
   fPos = TMath::Min(fPos, fRange-fPsize);

   SendMessage(fMsgWindow, MK_MSG(kC_VSCROLL, kSB_SLIDERPOS), fPos, 0);
   PositionChanged(fPos);
   RangeChanged(fRange);
   PageSizeChanged(fPsize);
}

//______________________________________________________________________________
void TGVScrollBar::SetPosition(Int_t pos)
{
   // Set logical slider position of vertical scrollbar.

   fPos = TMath::Max(pos, 0);
   fPos = TMath::Min(pos, fRange-fPsize);

   fY0 = fgScrollBarWidth + fPos * fSliderRange / TMath::Max(fRange-fPsize, 1);
   fY0 = TMath::Max(fY0, fgScrollBarWidth);
   fY0 = TMath::Min(fY0, fgScrollBarWidth + fSliderRange);

   fSlider->MoveResize(0, fY0, fgScrollBarWidth, fSliderSize);

   SendMessage(fMsgWindow, MK_MSG(kC_VSCROLL, kSB_SLIDERPOS), fPos, 0);
   PositionChanged(fPos);
}

//______________________________________________________________________________
void TGHScrollBar::SavePrimitive(ostream &out, Option_t *option /*= ""*/)
{
    // Save an horizontal scrollbar as a C++ statement(s) on output stream out.

   if (fBackground != GetDefaultFrameBackground()) SaveUserColor(out, option);

   out <<"   TGHScrollBar *";
   out << GetName() << " = new TGHScrollBar(" << fParent->GetName()
       << "," << GetWidth() << "," << GetHeight();

   if (fBackground == GetDefaultFrameBackground()) {
      if (!GetOptions()) {
         out <<");" << endl;
      } else {
         out << "," << GetOptionString() <<");" << endl;
      }
   } else {
      out << "," << GetOptionString() << ",ucolor);" << endl;
   }

   out << "   " << GetName() <<"->SetRange(" << GetRange() << "," << GetPageSize() << ");" << endl;
   out << "   " << GetName() <<"->SetPosition(" << GetPosition() << ");" << endl;
}

//______________________________________________________________________________
void TGVScrollBar::SavePrimitive(ostream &out, Option_t *option /*= ""*/)
{
    // Save an vertical scrollbar as a C++ statement(s) on output stream out.

   if (fBackground != GetDefaultFrameBackground()) SaveUserColor(out, option);

   out<<"   TGVScrollBar *";
   out << GetName() <<" = new TGVScrollBar("<< fParent->GetName()
       << "," << GetWidth() << "," << GetHeight();

   if (fBackground == GetDefaultFrameBackground()) {

      if (!GetOptions()) {
         out <<");" << endl;
      } else {
         out << "," << GetOptionString() <<");" << endl;
      }
   } else {
      out << "," << GetOptionString() << ",ucolor);" << endl;
   }

   out << "   " << GetName() <<"->SetRange(" << GetRange() << "," << GetPageSize() << ");" << endl;
   out << "   " << GetName() <<"->SetPosition(" << GetPosition() << ");" << endl;
}
