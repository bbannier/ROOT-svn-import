// Copyright (C) 2008, Rene Brun and Fons Rademakers.
// All rights reserved. License: MIT/X11, see LICENSE
// Author: Axel Naumann, 2008

#ifndef INCLUDE_INUIT_EVENTDRIVERWIN
#define INCLUDE_INUIT_EVENTDRIVERWIN

#include "Inuit/Drivers/InputDriver.h"

namespace Inuit {

   class InputDriverWin: public InputDriver {
   public:
      InputDriverWin(TerminalDriver& term);
      ~InputDriverWin();

      EInputStatus ProcessOneInput(Input& e);
      EInputStatus ProcessAvailableInputs(std::list<Input>& e, bool block = false);
      bool InputsAvailable() const;
   };
}

#endif // INCLUDE_INUIT_EVENTDRIVERWIN
