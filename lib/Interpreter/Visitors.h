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

#include <string>
#include <cstdio>
#include <vector>

#include <llvm/ADT/OwningPtr.h>

#include <clang/AST/AST.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/AST/DeclGroup.h>

namespace clang {
	class ASTContext;
	class SourceManager;
} // namespace clang

namespace cling {
   
   // StmtFinder will attempt to find a clang Stmt at the specified
   // offset in the source (pos).
   class StmtFinder : public clang::StmtVisitor<StmtFinder> {
      
   public:
      
      StmtFinder(unsigned pos, const clang::SourceManager& sm);
      ~StmtFinder();
      
      void VisitChildren(clang::Stmt *S);
      void VisitStmt(clang::Stmt *S);
      clang::Stmt * getStmt() const;
      
   private:
      
      unsigned m_pos;
      const clang::SourceManager& m_srcMgr;
      clang::Stmt *m_stmt;
      
   };
   
   // StmtSplitter will extract clang Stmts from the specified source.
   class StmtSplitter : public clang::StmtVisitor<StmtSplitter> {
      
   public:
      
      StmtSplitter(const std::string& src,
                   const clang::SourceManager& sm,
                   const clang::LangOptions& options,
                   std::vector<clang::Stmt*> *stmts);
      ~StmtSplitter();
      
      void VisitChildren(clang::Stmt *S);
      void VisitStmt(clang::Stmt *S);
      
   private:
      
      const std::string& _src;
      const clang::SourceManager& m_srcMgr;
      const clang::LangOptions& _options;
      std::vector<clang::Stmt*> *m_vec_stmts;
      
   };
   
   // ASTConsumer that visits function body Stmts and passes
   // those to a specific StmtVisitor.
   template <typename T>
   class FunctionBodyConsumer : public clang::ASTConsumer {
      
   private:
      
      T *m_visitor;
      std::string _funcName;
      
   public:
      
      FunctionBodyConsumer<T>(T *SV, const char *funcName) : m_visitor(SV), _funcName(funcName) {}
      ~FunctionBodyConsumer<T>() {}
      
      void HandleTopLevelDecl(clang::DeclGroupRef D) {
         for (clang::DeclGroupRef::iterator I = D.begin(), E = D.end(); I != E; ++I) {
            if (clang::FunctionDecl *FD = dyn_cast<clang::FunctionDecl>(*I)) {
               fprintf(stderr,"will visit:\n"); FD->dump();
               if (FD->getNameAsCString() == _funcName) {
                  if (clang::Stmt *S = FD->getBody()) {
                     m_visitor->VisitChildren(S);
                  }
               }
            }
         }
      }
      
   };
   
} // namespace ccons

#endif // CLING_VISITORS_H
