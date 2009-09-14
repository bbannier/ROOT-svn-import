#ifndef INCLUDE_TTERMMANIP_H
#define INCLUDE_TTERMMANIP_H

#include <map>
#include <cstring>

// setupterm must be called before TTermManip can be created!
class TTermManip {
public:
   TTermManip();
   ~TTermManip() { ResetTerm(); }

   bool SetColor(unsigned char r, unsigned char g, unsigned char b);

   void
   StartUnderline() {
      if (!fCurrentlyUnterlined) {
         WriteTerm(fStartUnderline);
         fCurrentlyUnterlined = true;
      }
   }


   void
   StopUnderline() {
      if (fCurrentlyUnterlined) {
         WriteTerm(fStopUnderline);
         fCurrentlyUnterlined = false;
      }
   }


   bool ResetTerm();
   void SetDefaultColor();

private:
   class Color {
   public:
      Color(unsigned char r = 0, unsigned char g = 0, unsigned char b = 0):
         fR((r* 1001) / 256),
         fG((g* 1001) / 256),
         fB((b* 1001) / 256) {
         // Re-normalize RGB components from 0 to 255 to 0 to 1000
      }


      bool
      operator <(const Color& c) const {
         return fR < c.fR
                || (fR == c.fR && (fG < c.fG
                                   || (fG == c.fG && fB < c.fB)));
      }


      unsigned char fR, fG, fB;
   };

   char* GetTermStr(const char* cap);

   bool WriteTerm(char* termstr);

   bool WriteTerm(char* termstr, int i);

   int AllocColor(const Color& col);

   static int
   DefaultPutchar(int c) {
      // tputs takes int(*)(char) on solaris, so wrap putchar
      return putchar(c);
   }


   bool fColorCapable;
   bool fUsePairs;
   bool fAnsiColors; // whether fSetFg, Bg use ANSI
   bool fCanChangeColors; // whether the terminal can redefine existing colors
   char* fOrigColors; // reset colors
   char* fInitColor; // initialize a color
   char* fInitPair; // initialize pair
   char* fSetPair; // set color to a pair
   char* fSetFg; // set foreground color
   char* fSetBold; // set bold color
   char* fSetDefault; // set normal color
   char* fStartUnderline; // start underline;
   char* fStopUnderline; // stop underline;
   typedef int (*PutcFunc_t)(int);
   PutcFunc_t fPutc;
   int fCurrentColorIdx;   // index if the currently active color
   bool fCurrentlyBold;  // whether bold is active
   bool fCurrentlyUnterlined;  // whether underlining is active

   static const int fgStartColIdx = 5;
   typedef std::map<Color, int> ColorMap_t;
   ColorMap_t fColors;
};

#endif // INCLUDE_TTERMMANIP_H
