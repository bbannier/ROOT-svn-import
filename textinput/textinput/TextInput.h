//===--- TextInput.h - Main Interface ---------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file defines the main interface for the TextInput library.
//
//  Axel Naumann <axel@cern.ch>, 2011-05-12
//===----------------------------------------------------------------------===//

#ifndef TEXTINPUT_TEXTINPUT_H
#define TEXTINPUT_TEXTINPUT_H

#include <string>

namespace textinput {
  class TextInputContext;
  class Reader;
  class Display;
  class EditorRange;
  class Colorizer;

  // Main interface to textinput library.
  class TextInput {
  public:
    // State of input
    enum EReadResult {
      kRRNone, // uninitialized
      kRRReadEOLDelimiter, // end of line is entered, can take input
      kRRCharLimitReached, // SetMaxPendingCharsToRead() of input are read
      kRRNoMorePendingInput, // no input available
      kRREOF // end of file has been reached
    };

    TextInput(Reader& reader, Display& display,
              const char* HistFile = 0);
    ~TextInput();

    // Getters
    const TextInputContext* GetContext() const { return fContext; }
    bool IsInputHidden() const { return fHidden; }

    size_t GetMaxPendingCharsToRead() const { return fMaxChars; }
    bool IsReadingAllPendingChars() const { return fMaxChars == (size_t) -1; }
    bool IsBlockingUntilEOL() const { return fMaxChars == 0; }

    // Setters
    void SetPrompt(const char* P);
    void HideInput(bool hidden = true) { fHidden = hidden; }
    void SetColorizer(Colorizer* C);
    
    void SetMaxPendingCharsToRead(size_t nMax) { fMaxChars = nMax; }
    void SetReadingAllPendingChars() { fMaxChars = (size_t) -1; }
    void SetBlockingUntilEOL() { fMaxChars = 0; }

    // Read interface
    EReadResult ReadInput();
    EReadResult GetReadState() const { return fLastReadResult; }
    const std::string& GetInput();
    void TakeInput(std::string& input); // Take and reset input
    bool AtEOL() const { return fLastReadResult == kRRReadEOLDelimiter || AtEOF(); }
    bool AtEOF() const { return fLastReadResult == kRREOF; }
    bool HavePendingInput() const;
    
    // Display interface
    void UpdateDisplay(const EditorRange& R);
    
    void GrabInputOutput() const;
    void ReleaseInputOutput() const;
    
  private:
    void EmitSignal(char C, EditorRange& R);
    
    bool fHidden; // whether input should be shown
    size_t fMaxChars; // Num chars to read; 0 for blocking, -1 for all available
    EReadResult fLastReadResult; // current input state
    TextInputContext* fContext; // context object
    mutable bool fActive; // whether textinput is controlling input/output
  };
}
#endif
