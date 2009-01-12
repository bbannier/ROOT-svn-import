// Copyright (C) 2008, Rene Brun and Fons Rademakers.
// All rights reserved. License: MIT/X11, see LICENSE
// Author: Axel Naumann, 2008

#include "Inuit/Widgets/EditLine.h"
#include "Inuit/Widgets/Frame.h"
#include "Inuit/Drivers/TerminalDriver.h"
#include "Inuit/Event.h"
#include <assert.h>

using namespace Inuit;

void EditLine::SetText(const char* text) {
   if (fText == text) return;
   fText = text;
   fCursorPos = 0;
   fCropLeft = 0;
   Draw();
}

bool EditLine::HandleEvent(const Event& event) {
   static const char* word_delim = " .,;:!@#$%^&*()-+={}[]\\|\"'<>~`";
   const Input& input = event.GetInput();
   if (event.Get() == Event::kRawInput && input.GetType() == Input::kTypePrintable) {
      if (!(input.GetModifiers() & (Input::kModAlt | Input::kModCtrl | Input::kModUp)) 
         && (fMaxLen < 0 || fText.length() < (size_t) fMaxLen)) {
         if (fInsert)
            fText.insert(fCursorPos, std::string(1, input.GetPrintable()));
         else 
            fText.replace(fCursorPos, 1, std::string(1, input.GetPrintable()));
         ++fCursorPos;
         Draw();
      } else return false;
   } else {
      switch (event.Get()) {
         case Event::kMoveLeft:
            if (fCursorPos <= 0) break;
            --fCursorPos;
            Draw();
            break;
         case Event::kMoveWordLeft:
            if (fCursorPos <= 0) break;
            if (fCursorPos == 1) fCursorPos = 0;
            else {
               size_t pos = GetText().find_last_of(word_delim, fCursorPos - 2);
               if (pos == std::string::npos) fCursorPos = 0;
               else fCursorPos = pos + 1;
            }
            Draw();
            break;
         case Event::kMoveRight:
            if (fCursorPos >= (int) fText.length()) break;
            ++fCursorPos;
            Draw();
            break;
         case Event::kMoveWordRight:
            if (fCursorPos >= (int) fText.length()) break;
            if (fCursorPos == (int) fText.length() - 1) fCursorPos = (int) fText.length();
            else {
               size_t pos = GetText().find_first_of(word_delim, fCursorPos + 2);
               if (pos == std::string::npos) fCursorPos = (int) fText.length();
               else fCursorPos = pos - 1;
            }
            Draw();
            break;
         case Event::kToggleInsert:
            fInsert = !fInsert;
            GetParent()->GetTerminalDriver().SetCursorSize(fInsert ? 20 : 100);
            break;
         case Event::kDelete:
            if (fCursorPos < (int) fText.length()) {
               fText.erase(fCursorPos, 1);
               Draw();
            }
            break;
         case Event::kBackspace:
            if (fCursorPos > 0) {
               --fCursorPos;
               fText.erase(fCursorPos, 1);
               Draw();
            }
            break;
         default: return false;
      }
   }
   return true;
}

void EditLine::Draw() {
   TerminalDriver& td = GetParent()->GetTerminalDriver();
   Pos pos = GetPos();
   td.Goto(pos);

   // This is what the most complex line looks like:
   // fText might be "aiuilahilughslihgsfd_hkwlahlhkakjghkldshfhgshdkjshvk"
   // Display:
   // SHORTPROMPT: << iuilahilughslihgsfd_hkwlahlhkakjghkldshfhg >>
   // fCropLeft is this 1 ("a" is not shown)

   bool leftOK  = fCursorPos >= fCropLeft; // left
   // Now adjust left if needed:
   if (!leftOK) {
      // The cursor left the view at the left side. Reduce fCropLeft until it is visible again:
      assert(fCropLeft);
      fCropLeft = fCursorPos;
      assert(fCropLeft >= 0);
   }

   size_t promptLenLong = fPromptLen;
   size_t promptLenExt = fPromptShortLen + fPromptCutLeft.length();
   size_t promptLen = fCropLeft ? promptLenExt : fPromptLen;

   size_t numCharsShown = fWidth - promptLen;

   int cropRight = fCropLeft + numCharsShown;
   if (cropRight > GetText().length()) cropRight = 0;
   else cropRight -= fPromptCutRight.length();
   bool rightOK = !cropRight || fCursorPos < cropRight;
   // Adjust right if needed:
   if (!rightOK) {
      // The cursor left the view at the right side. Increase fCropLeft until it is visible again:
      cropRight = fCursorPos + 1;
      fCropLeft = cropRight - (fWidth - promptLenExt);
      if (cropRight > GetText().length()) cropRight = 0;
      else fCropLeft += fPromptCutRight.length();
      assert(fCropLeft);
   }

   assert(!cropRight || fCursorPos < cropRight);
   assert(fCursorPos >= fCropLeft);

   std::string text(fCropLeft ? (fPromptShort + fPromptCutLeft) : fPrompt);
   text += GetText().substr(fCropLeft, cropRight ? cropRight - fCropLeft : -1);
   if (cropRight) text += fPromptCutRight;
   else text += std::string(fWidth - text.length() - 1, ' ');

   td.WriteString(text.c_str());
   pos.fW = fCursorPos - fCropLeft + (fCropLeft ? promptLenExt : fPromptLen);
   td.Goto(pos);
}
