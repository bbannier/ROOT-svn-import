// Copyright (C) 2008, Rene Brun and Fons Rademakers.
// All rights reserved. License: MIT/X11, see LICENSE
// Author: Axel Naumann, 2008

#ifndef INCLUDE_INUIT_WIDGET
#define INCLUDE_INUIT_WIDGET

#include "Inuit/Basics.h"
#include "Inuit/Event.h"

namespace Inuit {
   class Frame;

   class Widget {
   public:
      Widget(Frame* parent, const Pos& pos): fPos(pos), fParent(parent), fFlags(0) {}
      virtual ~Widget() {}

      const Pos& GetPos() const { return fPos; }
      Frame* GetParent() const { return fParent; }
      virtual Pos GetSize() const = 0;
      bool IsEnabled() const { return (fFlags & kEnabled) != 0; }
      bool HasFocus() const { return (fFlags & kHasFocus) != 0; }
      virtual bool HandleEvent(const Event& event);
      virtual Widget* GetFocusedWidget() {
         return this;
      }

      virtual void Draw() = 0;
      virtual void SetFocus() {
         if (IsEnabled()) fFlags |= kHasFocus;
         // and redraw with white instead of grey...
      };

   protected:
      enum EFlags {
         kEnabled = 0x01, // can get focus
         kHasFocus = 0x02
      };
   private:
      Pos fPos;
      Frame* fParent;
      char fFlags;
   };
}

#endif // INCLUDE_INUIT_WIDGET
