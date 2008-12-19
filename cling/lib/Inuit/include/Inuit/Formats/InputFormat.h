// Copyright (C) 2008, Rene Brun and Fons Rademakers.
// All rights reserved. License: MIT/X11, see LICENSE
// Author: Axel Naumann, 2008

#ifndef INCLUDE_INUIT_InputFormat
#define INCLUDE_INUIT_InputFormat

#include "Inuit/Event.h"
#include "Inuit/Input.h"

#include <map>

namespace Inuit {
   class InputFormat {
   public:
      InputFormat(const char* config = 0);
      virtual ~InputFormat() {}

      virtual Event InterpretInput(const Input& e) const;
   private:
      std::map<Input, Event> fFormatMap;
   };
}

#endif // INCLUDE_INUIT_InputFormat
