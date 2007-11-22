// @(#)root/reve:$Id$
// Authors: Matevz Tadel & Alja Mrak-Tadel: 2006, 2007

/*************************************************************************
 * Copyright (C) 1995-2007, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TEveTrackRnrStyleEditor
#define ROOT_TEveTrackRnrStyleEditor

#include <TGedFrame.h>

class TGButton;
class TGCheckButton;
class TGNumberEntry;
class TGColorSelect;
class TGComboBox;
class TGLineWidthComboBox;
class TGLineStyleComboBox;

class TAttMarkerEditor;

class TEveTrackRnrStyle;

class TEveGValuator;
class TEveGDoubleValuator;
class TEveTrackRnrStyleSubEditor;

class TEveTrackRnrStyleSubEditor : public TGVerticalFrame
{
  friend class TEveTrackRnrStyleEditor;
  friend class TEveTrackListEditor;

private:
  TEveTrackRnrStyleSubEditor(const TEveTrackRnrStyleSubEditor&);            // Not implemented
  TEveTrackRnrStyleSubEditor& operator=(const TEveTrackRnrStyleSubEditor&); // Not implemented

protected:
  TEveTrackRnrStyle  *fM;

  TEveGValuator      *fMaxR;
  TEveGValuator      *fMaxZ;
  TEveGValuator      *fMaxOrbits;
  TEveGValuator      *fMinAng;
  TEveGValuator      *fDelta;

  TGCheckButton      *fRnrFV;

  TGCompositeFrame   *fPMFrame;
  TGButton           *fFitDaughters;
  TGButton           *fFitReferences;
  TGButton           *fFitDecay;
  TGButton           *fRnrDaughters;
  TGButton           *fRnrReferences;
  TGButton           *fRnrDecay;

  TGCompositeFrame   *fRefsCont;

  TAttMarkerEditor   *fPMAtt;
  TAttMarkerEditor   *fFVAtt;

public:
  TEveTrackRnrStyleSubEditor(const TGWindow* p);
  virtual ~TEveTrackRnrStyleSubEditor() {}

  void SetModel(TEveTrackRnrStyle* m);

  void Changed(); //*SIGNAL*

  void DoMaxR();
  void DoMaxZ();
  void DoMaxOrbits();
  void DoMinAng();
  void DoDelta();

  void DoFitPM();
  void DoRnrPM();

  void DoRnrFV();

  void CreateRefsContainer(TGVerticalFrame* p);

  ClassDef(TEveTrackRnrStyleSubEditor, 0) // Sub-editor for TEveTrackRnrStyle class.
};

/******************************************************************************/
// TEveTrackRnrStyleEditor
/******************************************************************************/

class TEveTrackRnrStyleEditor : public TGedFrame
{
private:
  TEveTrackRnrStyleEditor(const TEveTrackRnrStyleEditor&);            // Not implemented
  TEveTrackRnrStyleEditor& operator=(const TEveTrackRnrStyleEditor&); // Not implemented

  void CreateRefTab();
protected:
  TEveTrackRnrStyle           *fM;           // Model object.
  TEveTrackRnrStyleSubEditor  *fRSSubEditor; // Render-style sub-editor.

public:
  TEveTrackRnrStyleEditor(const TGWindow* p=0, Int_t width=170, Int_t height=30,
                  UInt_t options=kChildFrame, Pixel_t back=GetDefaultFrameBackground());
  ~TEveTrackRnrStyleEditor();

  virtual void SetModel(TObject* obj);

  ClassDef(TEveTrackRnrStyleEditor, 1); // Editor for TEveTrackRnrStyle class.
};

#endif
