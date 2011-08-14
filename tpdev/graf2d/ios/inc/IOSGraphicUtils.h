#ifndef ROOT_IOSGraphicUtils
#define ROOT_IOSGraphicUtils

#ifndef ROOT_Rtypes
#include "Rtypes.h"
#endif

namespace ROOT_iOS {
namespace GraphicUtils {

//Generic graphic utils.
void GetColorForIndex(Color_t colorIndex, Float_t &r, Float_t &g, Float_t &b);


//Encode object's ID (unsigned integer) as an RGB triplet.
class IDEncoder {
public:
   IDEncoder(UInt_t radix, UInt_t channelSize);
   
   Bool_t IdToColor(UInt_t objId, Float_t *rgb) const;
   UInt_t ColorToId(UInt_t r, UInt_t g, UInt_t b) const;
   
private:
   UInt_t FixValue(UInt_t val) const;

   const UInt_t fRadix;
   const UInt_t fRadix2;
   const UInt_t fChannelSize;
   const UInt_t fStepSize;
   const UInt_t fMaxID;
};

} //namespace GraphicUtils
} //namespace ROOT_iOS

#endif
