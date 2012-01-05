#include <stdexcept>
#include <sstream>
#include <cassert>
#include <cctype>

#import <Cocoa/Cocoa.h>

#include "QuartzFonts.h"
#include "CocoaUtils.h"
#include "TError.h"

//
// This code is just a temporary
// hack done fast...
//


namespace ROOT {
namespace MacOSX {
namespace Quartz {

typedef std::string::size_type size_type;

//______________________________________________________________________________
template<class T>
void StringToInt(const std::string &str, const std::string &componentName, T &num)
{
   for (auto symbol : str) {
      if (!std::isdigit(symbol))
         throw std::runtime_error("bad symbol while converting component " + componentName + " into number");
   }
   
   std::istringstream in(str);
   in>>num;
}

//______________________________________________________________________________
size_type GetXLFDNameComponentAsString(const std::string &name, const std::string & componentName, 
                                       size_type pos, std::string &component)
{
   const size_type length = name.length();
   if (pos + 1 >= length)
      throw std::runtime_error("Unexpected end of name while parsing " + componentName);

   //Starting symbol must be '-'.   
   if (name[pos] != '-')
      throw std::runtime_error("Component " + componentName + " must start from '-'");

   const size_type start = ++pos;
   ++pos;
   while (pos < length && name[pos] != '-')
      ++pos;
      
   if (pos - start)
      component = name.substr(start, pos - start);
   else
      component = "";
   
   return pos;
}

//______________________________________________________________________________
template<class T>
size_type GetXLFDNameComponentAsInteger(const std::string &name, const std::string &componentName, size_type pos, T &component)
{
   std::string num;
   pos = GetXLFDNameComponentAsString(name, componentName, pos, num);
   StringToInt(num, componentName, component);

   return pos;
}

//______________________________________________________________________________
size_type ParseFoundry(const std::string &name, size_type pos, XLFDName &/*dst*/)
{
   //We ignore foundry at the moment(?)
   std::string dummy;
   return GetXLFDNameComponentAsString(name, "foundry", pos, dummy);
}

//______________________________________________________________________________
size_type ParseFamilyName(const std::string &name, size_type pos, XLFDName &dst)
{
   return GetXLFDNameComponentAsString(name, "family name", pos, dst.fFamilyName);
}

//______________________________________________________________________________
size_type ParseWeight(const std::string &name, size_type pos, XLFDName &dst)
{
   //Weight can be an integer, can be a word, can be a combination of a word
   //and integer.
   std::string weight;
   pos = GetXLFDNameComponentAsString(name, "weight", pos, weight);

   if (weight != "bold")
      dst.fWeight = FontWeight::medium;
   else
      dst.fWeight = FontWeight::bold;

   return pos;
}

//______________________________________________________________________________
size_type ParseSlant(const std::string &name, size_type pos, XLFDName &dst)
{
   //Slant can be regular or italic now.
   std::string slant;
   pos = GetXLFDNameComponentAsString(name, "slant", pos, slant);
   
   if (slant == "r" || slant == "R") {
      dst.fSlant = FontSlant::regular;
      return pos;
   }
   
   if (slant == "i" || slant == "I") {
      dst.fSlant = FontSlant::italic;
      return pos;
   }
   
   throw std::runtime_error("ParseSlant: unknown slant: " + slant);
   return pos;//never executed.
}

//______________________________________________________________________________
size_type ParseSetwidth(const std::string &name, size_type pos, XLFDName &/*dst*/)
{
   //Setwidth is ignored now.
   std::string dummy;
   return GetXLFDNameComponentAsString(name, "setwidth", pos, dummy);
}

//______________________________________________________________________________
size_type ParseAddstyle(const std::string &name, size_type pos, XLFDName &/*dst*/)
{
   //Ignored at the moment.
   std::string dummy;
   return GetXLFDNameComponentAsString(name, "addstyle", pos, dummy);
}

//______________________________________________________________________________
size_type ParsePixelSize(const std::string &name, size_type pos, XLFDName &dst)
{
   return GetXLFDNameComponentAsInteger(name, "pixel size", pos, dst.fPixelSize);
}

//______________________________________________________________________________
size_type ParsePointSize(const std::string &name, size_type pos, XLFDName &/*dst*/)
{
   //Ignored at the moment.
   std::string dummy;
   return GetXLFDNameComponentAsString(name, "point size", pos, dummy);
}

//______________________________________________________________________________
size_type ParseHoriz(const std::string &name, size_type pos, XLFDName &/*dst*/)
{
   //Ignored at the moment.
   std::string dummy;
   return GetXLFDNameComponentAsString(name, "horizontal", pos, dummy);
}

//______________________________________________________________________________
size_type ParseVert(const std::string &name, size_type pos, XLFDName &/*dst*/)
{
   //Ignored at the moment.
   std::string dummy;
   return GetXLFDNameComponentAsString(name, "vertical", pos, dummy);
}

//______________________________________________________________________________
size_type ParseSpacing(const std::string &name, size_type pos, XLFDName &/*dst*/)
{
   //Ignored at the moment.
   std::string dummy;
   return GetXLFDNameComponentAsString(name, "spacing", pos, dummy);
}

//______________________________________________________________________________
size_type ParseAvgwidth(const std::string &name, size_type pos, XLFDName &/*dst*/)
{
   //Ignored at the moment.
   std::string dummy;
   return GetXLFDNameComponentAsString(name, "average width", pos, dummy);
}

//______________________________________________________________________________
size_type ParseRgstry(const std::string &name, size_type pos, XLFDName &dst)
{
   return GetXLFDNameComponentAsString(name, "language", pos, dst.fRgstry);
}

//______________________________________________________________________________
size_type ParseEncoding(const std::string &name, size_type pos, XLFDName &dst)
{
   return GetXLFDNameComponentAsString(name, "encoding", pos, dst.fRgstry);
}

//______________________________________________________________________________
bool ParseXLFDName(const std::string &xlfdName, XLFDName &dst)
{
   assert(xlfdName.length() && "XLFD name is a string with a zero length");

   if (!xlfdName.length()) {
      ::Warning("ROOT::MacOSX::Quartz::ParseXLFDName: ", "XLFD name is a string with a zero length");
      return false;
   }
   
   try {
      std::string::size_type pos = 0;
      
      pos = ParseFoundry(xlfdName, pos, dst);
      pos = ParseFamilyName(xlfdName, pos, dst);
      pos = ParseWeight(xlfdName, pos, dst);      
      pos = ParseSlant(xlfdName, pos, dst);
      pos = ParseSetwidth(xlfdName, pos, dst);
      pos = ParseAddstyle(xlfdName, pos, dst);
      pos = ParsePixelSize(xlfdName, pos, dst);
      pos = ParsePointSize(xlfdName, pos, dst);
      pos = ParseHoriz(xlfdName, pos, dst);
      pos = ParseVert(xlfdName, pos, dst);
      pos = ParseSpacing(xlfdName, pos, dst);
      pos = ParseAvgwidth(xlfdName, pos, dst);
      pos = ParseRgstry(xlfdName, pos, dst);
      pos = ParseEncoding(xlfdName, pos, dst);

      return true;
   } catch (const std::exception &e) {
      ::Error("ROOT::MacOSX::Quartz::ParseXLFDName", "Failed to parse XLFD name - %s", e.what());
      return false;
   }
}

//______________________________________________________________________________
FontStruct_t FontManager::LoadFont(const XLFDName &xlfd)
{
   using ROOT::MacOSX::Util::CFGuard;
   
   //This code is just a sketch. I have to check later,
   //how to correctly create font with attributes from xlfd,
   //if matching between name from xlfd and MacOS X font is correct.

   //CF expects CFStringRef, not c-string.
   CFGuard<CFStringRef> fontName(CFStringCreateWithCString(kCFAllocatorDefault, xlfd.fFamilyName.c_str(), kCFStringEncodingMacRoman), false);//false - no initial retain.
   CFGuard<CTFontRef> font(CTFontCreateWithName(fontName.Get(), xlfd.fPixelSize, 0), false);//0 is for CGAffineTransform, false - no initial retain.
   
   
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
   
   CTLineGuard ctLine(textLine.c_str(), fontRef);
   ctLine.GetBounds(w, h);

   return w;

}

//_________________________________________________________________
void FontManager::GetFontProperties(FontStruct_t font, int &maxAscent, int &maxDescent)
{
   CTFontRef fontRef = (CTFontRef)font;

   assert(fLoadedFonts.find(fontRef) != fLoadedFonts.end() && "Font was not created by font manager");

   //Instead of this, use CT funtion to request ascent/descent.
   CTLineGuard ctLine("LALALA", fontRef);
   ctLine.GetAscentDescent(maxAscent, maxDescent);
}

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
CTLineGuard::CTLineGuard(const char * /*textLine*/, CTFontRef /*font*/, Color_t /*color*/)
                  : fCTLine(0)
{
   //Create attributed string with font and color.
   //
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
void CTLineGuard::GetAscentDescent(Int_t &asc, Int_t &desc)const
{
   CGFloat ascent = 0.f, descent = 0.f, leading = 0.f;
   CTLineGetTypographicBounds(fCTLine, &ascent, &descent, &leading);
   asc = int(ascent);
   desc = int(descent);
}

//_________________________________________________________________
void CTLineGuard::Init(const char *textLine, UInt_t nAttribs, CFStringRef *keys, CFTypeRef *values)
{
   using ROOT::MacOSX::Util::CFGuard;
   CFGuard<CFDictionaryRef> stringAttribs(CFDictionaryCreate(kCFAllocatorDefault, 
                                          (const void **)keys, (const void **)values, 
                                          nAttribs, &kCFTypeDictionaryKeyCallBacks, 
                                          &kCFTypeDictionaryValueCallBacks), false);
   if (!stringAttribs.Get())
      throw std::runtime_error("CTLineGuard: null attribs");

   CFGuard<CFStringRef> wrappedCString(CFStringCreateWithCString(kCFAllocatorDefault, textLine, kCFStringEncodingMacRoman), false);
   if (!wrappedCString.Get())
      throw std::runtime_error("CTLineGuard: cstr wrapper");

   CFGuard<CFAttributedStringRef> attributedString(CFAttributedStringCreate(kCFAllocatorDefault, wrappedCString.Get(), stringAttribs.Get()), false);
   fCTLine = CTLineCreateWithAttributedString(attributedString.Get());

   if (!fCTLine)
      throw std::runtime_error("CTLineGuard: attrib string");
}

}
}
}
