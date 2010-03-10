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

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclGroup.h"
#include "clang/AST/Decl.h"
#include "clang/AST/ExprCXX.h"
#include "clang/AST/Expr.h"
#include "clang/AST/Stmt.h"
#include "clang/AST/StmtVisitor.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/IdentifierTable.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/Version.h"
#include "clang/CodeGen/ModuleBuilder.h"
#include "clang/Driver/Arg.h"
#include "clang/Driver/ArgList.h"
#include "clang/Driver/CC1Options.h"
#include "clang/Driver/DriverDiagnostic.h"
#include "clang/Driver/OptTable.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/CompilerInvocation.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/FrontendDiagnostic.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Frontend/HeaderSearchOptions.h"
#include "clang/Frontend/TextDiagnosticBuffer.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Lex/HeaderSearch.h"
#include "clang/Lex/MacroInfo.h"
#include "clang/Lex/PPCallbacks.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/lib/Sema/Sema.h"
#include "clang/Parse/Parser.h"
#include "clang/Sema/ParseAST.h"
#include "llvm/ADT/OwningPtr.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/Assembly/PrintModulePass.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/Function.h"
#include "llvm/GlobalVariable.h"
#include "llvm/Linker.h"
#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
#include "llvm/PassManager.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/System/DynamicLibrary.h"
#include "llvm/System/Path.h"
#include "llvm/System/Process.h"
#include "llvm/System/Signals.h"
#include "llvm/Target/TargetSelect.h"
#include "llvm/Transforms/Utils/Cloning.h"

#include <cstdio>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include <limits.h>
#include <stdint.h>

namespace cling {

// Get the source range of the specified Stmt.
std::pair<unsigned, unsigned>
getStmtRange(const clang::Stmt* S,
   const clang::SourceManager& SM,
   const clang::LangOptions& LO)
{
  // Get the source range of the specified Stmt.
  clang::SourceLocation SLoc = SM.getInstantiationLoc(S->getLocStart());
  clang::SourceLocation ELoc = SM.getInstantiationLoc(S->getLocEnd());
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
    std::pair<const char*, const char*> BD = SM.getBufferData(LocInfo.first);
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
  clang::SourceLocation ELoc = SM.getInstantiationLoc(S->getLocEnd());
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
    end = SM.getFileOffset(ELoc);
  }
  if (SLoc.isValid() && !ELoc.isValid()) {
    clang::SourceLocation Loc = SM.getInstantiationLoc(SLoc);
    std::pair<clang::FileID, unsigned> LocInfo = SM.getDecomposedLoc(Loc);
    std::pair<const char*, const char*> BD = SM.getBufferData(LocInfo.first);
    end = BD.second - BD.first;
  }
  else if (ELoc.isValid()) {
    clang::SourceLocation Loc = SM.getInstantiationLoc(ELoc);
    std::pair<clang::FileID, unsigned> LocInfo = SM.getDecomposedLoc(Loc);
    std::pair<const char*, const char*> BD = SM.getBufferData(LocInfo.first);
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

