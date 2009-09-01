#include "TTermManip.h"

#ifndef _MSC_VER
#include <curses.h>
#include <termcap.h>
extern "C" int setupterm(const char* term, int fd, int* perrcode);
#else
#include "win32vt100.h"
#endif

TTermManip::TTermManip():
   fColorCapable(false),
   fUsePairs(false),
   fAnsiColors(true),
   fCanChangeColors(false),
   fOrigColors(0),
   fInitColor(0),
   fInitPair(0),
   fSetPair(0),
   fSetFg(0),
   fSetBold(0),
   fSetDefault(0),
   fStartUnderline(0),
   fStopUnderline(0),
   fPutc(DefaultPutchar)
   {
   // Initialize color management
   fOrigColors = GetTermStr("oc");
   ResetTerm();
   // Use pairs where possible
   fInitPair = GetTermStr("initp");
   fUsePairs = (fInitPair != 0);
   if (fUsePairs) {
      fSetPair = GetTermStr("scp");
      fCanChangeColors = true;
   } else {
      fInitColor = GetTermStr("initc");
      fCanChangeColors = (fInitColor != 0);
      fSetFg = GetTermStr("setaf");
      fAnsiColors = true;
      if (!fSetFg) {
         fSetFg = GetTermStr("setf");
         fAnsiColors = false;
      }
   }

   fSetBold = GetTermStr("bold");
   fSetDefault = GetTermStr("rs2");
   fStartUnderline = GetTermStr("smul");
   fStopUnderline = GetTermStr("rmul");

   fColorCapable = fUsePairs || fSetFg;
}

int TTermManip::AllocColor(const Color& col) {
   ColorMap_t::iterator iCol2 = fColors.find(col);
   int idx = -1;
   if (iCol2 != fColors.end()) {
      idx = iCol2->second;
   } else {
      // inserted; set pair idx starting at fgStartColIdx
      idx = fColors.size() - 1 + fgStartColIdx;
      fColors[col] = idx;
      char* initcolcmd = 0;
      if (fUsePairs) {
         initcolcmd = tparm(fInitPair, idx, 0, 0, 0, col.fR, col.fG, col.fB, 0, 0);
      } else if (fInitColor) {
         initcolcmd = tparm(fInitColor, idx, col.fR, col.fG, col.fB, 0, 0, 0, 0, 0);
      }
      if (initcolcmd) {
         tputs(initcolcmd, 1, fPutc);
      }
   }
   return idx;
}

bool TTermManip::SetColor(unsigned char r, unsigned char g, unsigned char b) {
   // RGB colors range from 0 to 255
   if (fCanChangeColors) {
      int idx = AllocColor(Color(r,g,b));
      if (fSetPair) {
         WriteTerm(fSetPair, idx);
      } else if (fSetFg) {
         WriteTerm(fSetFg, idx);
      }
   } else if (fSetFg) {
      if (r > 127 || b > 127 || g > 127) {
         if (fSetBold) {
            WriteTerm(fSetBold);
         }
         r /= 2;
         b /= 2;
         g /= 2;
      } else {
         if (fSetDefault) {
            WriteTerm(fSetDefault);
         }
      }
      int sum = r + g + b;
      r = r > sum / 4;
      g = g > sum / 4;
      b = b > sum / 4;
      int idx = 0;
      if (fAnsiColors) {
         idx = r + (g * 2) + (b * 4);
      } else {
         idx = (r * 4) + (g * 2) + b;
      }
      WriteTerm(fSetFg, idx);
   } else {
      printf("ERROR cannot set colors!\n");
      return false;
   }
   return true;
}


char* TTermManip::GetTermStr(const char* cap) {
   char capid[8];
   strcpy(capid, cap);
   char* termstr = tigetstr(capid);
   if (termstr == (char*)-1) {
      printf("ERROR unknown capability %s\n", cap);
      return NULL;
   } else if (termstr == 0) {
      // printf("ERROR capability %s not supported\n", cap);
      return NULL;
   }
   return termstr;
}

bool TTermManip::WriteTerm(char* termstr) {
   if (!termstr) return false;
   tputs(tparm(termstr, 0, 0, 0, 0, 0, 0, 0, 0, 0), 1, fPutc);
   /*if (tputs(tparm(termstr, 0, 0, 0, 0, 0, 0, 0, 0, 0), 1, fPutc) == ERR) {
      printf("ERROR writing %s\n", termstr);
      return false;
   }*/
   fflush(stdout);
   return true;
}

bool TTermManip::WriteTerm(char* termstr, int i) {
   if (!termstr) return false;
   tputs(tparm(termstr, i, 0, 0, 0, 0, 0, 0, 0, 0), 1, fPutc);
   /*if (tputs(tparm(termstr, i, 0, 0, 0, 0, 0, 0, 0, 0), 1, fPutc) == ERR) {
      printf("ERROR writing %s %d\n", termstr, i);
      return false;
   }*/
   fflush(stdout);
   return true;
}

#ifdef TEST_TERMMANIP
void testcolor(TTermManip& tm, int r, int g, int b) {
   tm.SetColor(r,g,b);
   if (r%2) {
      tm.StartUnderline();
   }
   printf("HELLO %d %d %d\n", r, g, b);
   if (r%2) {
      tm.StopUnderline();
   }
}

void testall(TTermManip& tm, int h) {
   testcolor(tm, h, 0, 0);
   testcolor(tm, 0, h, 0);
   testcolor(tm, 0, 0, h);
   testcolor(tm, h, h, 0);
   testcolor(tm, h, 0, h);
   testcolor(tm, 0, h, h);
   testcolor(tm, h, h, h);
}

int main(int, char*[]) {
   int errcode;
   if (ERR == setupterm(0, 1, &errcode)) {
      printf("ERROR in setupterm: %d\n", errcode);
      return 1;
   }
   TTermManip tm;
   testall(tm, 31);
   testall(tm, 127);
   testall(tm, 128);
   testall(tm, 255);

   testcolor(tm, 0, 0, 0);

   return 0;
}
#endif
