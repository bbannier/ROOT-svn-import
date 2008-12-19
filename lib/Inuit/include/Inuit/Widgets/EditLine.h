// Copyright (C) 2008, Rene Brun and Fons Rademakers.
// All rights reserved. License: MIT/X11, see LICENSE
// Author: Axel Naumann, 2008

#ifndef INCLUDE_INUIT_EDITLINE
#define INCLUDE_INUIT_EDITLINE

#include "Inuit/Widgets/Widget.h"
#include <string>

namespace Inuit {
   class EditLine: public Widget {
   public:
      EditLine(Frame* parent, const Pos& pos, int len):
         Widget(parent, pos), fLen(len), fMaxChar(-1), fCursorPos(0), fLeftmostChar(0),
            fInsert(true), fPromptCutLeft("<< "), fPromptCutRight(" >>") {}
      ~EditLine() {}

      Pos GetSize() const { return Pos(fLen, 1); }
      const std::string& GetText() const { return fText; }

      void SetText(const char* text);

      void SetMaxCharacters(int maxlen = -1) {
         // maximum text size; -1 means none
         fMaxChar = maxlen;
      }

      void SetPromptCutLeft(const char* p) { fPromptCutLeft = p; }
      void SetPromptCutRight(const char* p) { fPromptCutRight = p; }

      bool HandleEvent(const Event& event);
      void Draw();
      virtual void SetFocus() {};

   private:
      int fLen; // size of the line
      int fMaxChar; // maximum text size
      int fCursorPos; // position of cursor within string
      int fLeftmostChar; // index of leftmost visible char within string
      bool fInsert; // or overwrite mode
      std::string fText;
      std::string fPromptCutLeft;
      std::string fPromptCutRight;
   };
}


#endif // INCLUDE_INUIT_EDITLINE
