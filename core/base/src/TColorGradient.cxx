#include <stdexcept>

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
   if (gROOT) {
      if (gROOT->GetColor(colorIndex)) {
         Error("TColorGradient", "Color with index %d is already defined", colorIndex);
         throw std::runtime_error("Such color is already defined");
      }
      
      if (TObjArray *colors = (TObjArray*)gROOT->GetListOfColors()) {
         const TString colorName(TString::Format("Color%d", colorIndex));
         SetName(colorName);
         fNumber = colorIndex;
         colors->AddAtAndExpand(this, colorIndex);
      } else {
         Error("TColorGradient", "List of colors is a null pointer in gROOT");
         throw std::runtime_error("List of colors is a null pointer");
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
