// Copyright (C) 2008, Rene Brun and Fons Rademakers.
// All rights reserved. License: MIT/X11, see LICENSE
// Author: Axel Naumann, 2008

#include "Inuit/Widgets/EditLine.h"
#include "Inuit/Widgets/Frame.h"
#include "Inuit/Drivers/TerminalDriver.h"
#include "Inuit/Event.h"
#include <string.h>

using namespace Inuit;

void EditLine::SetText(const char* text) {
   if (fText == text) return;
   fText = text;
   fLen = strlen(text);
   fCursorPos = 0;
   fLeftmostChar = -1;
   Draw();
}

bool EditLine::HandleEvent(const Event& event) {
   static const char* word_delim = " .,;:!@#$%^&*()-+={}[]\\|\"'<>~`";
   const Input& input = event.GetInput();
   if (event.Get() == Event::kRawInput && input.GetType() == Input::kTypePrintable) {
      if (!(input.GetModifiers() & (Input::kModAlt | Input::kModCtrl | Input::kModUp)) 
         && (fMaxChar < 0 || fText.length() < (size_t) fMaxChar)) {
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

   if (!fLen) return;

   if (fLeftmostChar > fCursorPos)
      fLeftmostChar = fCursorPos;
   else if (fLeftmostChar + fLen < fCursorPos + 1)
      fLeftmostChar = fCursorPos + 1 - fLen;

   // >>_ ab
   if (fLeftmostChar != 0
      && fLeftmostChar + (int) fPromptCutLeft.length() > fCursorPos)
      fLeftmostChar = fCursorPos - fPromptCutLeft.length();
   if (fLeftmostChar < 0) fLeftmostChar = 0;

   int len = GetText().length();
   //         yz _<<
   if (len - fLeftmostChar >= fLen
      && fLeftmostChar + fLen - (int) fPromptCutRight.length() <= fCursorPos)
      fLeftmostChar = fCursorPos - fLen + (int) fPromptCutRight.length() + 1;

   std::string text(GetText().substr(fLeftmostChar));

   if (fLeftmostChar != 0)
      text.replace(0, fPromptCutLeft.length(), fPromptCutLeft);

   len = text.length();
   if (len >= fLen) {
      text.replace(fLen - fPromptCutRight.length(), fPromptCutRight.length(), fPromptCutRight);
      text.erase(fLen);
   } else {
      text += std::string(fLen - text.length(), ' ');
   }
   td.WriteString(text.c_str());
   pos.fX = fCursorPos - fLeftmostChar;
   td.Goto(pos);
}
