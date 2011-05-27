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
  private:
    std::set<unsigned> m_IgnoredWarnings;
  public:
    DiagnosticPrinter();
    virtual ~DiagnosticPrinter();
    void resetCounts();
    virtual void HandleDiagnostic (clang::Diagnostic::Level Level,
                                   const clang::DiagnosticInfo &Info);
    bool isIgnoredWarning(unsigned ID) {
      return m_IgnoredWarnings.find(ID) != m_IgnoredWarnings.end();
    }
    void ignoreWarning(unsigned ID) {
      if (!isIgnoredWarning(ID)) {
        m_IgnoredWarnings.insert(ID);
      }
    }
    void removeIgnoredWarning(unsigned ID) {
      if (isIgnoredWarning(ID)) {
        m_IgnoredWarnings.erase(ID);
      }
    }
  };
} // namespace cling

#endif // CLING_DIAGNOSTICS_H
