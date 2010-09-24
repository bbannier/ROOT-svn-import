// @(#)root/eve:$Id$
// Author: Matevz Tadel 2007

/*************************************************************************
 * Copyright (C) 1995-2007, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include "TGLAutoRotator.h"

#include "TGLViewer.h"
#include "TGLCamera.h"

#include "TMath.h"
#include "TTimer.h"

//______________________________________________________________________________
//
// Automatically rotates GL camera.
//
// W's are angular velocities.
// Min/MaxTheta is in units of Pi/2
// Min/MaxDolly is a factor of starting camera position.

ClassImp(TGLAutoRotator);

//______________________________________________________________________________
TGLAutoRotator::TGLAutoRotator(TGLViewer* v) :
   fViewer(v),
   fTimer(new TTimer),
   fWPhi(0.2),
   fWTheta(0.1),  fMaxTheta(0.5), fMinTheta(-0.5),
   fWDolly(0.05), fMinDolly(0.5), fMaxDolly(1.5)
{
   // Constructor.

   fTimer->Connect("Timeout()", "TGLAutoRotator", this, "Timeout()");
}

//______________________________________________________________________________
TGLAutoRotator::~TGLAutoRotator()
{
   // Destructor.

   delete fTimer;
}

//==============================================================================

//______________________________________________________________________________
void TGLAutoRotator::Start(Double_t delta_t)
{
   if (fTimerRunning)
   {
      Error("Start", "Already running.");
      return;
   }

   fCamera = & fViewer->CurrentCamera();

   fDt = delta_t;
   fTime = 0;
   fDeltaTheta = 0;

   fTimerRunning = kTRUE;
   fTimer->SetTime(TMath::Nint(1000*delta_t));
   fTimer->Reset();
   fTimer->TurnOn();
}

//______________________________________________________________________________
void TGLAutoRotator::Stop()
{
   if (!fTimerRunning)
   {
      Error("Stop", "Already running.");
      return;
   }

   fTimer->TurnOff();
   fTimerRunning = kFALSE;
}

//______________________________________________________________________________
void TGLAutoRotator::Timeout()
{
   if (!fTimerRunning || gTQSender != fTimer)
   {
      Error("Timeout", "Not running or not called via timer.");
      return;
   }

   fTimer->TurnOff();

   // Double_t dtheta;
   if (fDeltaTheta == 0)
   {

   }
   else
   {

   }

   fViewer->RequestDraw(TGLRnrCtx::kLODHigh);

   fTimer->TurnOn();
}
