//--------------------------------------------------------------------*- C++ -*-
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Axel Naumann <axel@cern.ch>
//------------------------------------------------------------------------------

#include "cling/Interpreter/Diagnostics.h"

#include "clang/Frontend/DiagnosticOptions.h"

#include "llvm/Support/raw_ostream.h"

using namespace clang;

namespace {
   static
   const clang::DiagnosticOptions& getDefaultDiagOpts() {
      static clang::DiagnosticOptions opt;
      if (!opt.ShowColors) {
         // initialize:
         opt.ShowColors = 1;
      }
      return opt;
   }
}

namespace cling {
   DiagnosticPrinter::DiagnosticPrinter():
      clang::TextDiagnosticPrinter(llvm::errs(), getDefaultDiagOpts()) {}
   DiagnosticPrinter::~DiagnosticPrinter() {}

  void DiagnosticPrinter::HandleDiagnostic (Diagnostic::Level Level,
                                            const DiagnosticInfo &Info){
  }
   
} // namespace cling
