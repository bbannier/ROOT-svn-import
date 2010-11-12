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

#include "clang/AST/Decl.h"

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
      // Reject constructor, destructor, operator
      if (!FD->getDeclName().isIdentifier()) {
         continue;
      }
      if (FD->getName() == m_funcName) {
	clang::Stmt* S = FD->getBody();
	if (S) {
	  m_visitor.VisitChildren(S);
	}
      }
    }
  }
}

} // namespace cling
