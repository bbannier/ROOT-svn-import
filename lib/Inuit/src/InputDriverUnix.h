// Copyright (C) 2008,  Rene Brun and Fons Rademakers.
// All rights reserved.
// License: LGPL'ed v3 or later (up to you), see http://www.gnu.org/licenses/lgpl.html
// Author: Axel Naumann, 2008

#ifndef INCLUDE_INUIT_EVENTDRIVERUNIX
#define INCLUDE_INUIT_EVENTDRIVERUNIX

#include "Inuit/Drivers/InputDriver.h"

namespace Inuit {

   class InputDriverUnix: public InputDriver {
   public:
      InputDriverUnix(TerminalDriver& term);
      ~InputDriverUnix();

      EInputStatus ProcessOneInput(Input& e);
      EInputStatus ProcessAvailableInputs(std::list<Input>& e, bool block = false);
      bool InputsAvailable() const;
   };
}

#endif // INCLUDE_INUIT_EVENTDRIVERUNIX
