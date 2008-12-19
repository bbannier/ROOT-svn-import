// Copyright (C) 2008, Rene Brun and Fons Rademakers.
// All rights reserved. License: MIT/X11, see LICENSE
// Author: Axel Naumann, 2008

#ifndef INCLUDE_INUIT_UI
#define INCLUDE_INUIT_UI

#include "Inuit/Widgets/Frame.h"
#include "Inuit/Drivers/InputDriver.h"
#include "Inuit/Formats/InputFormat.h"
#include <set>
#include <map>

namespace Inuit {
   class UI;
   class InputFormat;

   typedef bool (*InputHandler_t)(UI& ui, const Input&, void* userParam);

   class UI: public Frame {
   public:
      UI(const Pos& size, InputDriver& ed):
         Frame(0, Pos(0, 0), size),
         fInputDriver(ed), fQuitRequested(false) {}
      virtual ~UI() {}

      TerminalDriver& GetTerminalDriver() const {
         return fInputDriver.GetTerminalDriver(); }
      bool HandleEvent(const Event& event) {
         return false;
      }

      void ProcessInputs();

      void RequestQuit() { fQuitRequested = true; }
      void AddInputHandler(InputHandler_t handler, void* userParam) {
         fHandlers.insert(std::make_pair(handler, userParam));
      }

      void SetInputFormat(const InputFormat& format) {
         fInputFormat = format;
      }

   private:
      bool PassToHandlers(const Input& e);

      InputDriver& fInputDriver;
      InputFormat fInputFormat;
      std::set<std::pair<InputHandler_t, void*> > fHandlers;
      bool fQuitRequested;
   };
}

#endif // INCLUDE_INUIT_UI
