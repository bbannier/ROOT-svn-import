//Author: Timur Pocheptsov 20/03/2012
#ifndef ROOT_TColorExtended
#define ROOT_TColorExtended

#include <vector>

#ifndef ROOT_Rtypes
#include "Rtypes.h"
#endif

#ifndef ROOT_TColor
#include "TColor.h"
#endif

///////////////////////////////////////////////////////////
//                                                       //
// TColorExtended extends basic TColor.                  //
// Actuaclly, this is not a simple color, but            //
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

class TColorExtended : public TColor {
public:
   enum EGradientDirection {
      kGDVertical,
      kGDHorizontal
   };

   typedef std::vector<Color_t>::size_type SizeType_t;

private:

   EGradientDirection fGradientDirection;//At the moment, we have only linear gradient.
   //Should be normalized : [0, 1].
   std::vector<Double_t> fColorPositions;//Positions of color nodes in gradient. 

   //fColors[i] CAN NOT be a TColorExtended object.
   std::vector<Color_t>  fColors; //Indices in a gROOT's list of colors.

   Bool_t             fHasShadow;
public:

   TColorExtended(Color_t newColor, EGradientDirection gradType, UInt_t nPoints,
                  const Double_t *points, const Color_t *indices, Bool_t hasShadow);

   EGradientDirection GetGradientDirection()const;
   SizeType_t GetNumberOfSteps()const;
   const Double_t *GetColorPositions()const;
   const Color_t *GetColors()const;
   
   Bool_t HasShadow()const;
   //For the moment I do not have setters, let TColorExtended be non-mutable.
   //Later, if we had a GUI in ged, I'll add setters.
   
   ClassDef(TColorExtended, 0) //Extended drawing parameters.
};

#endif
