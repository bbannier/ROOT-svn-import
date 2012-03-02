//Author: Timur Pocheptsov (DrawText by Olivier Couet).

#include <stdexcept>
#include <cassert>
#include <vector>

#include "TEnv.h"
#include "TSystem.h"

#include "QuartzText.h"
#include "CocoaUtils.h"
#include "TError.h"


namespace ROOT {
namespace Quartz {

//_________________________________________________________________
TextLine::TextLine(const char *textLine, CTFontRef font)
             : fCTLine(0)
{
   //Create attributed string with one attribue: the font.
   CFStringRef keys[] = {kCTFontAttributeName};
   CFTypeRef values[] = {font};

   Init(textLine, 1, keys, values);
}


//_________________________________________________________________
TextLine::TextLine(const char * /*textLine*/, CTFontRef /*font*/, Color_t /*color*/)
            : fCTLine(0)
{
   //Create attributed string with font and color.
}


//_________________________________________________________________
TextLine::TextLine(const char *textLine, CTFontRef font, const CGFloat *rgb)
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
TextLine::~TextLine()
{
   CFRelease(fCTLine);
}


//_________________________________________________________________
void TextLine::GetBounds(UInt_t &w, UInt_t &h)const
{
   CGFloat ascent = 0., descent = 0., leading = 0.;
   w = UInt_t(CTLineGetTypographicBounds(fCTLine, &ascent, &descent, &leading));
   h = UInt_t(ascent);// + descent + leading);
}


//_________________________________________________________________
void TextLine::GetAscentDescent(Int_t &asc, Int_t &desc)const
{
   CGFloat ascent = 0., descent = 0., leading = 0.;
   CTLineGetTypographicBounds(fCTLine, &ascent, &descent, &leading);
   asc = Int_t(ascent);
   desc = Int_t(descent);
}


//_________________________________________________________________
void TextLine::Init(const char *textLine, UInt_t nAttribs, CFStringRef *keys, CFTypeRef *values)
{
   using MacOSX::Util::StrongReferenceCF;
   
   //Strong reference must be replaced with scope guards.
   StrongReferenceCF<CFDictionaryRef> stringAttribs(CFDictionaryCreate(kCFAllocatorDefault,
                                                    (const void **)keys, (const void **)values,
                                                    nAttribs, &kCFTypeDictionaryKeyCallBacks,
                                                    &kCFTypeDictionaryValueCallBacks), false);
   if (!stringAttribs.Get())
      throw std::runtime_error("CTLineGuard: null attribs");

   StrongReferenceCF<CFStringRef> wrappedCString(CFStringCreateWithCString(kCFAllocatorDefault, textLine, kCFStringEncodingMacRoman), false);
   if (!wrappedCString.Get())
      throw std::runtime_error("CTLineGuard: cstr wrapper");

   StrongReferenceCF<CFAttributedStringRef> attributedString(CFAttributedStringCreate(kCFAllocatorDefault, wrappedCString.Get(), stringAttribs.Get()), false);
   fCTLine = CTLineCreateWithAttributedString(attributedString.Get());

   if (!fCTLine)
      throw std::runtime_error("CTLineGuard: attrib string");
}

//_________________________________________________________________
void TextLine::DrawLine(CGContextRef ctx)const
{
   assert(ctx != nullptr && "DrawLine, ctx is null");

   CTLineDraw(fCTLine, ctx);
}

//DrawText, GetTextExtent by Olivier Couet.

namespace {

const int fmdNOfFonts = 13;
const CFStringRef fixedFontNames[fmdNOfFonts] =
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


//_________________________________________________________________
CTFontRef CreateFont(Font_t fontIndex, Float_t fontSize)
{
   fontIndex /= 10;

   if (fontIndex > fmdNOfFonts || !fontIndex)
      return nullptr;

   fontIndex -= 1;
   CTFontRef font(CTFontCreateWithName(fixedFontNames[fontIndex], fontSize, 0));

   return font;
}

}

//______________________________________________________________________________
void DrawText(CGContextRef ctx, Double_t x, Double_t y, Float_t /*angle*/, Int_t align, Int_t font, Float_t size, const char *text)
{
   if (font / 10 == 12) {
      //Font creation.
      const char *ttpath = gEnv->GetValue("Root.TTFontPath","$(ROOTSYS)/fonts");
      char *ttfont = gSystem->Which(ttpath, "symbol.ttf", kReadPermission);

      CFStringRef path = CFStringCreateWithCString(kCFAllocatorDefault, ttfont, kCFURLPOSIXPathStyle);//[0

      CFArrayRef arr = CTFontManagerCreateFontDescriptorsFromURL(CFURLCreateWithFileSystemPath(kCFAllocatorDefault, path, kCFURLPOSIXPathStyle, false));//[1
      CFRelease(path);//0]

      CTFontDescriptorRef fontDesc = (CTFontDescriptorRef)CFArrayGetValueAtIndex(arr, 0);//[2
      CTFontRef fontref = CTFontCreateWithFontDescriptor(fontDesc, size, 0);//[3
      CFRelease(arr);//1]

      //Unicode string creation.
      CFStringRef keys[] = {kCTFontAttributeName};
      CFTypeRef values[] = {fontref};
      
      //This code is hack, we use the fact, that our symbol.ttf has quite special code
      //positions for symbols. This crap must be replaced with correct font rendering.
      unsigned len = (int)strlen(text);
      std::vector<UniChar> unichars(len);
      for (unsigned i = 0; i < len; ++i)
         unichars[i] = 0xF000 + (unsigned char)text[i];


      const CFStringRef wrappedCString = CFStringCreateWithCharacters(kCFAllocatorDefault, &unichars[0], unichars.size() / sizeof unichars[0]);//[4
      const CFDictionaryRef stringAttribs = CFDictionaryCreate(kCFAllocatorDefault, (const void **)keys, (const void **)values,
                                                               1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);//[5
      const CFAttributedStringRef attributedString = CFAttributedStringCreate(kCFAllocatorDefault, wrappedCString, stringAttribs);//[6
      const CTLineRef ctLine = CTLineCreateWithAttributedString(attributedString);//[7

      //Render the string.
      CGContextSetTextPosition(ctx, 0.f, 0.f);
      CGContextTranslateCTM(ctx, x, y);
      CTLineDraw(ctLine, ctx);

      CFRelease(ctLine);//7]
      CFRelease(fontref);//3]
      CFRelease(wrappedCString);//4]
      CFRelease(stringAttribs);//5]
      CFRelease(attributedString);//6]
   
      return;
   }

   CGContextSetAllowsAntialiasing(ctx, 1);

   CTFontRef fontref = CreateFont(font, size);
   if (!fontref)
      return;

   TextLine ctLine(text, fontref);

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
         yc = y;
         break;
      case 2:
         yc = y + 0.5 * h;
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

   ctLine.DrawLine(ctx);

   CFRelease(fontref);
}


//______________________________________________________________________________
void GetTextExtent(UInt_t &w, UInt_t &h, Int_t font, Float_t size, const char *text)
{
   // Get text extent.
   if (CTFontRef fontref = CreateFont(font, size)) {
      TextLine ctLine(text, fontref);
      ctLine.GetBounds(w, h);
      CFRelease(fontref);
   }
}

}
}
