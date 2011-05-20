//===--- TerminalDisplayUnix.h - Output To UNIX Terminal --------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file defines the interface for writing to a UNIX terminal. It tries to
//  support all "common" terminal types.
//
//  Axel Naumann <axel@cern.ch>, 2011-05-12
//===----------------------------------------------------------------------===//

#ifndef TEXTINPUT_TERMINALDISPLAYUNIX_H
#define TEXTINPUT_TERMINALDISPLAYUNIX_H

#include "textinput/TerminalDisplay.h"

struct termios;

namespace textinput {
  struct Color;

  // Output to tty / pipe / file.
  class TerminalDisplayUnix: public TerminalDisplay {
  public:
    TerminalDisplayUnix();
    ~TerminalDisplayUnix();
    
    void HandleResizeSignal();
    
    void Attach();
    void Detach();
    
  protected:
    void MoveUp(size_t nLines = 1);
    void MoveDown(size_t nLines = 1);
    void MoveLeft(size_t nCols = 1);
    void MoveRight(size_t nCols = 1);
    void MoveInternal(char What, size_t n);
    void MoveFront();
    void SetColor(char CIdx, const Color& C);
    void WriteRawString(const char* text, size_t len);
    void ActOnEOL();
    void EraseToRight();
    int GetClosestColorIdx256(const Color& C);
    int GetClosestColorIdx16(const Color& C);

  private:
    int fFD; // output file descriptor
    termios* fOldTIOS; // tty configuration before grabbing
    termios* fMyTIOS; // tty configuration while active
    bool fIsAttached; // whether tty is configured
    size_t fNColors; // number of colors supported by output
    bool fIsTTY; // whether output is to a terminal
  };
}
#endif // TEXTINPUT_TERMINALDISPLAYUNIX_H
