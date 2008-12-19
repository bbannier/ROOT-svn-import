// Copyright (C) 2008, Rene Brun and Fons Rademakers.
// All rights reserved. License: MIT/X11, see LICENSE
// Author: Axel Naumann, 2008

#include "Inuit/Drivers/InputDriver.h"

#ifdef _WIN32
#define MT_EVENTDRIVER InputDriverWin
#include "InputDriverWin.h"
#else
#define MT_EVENTDRIVER InputDriverUnix
#include "InputDriverUnix.h"
#endif

using namespace Inuit;

InputDriver& InputDriver::Instance(TerminalDriver& term) {
   static MT_EVENTDRIVER s(term);
   return s;
}

void InputDriver::AddInterruptInput(const Input& e) {
   fInterrupts.insert(e);
}

