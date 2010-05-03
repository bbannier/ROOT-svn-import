#ifndef CLING_CLANG_UTILS_H
#define CLING_CLANG_UTILS_H

/*************************************************************************
 * Copyright (C) 2009-2010, Cling team.                                  *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see LICENSE.                                  *
 * For the list of contributors see CREDITS.                             *
 *************************************************************************/

// version: $Id$
// author:  Alexei Svitkine

#include "clang/Basic/SourceLocation.h"

#include <utility>

namespace clang {
   class SourceManager;
   class LangOptions;
   class Stmt;
}

namespace cling {
   
   // Get the source range of the specified Stmt.
   std::pair<unsigned, unsigned>
   getStmtRange(const clang::Stmt* S,
      const clang::SourceManager& SM,
      const clang::LangOptions& LO);
   
   // Get the source range of the specified Stmt, ensuring that a semicolon is
   // included, if necessary - since the clang ranges do not guarantee this.
   std::pair<unsigned, unsigned>
   getStmtRangeWithSemicolon(const clang::Stmt* S,
      const clang::SourceManager& SM,
      const clang::LangOptions& LO);
   
   
   // Get the source range of the specified Stmt, ensuring that a semicolon is
   // included, if necessary - since the clang ranges do not guarantee this.
   std::pair<unsigned, unsigned>
   getRangeWithSemicolon(clang::SourceLocation SLoc,
      clang::SourceLocation ELoc,
      const clang::SourceManager& SM,
      const clang::LangOptions& LO);

   // Get the end source location for a statement, if necessary
   // and possible asking its declaration.
   clang::SourceLocation getStmtEndLoc(const clang::Stmt* S);

} // namespace cling

#endif // CLING_CLANG_UTILS_H
