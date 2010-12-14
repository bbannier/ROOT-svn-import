
/*************************************************************************
 * Copyright (C) 2009-2010, Cling team.                                  *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see LICENSE.                                  *
 * For the list of contributors see CREDITS.                             *
 *************************************************************************/

// version: $Id: ParseEnvironment.h 30397 2009-09-24 13:08:16Z axel $
// author: Axel Naumann, 2010-12-14

#include "cling/Interpreter/Diagnostics.h"

#include "clang/Frontend/DiagnosticOptions.h"

#include "llvm/Support/raw_ostream.h"

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

   void DiagnosticPrinter::ResetCounts() {
      NumWarnings = 0;
      NumErrors = 0;
   }
   
} // namespace cling
