// @(#)root/eve:$Id$
// Author: Matevz Tadel 2007

/*************************************************************************
 * Copyright (C) 1995-2007, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_CLASS
#define ROOT_CLASS

#include "TGedFrame.h"

class TGButton;
class TGCheckButton;
class TGNumberEntry;
class TGColorSelect;

class STEM;

class XXCLASS : public TGVerticalFrame
{
private:
   XXCLASS(const XXCLASS&);            // Not implemented
   XXCLASS& operator=(const XXCLASS&); // Not implemented

protected:
   STEM            *fM;  // Model object.

public:
   XXCLASS(const TGWindow* p);
   virtual ~XXCLASS() {}

   void SetModel(STEM* m);

   void Changed(); //*SIGNAL*

   // void DoABCD();

   ClassDef(XXCLASS, 0); // GUI sub-editor for STEM.
};


class CLASS : public TGedFrame
{
private:
   CLASS(const CLASS&);            // Not implemented
   CLASS& operator=(const CLASS&); // Not implemented

protected:
   STEM            *fM;  // Model object.
   XXCLASS         *fSE; // Sub-editor, the actual GUI.

public:
   CLASS(const TGWindow* p=0, Int_t width=170, Int_t height=30,
         UInt_t options=kChildFrame, Pixel_t back=GetDefaultFrameBackground());
   virtual ~CLASS() {}

   virtual void SetModel(TObject* obj);

   void DoXYZZ();

   ClassDef(CLASS, 0); // GUI editor for STEM.
};

#endif
