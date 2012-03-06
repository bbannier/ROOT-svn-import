#include "QuartzText.h"
#include "GuiFonts.h"

namespace ROOT {
namespace MacOSX {
namespace Details {

//_________________________________________________________________
FontManager::FontManager()
{
}

//______________________________________________________________________________
FontStruct_t FontManager::LoadFont(const X11::XLFDName &xlfd)
{
   using Util::CFStrongReference;

   //This code is just a sketch. I have to check later,
   //how to correctly create font with attributes from xlfd,
   //if matching between name from xlfd and MacOS X font is correct.

   //CF expects CFStringRef, not c-string.
   //Instead of StrongReference, use ScopeGuard class.
   CFStrongReference<CFStringRef> fontName(CFStringCreateWithCString(kCFAllocatorDefault, xlfd.fFamilyName.c_str(), kCFStringEncodingMacRoman), false);//false - no initial retain.
   //TODO: pixelSize + 2 - this is just a temporary hack, because text in GUI is too tiny.
   CFStrongReference<CTFontRef> font(CTFontCreateWithName(fontName.Get(), xlfd.fPixelSize + 2, 0), false);//0 is for CGAffineTransform, false - no initial retain.

   //What if this font was "loaded" already?
   if (fLoadedFonts.find(font.Get()) == fLoadedFonts.end())
      fLoadedFonts[font.Get()] = font;

   return reinterpret_cast<FontStruct_t>(font.Get());
}


//______________________________________________________________________________
void FontManager::UnloadFont(FontStruct_t font)
{
   CTFontRef fontRef = (CTFontRef)font;
   auto fontIter = fLoadedFonts.find(fontRef);

   assert(fontIter != fLoadedFonts.end() && "Attempt to unload font, not created by font manager");

   fLoadedFonts.erase(fontIter);
}


//______________________________________________________________________________
unsigned FontManager::GetTextWidth(FontStruct_t font, const char *text, int nChars)
{
   //This is a temporary hack!!!
   CTFontRef fontRef = (CTFontRef)font;
   assert(fLoadedFonts.find(fontRef) != fLoadedFonts.end() && "Font was not created by font manager");

   //nChars is either positive, or negative (take all string).
   if (nChars < 0)
      nChars = std::strlen(text);

   std::string textLine(text, nChars);

   unsigned w = 0, h = 0;

   Quartz::TextLine quartzTextLine(textLine.c_str(), fontRef);
   quartzTextLine.GetBounds(w, h);

   return w;
}


//_________________________________________________________________
void FontManager::GetFontProperties(FontStruct_t font, int &maxAscent, int &maxDescent)
{
   CTFontRef fontRef = (CTFontRef)font;

   assert(fLoadedFonts.find(fontRef) != fLoadedFonts.end() && "Font was not created by font manager");

   //Instead of this, use CT funtion to request ascent/descent.
   Quartz::TextLine textLine("LALALA", fontRef);
   textLine.GetAscentDescent(maxAscent, maxDescent);
}

}//Details
}//MacOSX
}//ROOT

