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
#include "TGSplitter.h"
#include "TGSplitFrame.h"
#include "TString.h"
#include "Riostream.h"

ClassImp(TGSplitFrame)

//______________________________________________________________________________
TGSplitFrame::TGSplitFrame(const TGWindow *p, UInt_t w, UInt_t h,
        UInt_t options) : TGCompositeFrame(p, w, h, options), 
        fFrame(0), fSplitter(0), fFirst(0), fSecond(0)
{
   // Default constructor.

}

//______________________________________________________________________________
TGSplitFrame::~TGSplitFrame()
{
   // Destructor. Make cleanup.

   Cleanup();
}

//______________________________________________________________________________
void TGSplitFrame::AddFrame(TGFrame *f, TGLayoutHints *l)
{
   TGCompositeFrame::AddFrame(f, l);
   fFrame = f;
}

//______________________________________________________________________________
void TGSplitFrame::Cleanup()
{
   // recursively cleanup child frames.

   if (fFirst) {
      fFirst->Cleanup();
      delete fFirst;
      fFirst = 0;
   }
   if (fSecond) {
      fSecond->Cleanup();
      delete fSecond;
      fSecond = 0;
   }
   if (fSplitter) {
      delete fSplitter;
      fSplitter = 0;
   }
}

//______________________________________________________________________________
void TGSplitFrame::HSplit(UInt_t h)
{
   // Horizontally split the frame.

   // return if already splitted
   if ((fSplitter != 0) || (fFirst != 0) || (fSecond != 0) || (fFrame != 0))
      return;
   UInt_t height = (h > 0) ? h : fHeight/2;
   // set correct option (vertical frame)
   ChangeOptions((GetOptions() & ~kHorizontalFrame) | kVerticalFrame);
   // create first split frame with fixed height - required for the splitter
   fFirst = new TGSplitFrame(this, fWidth, height, kSunkenFrame | kFixedHeight);
   // create second split frame
   fSecond = new TGSplitFrame(this, fWidth, height, kSunkenFrame);
   // create horizontal splitter
   fSplitter = new TGHSplitter(this, 4, 4);
   // set the splitter's frame to the first one
   fSplitter->SetFrame(fFirst, kTRUE);
   // add all frames
   AddFrame(fFirst, new TGLayoutHints(kLHintsExpandX));
   AddFrame(fSplitter, new TGLayoutHints(kLHintsLeft | kLHintsTop | 
            kLHintsExpandX));
   AddFrame(fSecond, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
}

//______________________________________________________________________________
void TGSplitFrame::VSplit(UInt_t w)
{
   // Vertically split the frame.

   // return if already splitted
   if ((fSplitter != 0) || (fFirst != 0) || (fSecond != 0) || (fFrame != 0))
      return;
   UInt_t width = (w > 0) ? w : fWidth/2;
   // set correct option (horizontal frame)
   ChangeOptions((GetOptions() & ~kVerticalFrame) | kHorizontalFrame);
   // create first split frame with fixed width - required for the splitter
   fFirst = new TGSplitFrame(this, width, fHeight, kSunkenFrame | kFixedWidth);
   // create second split frame
   fSecond = new TGSplitFrame(this, width, fHeight, kSunkenFrame);
   // create vertical splitter
   fSplitter = new TGVSplitter(this, 4, 4);
   // set the splitter's frame to the first one
   fSplitter->SetFrame(fFirst, kTRUE);
   // add all frames
   AddFrame(fFirst, new TGLayoutHints(kLHintsExpandY));
   AddFrame(fSplitter, new TGLayoutHints(kLHintsLeft | kLHintsTop | 
            kLHintsExpandY));
   AddFrame(fSecond, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
}

//______________________________________________________________________________
void TGSplitFrame::SavePrimitive(ostream &out, Option_t *option /*= ""*/)
{
   // Save a splittable frame as a C++ statement(s) on output stream out.

   if (fBackground != GetDefaultFrameBackground()) SaveUserColor(out, option);

   out << endl << "   // splittable frame" << endl;
   out << "   TGSplitFrame *";
   out << GetName() << " = new TGSplitFrame(" << fParent->GetName()
       << "," << GetWidth() << "," << GetHeight();

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
