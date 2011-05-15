//===--- TerminalReaderUnix.h - Input From UNIX Terminal --------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file defines the interface reading from a UNIX terminal. It tries to
//  support all common terminal types.
//
//  Axel Naumann <axel@cern.ch>, 2011-05-12
//===----------------------------------------------------------------------===//

#ifndef TEXTINPUT_STREAMREADERUNIX_H
#define TEXTINPUT_STREAMREADERUNIX_H

#include "textinput/StreamReader.h"
#include <queue>

struct termios;

namespace textinput {
  // Input from a tty, file descriptor, or pipe
  class StreamReaderUnix: public StreamReader {
  public:
    StreamReaderUnix();
    ~StreamReaderUnix();

    void GrabInputFocus();
    void ReleaseInputFocus();

    bool HavePendingInput();
    bool ReadInput(size_t& nRead, InputData& in);

  private:
    int ReadRawCharacter();
    bool ProcessCSI(InputData& in);

    int fFD; // file descriptor
    bool fHaveInputFocus; // whether we configured the tty
    bool fIsTTY; // whether input FD is a tty
    termios* fOldTIOS; // config before input grabbing
    termios* fMyTIOS; // my config for input (while active)
    std::queue<char> fReadAheadBuffer; // input chars we read too much (CSI)
  };
}

#endif // TEXTINPUT_STREAMREADERUNIX_H
