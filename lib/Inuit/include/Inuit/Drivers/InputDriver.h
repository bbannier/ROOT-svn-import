// Copyright (C) 2008, Rene Brun and Fons Rademakers.
// All rights reserved. License: MIT/X11, see LICENSE
// Author: Axel Naumann, 2008

#ifndef INCLUDE_INUIT_EVENTDRIVER
#define INCLUDE_INUIT_EVENTDRIVER

#include <list>
#include <set>
#include "Inuit/Input.h"

namespace Inuit {

   class TerminalDriver;

   class InputDriver {
   public:
      virtual ~InputDriver() {}

      static InputDriver& Instance(TerminalDriver& term);

      enum EInputStatus {
         kESIdle, // found no events to process, event parameter is invalid 
         kESProcessed, // found events to process and is now done, event parameter is valid
         kESInterrupted, // ProcessAvailableInputs() returns prematurely because of an interrupt event
         kNumInputStatus
      };

      virtual EInputStatus ProcessOneInput(Input& e) = 0;
      virtual EInputStatus ProcessAvailableInputs(std::list<Input>& e, bool block = false) = 0;
      virtual bool InputsAvailable() const { return true; }

      TerminalDriver& GetTerminalDriver() const { return *fTerm; }

      void AddInterruptInput(const Input& e);
      const std::set<Input>& GetInterruptInputs() const { return fInterrupts; } 

   protected:
      InputDriver(TerminalDriver& term): fTerm(&term) {}

   private:
      std::set<Input> fInterrupts; // events causing an interrupt of ProcessAvailableInputs()
      TerminalDriver* fTerm; // terminal driver
   };
}

#endif // INCLUDE_INUIT_EVENTDRIVER
