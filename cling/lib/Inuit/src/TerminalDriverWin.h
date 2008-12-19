// Copyright (C) 2008, Rene Brun and Fons Rademakers.
// All rights reserved. License: MIT/X11, see LICENSE
// Author: Axel Naumann, 2008

#ifndef INCLUDE_INUIT_TERMINALDRIVERWIN
#define INCLUDE_INUIT_TERMINALDRIVERWIN

#include "Inuit/Drivers/TerminalDriver.h"
#include <windows.h> 

namespace Inuit {

   class TerminalDriverWin: public TerminalDriver {
   public:
      TerminalDriverWin();
      ~TerminalDriverWin();

      EErrorCode Goto(const Pos& p);
      EErrorCode WriteChar(char c);
      EErrorCode WriteString(const char* s);
      EErrorCode SetForegroundColor(int fg) { return kErrSuccess; }
      EErrorCode SetBackgroundColor(int bg) { return kErrSuccess; }
      EErrorCode SetCursorSize(char percent_size = 20);
      EErrorCode Clear();
      EErrorCode SetManagedMode(bool managed);
      Pos GetSize() const;

      HANDLE GetStdIn()  const { return fStdIn; }
      HANDLE GetStdOut() const { return fStdOut; }

   private:
      HANDLE fStdIn;
      HANDLE fStdOut;
      int fConsoleOffsetY; // distance of buffer's line 0 to screen's line 0
      bool fManaged; // raw I/O (managed) or high-level
      DWORD fStartupConsoleModeIn; // console mode settings at startup
      DWORD fStartupConsoleModeOut; // console mode settings at startup
   };
}

#endif INCLUDE_INUIT_TERMINALDRIVERWIN
