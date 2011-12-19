#ifndef ROOT_QuartzFonts
#define ROOT_QuartzFonts

#include <string>

namespace ROOT {
namespace MacOSX {
namespace Quartz {

enum class Slant {
   regular,
   italic
};

struct XLFDName {
   //foundry *
   std::string fFamilyName;
   unsigned fWeight;
   Slant fSlant;
   //width  *
   //addstyle *
   unsigned fPixelSize;
   //points *
   //horiz *
   //vert *
   //spacing *
   //avgwidth *
   std::string fRgstry;
   std::string fEncoding;
};

bool ParseXLFDName(const std::string &xlfdName, XLFDName &dst);

}
}
}

#endif
