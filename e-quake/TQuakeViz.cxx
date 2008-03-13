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
   TEveElementList("QuakeViz", "", kTRUE)
{
   // Constructor.
}

void TQuakeViz::ReadData(const Text_t* file)
{
   fData.clear();

   QData_t q;
   Int_t   year, month, day, hour, min;
   Float_t sec;

   fMinLat = fMinLon = fMinDepth = fMinStr =  1e6;
   fMaxLat = fMaxLon = fMaxDepth = fMaxStr = -1e6;

   FILE* fp = fopen(file, "r");
   while (fscanf(fp,
                 "%d %d %d %d %d %f "
                 "%f %f %f %f",
                 &year, &month, &day, &hour, &min, &sec,
                 &q.fLat, &q.fLon, &q.fDepth, &q.fStr
                 ) == 10)
   {
      q.fTime.Set(year, month, day, hour, min, TMath::Nint(sec), 0, kFALSE, 0);
      q.fDepth *= 0.01f; // Brutal fix of depth-scale.

      if (q.fLat < fMinLat) fMinLat = q.fLat;
      if (q.fLat > fMaxLat) fMaxLat = q.fLat;
      if (q.fLon < fMinLon) fMinLon = q.fLon;
      if (q.fLon > fMaxLon) fMaxLon = q.fLon;
      if (q.fDepth < fMinDepth) fMinDepth = q.fDepth;
      if (q.fDepth > fMaxDepth) fMaxDepth = q.fDepth;
      if (q.fStr < fMinStr) fMinStr = q.fStr;
      if (q.fStr > fMaxStr) fMaxStr = q.fStr;

      if (q.fLat < 45) printf("foo %d\n", (Int_t) fData.size());

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
}

/******************************************************************************/

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
      BBoxCheckPoint(i->fLat, i->fLon, i->fDepth);
   }
}

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

