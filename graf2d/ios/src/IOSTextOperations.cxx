#include <stdexcept>
#include <utility>
#include <string>

#include <CoreFoundation/CFAttributedString.h>
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFBase.h>

#include <CoreText/CTStringAttributes.h>

#include <CoreGraphics/CGColorSpace.h>
#include <CoreGraphics/CGColor.h>

#include "TVirtualX.h"
#include "TColor.h"
#include "TROOT.h"

#include "IOSTextOperations.h"
#include "IOSGraphicUtils.h"

namespace ROOT_iOS
{

const CFStringRef fixedFontNames[FontManager::fmdNOfFonts] = 
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
                                      CFSTR("Helvetica"),//CFSTR("symbol"),//No custom fonts yet.
                                      CFSTR("TimesNewRomanPSMT")
                                     };

const char *cStrFontNames[FontManager::fmdNOfFonts] = 
                              {
                               "TimesNewRomanPS-ItalicMT",
                               "TimesNewRomanPS-BoldMT",
                               "TimesNewRomanPS-BoldItalicMT",
                               "Helvetica",
                               "Helvetica-Oblique",
                               "Helvetica-Bold",
                               "Helvetica-BoldOblique",
                               "Courier",
                               "Courier-Oblique",
                               "Courier-Bold",
                               "Courier-BoldOblique",
                               "symbol",//No custom fonts yet.
                               "TimesNewRomanPSMT"
                              };

//_________________________________________________________________
CTLineGuard::CTLineGuard(const char *textLine, CTFontRef font)
                  : fCTLine(0)
{
   //Create attributed string with one attribue: the font.
   CFStringRef keys[] = {kCTFontAttributeName};
   CFTypeRef values[] = {font};
   
   Init(textLine, 1, keys, values);
}

//_________________________________________________________________
CTLineGuard::CTLineGuard(const char *textLine, CTFontRef font, Color_t /*color*/)
                  : fCTLine(0)
{
   //Create attributed string with font and color.
   Util::RefGuardGeneric<CGColorSpaceRef, CGColorSpaceRelease> rgbColorSpace(CGColorSpaceCreateDeviceRGB());
   if (!rgbColorSpace.Get())
      throw std::runtime_error("CTLineGuard: color space");


   Float_t rgba[] = {0.f, 0.f, 0.f, 1.f};
   GraphicUtils::GetColorForIndex(gVirtualX->GetTextColor(), rgba[0], rgba[1], rgba[2]);

   Util::RefGuardGeneric<CGColorRef, CGColorRelease> textColor(CGColorCreate(rgbColorSpace.Get(), rgba));
   //Not clear from docs, if textColor.Get() can be 0.
   
   CFStringRef keys[] = {kCTFontAttributeName, kCTForegroundColorAttributeName};
   CFTypeRef values[] = {font, textColor.Get()};

   Init(textLine, 2, keys, values);
}

//_________________________________________________________________
CTLineGuard::~CTLineGuard()
{
   CFRelease(fCTLine);
}

//_________________________________________________________________   
void CTLineGuard::GetBounds(UInt_t &w, UInt_t &h)const
{
   CGFloat ascent = 0.f, descent = 0.f, leading = 0.f;
   w = UInt_t(CTLineGetTypographicBounds(fCTLine, &ascent, &descent, &leading));
   h = UInt_t(ascent);// + descent + leading);
}

//_________________________________________________________________
void CTLineGuard::Init(const char *textLine, UInt_t nAttribs, CFStringRef *keys, CFTypeRef *values)
{
   Util::RefGuard<CFDictionaryRef> stringAttribs(
                                                 CFDictionaryCreate(kCFAllocatorDefault, 
                                                                    (const void **)keys,
                                                                    (const void **)values, 
                                                                    nAttribs,
                                                                    &kCFTypeDictionaryKeyCallBacks, 
                                                                    &kCFTypeDictionaryValueCallBacks)
                                                );

   if (!stringAttribs.Get())
      throw std::runtime_error("CTLineGuard: null attribs");

   Util::RefGuard<CFStringRef> wrappedCString(CFStringCreateWithCString(kCFAllocatorDefault, textLine, kCFStringEncodingMacRoman));
   if (!wrappedCString.Get())
      throw std::runtime_error("CTLineGuard: cstr wrapper");

   Util::RefGuard<CFAttributedStringRef> attributedString(CFAttributedStringCreate(kCFAllocatorDefault, wrappedCString.Get(), stringAttribs.Get()));
   fCTLine = CTLineCreateWithAttributedString(attributedString.Get());

   if (!fCTLine)
      throw std::runtime_error("CTLineGuard: attrib string");
}

//_________________________________________________________________
FontManager::FontManager()
               : fSelectedFont(0)
{
}

//_________________________________________________________________
FontManager::~FontManager()
{
   for (UInt_t i = 0; i < fmdNOfFonts; ++i)
      for (FontMapIter_t it = fFonts[i].begin(); it != fFonts[i].end(); ++it)
         CFRelease(it->second);
}

//_________________________________________________________________
CTFontRef FontManager::SelectFont(Font_t fontIndex, Float_t fontSize)
{
   fontIndex /= 10;

   if (fontIndex > fmdNOfFonts || !fontIndex)
      throw std::runtime_error("SelectFont: index");
   
   fontIndex -= 1;
   
   const UInt_t fixedSize = UInt_t(fontSize);
   FontMapIter_t it = fFonts[fontIndex].find(fixedSize);
   
   if (it == fFonts[fontIndex].end()) {
      //Insert the new font.
      Util::RefGuard<CTFontRef> font(CTFontCreateWithName(fixedFontNames[fontIndex], fixedSize, 0));
      if (!font.Get()) //With Apple's lame documentation it's not clear, if function can return 0.
         throw std::runtime_error(std::string("SelectFont: create") + cStrFontNames[fontIndex]);
 
      fFonts[fontIndex][fixedSize] = font.Get();
      return fSelectedFont = font.Release();
   }

   return fSelectedFont = it->second;
}

//_________________________________________________________________
void FontManager::GetTextBounds(UInt_t &w, UInt_t &h, const char *text)const
{
   if (!fSelectedFont)
      throw std::runtime_error("GetTextBounds: not selected");
   
   CTLineGuard ctLine(text, fSelectedFont);
   ctLine.GetBounds(w, h);
}

//_________________________________________________________________
Double_t FontManager::GetAscent()const
{
   if (!fSelectedFont)
      throw std::runtime_error("GetAscent");

   return CTFontGetAscent(fSelectedFont);
}

//_________________________________________________________________
Double_t FontManager::GetDescent()const
{
   if (!fSelectedFont)
      throw std::runtime_error("GetDescent");

   return CTFontGetDescent(fSelectedFont);
}

//_________________________________________________________________
Double_t FontManager::GetLeading()const
{
   if (!fSelectedFont)
      throw std::runtime_error("GetLeading");
      
   return CTFontGetLeading(fSelectedFont);
}

}
