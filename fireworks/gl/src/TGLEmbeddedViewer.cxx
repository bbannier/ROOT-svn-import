// @(#)root/gl:$Id$
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
#include "TGLWidget.h"
#include "TGLSAFrame.h"
#include "TString.h"
#include "TGLPShapeObj.h"
#include "TGLPhysicalShape.h"
#include "TGLLogicalShape.h"
#include "TGLEmbeddedViewer.h"

ClassImp(TGLEmbeddedViewer)

//______________________________________________________________________________
TGLEmbeddedViewer::TGLEmbeddedViewer(const TGWindow *parent, TVirtualPad *pad) :
   TGLViewer(pad, 0, 0, 400, 300),
   fFrame(0),
   fGLArea(0)
{
   // Default constructor;

   fFrame = new TGCompositeFrame(parent);

   CreateFrames();

   fFrame->MapSubwindows();
   fFrame->Resize(fFrame->GetDefaultSize());
   fFrame->Resize(400, 300);
}

//______________________________________________________________________________
TGLEmbeddedViewer::~TGLEmbeddedViewer()
{
   // Destroy standalone viewer object.

//   delete fGLArea;
   delete fFrame;
}

//______________________________________________________________________________
void TGLEmbeddedViewer::CreateFrames()
{
   // Internal frames creation.

   fGLWindow = new TGLWidget(*fFrame, kTRUE, 10, 10, 0);
   // Direct events from the TGWindow directly to the base viewer

   fGLWindow->Connect("HandleButton(Event_t*)", "TGLViewer", this, 
                      "HandleButton(Event_t*)");
   fGLWindow->Connect("HandleDoubleClick(Event_t*)", "TGLViewer", this, 
                      "HandleDoubleClick(Event_t*)");
   fGLWindow->Connect("HandleKey(Event_t*)", "TGLViewer", this, 
                      "HandleKey(Event_t*)");
   fGLWindow->Connect("HandleMotion(Event_t*)", "TGLViewer", this, 
                      "HandleMotion(Event_t*)");
   fGLWindow->Connect("Repaint()", "TGLViewer", this, "Repaint()");
   fGLWindow->Connect("HandleConfigureNotify(Event_t*)", "TGLViewer", this, 
                      "HandleConfigureNotify(Event_t*)");
   fGLWindow->Connect("HandleFocusChange(Event_t*)", "TGLViewer", this, 
                      "HandleFocusChange(Event_t*)");
   fGLWindow->Connect("HandleCrossing(Event_t*)", "TGLViewer", this, 
                      "HandleCrossing(Event_t*)");

   fFrame->AddFrame(fGLWindow, new TGLayoutHints(kLHintsExpandX | 
                    kLHintsExpandY, 2, 2, 2, 2));
}

