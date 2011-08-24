//--------------------------------------------------------------------*- C++ -*-
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Lukasz Janyst <ljanyst@cern.ch>
//------------------------------------------------------------------------------

#ifndef CLING_METAPROCESSOR_H
#define CLING_METAPROCESSOR_H

#include "llvm/ADT/OwningPtr.h"

#include <string>

namespace cling {

  class Interpreter;
  class InputValidator;

  //---------------------------------------------------------------------------
  // Class for the user interaction with the interpreter
  //---------------------------------------------------------------------------
  class MetaProcessor {
  private:
    Interpreter& m_Interp; // the interpreter
    bool m_QuitRequested; // quitting?
    llvm::OwningPtr<InputValidator> m_InputValidator; // balanced paren etc
  private:
    bool ProcessMeta(const std::string& input_line);
  public:
    MetaProcessor(Interpreter& interp);
    ~MetaProcessor();
    int process(const char* input_line);
    bool isQuitRequested() const { return m_QuitRequested; }
    void requestQuit(bool req) { m_QuitRequested = req; }
  };
} // end namespace cling

#endif // CLING_METAPROCESSOR_H


