//--------------------------------------------------------------------*- C++ -*-
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Axel Naumann <axel@cern.ch>
//------------------------------------------------------------------------------

#ifndef CLING_DIAGNOSTICS_H
#define CLING_DIAGNOSTICS_H

#include "clang/Frontend/TextDiagnosticPrinter.h"

#include <set>

namespace cling {
  
  // Gives access to interpreter diagnostics
  
  class DiagnosticPrinter: public clang::TextDiagnosticPrinter {
  public:
    DiagnosticPrinter();
    virtual ~DiagnosticPrinter();
    virtual void HandleDiagnostic (clang::Diagnostic::Level Level,
                                   const clang::DiagnosticInfo &Info);
  };
} // namespace cling

#endif // CLING_DIAGNOSTICS_H
