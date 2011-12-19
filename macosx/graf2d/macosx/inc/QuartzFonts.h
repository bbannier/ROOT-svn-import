#ifndef ROOT_QuartzFonts
#define ROOT_QuartzFonts

#include <string>
#include <map>

#include <ApplicationServices/ApplicationServices.h>

#include "CocoaUtils.h"
#include "GuiTypes.h"

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
   FontManager(){}
   
   FontStruct_t LoadFont(const XLFDName &xlfd);
   void UnloadFont(FontStruct_t font);

   unsigned GetTextWidth(FontStruct_t font, const char *text, int nChars);
   void GetFontProperties(FontStruct_t font, int &maxAscent, int &maxDescent);

private:
   typedef ROOT::MacOSX::Util::CFGuard<CTFontRef> CTFontGuard_t;
   std::map<CTFontRef, CTFontGuard_t> fLoadedFonts;
   
   FontManager(const FontManager &rhs) = delete;
   FontManager &operator = (const FontManager &rhs) = delete;
};

//Find better name.
class CTLineGuard {
   friend class Painter;

public:
   CTLineGuard(const char *textLine, CTFontRef font);
   CTLineGuard(const char *textLine, CTFontRef font, Color_t color);

   ~CTLineGuard();
   
   void GetBounds(UInt_t &w, UInt_t &h)const;
   void GetAscentDescent(Int_t &asc, Int_t &desc)const;
   
private:

   void Init(const char *textLine, UInt_t nAttribs, CFStringRef *keys, CFTypeRef *values);

   CTLineRef fCTLine; //Core Text line, created from Attributed string.
   
   CTLineGuard(const CTLineGuard &rhs) = delete;
   CTLineGuard &operator = (const CTLineGuard &rhs) = delete;
};


}
}
}

#endif
