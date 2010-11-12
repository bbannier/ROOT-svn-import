#ifndef CLING_VISITORS_H
#define CLING_VISITORS_H

/*************************************************************************
 * Copyright (C) 2009-2010, Cling team.                                  *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see LICENSE.                                  *
 * For the list of contributors see CREDITS.                             *
 *************************************************************************/

// version: $Id$
// author:  Alexei Svitkine

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/DeclGroup.h"
#include "clang/AST/Stmt.h"
#include "clang/AST/StmtVisitor.h"

#include <string>
#include <vector>

namespace cling {
   
// StmtSplitter will extract clang Stmts from the specified source.
class StmtSplitter : public clang::StmtVisitor<StmtSplitter>
{
private:

  std::vector<clang::Stmt*>& m_vec_stmt;

public:

  StmtSplitter(std::vector<clang::Stmt*>& vec_stmt)
      : m_vec_stmt(vec_stmt)
  {
  }

  ~StmtSplitter();

  void VisitStmt(clang::Stmt *S)
  {
    m_vec_stmt.push_back(S);
  }

  void VisitDeclStmt(clang::DeclStmt *S)
  {
    m_vec_stmt.push_back(S);
  }

  void VisitChildren(clang::Stmt* S);

};


// ASTConsumer that visits function body Stmts and passes
// those to a specific StmtVisitor.
class FunctionBodyConsumer : public clang::ASTConsumer
{
private:
  StmtSplitter& m_visitor;
  std::string m_funcName;

public:
  FunctionBodyConsumer(StmtSplitter& visitor, const char* funcName);
  ~FunctionBodyConsumer();
  void HandleTopLevelDecl(clang::DeclGroupRef D);
};

} // namespace cling

#endif // CLING_VISITORS_H
