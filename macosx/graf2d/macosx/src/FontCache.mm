#include <stdexcept>
#include <cassert>

#include "CocoaUtils.h"
#include "QuartzText.h"
#include "FontCache.h"
#include "TSystem.h"
#include "TError.h"
#include "TEnv.h"

namespace ROOT {
namespace MacOSX {
namespace Details {

namespace {

//ROOT uses indices for fonts.
//Later, I'll find (I promise! ;) better
//way to map font indices to actual fonts
//(families, etc.) - I simply do not have any time now.

const int fmdNOfFonts = 13;
const CFStringRef fixedFontNames[FontCache::nPadFonts] =
                                     {
                                      CFSTR("TimesNewRomanPS-ItalicMT"),
                                      CFSTR("TimesNewRomanPS-BoldMT"),
                                      CFSTR("TimesNewRomanPS-BoldItalicMT"),
                                      CFSTR("Helvetica"),
                                      CFSTR("Helvetica-Oblique"),
                                      CFSTR("Helvetica-Bold"),
                                      CFSTR("Helvetica-BoldOblique"),
                                      CFSTR("Courier"),
                                      CFSTR("Courier-Oblique"),
                                      CFSTR("Courier-Bold"),
                                      CFSTR("Courier-BoldOblique"),
                                      CFSTR("Symbol"),
                                      CFSTR("TimesNewRomanPSMT")
                                     };


}

//_________________________________________________________________
FontCache::FontCache()
{
}

//______________________________________________________________________________
FontStruct_t FontCache::LoadFont(const X11::XLFDName &xlfd)
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
void FontCache::UnloadFont(FontStruct_t font)
{
   CTFontRef fontRef = (CTFontRef)font;
   auto fontIter = fLoadedFonts.find(fontRef);

   assert(fontIter != fLoadedFonts.end() && "Attempt to unload font, not created by font manager");

   fLoadedFonts.erase(fontIter);
}


//______________________________________________________________________________
unsigned FontCache::GetTextWidth(FontStruct_t font, const char *text, int nChars)
{
   //This is a temporary hack!!!
   CTFontRef fontRef = (CTFontRef)font;
   assert(fLoadedFonts.find(fontRef) != fLoadedFonts.end() && "Font was not created by font manager");

   //nChars is either positive, or negative (take all string).
   if (nChars < 0)
      nChars = std::strlen(text);

   std::string textLine(text, nChars);

   unsigned w = 0, h = 0;

   try {
      const Quartz::TextLine quartzTextLine(textLine.c_str(), fontRef);
      quartzTextLine.GetBounds(w, h);
   } catch (const std::exception &) {
      throw;
   }

   return w;
}


//_________________________________________________________________
void FontCache::GetFontProperties(FontStruct_t font, int &maxAscent, int &maxDescent)
{
   CTFontRef fontRef = (CTFontRef)font;

   assert(fLoadedFonts.find(fontRef) != fLoadedFonts.end() && "Font was not created by font manager");

   //Instead of this, use CT funtion to request ascent/descent.
   try {
      const Quartz::TextLine textLine("LALALA", fontRef);
      textLine.GetAscentDescent(maxAscent, maxDescent);
   } catch (const std::exception &) {
      throw;
   }
}


//_________________________________________________________________
CTFontRef FontCache::SelectFont(Font_t fontIndex, Float_t fontSize)
{
   fontIndex /= 10;

   if (fontIndex > nPadFonts || !fontIndex) {
      ::Warning("FontCache::SelectFont", "Font with index %d was requested", fontIndex);
      fontIndex = 1;
   }
   
   fontIndex -= 1;
   
   if (fontIndex == 11)//Special case, our own symbol.ttf file.
      return SelectSymbolFont(fontSize);
   
   const UInt_t fixedSize = UInt_t(fontSize);
   auto it = fFonts[fontIndex].find(fixedSize);
   
   if (it == fFonts[fontIndex].end()) {
      //Insert the new font.
      try {
         const CTFontGuard_t font(CTFontCreateWithName(fixedFontNames[fontIndex], fixedSize, 0), false);
         if (!font.Get()) {//With Apple's lame documentation it's not clear, if function can return 0.
            ::Error("FontCache::SelectFont", "CTFontCreateWithName failed for font %d", fontIndex);
            return nullptr;
         }
    
         fFonts[fontIndex][fixedSize] = font;//Insetion can throw.
         return fSelectedFont = font.Get();
      } catch (const std::exception &) {//Bad alloc.
         return nullptr;
      }
   }

   return fSelectedFont = it->second.Get();
}

//_________________________________________________________________
CTFontRef FontCache::SelectSymbolFont(Float_t fontSize)
{
   const UInt_t fixedSize = UInt_t(fontSize);
   auto it = fFonts[11].find(fixedSize);//In ROOT, 11 is a font from symbol.ttf.
   
   if (it == fFonts[11].end()) {
      //This GetValue + Which I took from Olivier's code.
      const char *fontDirectoryPath = gEnv->GetValue("Root.TTFontPath","$(ROOTSYS)/fonts");//This one I do not own.
      char *fontFileName = gSystem->Which(fontDirectoryPath, "symbol.ttf", kReadPermission);//This must be deleted.

      try {
      NSLog(@"Create symbol.ttf");
         const Util::CFScopeGuard<CFStringRef> path(CFStringCreateWithCString(kCFAllocatorDefault, fontFileName, kCFURLPOSIXPathStyle));
         if (!path.Get()) {
            ::Error("FontCache::SelectSymbolFont", "CFStringCreateWithCString failed");
            delete [] fontFileName;
            return nullptr;
         }
         
         const Util::CFScopeGuard<CFArrayRef> arr(CTFontManagerCreateFontDescriptorsFromURL(CFURLCreateWithFileSystemPath(kCFAllocatorDefault, path.Get(), kCFURLPOSIXPathStyle, false)));
         if (!arr.Get()) {
            ::Error("FontCache::SelectSymbolFont", "CTFontManagerCreateFontDescriptorsFromURL failed");
            delete [] fontFileName;
            return nullptr;
         }

         CTFontDescriptorRef fontDesc = (CTFontDescriptorRef)CFArrayGetValueAtIndex(arr.Get(), 0);
         const CTFontGuard_t font(CTFontCreateWithFontDescriptor(fontDesc, fixedSize, 0), false);
         if (!font.Get()) {
            ::Error("FontCache::SelectSymbolFont", "CTFontCreateWithFontDescriptor failed");
            delete [] fontFileName;
            return nullptr;
         }

         fFonts[11][fixedSize] = font;//This can throw.
         return fSelectedFont = font.Get();
      } catch (const std::exception &) {//Bad alloc.
         return nullptr;
      }
   }

   return fSelectedFont = it->second.Get();
}


//_________________________________________________________________   
void FontCache::GetTextBounds(UInt_t &w, UInt_t &h, const char *text)const
{
   assert(fSelectedFont != nullptr && "GetTextBounds: no font was selected");
   
   try {
      const Quartz::TextLine ctLine(text, fSelectedFont);
      ctLine.GetBounds(w, h);
   } catch (const std::exception &) {
      throw;
   }
}


//_________________________________________________________________
double FontCache::GetAscent()const
{
   assert(fSelectedFont != nullptr && "GetAscent, no font was selected");
   return CTFontGetAscent(fSelectedFont);
}


//_________________________________________________________________
double FontCache::GetDescent()const
{
   assert(fSelectedFont != nullptr && "GetDescent, no font was selected");
   return CTFontGetDescent(fSelectedFont);
}

//_________________________________________________________________
double FontCache::GetLeading()const
{
   assert(fSelectedFont != nullptr && "GetLeading, no font was selected");
   return CTFontGetLeading(fSelectedFont);
}


}//Details
}//MacOSX
}//ROOT
