//===--- TextInputContext.h - Object Holder ----------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file defines the internal interface for TextInput's auxilliary
//  objects.
//
//  Axel Naumann <axel@cern.ch>, 2011-05-12
//===----------------------------------------------------------------------===//

#ifndef TEXTINPUT_TEXTINPUTCONFIG_H
#define TEXTINPUT_TEXTINPUTCONFIG_H

#include <vector>
#include "textinput/Text.h"

namespace textinput {
  class Reader;
  class Display;
  class KeyBinding;
  class Editor;
  class SignalHandler;
  class Colorizer;
  class History;

  // Context for textinput library. Collection of internal objects.
  class TextInputContext {
  public:
    TextInputContext(const char* HistFile);
    ~TextInputContext();

    KeyBinding* GetKeyBinding() const { return fBind; }
    Editor* GetEditor() const { return fEdit; }
    SignalHandler* GetSignalHandler() const { return fSignal; }
    Colorizer* GetColorizer() const { return fColor; }
    void SetColorizer(Colorizer* C) { fColor = C; }
    History* GetHistory() const { return fHist; }

    const Text& GetPrompt() const { return fPrompt; }
    Text& GetPrompt() { return fPrompt; }
    void SetPrompt(const Text& P) { fPrompt = P; }

    const Text& GetLine() const { return fLine; }
    Text& GetLine() { return fLine; }
    void SetLine(const Text& T) { fLine = T; }

    size_t GetCursor() const { return fCursor; }
    void SetCursor(size_t C) { fCursor = C; }

    const std::vector<Display*>& GetDisplays() const { return fDisplays; }
    const std::vector<Reader*>& GetReaders() const { return fReaders; }
    TextInputContext& AddReader(Reader& R);
    TextInputContext& AddDisplay(Display& D);

  private:
    std::vector<Reader*> fReaders; // readers to use
    std::vector<Display*> fDisplays; // displays to write to
    KeyBinding* fBind; // key binding to use
    Editor* fEdit; // editor to use
    SignalHandler* fSignal; // signal handler to use
    Colorizer* fColor; // colorizer to use
    History* fHist; // history to use
    Text fPrompt; // current prompt
    Text fLine; // current input
    size_t fCursor; // input cursor position in fLine
  };
}
#endif
