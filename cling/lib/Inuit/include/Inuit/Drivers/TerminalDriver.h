// Copyright (C) 2008, Rene Brun and Fons Rademakers.
// All rights reserved. License: MIT/X11, see LICENSE
// Author: Axel Naumann, 2008

#ifndef INCLUDE_INUIT_TERMINALDRIVER
#define INCLUDE_INUIT_TERMINALDRIVER

#include "Inuit/Basics.h"

namespace Inuit {

   class TerminalDriver {
   public:
      virtual ~TerminalDriver();

      static TerminalDriver& Instance();

      enum EErrorCode {
         kErrSuccess,
         kUnsupported,
         kOutOfRange,
         kInvalidParameter,
         kNumErrorCodes
      };

      virtual void Update() = 0;
      virtual EErrorCode Goto(const Pos& p) = 0;
      virtual EErrorCode WriteChar(char c) = 0;
      virtual EErrorCode WriteString(const char* s) = 0;
      virtual EErrorCode SetForegroundColor(int fg) = 0;
      virtual EErrorCode SetBackgroundColor(int bg) = 0;
      virtual EErrorCode SetCursorSize(char percent_size = 20) = 0;
      virtual EErrorCode Clear() = 0;
      virtual EErrorCode PageFeed();

      virtual EErrorCode SetManagedMode(bool managed) = 0;

      int GetCursorSize() const { return fSize; }
      virtual Pos GetCursorPos() const = 0;
      virtual Pos GetSize() const = 0;

   protected:
      TerminalDriver(): fSize(20) {}

   protected:
      Pos fPos; // cursor position
      int fSize; // cursor size in percent
   };
}
#endif // INCLUDE_INUIT_TERMINALDRIVER
