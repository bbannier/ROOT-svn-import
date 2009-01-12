// Copyright (C) 2008, Rene Brun and Fons Rademakers.
// All rights reserved. License: MIT/X11, see LICENSE
// Author: Axel Naumann, 2008

#include "Inuit/Drivers/TerminalDriver.h"

#include <stdio.h>

#ifdef _WIN32
#include "TerminalDriverWin.h"
#define MT_TERMINALDRIVER TerminalDriverWin
#else
#include "TerminalDriverUnix.h"
#define MT_TERMINALDRIVER TerminalDriverUnix
#endif

using namespace Inuit;

TerminalDriver& TerminalDriver::Instance() {
   static MT_TERMINALDRIVER s;
   return s;
}

TerminalDriver::~TerminalDriver() {}

TerminalDriver::EErrorCode TerminalDriver::PageFeed() {
// Clear the screen by inserting empty lines into the buffer:
   for (int nLines = GetSize().fH; nLines > 0; --nLines)
      printf("\n");
   return kErrSuccess;
}
