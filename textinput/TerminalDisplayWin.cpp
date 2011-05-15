//===--- TerminalDisplayWin.h - Output To Windows Console -------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file defines the interface for writing to a Windows console
//  i.e. cmd.exe.
//
//  Axel Naumann <axel@cern.ch>, 2011-05-12
//===----------------------------------------------------------------------===//

#ifdef WIN32
#include "textinput/TerminalDisplayWin.h"

#include "textinput/Color.h"

namespace textinput {
  TerminalDisplayWin::TerminalDisplayWin():
    StartLine(0), IsAttached(false), IsConsole(false) {
    Out = ::GetStdHandle(STD_OUTPUT_HANDLE);
    IsConsole = ::GetConsoleMode(Out, &OldMode) != 0;
    MyMode = OldMode | ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT;

    HandleResizeEvent();
  }

  TerminalDisplayWin::~TerminalDisplayWin() {
  }
  
  void
  TerminalDisplayWin::HandleResizeEvent() {
    CONSOLE_SCREEN_BUFFER_INFO Info;
    if (!::GetConsoleScreenBufferInfo(Out, &Info)) {
      HandleError("getting console info");
      return;
    }
    SetWidth(Info.dwSize.X);
  }

  void
  TerminalDisplayWin::SetColor(char CIdx, const Color& C) {
    WORD Attribs = 0;
    // There is no underline since DOS has died.
    if (C.Modifiers & Color::kModUnderline) Attribs |= BACKGROUND_INTENSITY;
    if (C.Modifiers & Color::kModBold) Attribs |= FOREGROUND_INTENSITY;
    if (C.R > 63) Attribs |= FOREGROUND_RED;
    if (C.G > 63) Attribs |= FOREGROUND_GREEN;
    if (C.B > 63) Attribs |= FOREGROUND_BLUE;
    ::SetConsoleTextAttribute(Out, Attribs);
  }

  void
  TerminalDisplayWin::Move(Pos P) {
    MoveInternal(P);
    WritePos = P;
  }
  
  void
  TerminalDisplayWin::MoveInternal(Pos P) {
    COORD C = {P.Col, P.Line + StartLine};
    ::SetConsoleCursorPosition(Out, C);
  }
  
  void
  TerminalDisplayWin::MoveFront() {
    Pos P(WritePos);
    P.Col = 0;
    MoveInternal(P);
  }
  
  void
  TerminalDisplayWin::MoveUp(size_t nLines /* = 1 */) {
    Pos P(WritePos);
    --P.Line;
    MoveInternal(P);
  }

  void
  TerminalDisplayWin::MoveDown(size_t nLines /* = 1 */) {
    Pos P(WritePos);
    ++P.Line;
    MoveInternal(P);
  }
  
  void
  TerminalDisplayWin::MoveRight(size_t nCols /* = 1 */) {
    Pos P(WritePos);
    ++P.Col;
    MoveInternal(P);
  }
  
  void
  TerminalDisplayWin::MoveLeft(size_t nCols /* = 1 */) {
    Pos P(WritePos);
    --P.Col;
    MoveInternal(P);
  }
  
  void
  TerminalDisplayWin::EraseToRight() {
    DWORD NumWritten;
    COORD C = {WritePos.Col, WritePos.Line + StartLine};
    ::FillConsoleOutputCharacter(Out, ' ', GetWidth() - C.X, C,
      &NumWritten);
    // It wraps, so move up and reset WritePos:
    //MoveUp();
    //++WritePos.Line;
  }

  void
  TerminalDisplayWin::WriteRawString(const char *text, size_t len) {
    DWORD NumWritten = 0;
    if (IsConsole) {
      WriteConsole(Out, text, (DWORD) len, &NumWritten, NULL);
    } else {
      WriteFile(Out, text, (DWORD) len, &NumWritten, NULL);
    }
    if (NumWritten != len) {
      HandleError("writing to output");
    }
  }
  
  void
  TerminalDisplayWin::Attach() {
    // set to noecho
    if (IsAttached) return;
    if (IsConsole && !SetConsoleMode(Out, MyMode)) {
      HandleError("attaching to console output");
    }
    CONSOLE_SCREEN_BUFFER_INFO Info;
    ::GetConsoleScreenBufferInfo(Out, &Info);
    StartLine = Info.dwCursorPosition.Y;
    if (Info.dwCursorPosition.X) {
      // Whooa - where are we?! Newline and cross fingers:
      WriteRawString("\n", 1);
      ++StartLine;
    }

    IsAttached = true;
    NotifyTextChange(Range::AllWithPrompt());
  }
  
  void
  TerminalDisplayWin::Detach() {
    if (!IsAttached) return;
    if (IsConsole && !SetConsoleMode(Out, OldMode)) {
      HandleError("detaching to console output");
    }
    TerminalDisplay::Detach();
    IsAttached = false;
  }
        
  void
  TerminalDisplayWin::HandleError(const char* Where) const {
    DWORD Err = GetLastError(); 
    LPVOID MsgBuf = 0;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
      FORMAT_MESSAGE_IGNORE_INSERTS, NULL, Err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      (LPTSTR) &MsgBuf, 0, NULL);

    printf("Error %d in textinput::TerminalDisplayWin %s: %s\n", Err, Where, MsgBuf); 
    LocalFree(MsgBuf);
  }

}

#endif // WIN32
