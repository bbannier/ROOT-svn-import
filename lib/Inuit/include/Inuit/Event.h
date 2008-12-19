// Copyright (C) 2008, Rene Brun and Fons Rademakers.
// All rights reserved. License: MIT/X11, see LICENSE
// Author: Axel Naumann, 2008

#ifndef INCLUDE_INUIT_EVENT
#define INCLUDE_INUIT_EVENT

#include "Inuit/Input.h"

namespace Inuit {
   class Event {
   public:
      enum EEvent {
         kMoveLeft = Input::kNPArrowLeft,
         kMoveRight = Input::kNPArrowRight,
         kMoveUp = Input::kNPArrowUp,
         kMoveDown = Input::kNPArrowDown,
         kMoveBegin = Input::kNPHome,
         kMoveEnd = Input::kNPEnd,
         kToggleInsert = Input::kNPInsert,
         kDelete = Input::kNPDelete,
         kBackspace = Input::kNPBackspace,

         kEEvent_No_Input_Translation = Input::kNumNonPrintable,
         kRawInput, // default
         kMoveWordLeft,
         kMoveWordRight,
         kSelectBegin,
         kSelectExtend,
         kSelectEnd,
         kCutWord,
         kCutLine,
         kCutSelection,
         kCopyWord,
         kCopyLine,
         kCopySelection,
         kPaste,
         kNumEvents
      };

      Event(EEvent in, const Input& ev): fEvent(in), fInput(ev) {}

      EEvent Get() const { return fEvent; }
      const Input& GetInput() const { return fInput; }

      void Set(EEvent ev) { fEvent = ev; }
      void SetInput(const Input& input) { fInput = input; }
   private:
      EEvent fEvent;
      Input  fInput;
   };
}

#endif // INCLUDE_INUIT_EVENT
