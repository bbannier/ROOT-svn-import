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
// ATheta -- Theta amplitude in units of Pi/2.
// ADolly -- In/out amplitude in units of initial distance.

ClassImp(TGLAutoRotator);

//______________________________________________________________________________
TGLAutoRotator::TGLAutoRotator(TGLViewer* v) :
   fViewer(v),
   fTimer(new TTimer),
   fDt    (0.02),
   fWPhi  (0.30),
   fWTheta(0.15), fATheta(0.5),
   fWDolly(0.10), fADolly(0.4),
   fTimerRunning(kFALSE)
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
void TGLAutoRotator::SetDt(Double_t dt)
{
   // Set time between two redraws.
   // Range: 0.01 -> 1.

   fDt = TMath::Range(0.01, 1.0, dt);
}

//______________________________________________________________________________
void TGLAutoRotator::SetATheta(Double_t a)
{
   // Set relative amplitude of theta oscilation.
   // Value range: 0.01 -> 1.

   a = TMath::Range(0.01, 1.0, a);
   if (fTimerRunning)
   {
      fThetaA0 = fThetaA0 * a / fATheta;
   }
   fATheta = a;
}

//______________________________________________________________________________
void TGLAutoRotator::SetADolly(Double_t a)
{
   // Set relative amplitude of forward/backward oscilation.
   // Value range: 0.01 -> 1.

  a = TMath::Range(0.01, 1.0, a);
  if (fTimerRunning)
  {
     fDollyA0 = fDollyA0 * a / fADolly;
  }
  fADolly = a;
}

//==============================================================================

//______________________________________________________________________________
void TGLAutoRotator::Start()
{
   if (fTimerRunning)
   {
      Stop();
   }

   fCamera = & fViewer->CurrentCamera();

   fTime = 0;

   fThetaA0 = fATheta * TMath::PiOver2();
   fDollyA0 = fADolly * fCamera->GetCamTrans().GetBaseVec(4).Mag();

   fTimerRunning = kTRUE;
   fTimer->SetTime(TMath::Nint(1000*fDt));
   fTimer->Reset();
   fTimer->TurnOn();
}

//______________________________________________________________________________
void TGLAutoRotator::Stop()
{
   if (fTimerRunning)
   {
      fTimer->TurnOff();
      fTimerRunning = kFALSE;
   }
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

   using namespace TMath;

   fTime += fDt;
   Double_t delta_p = fWPhi*fDt;
   Double_t delta_t = fThetaA0*fWTheta*Cos(fWTheta*fTime)*fDt;
   Double_t delta_d = fDollyA0*fWDolly*Cos(fWDolly*fTime)*fDt;
   Double_t th      = fCamera->GetTheta();

   // printf("t=%f, d_p=%f, d_t=%f, d_d=%f, th=%f\n", fTime, delta_p,
   // delta_t, delta_d, th);

   if (th + delta_t > 3.0 || th + delta_t < 0.1416)
      delta_t = 0;

   fCamera->RotateRad(delta_t, delta_p);
   fCamera->RefCamTrans().MoveLF(1, -delta_d);

   fViewer->RequestDraw(TGLRnrCtx::kLODHigh);

   fTimer->SetTime(TMath::Nint(1000*fDt));
   fTimer->TurnOn();
}
