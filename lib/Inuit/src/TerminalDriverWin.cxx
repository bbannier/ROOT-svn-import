// Copyright (C) 2008, Rene Brun and Fons Rademakers.
// All rights reserved. License: MIT/X11, see LICENSE
// Author: Axel Naumann, 2008

#include "TerminalDriverWin.h"
#include <stdio.h>

using namespace Inuit;

TerminalDriverWin::TerminalDriverWin(): fConsoleOffsetY(0), fManaged(false) {
   fStdIn  = ::GetStdHandle(STD_INPUT_HANDLE);
   fStdOut = ::GetStdHandle(STD_OUTPUT_HANDLE);

   // get sizes:
   CONSOLE_SCREEN_BUFFER_INFO csbi;
   ::GetConsoleScreenBufferInfo(fStdOut, &csbi);

   ::GetConsoleMode(fStdIn, &fStartupConsoleModeIn);
   ::GetConsoleMode(fStdOut, &fStartupConsoleModeOut);

   // make cursor visible at X = 0
   csbi.dwCursorPosition.X = 0 ;
   ::SetConsoleCursorPosition(fStdOut, csbi.dwCursorPosition);

   // left, top, right, bottom:
   fConsoleOffsetY = csbi.srWindow.Top;

   SetManagedMode(true);
}

TerminalDriverWin::~TerminalDriverWin() {
   // go to bottom of screen.
   COORD bottom = {0, GetSize().fY - 1 + fConsoleOffsetY - 1};
   ::SetConsoleCursorPosition(fStdOut, bottom);
}

TerminalDriverWin::EErrorCode TerminalDriverWin::SetManagedMode(bool managed) {
   // Managed: the user decides what to echo, low-level terminal I/O.
   // Unmanaged: use the high level I/O
   if (managed == fManaged) return kErrSuccess;
   fManaged = managed;
   if (managed) {
      ::SetConsoleMode(fStdIn, ENABLE_EXTENDED_FLAGS | 
         ENABLE_INSERT_MODE | // insert not overwrite
         ENABLE_MOUSE_INPUT | // mouse over window generates events
         ENABLE_QUICK_EDIT_MODE | // mouse can mark and paste
         ENABLE_WINDOW_INPUT); // window resize causes events

      ::SetConsoleMode(fStdOut, ENABLE_PROCESSED_OUTPUT);
   } else {
      ::SetConsoleMode(fStdIn, fStartupConsoleModeIn);
      ::SetConsoleMode(fStdOut, fStartupConsoleModeOut);
   }
   return kErrSuccess;
}

TerminalDriver::EErrorCode TerminalDriverWin::WriteChar(char c) {
   const char s[] = {c, 0};
   printf("%s", s); 
   return kErrSuccess;
}

TerminalDriver::EErrorCode TerminalDriverWin::WriteString(const char* s) {
   printf("%s", s);
   return kErrSuccess;
}

TerminalDriver::EErrorCode TerminalDriverWin::Clear() {
   // Get the number of character cells in the current buffer. 
   CONSOLE_SCREEN_BUFFER_INFO csbi; 
   Pos size = GetSize();
   DWORD dwConSize = size.fX * size.fY;

   // Fill the entire screen with blanks.
   COORD coordScreen = {0, fConsoleOffsetY};    // home for the cursor 
   DWORD cCharsWritten;
   FillConsoleOutputCharacter(fStdOut, ' ',
                              dwConSize, coordScreen, &cCharsWritten);

   // Get the current text attribute.
   GetConsoleScreenBufferInfo(fStdOut, &csbi);

   // Set the buffer's attributes accordingly.
   FillConsoleOutputAttribute(fStdOut, csbi.wAttributes,
                              dwConSize, coordScreen, &cCharsWritten);

   // Put the cursor at its home coordinates.
   COORD coord = {fPos.fX, fPos.fY + fConsoleOffsetY};
   ::SetConsoleCursorPosition(::GetStdHandle(STD_OUTPUT_HANDLE), coord);
   return kErrSuccess;
}


TerminalDriver::EErrorCode TerminalDriverWin::Goto(const Pos& p) { 
   fPos = p;
   COORD coord = {p.fX, p.fY + fConsoleOffsetY};
   ::SetConsoleCursorPosition(::GetStdHandle(STD_OUTPUT_HANDLE), coord);
   return kErrSuccess;
}

      
Pos TerminalDriverWin::GetSize() const {
   CONSOLE_SCREEN_BUFFER_INFO csbi;
   GetConsoleScreenBufferInfo(fStdOut, &csbi);

   // left, top, right, bottom:
   return Pos(csbi.srWindow.Right - csbi.srWindow.Left + 1,
              csbi.srWindow.Bottom - csbi.srWindow.Top + 1);
}

TerminalDriver::EErrorCode TerminalDriverWin::SetCursorSize(char percent_size) {
   CONSOLE_CURSOR_INFO cci;
   cci.bVisible = (percent_size > 0);
   cci.dwSize = percent_size;
   BOOL ret = ::SetConsoleCursorInfo(GetStdOut(), &cci);
   return kErrSuccess;
}
