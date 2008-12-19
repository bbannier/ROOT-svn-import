// Copyright (C) 2008,  Rene Brun and Fons Rademakers.
// All rights reserved.
// License: LGPL'ed v3 or later (up to you), see http://www.gnu.org/licenses/lgpl.html
// Author: Axel Naumann, 2008

#include "InputDriverUnix.h"

#include "TerminalDriverUnix.h"
#include <ncurses/curses.h>

using namespace Inuit;

InputDriverUnix::InputDriverUnix(TerminalDriver& term):
InputDriver(term) {}

InputDriverUnix::~InputDriverUnix() {}

InputDriver::EInputStatus InputDriverUnix::ProcessOneInput(Input& e) {
   return InputDriver::kESIdle;
}

InputDriver::EInputStatus InputDriverUnix::ProcessAvailableInputs(std::list<Input>& e, bool block) {
   TerminalDriverUnix& term = (TerminalDriverUnix&) GetTerminalDriver();

   nodelay(stdscr, !block);
   int c = getch();
   if (c == ERR) return kESIdle;

   Input ev;
   int data = 0;
   int modifier = 0;
   Input::EType type = Input::kTypeNonPrintable;
   switch (c & ~128) {
      case 27: // (terminal?) escape:
         data = Input::kNPEscape;
         c = getch();
         if (c == 91) {
            c = getch();
            switch (c) {
               case 50: data = Input::kNPInsert; break;
               case 51: data = Input::kNPDelete; break;
               case 1:
                  c = getch();
                  switch (c) {
                     case 59:
                        c = getch();
                        switch (c) {
                           case 53:
                              c = getch();
                              switch (c) {
                                 case 67: data = Input::kNPArrowRight; modifier = Input::kModCtrl; break;
                                 case 68: data = Input::kNPArrowLeft;  modifier = Input::kModCtrl; break;
                              }
                              break;
                        }
                        break;
                  }
                  break;
               default: break;
            };
         } else if (c != ERR)
            ungetch(c);

      case 515: data = Input::kNPArrowLeft;  modifier = Input::kModCtrl; break;
      case 517: data = Input::kNPArrowRight; modifier = Input::kModCtrl; break;
      case 520: data = Input::kNPArrowUp; modifier = Input::kModCtrl; break;
      case 513: data = Input::kNPArrowDown; modifier = Input::kModCtrl; break;
      case KEY_BREAK: data = Input::kNPBreak; modifier = Input::kModCtrl; break;
      case KEY_BACKSPACE: data = Input::kNPBackspace; break;
      case 9: data = Input::kNPTab; break;
      case KEY_ENTER: data = Input::kNPEnter; break;
      case ' ': type = Input::kTypePrintable; data = ' '; break;
      case KEY_PPAGE: data = Input::kNPPageUp; break;
      case KEY_NPAGE: data = Input::kNPPageDown; break;
      case KEY_END: data = Input::kNPEnd; break;
      case KEY_HOME: data = Input::kNPHome; break;
      case KEY_LEFT: data = Input::kNPArrowLeft; break;
      case KEY_UP: data = Input::kNPArrowUp; break;
      case KEY_RIGHT: data = Input::kNPArrowRight; break;
      case KEY_DOWN: data = Input::kNPArrowDown; break;
      case KEY_PRINT: data = Input::kNPPrintScreen; break;
      case '*':
      case '+':
      case '-':
      case '.':
      case '/': type = Input::kTypePrintable; break;
      case KEY_F(1): data = Input::kNPF1; break;
      case KEY_F(2): data = Input::kNPF2; break;
      case KEY_F(3): data = Input::kNPF3; break;
      case KEY_F(4): data = Input::kNPF4; break;
      case KEY_F(5): data = Input::kNPF5; break;
      case KEY_F(6): data = Input::kNPF6; break;
      case KEY_F(7): data = Input::kNPF7; break;
      case KEY_F(8): data = Input::kNPF8; break;
      case KEY_F(9): data = Input::kNPF9; break;
      case KEY_F(10): data = Input::kNPF10; break;
      case KEY_F(11): data = Input::kNPF11; break;
      case KEY_F(12): data = Input::kNPF12; break;
      default:
         if (data == 0 && ((c & 127) >= '0' && (c & 127) <= '9'
            || (c & 127) >= 'A' && (c & 127) <= 'Z'
            || (c & 127) >= 'a' && (c & 127) <= 'z'
            || (c & 127) < 32))
            type = Input::kTypePrintable;
         else type = Input::kNumTypes;
   };
   if (type == Input::kNumTypes) return kESIdle;
   if (type == Input::kTypePrintable)
      data = c;

   modifier |= Input::kModDown;

   if (c & 128)
      modifier |= Input::kModAlt;
   if ((c & 127) < 32) {
      modifier |= Input::kModCtrl;
      data = c + '@'; // ^@ == 0.
   }
   if (isupper(c & 127))
      modifier |= Input::kModShift;
   ev.Set(type, data, 1, modifier);
   e.push_back(ev);
   return InputDriver::kESProcessed;
}

bool InputDriverUnix::InputsAvailable() const {
   nodelay(stdscr, true);
   int c = getch();
   if (c != ERR) ungetch(c);
   return c != ERR;
}
