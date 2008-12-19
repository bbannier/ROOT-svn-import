// Copyright (C) 2008,  Rene Brun and Fons Rademakers.
// All rights reserved.
// License: LGPL'ed v3 or later (up to you), see http://www.gnu.org/licenses/lgpl.html
// Author: Axel Naumann, 2008

#ifndef INCLUDE_INUIT_TERMINALDRIVERUNIX
#define INCLUDE_INUIT_TERMINALDRIVERUNIX

#include "Inuit/Drivers/TerminalDriver.h"

namespace Inuit {

   class TerminalDriverUnix: public TerminalDriver {
   public:
      TerminalDriverUnix();
      ~TerminalDriverUnix();

      EErrorCode Goto(const Pos& p);
      EErrorCode WriteChar(char c);
      EErrorCode WriteString(const char* s);
      EErrorCode SetForegroundColor(int fg) { return kErrSuccess; }
      EErrorCode SetBackgroundColor(int bg) { return kErrSuccess; }
      EErrorCode SetCursorSize(char percent_size = 20);
      EErrorCode Clear();
      EErrorCode SetManagedMode(bool managed);
      Pos GetSize() const;

   private:
      int fConsoleOffsetY; // distance of buffer's line 0 to screen's line 0
      bool fManaged; // raw I/O (managed) or high-level
   };
}

#endif // INCLUDE_INUIT_TERMINALDRIVERUNIX
