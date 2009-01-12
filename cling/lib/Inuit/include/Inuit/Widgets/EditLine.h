// Copyright (C) 2008, Rene Brun and Fons Rademakers.
// All rights reserved. License: MIT/X11, see LICENSE
// Author: Axel Naumann, 2008

#ifndef INCLUDE_INUIT_EDITLINE
#define INCLUDE_INUIT_EDITLINE

#include "Inuit/Widgets/Widget.h"
#include <string>
#include <string.h>

namespace Inuit {
   class EditLine: public Widget {
   public:
      EditLine(Frame* parent, const Pos& pos, int width, const char* prompt,
               const char* prompt_short = 0):
         Widget(parent, pos), fWidth(width), fMaxLen(-1), fCursorPos(0), fCropLeft(0),
         fInsert(true), fPrompt(prompt), fPromptShort(prompt_short ? prompt_short : prompt),
         fPromptLen(strlen(prompt)), fPromptShortLen(strlen(prompt_short ? prompt_short : prompt)),
         fPromptCutLeft("<< "), fPromptCutRight(" >>") {}
      ~EditLine() {}

      Pos GetSize() const { return Pos(fWidth, 1); }
      const std::string& GetText() const { return fText; }

      void SetText(const char* text);

      void SetMaxCharacters(int maxlen = -1) {
         // maximum text size; -1 means none
         fMaxLen = maxlen;
      }

      void SetPromptCutLeft(const char* p) { fPromptCutLeft = p; }
      void SetPromptCutRight(const char* p) { fPromptCutRight = p; }

      bool HandleEvent(const Event& event);
      void Draw();
      virtual void SetFocus() {};

   private:
      int fWidth; // size of the line
      int fMaxLen; // maximum text size
      int fCursorPos; // position of cursor within string
      int fCropLeft; // index of leftmost visible char within string
      bool fInsert; // or overwrite mode
      std::string fText;
      std::string fPrompt;
      std::string fPromptShort;
      size_t fPromptLen;
      size_t fPromptShortLen;
      std::string fPromptCutLeft;
      std::string fPromptCutRight;
   };
}


#endif // INCLUDE_INUIT_EDITLINE
