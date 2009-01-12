// Copyright (C) 2008, Rene Brun and Fons Rademakers.
// All rights reserved. License: MIT/X11, see LICENSE
// Author: Axel Naumann, 2008

#ifndef INCLUDE_INUIT_BASICS
#define INCLUDE_INUIT_BASICS

namespace Inuit {
   struct Pos {
      Pos() {}
      Pos(int x, int y): fW(x), fH(y) {}
      int fW;
      int fH;

      Pos operator +(const Pos& p) const { return Pos(fW + p.fW, fH + p.fH); }
      Pos operator -(const Pos& p) const { return Pos(fW - p.fW, fH - p.fH); }
      Pos& operator +=(const Pos& p) { fW += p.fW; fH += p.fH; return *this; }
      Pos& operator -=(const Pos& p) { fW -= p.fW, fH -= p.fH; return *this; }
   };
}

#endif // INCLUDE_INUIT_BASICS
