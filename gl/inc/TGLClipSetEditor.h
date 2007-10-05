// @(#)root/gl:$Id$
// Author:  Matevz Tadel, Jun 2007

/*************************************************************************
 * Copyright (C) 1995-2004, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TGLClipSetEditor
#define ROOT_TGLClipSetEditor

#ifndef ROOT_TGedFrame
#include "TGedFrame.h"
#endif
#ifndef ROOT_TGLUtil
#include "TGLUtil.h"
#endif

class TGButton;
class TGCheckButton;
class TGNumberEntry;
class TGButtonGroup;
class TGLClipSet;


class TGLClipSetSubEditor : public TGVerticalFrame {

private:
   TGLClipSetSubEditor(const TGLClipSetSubEditor&);            // Not implemented
   TGLClipSetSubEditor& operator=(const TGLClipSetSubEditor&); // Not implemented

protected:
   TGLClipSet       *fM;

   EClipType         fCurrentClip;
   TGButtonGroup    *fTypeButtons;

   TGCompositeFrame *fPlanePropFrame;
   TGNumberEntry    *fPlaneProp[4];

   TGCompositeFrame *fBoxPropFrame;
   TGNumberEntry    *fBoxProp[6];
   TGCheckButton    *fClipInside;
   TGCheckButton    *fClipEdit;
   TGCheckButton    *fClipShow;
   TGButton         *fApplyButton;

public:
   TGLClipSetSubEditor(const TGWindow* p);
   virtual ~TGLClipSetSubEditor() {}

   void SetModel(TGLClipSet* m);

   void Changed(); //*SIGNAL*

   //Clipping manipulation
   void ClipValueChanged();
   void ClipTypeChanged(Int_t);
   void UpdateViewerClip();

   ClassDef(TGLClipSetSubEditor, 0) // Sub-editor for TGLClipSet
};


class TGLClipSetEditor : public TGedFrame {

private:
   TGLClipSetEditor(const TGLClipSetEditor&);            // Not implemented
   TGLClipSetEditor& operator=(const TGLClipSetEditor&); // Not implemented

protected:
   TGLClipSet           *fM;  // fModel dynamic-casted to TGLClipSet
   TGLClipSetSubEditor  *fSE;

public:
   TGLClipSetEditor(const TGWindow* p=0, Int_t width=170, Int_t height=30, UInt_t options=kChildFrame, Pixel_t back=GetDefaultFrameBackground());
   virtual ~TGLClipSetEditor() {}

   virtual void SetModel(TObject* obj);

   ClassDef(TGLClipSetEditor, 0) // Editor for TGLClipSet
};

#endif
