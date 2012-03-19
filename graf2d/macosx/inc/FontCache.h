//Author: Timur Pocheptsov.

#ifndef ROOT_FontCache
#define ROOT_FontCache

#include <map>

#ifndef ROOT_XLFDParser
#include "XLFDParser.h"
#endif
#ifndef ROOT_GuiTypes
#include "GuiTypes.h"
#endif

//////////////////////////////////////////////////////////////////
//                                                              //
// FontCache class:                                             //
// ROOT's GUI relies on TVirtualX to create and use fonts,      //
// fonts are referenced by integer identifiers.                 //
// Also, non-GUI graphics wants difference fonts.               //
// For the moment, this is quite lame implementation,           //
// which I will fix in a future (I promise! ;) ).               //
//                                                              //
//////////////////////////////////////////////////////////////////

namespace ROOT {
namespace MacOSX {
namespace Details {

class FontCache {
public:
   FontCache();
   
   FontStruct_t LoadFont(const X11::XLFDName &xlfd);
   void UnloadFont(FontStruct_t font);

   unsigned GetTextWidth(FontStruct_t font, const char *text, int nChars);
   void GetFontProperties(FontStruct_t font, int &maxAscent, int &maxDescent);

private:
   typedef Util::CFStrongReference<CTFontRef> CTFontGuard_t;
   
   //These are fonts for GUI. Weird map, as I can see now.
   std::map<CTFontRef, CTFontGuard_t> fLoadedFonts;
   //Fonts for TPad's graphics.
   
   
   FontCache(const FontCache &rhs) = delete;
   FontCache &operator = (const FontCache &rhs) = delete;   
};

}
}
}

#endif
