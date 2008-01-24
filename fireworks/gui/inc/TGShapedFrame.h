// @(#)root/gui:$Id$
// Author: Bertrand Bellenot 23/01/2008

/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TGShapedFrame
#define ROOT_TGShapedFrame

#ifndef ROOT_TGFrame
#include "TGFrame.h"
#endif

#ifndef ROOT_TString
#include "TString.h"
#endif

#ifndef ROOT_TImage
#include "TImage.h"
#endif

#ifndef ROOT_TGPicture
#include "TGPicture.h"
#endif

class TRootEmbeddedCanvas;
class TString;
class TH1;

class TGShapedFrame : public TGCompositeFrame {

private:
   TGShapedFrame(const TGShapedFrame&); // Not implemented
   TGShapedFrame& operator=(const TGShapedFrame&); // Not implemented

protected:
   const TGPicture      *fBgnd;     // picture used as background/shape
   TImage               *fImage;    // image used as background/shape
   virtual void          DoRedraw() {}

public:
   TGShapedFrame(const char *fname=0, const TGWindow *p=0, UInt_t w=1, UInt_t h=1, UInt_t options=0);
   virtual ~TGShapedFrame();

   const TGPicture   GetPicture() const { return *fBgnd; }
   TImage            GetImage() const { return *fImage; }

   virtual void      SavePrimitive(ostream &out, Option_t *option = "");

   ClassDef(TGShapedFrame, 0) // Shaped composite frame
};

class TGShapedToolTip : public TGShapedFrame {

private:
   TGShapedToolTip(const TGShapedToolTip&); // Not implemented
   TGShapedToolTip& operator=(const TGShapedToolTip&); // Not implemented

protected:
   Int_t                 fTextX, fTextY, fTextH;
   TString               fTextCol;

   TRootEmbeddedCanvas  *fEc;       // embedded canvas for histogram
   TH1                  *fHist;     // user histogram
   TString               fText;     // info (as tool tip) text

   virtual void          DoRedraw() {}

public:
   TGShapedToolTip(const char *picname, Int_t cx=0, Int_t cy=0, Int_t cw=0, 
                   Int_t ch=0, Int_t tx=0, Int_t ty=0, Int_t th=0, 
                   const char *col="#ffffff");
   virtual ~TGShapedToolTip();

   virtual void   CloseWindow();
   void           CreateCanvas(Int_t cx, Int_t cy, Int_t cw, Int_t ch);
   void           CreateCanvas(Int_t cw, Int_t ch, TGLayoutHints *hints);
   TH1           *GetHisto() const { return fHist; }
   const char    *GetText() const { return fText.Data(); }
   void           Refresh();
   void           SetHisto(TH1 *hist);
   void           SetText(const char *text);
   void           SetTextColor(const char *col);
   void           SetTextAttributes(Int_t tx, Int_t ty, Int_t th, const char *col=0);
   void           Show(Int_t x, Int_t y, const char *text = 0, TH1 *hist = 0);

   ClassDef(TGShapedToolTip, 0) // Shaped composite frame
};

#endif
