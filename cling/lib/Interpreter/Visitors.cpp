/*************************************************************************
 * Copyright (C) 2009-2010, Cling team.                                  *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see LICENSE.                                  *
 * For the list of contributors see CREDITS.                             *
 *************************************************************************/

// version: $Id: ParseEnvironment.h 30604 2009-10-07 15:10:56Z pcanal $
// author:  Alexei Svitkine

#include "Visitors.h"

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
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/Linker.h"
#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
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
#include <string>
#include <utility>
#include <vector>

#include <limits.h>
#include <stdint.h>

namespace cling {
   
//
//  StmtSplitter
//

StmtSplitter::~StmtSplitter()
{
}

void StmtSplitter::VisitChildren(clang::Stmt* S)
{
  clang::Stmt::child_iterator I = S->child_begin();
  clang::Stmt::child_iterator E = S->child_end();
  for (; I != E; ++I) {
    if (*I) {
      Visit(*I);
    }
  }
}

//
//  FunctionBodyConsumer
//

FunctionBodyConsumer::FunctionBodyConsumer(StmtSplitter& visitor,
   const char* funcName)
   : clang::ASTConsumer(), m_visitor(visitor), m_funcName(funcName)
{
}

FunctionBodyConsumer::~FunctionBodyConsumer()
{
}

void FunctionBodyConsumer::HandleTopLevelDecl(clang::DeclGroupRef D)
{
  clang::DeclGroupRef::iterator I = D.begin();
  clang::DeclGroupRef::iterator E = D.end();
  for (; I != E; ++I) {
    clang::FunctionDecl* FD = dyn_cast<clang::FunctionDecl>(*I);
    if (FD) {
      if (FD->getNameAsCString() == m_funcName) {
	clang::Stmt* S = FD->getBody();
	if (S) {
	  m_visitor.VisitChildren(S);
	}
      }
    }
  }
}

} // namespace cling
