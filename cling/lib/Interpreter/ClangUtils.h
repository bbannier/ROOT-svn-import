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

#include <algorithm>

#include <llvm/ADT/OwningPtr.h>

namespace clang {
	class SourceManager;
	class Stmt;
	class Preprocessor;
	class LangOptions;
	class MacroInfo;
   class SourceLocation;
} // namespace clang

namespace cling {
   
   // Pair of start, end positions in the source.
   typedef std::pair<unsigned, unsigned> SrcRange;
   
   // Get the source range of the specified Stmt.
   SrcRange getStmtRange(const clang::Stmt *S,
                         const clang::SourceManager& sm,
                         const clang::LangOptions& options);
   
   // Get the source range of the specified Stmt, ensuring that a semicolon is
   // included, if necessary - since the clang ranges do not guarantee this.
   SrcRange getStmtRangeWithSemicolon(const clang::Stmt *S,
                                      const clang::SourceManager& sm,
                                      const clang::LangOptions& options);
   
   
   // Get the source range of the specified Stmt, ensuring that a semicolon is
   // included, if necessary - since the clang ranges do not guarantee this.
   SrcRange getRangeWithSemicolon(clang::SourceLocation SLoc,
                                  clang::SourceLocation ELoc,
                                  const clang::SourceManager& sm,
                                  const clang::LangOptions& options);

   // Get the source range of the macro definition excluding the #define.
   SrcRange getMacroRange(const clang::MacroInfo *MI,
                          const clang::SourceManager& sm,
                          const clang::LangOptions& options);
   
} // namespace ccons

#endif // CLING_CLANG_UTILS_H
