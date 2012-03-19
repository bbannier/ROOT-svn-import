//Author: Timur Pocheptsov.
#ifndef ROOT_QuartzText
#define ROOT_QuartzText

//This must be changed: different header for iOS and MacOSX.
#import <ApplicationServices/ApplicationServices.h>
#include <Cocoa/Cocoa.h>

#include "CocoaUtils.h"
#include "GuiTypes.h"


namespace ROOT {
namespace Quartz {
   
// Core Text's CTLine wrapper.
class TextLine {
public:
   TextLine(const char *textLine, CTFontRef font);
   TextLine(const char *textLine, CTFontRef font, Color_t color);
   TextLine(const char *textLine, CTFontRef font, const CGFloat *rgb);

   ~TextLine();

   void GetBounds(UInt_t &w, UInt_t &h)const;
   void GetAscentDescent(Int_t &asc, Int_t &desc)const;

   void DrawLine(CGContextRef ctx)const;
   void DrawText(CGContextRef ctx, Double_t x, Double_t y)const;
private:
   CTLineRef fCTLine; //Core Text line, created from Attributed string.

   void Init(const char *textLine, UInt_t nAttribs, CFStringRef *keys, CFTypeRef *values);

   TextLine(const TextLine &rhs) = delete;
   TextLine &operator = (const TextLine &rhs) = delete;
};

//By Olivier Couet.

void DrawText(CGContextRef ctx, Double_t x, Double_t y, Float_t angle, Int_t align, Int_t font, Float_t size, const char *text);
void GetTextExtent(UInt_t &w, UInt_t &h, Int_t font, Float_t size, const char *text);

}
}

#endif
