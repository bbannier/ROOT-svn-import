// Copyright (C) 2008, Rene Brun and Fons Rademakers.
// All rights reserved. License: MIT/X11, see LICENSE
// Author: Axel Naumann, 2008

#ifndef INCLUDE_INUIT_BASICS
#define INCLUDE_INUIT_BASICS

namespace Inuit {
   struct Pos {
      Pos() {}
      Pos(int x, int y): fX(x), fY(y) {}
      int fX;
      int fY;

      Pos operator +(const Pos& p) const { return Pos(fX + p.fX, fY + p.fY); }
      Pos operator -(const Pos& p) const { return Pos(fX - p.fX, fY - p.fY); }
      Pos& operator +=(const Pos& p) { fX += p.fX; fY += p.fY; return *this; }
      Pos& operator -=(const Pos& p) { fX -= p.fX, fY -= p.fY; return *this; }
   };
}

#endif // INCLUDE_INUIT_BASICS
