/*************************************************************************
 * Copyright (C) 2009-2010, Cling team.                                  *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see LICENSE.                                  *
 * For the list of contributors see CREDITS.                             *
 *************************************************************************/

// version: $Id: ParseEnvironment.h 30604 2009-10-07 15:10:56Z pcanal $
// author:  Alexei Svitkine

#include "ClangUtils.h"

#include <clang/AST/AST.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Lex/Lexer.h>
#include <clang/Lex/MacroInfo.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/Sema/SemaDiagnostic.h>

namespace cling {

// Get the source range of the specified Stmt.
SrcRange getStmtRange(const clang::Stmt *S,
                      const clang::SourceManager& sm,
                      const clang::LangOptions& options)
{
	clang::SourceLocation SLoc = sm.getInstantiationLoc(S->getLocStart());
	clang::SourceLocation ELoc = sm.getInstantiationLoc(S->getLocEnd());
	// This is necessary to get the correct range of function-like macros.
	if (SLoc == ELoc && S->getLocEnd().isMacroID())
		ELoc = sm.getInstantiationRange(S->getLocEnd()).second;
	unsigned start = sm.getFileOffset(SLoc);
	unsigned end   = sm.getFileOffset(ELoc);
	end += clang::Lexer::MeasureTokenLength(ELoc, sm, options);
	return SrcRange(start, end);
}

// Get the source range of the specified Stmt, ensuring that a semicolon is
// included, if necessary - since the clang ranges do not guarantee this.
SrcRange getStmtRangeWithSemicolon(const clang::Stmt *S,
                                   const clang::SourceManager& sm,
                                   const clang::LangOptions& options)
{
	clang::SourceLocation SLoc = sm.getInstantiationLoc(S->getLocStart());
	clang::SourceLocation ELoc = sm.getInstantiationLoc(S->getLocEnd());
	unsigned start = SLoc.isValid() ? sm.getFileOffset(SLoc) : 0;
	unsigned end   = ELoc.isValid() ? sm.getFileOffset(ELoc) : 0;

	// Below code copied from clang::Lexer::MeasureTokenLength():
   if (end!=0) {
      clang::SourceLocation Loc = sm.getInstantiationLoc(ELoc);
      std::pair<clang::FileID, unsigned> LocInfo = sm.getDecomposedLoc(Loc);
      std::pair<const char *,const char *> Buffer = sm.getBufferData(LocInfo.first);
      const char *StrData = Buffer.first+LocInfo.second;
      clang::Lexer TheLexer(Loc, options, Buffer.first, StrData, Buffer.second);
      clang::Token TheTok;
      TheLexer.LexFromRawLexer(TheTok);
      // End copied code.
      end += TheTok.getLength();

      // Check if we the source range did include the semicolon.
      if (TheTok.isNot(clang::tok::semi) && TheTok.isNot(clang::tok::r_brace)) {
         TheLexer.LexFromRawLexer(TheTok);
         if (TheTok.is(clang::tok::semi)) {
            end += TheTok.getLength();
         }
      }
   } else {
      clang::SourceLocation Loc = sm.getInstantiationLoc(SLoc);
      std::pair<clang::FileID, unsigned> LocInfo = sm.getDecomposedLoc(Loc);
      std::pair<const char *,const char *> Buffer = sm.getBufferData(LocInfo.first);
      end = Buffer.second - Buffer.first;
   }

	return SrcRange(start, end);
}

// Get the source range of the macro definition excluding the #define.
SrcRange getMacroRange(const clang::MacroInfo *MI,
                       const clang::SourceManager& sm,
                       const clang::LangOptions& options)
{
	clang::SourceLocation SLoc = sm.getInstantiationLoc(MI->getDefinitionLoc());
	clang::SourceLocation ELoc = sm.getInstantiationLoc(MI->getDefinitionEndLoc());
	unsigned start = sm.getFileOffset(SLoc);
	unsigned end   = sm.getFileOffset(ELoc);
	end += clang::Lexer::MeasureTokenLength(ELoc, sm, options);
	return SrcRange(start, end);

}

} // namespace ccons
