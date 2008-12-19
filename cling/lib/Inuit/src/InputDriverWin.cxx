// Copyright (C) 2008, Rene Brun and Fons Rademakers.
// All rights reserved. License: MIT/X11, see LICENSE
// Author: Axel Naumann, 2008

#include "InputDriverWin.h"

#include <windows.h>
#include "TerminalDriverWin.h"

using namespace Inuit;

InputDriverWin::InputDriverWin(TerminalDriver& term):
InputDriver(term) {}

InputDriverWin::~InputDriverWin() {}

InputDriver::EInputStatus InputDriverWin::ProcessOneInput(Input& e) {
   return InputDriver::kESIdle;
}

InputDriver::EInputStatus InputDriverWin::ProcessAvailableInputs(std::list<Input>& e, bool block) {
   TerminalDriverWin& term = (TerminalDriverWin&) GetTerminalDriver();

   if (!block) {
      DWORD numInputs = 0;
      ::GetNumberOfConsoleInputEvents(term.GetStdIn(), &numInputs);
      if (!numInputs) return InputDriver::kESIdle;
   }

   static const DWORD nEventRecords = 1024;
   INPUT_RECORD inputRecords[nEventRecords];
   DWORD nEventRecordsRead = 0;
   ReadConsoleInput(term.GetStdIn(), inputRecords, nEventRecords, &nEventRecordsRead);

   for (DWORD i = 0; i < nEventRecordsRead; ++i) {
      Input ev;
      if (inputRecords[i].EventType == KEY_EVENT) {
         int data = 0;
         int modifier = 0;
         Input::EType type = Input::kTypeNonPrintable;
         WORD vkc = inputRecords[i].Event.KeyEvent.wVirtualKeyCode;
         switch (vkc) {
            case VK_CANCEL: data = Input::kNPBreak; modifier = Input::kModCtrl; break;
            case VK_BACK: data = Input::kNPBackspace; break;
            case VK_TAB: data = Input::kNPTab; break;
            // case VK_CLEAR:
            case VK_RETURN: data = Input::kNPEnter; break;
            case VK_SHIFT: data = Input::kNPShift; break;
            case VK_CONTROL: data = Input::kNPCtrl; break;
            case VK_MENU: data = Input::kNPAlt; break;
            case VK_PAUSE: data = Input::kNPPause; break;
            case VK_CAPITAL: data = Input::kNPCapsLock; break;
            // case VK_KANA:
            // case VK_HANGUL:
            // case VK_JUNJA:
            // case VK_FINAL:
            // case VK_HANJA:
            // case VK_KANJI:
            case VK_ESCAPE: data = Input::kNPEscape; break;
            // case VK_CONVERT:
            // case VK_NONCONVERT:
            // case VK_ACCEPT:
            // case VK_MODECHANGE:
            case VK_SPACE: type = Input::kTypePrintable; data = ' '; break;
            case VK_PRIOR: data = Input::kNPPageUp; break;
            case VK_NEXT: data = Input::kNPPageDown; break;
            case VK_END: data = Input::kNPEnd; break;
            case VK_HOME: data = Input::kNPHome; break;
            case VK_LEFT: data = Input::kNPArrowLeft; break;
            case VK_UP: data = Input::kNPArrowUp; break;
            case VK_RIGHT: data = Input::kNPArrowRight; break;
            case VK_DOWN: data = Input::kNPArrowDown; break;
            // case VK_SELECT:
            // case VK_PRINT:
            // case VK_EXECUTE:
            case VK_SNAPSHOT: data = Input::kNPPrintScreen; break;
            case VK_INSERT: data = Input::kNPInsert; break;
            case VK_DELETE: data = Input::kNPDelete; break;
            // case VK_HELP:
            case VK_NUMPAD0:
            case VK_NUMPAD1:
            case VK_NUMPAD2:
            case VK_NUMPAD3:
            case VK_NUMPAD4:
            case VK_NUMPAD5:
            case VK_NUMPAD6:
            case VK_NUMPAD7:
            case VK_NUMPAD8:
            case VK_NUMPAD9:
            case VK_MULTIPLY:
            case VK_ADD:
            //case VK_SEPARATOR
            case VK_SUBTRACT:
            case VK_DECIMAL:
            case VK_DIVIDE: type = Input::kTypePrintable; break;
            case VK_F1: data = Input::kNPF1; break;
            case VK_F2: data = Input::kNPF2; break;
            case VK_F3: data = Input::kNPF3; break;
            case VK_F4: data = Input::kNPF4; break;
            case VK_F5: data = Input::kNPF5; break;
            case VK_F6: data = Input::kNPF6; break;
            case VK_F7: data = Input::kNPF7; break;
            case VK_F8: data = Input::kNPF8; break;
            case VK_F9: data = Input::kNPF9; break;
            case VK_F10: data = Input::kNPF10; break;
            case VK_F11: data = Input::kNPF11; break;
            case VK_F12: data = Input::kNPF12; break;
            // case VK_F13:
            // case VK_F14:
            // case VK_F15:
            // case VK_F16:
            // case VK_F17:
            // case VK_F18:
            // case VK_F19:
            // case VK_F20:
            // case VK_F21:
            // case VK_F22:
            // case VK_F23:
            // case VK_F24:
            case VK_NUMLOCK: data = Input::kNPNumLock; break;
            case VK_SCROLL: data = Input::kNPScrollLock; break;
            case VK_LSHIFT:
            case VK_RSHIFT: data = Input::kNPShift; break;
            case VK_LCONTROL:
            case VK_RCONTROL: data = Input::kNPCtrl; break;
            case VK_LMENU:
            case VK_RMENU: data = Input::kNPAlt; break;
            // case VK_BROWSER_BACK:
            // case VK_BROWSER_FORWARD:
            // case VK_BROWSER_REFRESH
            // case VK_BROWSER_STOP
            // case VK_BROWSER_SEARCH
            // case VK_BROWSER_FAVORITES
            // case VK_BROWSER_HOME
            // case VK_VOLUME_MUTE
            // case VK_VOLUME_DOWN
            // case VK_VOLUME_UP
            // case VK_MEDIA_NEXT_TRACK
            // case VK_MEDIA_PREV_TRACK
            // case VK_MEDIA_STOP
            // case VK_MEDIA_PLAY_PAUSE
            // case VK_LAUNCH_MAIL
            // case VK_LAUNCH_MEDIA_SELECT
            // case VK_LAUNCH_APP1
            // case VK_LAUNCH_APP2
            case VK_OEM_1:
            case VK_OEM_PLUS:
            case VK_OEM_COMMA:
            case VK_OEM_MINUS:
            case VK_OEM_PERIOD:
            case VK_OEM_2:
            case VK_OEM_3:
            case VK_OEM_4:
            case VK_OEM_5:
            case VK_OEM_6:
            case VK_OEM_7: type = Input::kTypePrintable; break;
            // case VK_OEM_8:
            // case VK_OEM_102:
            // case VK_PROCESSKEY:
            // case VK_PACKET:
            // case VK_ATTN
            // case VK_CRSEL
            // case VK_EXSEL
            // case VK_EREOF
            // case VK_PLAY
            // case VK_ZOOM
            // case VK_NONAME
            // case VK_PA1
            // case VK_OEM_CLEAR
            default:
               if (data == 0 && (vkc >= '0' && vkc <= '9'
                  || vkc >= 'A' && vkc <= 'Z'))
                  type = Input::kTypePrintable;
               else type = Input::kNumTypes;
         };
         if (type == Input::kNumTypes) continue;
         if (type == Input::kTypePrintable)
            data = inputRecords[i].Event.KeyEvent.uChar.AsciiChar;

         if (inputRecords[i].Event.KeyEvent.bKeyDown)
            modifier |= Input::kModDown;
         else modifier |= Input::kModUp;

         DWORD ctrl = inputRecords[i].Event.KeyEvent.dwControlKeyState;
         if (ctrl & (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED))
            modifier |= Input::kModAlt;
         if (ctrl & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED))
            modifier |= Input::kModCtrl;
         if (ctrl & SHIFT_PRESSED)
            modifier |= Input::kModShift;
         if (ctrl & CAPSLOCK_ON)
            modifier |= Input::kModCapsLock;
         if (ctrl & NUMLOCK_ON)
            modifier |= Input::kModNumLock;
         if (ctrl & SCROLLLOCK_ON)
            modifier |= Input::kModScrollLock;

         ev.Set(type, data, inputRecords[i].Event.KeyEvent.wRepeatCount, modifier);
         }
      e.push_back(ev);
   }
   return InputDriver::kESProcessed;
}

bool InputDriverWin::InputsAvailable() const {
   DWORD numInputs = 0;
   TerminalDriverWin& term = (TerminalDriverWin&) GetTerminalDriver();
   ::GetNumberOfConsoleInputEvents(term.GetStdIn(), &numInputs);
   return numInputs > 0;
}
