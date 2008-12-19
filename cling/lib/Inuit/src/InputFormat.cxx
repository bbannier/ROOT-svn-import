// Copyright (C) 2008, Rene Brun and Fons Rademakers.
// All rights reserved. License: MIT/X11, see LICENSE
// Author: Axel Naumann, 2008

#include "Inuit/Formats/InputFormat.h"
#include "Inuit/Event.h"

using namespace Inuit;

InputFormat::InputFormat(const char* config) {
}

Event InputFormat::InterpretInput(const Input& i) const {
   Event e(Event::kRawInput, i);

   if (i.GetType() == Input::kTypePrintable) return e;
   else if (i.GetType() == Input::kTypeNonPrintable) {
      if (i.GetModifiers() & Input::kModUp)
         return e;
      if (i.GetModifiers() & Input::kModCtrl) {
         switch (i.GetNonPrintable()) {
            case Input::kNPArrowLeft: e.Set(Event::kMoveWordLeft); break;
            case Input::kNPArrowRight: e.Set(Event::kMoveWordRight); break;
         }
      } else {
         switch (i.GetNonPrintable()) {
         case Input::kNPArrowLeft:
         case Input::kNPArrowRight:
         case Input::kNPArrowUp:
         case Input::kNPArrowDown:
         case Input::kNPInsert:
         case Input::kNPDelete:
         case Input::kNPBackspace:
         case Input::kNPEnter:
            e.Set((Event::EEvent)i.GetNonPrintable()); break;
         }
      }
   }
   return e;
}
