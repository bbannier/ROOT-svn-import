// @(#)root/eve:$Id$
// Author: Matevz Tadel 2007

/*************************************************************************
 * Copyright (C) 1995-2007, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TQuakeVizEditor
#define ROOT_TQuakeVizEditor

#include "TGedFrame.h"

class TGButton;
class TGCheckButton;
class TGNumberEntry;
class TGColorSelect;
class TEveGValuator;
class TEveGDoubleValuator;

class TQuakeViz;

class TQuakeVizEditor : public TGedFrame
{
private:
   TQuakeVizEditor(const TQuakeVizEditor&);            // Not implemented
   TQuakeVizEditor& operator=(const TQuakeVizEditor&); // Not implemented

protected:
   TQuakeViz             *fM; // Model object.

   TGCheckButton         *fLighting;

   TEveGDoubleValuator   *fLimitStr;
   TEveGDoubleValuator   *fLimitDepth;

   TGCheckButton         *fLimitRange;
   TEveGValuator         *fYear;
   TEveGValuator         *fMonth;
   TEveGValuator         *fDay;
   TEveGValuator         *fHour;
   TEveGValuator         *fDayHalfRange;

public:
   TQuakeVizEditor(const TGWindow* p=0, Int_t width=170, Int_t height=30,
         UInt_t options=kChildFrame, Pixel_t back=GetDefaultFrameBackground());
   virtual ~TQuakeVizEditor() {}

   virtual void SetModel(TObject* obj);

   void DoLighting();

   void DoLimitStr();
   void DoLimitDepth();

   void DoLimitRange();
   void DoYear();
   void DoMonth();
   void DoDay();
   void DoHour();
   void DoDayHalfRange();

   ClassDef(TQuakeVizEditor, 0); // GUI editor for TQuakeViz.
};

#endif
