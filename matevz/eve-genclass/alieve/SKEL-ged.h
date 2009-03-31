// $Id$
// Author: Matevz Tadel 2009

/**************************************************************************
 * Copyright(c) 1998-2009, ALICE Experiment at CERN, all rights reserved. *
 * See http://aliceinfo.cern.ch/Offline/AliRoot/License.html for          *
 * full copyright notice.                                                 *
 **************************************************************************/

#ifndef CLASS_H
#define CLASS_H

#include "TGedFrame.h"

class TGButton;
class TGCheckButton;
class TGNumberEntry;
class TGColorSelect;

class STEM;

//______________________________________________________________________________
// Short description of CLASS
//

class CLASS : public TGedFrame
{
public:
   CLASS(const TGWindow* p=0, Int_t width=170, Int_t height=30,
         UInt_t options=kChildFrame, Pixel_t back=GetDefaultFrameBackground());
   virtual ~CLASS() {}

   virtual void SetModel(TObject* obj);

   // Declare callback/slot methods
   // void DoXYZZ();

protected:
   STEM            *fM; // Model object.

   // Declare widgets
   // TGSomeWidget*   fXYZZ;

private:
   CLASS(const CLASS&);            // Not implemented
   CLASS& operator=(const CLASS&); // Not implemented

   ClassDef(CLASS, 0); // GUI editor for STEM.
};

#endif
