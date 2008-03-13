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

#include "TTimeStamp.h"

#include <vector>

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
      TTimeStamp fTime;
      Float_t    fLat;
      Float_t    fLon;
      Float_t    fDepth;
      Float_t    fStr;
   };

   typedef std::vector<QData_t>           vQData_t;
   typedef std::vector<QData_t>::iterator vQData_i;

protected:
   vQData_t   fData;
   TTimeStamp fMinTime;
   TTimeStamp fMaxTime;
   Float_t    fMinLat,   fMaxLat,   fScaleLat;
   Float_t    fMinLon,   fMaxLon,   fScaleLon;
   Float_t    fMinDepth, fMaxDepth, fScaleDepth;
   Float_t    fMinStr,   fMaxStr,   fScaleStr;

   UChar_t    fTransparency;

public:
   TQuakeViz();
   virtual ~TQuakeViz() {}

   void ReadData(const Text_t* file);

   virtual Bool_t  CanEditMainTransparency() const { return kTRUE; }
   virtual UChar_t GetMainTransparency()     const { return fTransparency; }
   virtual void    SetMainTransparency(UChar_t t)  { fTransparency = t; }

   virtual void ComputeBBox();
   virtual void Paint(Option_t* option="");

   ClassDef(TQuakeViz, 1); // Short description.
};

#endif
