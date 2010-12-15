#ifndef CLING_DIAGNOSTICS_H
#define CLING_DIAGNOSTICS_H

/*************************************************************************
 * Copyright (C) 2009-2010, Cling team.                                  *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see LICENSE.                                  *
 * For the list of contributors see CREDITS.                             *
 *************************************************************************/

// version: $Id$
// author:  Axel Naumann, 2010-12-14

#include "clang/Frontend/TextDiagnosticPrinter.h"

namespace cling {

   // Gives access to interpreter diagnostics
   
   class DiagnosticPrinter: public clang::TextDiagnosticPrinter {
   public:
      DiagnosticPrinter();
      virtual ~DiagnosticPrinter();
      void resetCounts();
   };
} // namespace cling

#endif // CLING_DIAGNOSTICS_H
