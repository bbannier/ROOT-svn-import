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

#include "clang/AST/DeclGroup.h"
#include "clang/AST/Decl.h"
#include "clang/AST/Stmt.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Lex/Lexer.h"
#include "llvm/Support/MemoryBuffer.h"

namespace cling {

// Get the end source location for a statement, if necessary
// and possible asking its declaration.
clang::SourceLocation
getStmtEndLoc(const clang::Stmt* S)
{
   // Get the end source location for a statement, if necessary
   // and possible asking its declaration.
  clang::SourceLocation ELoc = S->getLocEnd();
  if (ELoc.isValid())
     return ELoc;

  // Is it a decl? Then try via that:
  const clang::DeclStmt* DS = dyn_cast<clang::DeclStmt>(S);
  if (!DS) return ELoc;

  if (DS->isSingleDecl()) {
     return DS->getSingleDecl()->getLocEnd();
  }

  if (!DS->getDeclGroup().isNull()) {
     const clang::DeclGroup& DG = DS->getDeclGroup().getDeclGroup();
     clang::Decl* D = DG[DG.size()-1];
     if (D) {
        return D->getLocEnd();
     }
  }
  return ELoc;
}

// Get the source range of the specified Stmt.
std::pair<unsigned, unsigned>
getStmtRange(const clang::Stmt* S,
   const clang::SourceManager& SM,
   const clang::LangOptions& LO)
{
  // Get the source range of the specified Stmt.
  clang::SourceLocation SLoc = SM.getInstantiationLoc(S->getLocStart());
  clang::SourceLocation ELoc = SM.getInstantiationLoc(getStmtEndLoc(S));
  // This is necessary to get the correct range of function-like macros.
  if ((SLoc == ELoc) && S->getLocEnd().isMacroID()) {
    ELoc = SM.getInstantiationRange(S->getLocEnd()).second;
  }
  if (!SLoc.isValid() && !ELoc.isValid()) {
    return std::pair<unsigned, unsigned>(0, 0);
  }
  unsigned start = 0;
  unsigned end = 0;
  if (SLoc.isValid()) {
    start = SM.getFileOffset(SLoc);
  }
  if (ELoc.isValid()) {
    end = SM.getFileOffset(ELoc) +
      clang::Lexer::MeasureTokenLength(ELoc, SM, LO);
  }
  else if (SLoc.isValid()) {
    clang::SourceLocation Loc = SM.getInstantiationLoc(SLoc);
    std::pair<clang::FileID, unsigned> LocInfo = SM.getDecomposedLoc(Loc);
    std::pair<const char*, const char*> BD = std::make_pair(
       SM.getBuffer(LocInfo.first)->getBufferStart(),
       SM.getBuffer(LocInfo.first)->getBufferEnd());
    end = BD.second - BD.first;
  }
  return std::pair<unsigned, unsigned>(start, end);
}

// Get the source range of the specified Stmt, ensuring that a semicolon is
// included, if necessary - since the clang ranges do not guarantee this.
std::pair<unsigned, unsigned>
getStmtRangeWithSemicolon(const clang::Stmt* S,
  const clang::SourceManager& SM,
  const clang::LangOptions& LO)
{
  // Get the source range of the specified Stmt, ensuring that a semicolon is
  // included, if necessary - since the clang ranges do not guarantee this.
  clang::SourceLocation SLoc = SM.getInstantiationLoc(S->getLocStart());
  clang::SourceLocation ELoc = SM.getInstantiationLoc(getStmtEndLoc(S));

  return getRangeWithSemicolon(SLoc, ELoc, SM, LO);
}

std::pair<unsigned, unsigned>
getRangeWithSemicolon(clang::SourceLocation SLoc,
  clang::SourceLocation ELoc,
  const clang::SourceManager& SM,
  const clang::LangOptions& LO)
{
  unsigned start = 0;
  unsigned end = 0;
  if (SLoc.isValid()) {
    start = SM.getFileOffset(SLoc);
  }
  if (ELoc.isValid()) {
    end = SM.getFileOffset(ELoc) +
       clang::Lexer::MeasureTokenLength(ELoc, SM, LO);;
  }
  if (SLoc.isValid() && !ELoc.isValid()) {
    clang::SourceLocation Loc = SM.getInstantiationLoc(SLoc);
    std::pair<clang::FileID, unsigned> LocInfo = SM.getDecomposedLoc(Loc);
    std::pair<const char*, const char*> BD = std::make_pair(
       SM.getBuffer(LocInfo.first)->getBufferStart(),
       SM.getBuffer(LocInfo.first)->getBufferEnd());
    end = BD.second - BD.first;
  }
  else if (ELoc.isValid()) {
    clang::SourceLocation Loc = SM.getInstantiationLoc(ELoc);
    std::pair<clang::FileID, unsigned> LocInfo = SM.getDecomposedLoc(Loc);
    std::pair<const char*, const char*> BD = std::make_pair(
       SM.getBuffer(LocInfo.first)->getBufferStart(),
       SM.getBuffer(LocInfo.first)->getBufferEnd());
    const char* StrData = BD.first + LocInfo.second;
    clang::Lexer TheLexer(Loc, LO, BD.first, StrData, BD.second);
    clang::Token TheTok;
    TheLexer.LexFromRawLexer(TheTok);
    end += TheTok.getLength();
    // Check if we the source range did include the semicolon.
    if (TheTok.isNot(clang::tok::semi) && TheTok.isNot(clang::tok::r_brace)) {
      TheLexer.LexFromRawLexer(TheTok);
      if (TheTok.is(clang::tok::semi)) {
        end += TheTok.getLength();
      }
    }
  }
  return std::pair<unsigned, unsigned>(start, end);
}

} // namespace cling

