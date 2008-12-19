// Copyright (C) 2008, Rene Brun and Fons Rademakers.
// All rights reserved. License: MIT/X11, see LICENSE
// Author: Axel Naumann, 2008

#ifndef INCLUDE_INUIT_INPUT
#define INCLUDE_INUIT_INPUT

#include <string>

namespace Inuit {
   class Input {
   public:

#define ADD_ENUMCONST(ID, VALUE) ID VALUE

      enum EType {
#include "Inuit/Enums/EType.h"
      };
      enum ENonPrintable {
#include "Inuit/Enums/ENonPrintable.h"
      };

      enum EMouse {
#include "Inuit/Enums/EMouse.h"
      };

      enum EWindow {
#include "Inuit/Enums/EWindow.h"
      };

      enum EModifiers {
#include "Inuit/Enums/EModifiers.h"
      };

#undef ADD_ENUMCONST

      const char* GetETypeName() const; 
      const char* GetENonPrintableName() const; 
      const char* GetEMouseName() const;
      const char* GetEWindowName() const;
      void GetEModifierName(std::string& name) const;

      Input(EType type = kNumTypes, int data = 0, int repeat = 0, int modifiers = 0):
         fType(type), fNonPrintable((ENonPrintable)data), fRepeat(repeat), fModifiers(modifiers) {}

      void Set(EType type = kNumTypes, int data = 0, int repeat = 0, int modifiers = 0) {
         fType = type; fNonPrintable = (ENonPrintable) data; fRepeat = repeat; fModifiers = modifiers;
      }

      EType GetType() const { return fType; }
      char GetPrintable() const { return fPrintable; }
      ENonPrintable GetNonPrintable() const { return fNonPrintable; }
      char GetMouse() const { return fMouse; }
      EWindow GetWindow() const { return fWindow; }
      int GetModifiers() const { return fModifiers; }
      int GetRepeat() const { return fRepeat; }

      bool operator<(const Input& e) const {
         return fType < e.fType 
            || fType == e.fType
               && (fPrintable < e.fPrintable 
                   || fPrintable == e.fPrintable && fModifiers < e.fModifiers);
      }
      bool operator==(const Input& e) const {
         return fType == e.fType && fPrintable == e.fPrintable && fModifiers == e.fModifiers;
      }
      bool operator!=(const Input& e) const {
         return fType != e.fType || fPrintable != e.fPrintable || fModifiers != e.fModifiers;
      }

   private:
      EType fType;
      union {
         char fPrintable;
         ENonPrintable fNonPrintable;
         char fMouse; // mask characterizing mouse event
         EWindow fWindow;
      };
      int fRepeat; // repitition count, e.g. for key-pressed
      int fModifiers; // mask of modifiers
   };
}

#endif // INCLUDE_INUIT_INPUT
