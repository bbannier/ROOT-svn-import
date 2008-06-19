// @(#)root/eve:$Id$
// Author: Matevz Tadel 2007

/*************************************************************************
 * Copyright (C) 1995-2007, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TQuakeViz
#define ROOT_TQuakeViz

#include "TEveElement.h"
#include "TAttBBox.h"

#include "TTimeStamp64.h"

#include <vector>

class TEveRGBAPalette;

class TQuakeViz : public TEveElementList,
		  public TAttBBox
{
   friend class TQuakeVizEditor;
   friend class TQuakeVizGL;

private:
   TQuakeViz(const TQuakeViz&);            // Not implemented
   TQuakeViz& operator=(const TQuakeViz&); // Not implemented

public:
   struct QData_t
   {
      TTimeStamp64 fTime;
      Float_t    fLat;
      Float_t    fLon;
      Float_t    fDepth;
      Float_t    fStr;
      Float_t    fDist;
      Float_t    fX, fY;

      QData_t() : fDist(0), fX(0), fY(0) {}

      void Print() const;
   };

   typedef std::vector<QData_t>           vQData_t;
   typedef std::vector<QData_t>::iterator vQData_i;

protected:
   vQData_t   fData;

   TTimeStamp64 fMinTime;
   TTimeStamp64 fMaxTime;
   Float_t    fMinLat,   fMaxLat;
   Float_t    fMinLon,   fMaxLon;
   Float_t    fMinDepth, fMaxDepth;
   Float_t    fMinStr,   fMaxStr;
   Float_t    fMinDist,  fMaxDist;  

   UChar_t    fTransparency;

   TEveRGBAPalette *fPalette;        // Pointer to signal-color palette.

   Bool_t           fLighting;

   Float_t          fLimitStrMin;
   Float_t          fLimitStrMax;
   Float_t          fLimitDepthMin;
   Float_t          fLimitDepthMax;

   Bool_t           fLimitRange;
   Int_t            fYear;
   Int_t            fMonth;
   Int_t            fDay;
   Int_t            fHour;
   Int_t            fDaysShown;
   Int_t            fYearsShown;

   Float_t          fCenterLat, fCenterLon;
   Float_t          fFactorLat, fFactorLon;
   Float_t          fFactorDepth;

public:
   TQuakeViz();
   virtual ~TQuakeViz() {}

   void ReadData(const Text_t* file);

   Bool_t AcceptForDraw(const QData_t& q) const
   { return (q.fDepth >= fLimitDepthMin && q.fDepth <= fLimitDepthMax) &&
         (q.fStr >= fLimitStrMin && q.fStr <= fLimitStrMax); }

   void InitVizState(Int_t dayHalfRange=100);

   virtual Bool_t  CanEditMainTransparency() const { return kTRUE; }
   virtual UChar_t GetMainTransparency()     const { return fTransparency; }
   virtual void    SetMainTransparency(UChar_t t)  { fTransparency = t; }

   virtual void ComputeBBox();
   virtual void Paint(Option_t* option="");

   TEveRGBAPalette* GetPalette() const { return fPalette; }
   void             SetPalette(TEveRGBAPalette* p);
   TEveRGBAPalette* AssertPalette();

   Bool_t GetLighting() const { return fLighting; }
   void SetLighting(Bool_t x) { fLighting = x; }

   Float_t GetLimitStrMin() const { return fLimitStrMin; }
   Float_t GetLimitStrMax() const { return fLimitStrMax; }
   void SetLimitStrMin(Float_t x) { fLimitStrMin = x; }
   void SetLimitStrMax(Float_t x) { fLimitStrMax = x; }
   void SetLimitStr(Float_t min, Float_t max)
   { fLimitStrMin = min; fLimitStrMax = max; }

   Float_t GetLimitDepthMin() const { return fLimitDepthMin; }
   Float_t GetLimitDepthMax() const { return fLimitDepthMax; }
   void SetLimitDepthMin(Float_t x) { fLimitDepthMin = x; }
   void SetLimitDepthMax(Float_t x) { fLimitDepthMax = x; }
   void SetLimitDepth(Float_t min, Float_t max)
   { fLimitDepthMin = min; fLimitDepthMax = max; }

   Bool_t GetLimitRange() const { return fLimitRange; }
   void SetLimitRange(Bool_t x) { fLimitRange = x; }

   Int_t GetYear() const { return fYear; }
   void SetYear(Int_t x) { fYear = x; }
   Int_t GetMonth() const { return fMonth; }
   void SetMonth(Int_t x) { fMonth = x; }
   Int_t GetDay() const { return fDay; }
   void SetDay(Int_t x) { fDay = x; }
   Int_t GetHour() const { return fHour; }
   void SetHour(Int_t x) { fHour = x; }
   Int_t GetDaysShown() const { return fDaysShown; }
   void SetDaysShown(Int_t x) { fDaysShown = x; }
   Int_t GetYearsShown() const { return fYearsShown; }
   void SetYearsShown(Int_t x) { fYearsShown = x; }

   Long64_t   GetLimitTimeDelta();
   TTimeStamp64 GetLimitTimeMean();
   TTimeStamp64 GetLimitTimeMin();
   TTimeStamp64 GetLimitTimeMax();

   ClassDef(TQuakeViz, 1); // Short description.
};

#endif
