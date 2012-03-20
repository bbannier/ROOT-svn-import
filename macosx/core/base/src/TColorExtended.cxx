#include <stdexcept>

#include "TColorExtended.h"
#include "TObjArray.h"
#include "TString.h"
#include "TError.h"
#include "TROOT.h"

ClassImp(TColorExtended)

//______________________________________________________________________________
TColorExtended::TColorExtended(Color_t colorIndex, EGradientDirection dir, UInt_t nPoints, const Double_t *points,
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
         Error("TColorExtended", "Color with index %d is already defined", colorIndex);
         throw std::runtime_error("Such color is already defined");
      }
      
      if (TObjArray *colors = (TObjArray*)gROOT->GetListOfColors()) {
         const TString colorName(TString::Format("Color%d", colorIndex));
         SetName(colorName.Data());

         colors->AddAtAndExpand(this, colorIndex);
      } else {
         Error("TColorExtended", "List of colors is a null pointer in gROOT");
         throw std::runtime_error("List of colors is a null pointer");
      }
   }
}

//______________________________________________________________________________
TColorExtended::EGradientDirection TColorExtended::GetGradientDirection()const
{
   //
   return fGradientDirection;
}

//______________________________________________________________________________
TColorExtended::SizeType_t TColorExtended::GetNumberOfSteps()const
{
   //
   return fColors.size();
}

//______________________________________________________________________________
const Double_t *TColorExtended::GetColorPositions()const
{
   //
   return &fColorPositions[0];
}

//______________________________________________________________________________
const Color_t *TColorExtended::GetColors()const
{
   //
   return &fColors[0];
}

//______________________________________________________________________________
Bool_t TColorExtended::HasShadow()const
{
   //
   return fHasShadow;
}
