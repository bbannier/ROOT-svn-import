//Author: Timur Pocheptsov 20/03/2012
#ifndef ROOT_TColorGradient
#define ROOT_TColorGradient

#include <vector>

#ifndef ROOT_Rtypes
#include "Rtypes.h"
#endif

#ifndef ROOT_TColor
#include "TColor.h"
#endif

///////////////////////////////////////////////////////////
//                                                       //
// TColorGradient extends basic TColor.                  //
// Actually, this is not a simple color, but             //
// linear gradient + shadow for filled area.             //
// Inheriting TColor, I can place these additional       //
// attributes inside gROOT's list of colors and          //
// use it for all TAttXXX descendants without            //
// modifying any existing code.                          //
// Shadow, of course, is not a property of any color,    //
// and gradient is not, but this is the best way         //
// to add new                                            //
// attributes to filled area without re-writing          //
// all the graphics code.                                //
// For the moment, this class is not serializable -      //
// you can not save such a fill color for your           //
// histogram.                                            //
//                                                       //
///////////////////////////////////////////////////////////

class TColorGradient : public TColor {
public:
   enum EGradientDirection {
      kGDVertical,
      kGDHorizontal
   };

   typedef std::vector<Color_t>::size_type SizeType_t;

private:

   //At the moment, we have only linear gradient, vertical or horizontal.
   EGradientDirection    fGradientDirection;
   //Positions of color nodes in gradient. Should be normalized : [0, 1].
   std::vector<Double_t> fColorPositions;

   std::vector<Double_t> fColors;//RGBA values.

public:

   //TColorGradient();

   TColorGradient(Color_t newColor, EGradientDirection direction, UInt_t nPoints,
                  const Double_t *points, const Color_t *colorIndices);
   TColorGradient(Color_t newColor, EGradientDirection direction, UInt_t nPoints,
                  const Double_t *points, const Double_t *colors);

   void ResetColor(EGradientDirection direction, UInt_t nPoints, const Double_t *points,
                   const Color_t *colorIndices);
   void ResetColor(EGradientDirection direction, UInt_t nPoints, const Double_t *points,
                   const Double_t *colorIndices);

   EGradientDirection GetGradientDirection()const;
   SizeType_t GetNumberOfSteps()const;
   const Double_t *GetColorPositions()const;
   const Double_t *GetColors()const;
   
private:
   void RegisterColor(Color_t colorIndex);
   
   ClassDef(TColorGradient, 1) //Extended drawing parameters.
};

#endif
