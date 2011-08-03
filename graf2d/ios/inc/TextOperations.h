#ifndef ROOT_TextOperations
#define ROOT_TextOperations

#include <map>

#include <CoreText/CTFont.h>
#include <CoreText/CTLine.h>

#ifndef ROOT_ResourceManagement
#include "ResourceManagement.h"
#endif

#ifndef ROOT_Rtypes
#include "Rtypes.h"
#endif

namespace ROOT_iOS {

class CTLineGuard : public Util::NonCopyable {
   friend class Painter;

public:
   CTLineGuard(const char *textLine, CTFontRef font);
   CTLineGuard(const char *textLine, CTFontRef font, Color_t color);
   ~CTLineGuard();
   
   void GetBounds(UInt_t &w, UInt_t &h)const;
   
private:

   void Init(const char *textLine, UInt_t nAttribs, CFStringRef *keys, CFTypeRef *values);

   CTLineRef fCTLine; //Core Text line, created from Attributed string.
};

//////////////////////////////////////////////////////////////////////
//
// Initial, very naive font management for iOS and Core Text.
// To be extended or completely changed in a future.
//
//////////////////////////////////////////////////////////////////////

class FontManager : public Util::NonCopyable {
public:
   enum FontManagerDefaults {
      fmdNOfFonts = 13
   };

   FontManager();
   ~FontManager();

   //Select the existing font or create a new one and select it.
   CTFontRef SelectFont(Font_t fontIndex, Float_t fontSize);
   
   //Typographical bounds (whatever it means),
   //for the current selected font and text.
   void GetTextBounds(UInt_t &w, UInt_t &h, const char *text)const;
   //
   double GetAscent()const;
   double GetDescent()const;
   double GetLeading()const;

private:
   typedef std::map<UInt_t, CTFontRef> FontMap_t;
   typedef FontMap_t::iterator FontMapIter_t;
   typedef FontMap_t::const_iterator FontMapConstIter_t;

   FontMap_t fFonts[fmdNOfFonts];
   CTFontRef fSelectedFont;
};

}


#endif