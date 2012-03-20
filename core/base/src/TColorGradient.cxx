#include "TColorGradient.h"
#include "TObjArray.h"
#include "TString.h"
#include "TError.h"
#include "TROOT.h"

ClassImp(TColorGradient)

//______________________________________________________________________________
TColorGradient::TColorGradient(Color_t colorIndex, EGradientDirection dir, UInt_t nPoints, const Double_t *points,
                               const Color_t *indices, Bool_t hasShadow)
                   : fGradientDirection(dir),
                     fColorPositions(points, points + nPoints),
                     fColors(indices, indices + nPoints),
                     fHasShadow(hasShadow)
{
   //I have no way to validate parameters here, so it's up to user
   //to pass correct arguments.
   fNumber = colorIndex;
   SetName(TString::Format("Color%d", colorIndex));

   if (gROOT) {
      if (gROOT->GetColor(colorIndex)) {
         Warning("TColorGradient", "Color with index %d is already defined", colorIndex);
         return;
      }
      
      if (TObjArray *colors = (TObjArray*)gROOT->GetListOfColors()) {
         colors->AddAtAndExpand(this, colorIndex);
      } else {
         Error("TColorGradient", "List of colors is a null pointer in gROOT, color was not registered");
         return;
      }
   }
}

//______________________________________________________________________________
TColorGradient::EGradientDirection TColorGradient::GetGradientDirection()const
{
   //
   return fGradientDirection;
}

//______________________________________________________________________________
TColorGradient::SizeType_t TColorGradient::GetNumberOfSteps()const
{
   //
   return fColors.size();
}

//______________________________________________________________________________
const Double_t *TColorGradient::GetColorPositions()const
{
   //
   return &fColorPositions[0];
}

//______________________________________________________________________________
const Color_t *TColorGradient::GetColors()const
{
   //
   return &fColors[0];
}

//______________________________________________________________________________
Bool_t TColorGradient::HasShadow()const
{
   //
   return fHasShadow;
}
