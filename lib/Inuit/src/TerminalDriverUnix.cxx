// Copyright (C) 2008,  Rene Brun and Fons Rademakers.
// All rights reserved.
// License: LGPL'ed v3 or later (up to you), see http://www.gnu.org/licenses/lgpl.html
// Author: Axel Naumann, 2008

#include "TerminalDriverUnix.h"
#include <ncurses/curses.h>

using namespace Inuit;

TerminalDriverUnix::TerminalDriverUnix(): fConsoleOffsetY(0) {
   // initialize curses
   initscr();
   raw();
   noecho();
   keypad(stdscr, true);
}

TerminalDriverUnix::~TerminalDriverUnix() {
   // reset curses
   def_prog_mode();
   endwin();
}

TerminalDriverUnix::EErrorCode TerminalDriverUnix::SetManagedMode(bool managed) {
   // Managed: the user decides what to echo, low-level terminal I/O.
   // Unmanaged: use the high level I/O
   if (managed == fManaged) return kErrSuccess;
   fManaged = managed;
   if (managed) {
      reset_prog_mode();
      refresh();
   } else {
      def_prog_mode();
      endwin();
   }
   return kErrSuccess;
}

TerminalDriver::EErrorCode TerminalDriverUnix::WriteChar(char c) {
   printw("%c", c);
   refresh();
   return kErrSuccess;
}

TerminalDriver::EErrorCode TerminalDriverUnix::WriteString(const char* s) {
   printw("%s", s);
   refresh();
   return kErrSuccess;
}

TerminalDriver::EErrorCode TerminalDriverUnix::Clear() {
   // Get the number of character cells in the current buffer. 
   printf("\033[r"); // scroll screen
   return kErrSuccess;
}


TerminalDriver::EErrorCode TerminalDriverUnix::Goto(const Pos& p) { 
   fPos = p;
   move(p.fY, p.fX);
   return kErrSuccess;
}

      
Pos TerminalDriverUnix::GetSize() const {
   int row, col;
   getmaxyx(stdscr,row,col);
   return Pos(col, row);
}

TerminalDriver::EErrorCode TerminalDriverUnix::SetCursorSize(char percent_size) {
   curs_set(percent_size > 50 ? 2 : (int)(percent_size >= 0)); 
   return kErrSuccess;
}
