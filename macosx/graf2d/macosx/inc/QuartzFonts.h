#ifndef ROOT_QuartzFonts
#define ROOT_QuartzFonts

#include <string>
#include <map>

#include "CocoaUtils.h"

namespace ROOT {
namespace MacOSX {
namespace Quartz {

//
// All stuff here is a subject to serious changes and modifications in a future
// (may be even complete removal). Now I need this only to emulate 'X11' behavior,
// expected from TGCocoa.
//

enum class FontSlant {
   regular,
   italic
};

enum class FontWeight {
   medium,
   bold
};

struct XLFDName {
   //foundry *
   std::string fFamilyName;
   FontWeight fWeight;
   FontSlant fSlant;
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

class FontManager {
public:
   //Select the existing font or create a new one and select it.
   CTFontRef LoadFont(const XLFDName &xlfd);

private:
   typedef ROOT::MacOSX::Util::CFGuard<CTFontRef> CTFontGuard_t;
   std::map<CTFontRef, CTFontGuard_t> fLoadedFonts;
};

}
}
}

#endif
