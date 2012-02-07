#include <stdexcept>
#include <sstream>
#include <cassert>
#include <cctype>

#import <Cocoa/Cocoa.h>

#include "QuartzText.h"
#include "CocoaUtils.h"
#include "TError.h"

//
// This code is just a temporary
// hack done fast...
//


namespace ROOT {
namespace Quartz {

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
                                      CFSTR("Helvetica"),
                                      CFSTR("TimesNewRomanPSMT")
                                     };

typedef std::string::size_type size_type;

//______________________________________________________________________________
void DrawText(CGContextRef ctx, Double_t x, Double_t y, Float_t angle,
              Int_t align,
              const char *text)
{

   // Draw text
   
   CGContextSetAllowsAntialiasing(ctx, 1);


   FontManager fm;
   CTFontRef font;
   font = fm.SelectFont(11,20); /// just to try

   CTLineGuard ctLine(text, font);
   
   UInt_t w = 0, h = 0;
   ctLine.GetBounds(w, h);
   
   Double_t xc = 0., yc = 0.;

   const UInt_t hAlign = UInt_t(align / 10);   
   switch (hAlign) {
      case 1:
         xc = x;
         break;
      case 2:
         xc = x + 0.5 * w;
         break;
      case 3:
         xc = x - w;
         break;
   }
   
   const UInt_t vAlign = UInt_t(align % 10);
   switch (vAlign) {
      case 1:
         yc = y + 0.5 * h;
         break;
      case 2:
         break;
      case 3:
         yc = y - 0.5 * h;
         break;
   }

   CGContextSetTextPosition(ctx, 0.f, 0.f);
   
   CGContextTranslateCTM(ctx, xc, yc);

   //CGContextTranslateCTM(ctx, x, y);
   //CGContextRotateCTM(ctx, gVirtualX->GetTextAngle() * TMath::DegToRad());
   //CGContextTranslateCTM(ctx, xc, yc);
   //CGContextTranslateCTM(ctx, -0.5 * w, -0.5 * h);

   CTLineDraw(ctLine.fCTLine, ctx);
}

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


//______________________________________________________________________________
FontStruct_t FontManager::LoadFont(const XLFDName &xlfd)
{
   using ROOT::MacOSX::Util::CFGuard;

   //This code is just a sketch. I have to check later,
   //how to correctly create font with attributes from xlfd,
   //if matching between name from xlfd and MacOS X font is correct.

   //CF expects CFStringRef, not c-string.
   CFGuard<CFStringRef> fontName(CFStringCreateWithCString(kCFAllocatorDefault, xlfd.fFamilyName.c_str(), kCFStringEncodingMacRoman), false);//false - no initial retain.

   //TODO: pixelSize + 2 - this is just a temporary hack, because text in GUI is too tiny.
   CFGuard<CTFontRef> font(CTFontCreateWithName(fontName.Get(), xlfd.fPixelSize + 2, 0), false);//0 is for CGAffineTransform, false - no initial retain.


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
CTFontRef FontManager::SelectFont(Font_t fontIndex, Float_t fontSize)
{
   fontIndex /= 10;

   if (fontIndex > fmdNOfFonts || !fontIndex)
      throw std::runtime_error("SelectFont: index");

   fontIndex -= 1;

   if (fontIndex == 11 && !fSymbolMap.size())
      InitSymbolMap();

   const UInt_t fixedSize = UInt_t(fontSize);
   FontMapIter_t it = fFonts[fontIndex].find(fixedSize);

   if (it == fFonts[fontIndex].end()) {
      //Insert the new font.
     CTFontRef font(CTFontCreateWithName(fixedFontNames[fontIndex], fixedSize, 0));
     fFonts[fontIndex][fixedSize] = font;
     return fSelectedFont = font;
   }

   return fSelectedFont = it->second;
}


//_________________________________________________________________
void FontManager::InitSymbolMap()
{
   fSymbolMap.clear();
   fSymbolMap.resize(1 << std::numeric_limits<unsigned char>::digits, 0);

   fSymbolMap[97]  = 0x3B1; //alpha
   fSymbolMap[98]  = 0x3B2; //beta
   fSymbolMap[103] = 0x3B3; //gamma
   fSymbolMap[100] = 0x3B4; //delta
   fSymbolMap[206] = 0x3F5; //epsilon
   fSymbolMap[122] = 0x3B6; //zeta
   fSymbolMap[104] = 0x3B7; //eta
   fSymbolMap[113] = 0x3B8; //theta
   fSymbolMap[105] = 0x3B9; //iota
   fSymbolMap[107] = 0x3BA; //kappa
   fSymbolMap[108] = 0x3BB; //lambda
   fSymbolMap[109] = 0x3BC; //mu
   fSymbolMap[110] = 0x3BD; //nu
   fSymbolMap[120] = 0x3BE; //xi
   fSymbolMap[111] = 0x3BF; //omicron
   fSymbolMap[112] = 0x3C0; //pi
   fSymbolMap[114] = 0x3C1; //rho
   fSymbolMap[115] = 0x3C3; //sigma
   fSymbolMap[116] = 0x3C4; //tau
   fSymbolMap[117] = 0x3C5; //upsilon
   fSymbolMap[102] = 0x3C6; //phi
   fSymbolMap[99]  = 0x3C7; //chi
   fSymbolMap[121] = 0x3C8; //psi
   fSymbolMap[119] = 0x3C9; //omega

   fSymbolMap[65] = 0x391; //Alpha
   fSymbolMap[66] = 0x392; //Beta
   fSymbolMap[71] = 0x393; //Gamma
   fSymbolMap[68] = 0x394; //Delta
   fSymbolMap[69] = 0x395; //Epsilon
   fSymbolMap[90] = 0x396; //Zeta
   fSymbolMap[72] = 0x397; //Eta
   fSymbolMap[81] = 0x398; //Theta
   fSymbolMap[73] = 0x399; //Iota
   fSymbolMap[75] = 0x39A; //Kappa
   fSymbolMap[76] = 0x39B; //Lambda
   fSymbolMap[77] = 0x39C; //Mu
   fSymbolMap[78] = 0x39D; //Nu
   fSymbolMap[88] = 0x39E; //Xi
   fSymbolMap[79] = 0x39F; //Omicron
   fSymbolMap[80] = 0x3A0; //Pi
   fSymbolMap[82] = 0x3A1; //Rho
   fSymbolMap[83] = 0x3A3; //Sigma
   fSymbolMap[84] = 0x3A4; //Tau
   fSymbolMap[85] = 0x3A5; //Upsilon
   fSymbolMap[70] = 0x3A6; //Phi
   fSymbolMap[67] = 0x3A7; //Chi
   fSymbolMap[89] = 0x3A8; //Psi
   fSymbolMap[87] = 0x3A9; //Omega

   fSymbolMap[101] = 0x3B5; //varepsilon
   fSymbolMap[74]  = 0x3D1; //vartheta
   fSymbolMap[86]  = 0x3C2; //varsigma
   fSymbolMap[161] = 0x3D2; //varUpsilon
   fSymbolMap[106] = 0x3D5; //varphi???
   fSymbolMap[118] = 0x3D6; //varomega?

   fSymbolMap[167] = 0x2663;
   fSymbolMap[195] = 0x2118;
   fSymbolMap[163] = 0x2264;
   fSymbolMap[187] = 0x2248;
   fSymbolMap[206] = 0x2208;
   fSymbolMap[201] = 0x2283;
   fSymbolMap[199] = 0x2229;
   fSymbolMap[211] = 0xA9;
   fSymbolMap[212] = 0x2122;
   fSymbolMap[180] = 0xD7;
   fSymbolMap[183] = 0x2022;
   fSymbolMap[166] = 0x192;
   fSymbolMap[178] = 0x2033;
   fSymbolMap[231] = 0x7C;
   fSymbolMap[232] = 0x23A9;
   fSymbolMap[175] = 0x2193;
   fSymbolMap[171] = 0x2194;
   fSymbolMap[223] = 0x21D3;
   fSymbolMap[219] = 0x21D4;
   fSymbolMap[234] = 0x23AA;
   fSymbolMap[104] = 0x127;

   fSymbolMap[168] = 0x2666;
   fSymbolMap[192] = 0x2135;
   fSymbolMap[179] = 0x2265;
   fSymbolMap[185] = 0x2260;
   fSymbolMap[207] = 0x2209;
   fSymbolMap[205] = 0x2286;
   fSymbolMap[200] = 0x222A;
   fSymbolMap[227] = 0xA9;
   fSymbolMap[228] = 0x2122;
   fSymbolMap[184] = 0xF7;
   fSymbolMap[176] = 0xB0;
   fSymbolMap[165] = 0x221E;
   fSymbolMap[208] = 0x2220;
   fSymbolMap[189] = 0x7C;
   fSymbolMap[230] = 0x23A7;
   fSymbolMap[172] = 0x2190;
   fSymbolMap[196] = 0x2297;
   fSymbolMap[220] = 0x21D0;
   fSymbolMap[213] = 0x220F;

   fSymbolMap[169] = 0x2665;
   fSymbolMap[193] = 0x2111;
   fSymbolMap[225] = 0x3008;
   fSymbolMap[186] = 0x2261;
   fSymbolMap[204] = 0x2282;
   fSymbolMap[202] = 0x2287;
   fSymbolMap[217] = 0x2227;
   fSymbolMap[210] = 0xAE;
   fSymbolMap[197] = 0xC5; //no need
   fSymbolMap[177] = 0xB1;
   fSymbolMap[188] = 0x2026;
   fSymbolMap[209] = 0x2207;
   fSymbolMap[191] = 0x21B5;
   fSymbolMap[190] = 0x2015;//WRONG
   fSymbolMap[236] = 0x23A7;
   fSymbolMap[173] = 0x2191;
   fSymbolMap[197] = 0x2295;
   fSymbolMap[221] = 0x21D1;
   fSymbolMap[229] = 0x2211;
   fSymbolMap[34] = 0x2200;

   fSymbolMap[170] = 0x2660;
   fSymbolMap[194] = 0x211C;
   fSymbolMap[241] = 0x3009;
   fSymbolMap[181] = 0x221D;
   fSymbolMap[203] = 0x2284;
   fSymbolMap[198] = 0x2205;
   fSymbolMap[218] = 0x2228;
   fSymbolMap[226] = 0xAE;
   fSymbolMap[229] = 0xE5; //no need
   fSymbolMap[164] = 0x2044;
   fSymbolMap[215] = 0x22C5;
   fSymbolMap[182] = 0x2202;
   fSymbolMap[216] = 0xAC;
   fSymbolMap[237] = 0x23A8;
   fSymbolMap[235] = 0x23A3;
   fSymbolMap[174] = 0x2192;
   fSymbolMap[214] = 0x221A;
   fSymbolMap[222] = 0x21D2;
   fSymbolMap[242] = 0x222B;
   fSymbolMap[36] = 0x2203;
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
CTLineGuard::CTLineGuard(const char *textLine, CTFontRef font, const CGFloat *rgb)
                  : fCTLine(0)
{
   //Create attributed string with font and color.
   //TODO: use RAII?
   CGColorSpaceRef rgbColorSpace = CGColorSpaceCreateDeviceRGB();//[1
   if (!rgbColorSpace)
      throw std::runtime_error("CTLineGuard: color space is null");

   CGColorRef textColor = CGColorCreate(rgbColorSpace, rgb);//[2
   //Not clear from docs, if textColor can be 0.

   CFStringRef keys[] = {kCTFontAttributeName, kCTForegroundColorAttributeName};
   CFTypeRef values[] = {font, textColor};

   Init(textLine, 2, keys, values);

   CGColorRelease(textColor);//2]
   CGColorSpaceRelease(rgbColorSpace);//1]
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
