// @(#)root/eve:$Id$
// Author: Matevz Tadel 2007

/*************************************************************************
 * Copyright (C) 1995-2007, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include "TQuakeViz.h"
#include "TEveTrans.h"
#include "TEveRGBAPalette.h"

#include "TVirtualPad.h"
#include "TBuffer3D.h"
#include "TBuffer3DTypes.h"
#include "TVirtualViewer3D.h"

#include "TMath.h"

#include <cstdio>

//______________________________________________________________________________
// Description of TQuakeViz
//

ClassImp(TQuakeViz);

//______________________________________________________________________________
TQuakeViz::TQuakeViz() :
   TEveElementList("QuakeViz", "", kTRUE),
   fTransparency(50),
   fPalette(0),
   fLighting(kTRUE),
   fLimitStrMin  (0), fLimitStrMax  (10),
   fLimitDepthMin(0), fLimitDepthMax(500),
   fLimitRange(kFALSE),
   fYear(2007), fMonth(1), fDay(1), fHour(0),
   fDaysShown(300),  fYearsShown(0),
   fCenterLat(45),   fCenterLon(26),
   fFactorLat(111.1329444), fFactorLon(78.5534),
   fFactorDepth(-1)
{
   // Constructor.

  InitMainTrans();
}

//______________________________________________________________________________
void TQuakeViz::ReadData(const Text_t* file)
{
   fData.clear();

   QData_t q;
   Int_t   year, month, day, hour, min;
   Float_t sec;

   fMinLat = fMinLon = fMinDepth = fMinStr = fMinDist =  1e6;
   fMaxLat = fMaxLon = fMaxDepth = fMaxStr = fMaxDist = -1e6;

   FILE* fp = fopen(file, "r");
   while (fscanf(fp,
                 "%d %d %d %d %d %f "
                 "%f %f %f %f %f",
                 &year, &month, &day, &hour, &min, &sec,
                 &q.fLat, &q.fLon, &q.fDepth, &q.fStr, &q.fDist
                 ) == 11)
   {
      q.fTime.Set(year, month, day, hour, min, TMath::Nint(sec), 0, kTRUE, 0);
      q.fDepth *= fFactorDepth; // Fix of depth-scale.

      if (q.fLat < fMinLat) fMinLat = q.fLat;
      if (q.fLat > fMaxLat) fMaxLat = q.fLat;
      if (q.fLon < fMinLon) fMinLon = q.fLon;
      if (q.fLon > fMaxLon) fMaxLon = q.fLon;
      if (q.fDepth < fMinDepth) fMinDepth = q.fDepth;
      if (q.fDepth > fMaxDepth) fMaxDepth = q.fDepth;
      if (q.fStr < fMinStr) fMinStr = q.fStr;
      if (q.fStr > fMaxStr) fMaxStr = q.fStr;
      if (q.fDist < fMinDist) fMinDist = q.fDist;
      if (q.fDist > fMaxDist) fMaxDist = q.fDist;

      if (q.fLat < 45) printf("foo %d\n", (Int_t) fData.size());

      q.fX = fFactorLon * (q.fLon - fCenterLon);
      q.fY = fFactorLat * (q.fLat - fCenterLat);

      fData.push_back(q);
   }
   fclose(fp);
   printf("Read %d e-quakes.\n", (Int_t) fData.size());
   fMinTime = fData.front().fTime;
   fMaxTime = fData.back() .fTime;
   printf("  First data-point: %s\n", fMinTime.AsString());
   printf("  Last  data-point: %s\n", fMaxTime.AsString());
   printf("  D_lat = %.2f, D_lon = %.2f, D_depth = %.2f, D_str = %.2f\n",
          fMaxLat - fMinLat, fMaxLon - fMinLon, fMaxDepth - fMinDepth, fMaxStr - fMinStr);

   fLimitStrMin   = fMinStr;
   fLimitStrMax   = fMaxStr;
   fLimitDepthMin = fMinDepth;
   fLimitDepthMax = fMaxDepth;
}

//______________________________________________________________________________
void TQuakeViz::InitVizState(Int_t dayHalfRange)
{
   // Init visualization state.
   // Set the central date to the time of the last e-quake and
   // set observation window to dayHalfRange in each direction.

   UInt_t y, m, d, h;

   fMinTime.GetDate(kFALSE, 0, &y, &m, &d);
   fMinTime.GetTime(kFALSE, 0, &h, 0, 0); // No use minutes/secs
   fYear  = y;
   fMonth = m;
   fDay   = d;
   fHour  = h;
   fDaysShown = dayHalfRange;
}

/******************************************************************************/

//______________________________________________________________________________
void TQuakeViz::ComputeBBox()
{
   // Compute bounding-box of the data.

   if (fData.empty())
   {
      BBoxZero();
      return;
   }

   BBoxInit();
   for (vQData_i i = fData.begin(); i != fData.end(); ++i)
   {
      BBoxCheckPoint(i->fX, i->fY, i->fDepth);
   }
}

//______________________________________________________________________________
void TQuakeViz::Paint(Option_t* /*option*/)
{
   // Paint point-set.

   static const TEveException eh("TQuakeViz::Paint ");

   if (fRnrSelf == kFALSE) return;

   TBuffer3D buff(TBuffer3DTypes::kGeneric);

   // Section kCore
   buff.fID           = this;
   buff.fColor        = GetMainColor();
   buff.fTransparency = GetMainTransparency();
   if (HasMainTrans())
      RefMainTrans().SetBuffer3D(buff);
   buff.SetSectionsValid(TBuffer3D::kCore);

   Int_t reqSections = gPad->GetViewer3D()->AddObject(buff);
   if (reqSections != TBuffer3D::kNone)
      Error(eh, "only direct GL rendering supported.");
}

/******************************************************************************/

//______________________________________________________________________________
void TQuakeViz::SetPalette(TEveRGBAPalette* p)
{
   // Set TEveRGBAPalette pointer.

   if (fPalette == p) return;
   if (fPalette) fPalette->DecRefCount();
   fPalette = p;
   if (fPalette) fPalette->IncRefCount();
}

//______________________________________________________________________________
TEveRGBAPalette* TQuakeViz::AssertPalette()
{
   // Make sure the TEveRGBAPalette pointer is not null.
   // If it is not set, a new one is instantiated and the range is set
   // to 0 - 100, as expected by the GL renderer.

   if (fPalette == 0) {
      fPalette = new TEveRGBAPalette(0, 100, kTRUE);
   }
   return fPalette;
}

Long64_t TQuakeViz::GetLimitTimeDelta()
{
  return 24ll*3600*(fDaysShown + 365*fYearsShown);
}

TTimeStamp64 TQuakeViz::GetLimitTimeMean()
{
  TTimeStamp64 t;
  t.SetSec(GetLimitTimeMin().GetSec()/2 + GetLimitTimeMax().GetSec()/2);
  return t;
}

TTimeStamp64 TQuakeViz::GetLimitTimeMin()
{
  return TTimeStamp64(fYear, fMonth, fDay, fHour, 0, 0);
}

TTimeStamp64 TQuakeViz::GetLimitTimeMax()
{
  TTimeStamp64 t;
  t.SetSec(GetLimitTimeMin().GetSec() + GetLimitTimeDelta());
  if (t < fMinTime || t > fMaxTime)
    t = fMaxTime;
  return t;
}

//==============================================================================
//==============================================================================

void TQuakeViz::QData_t::Print() const
{
   printf("Time: %s\n"
          "Lat=%.2f, Lon=%.2f, Depth=%.1f, Magnitude=%.1f, Dist=%.1f\n"
          "X=%.3f, Y=%.3f\n",
          fTime.AsString("s"), fLat, fLon, fDepth, fStr, fDist,
          fX, fY);
}
