//Author: Timur Pocheptsov.

#ifndef ROOT_GuiFonts
#define ROOT_GuiFonts

#include <map>

#ifndef ROOT_XLFDParser
#include "XLFDParser.h"
#endif
#ifndef ROOT_GuiTypes
#include "GuiTypes.h"
#endif

//////////////////////////////////////////////////////////////////
//                                                              //
// FontManager class:                                           //
// ROOT's GUI relies on TVirtualX to create and use fonts,      //
// fonts are referenced by integer identifiers.                 //
//                                                              //
//////////////////////////////////////////////////////////////////

namespace ROOT {
namespace MacOSX {
namespace Details {

class FontManager {
public:
   FontManager();
   
   FontStruct_t LoadFont(const X11::XLFDName &xlfd);
   void UnloadFont(FontStruct_t font);

   unsigned GetTextWidth(FontStruct_t font, const char *text, int nChars);
   void GetFontProperties(FontStruct_t font, int &maxAscent, int &maxDescent);

private:
   typedef Util::StrongReferenceCF<CTFontRef> CTFontGuard_t;
   std::map<CTFontRef, CTFontGuard_t> fLoadedFonts;
   
   FontManager(const FontManager &rhs) = delete;
   FontManager &operator = (const FontManager &rhs) = delete;   
};

}
}
}

#endif
