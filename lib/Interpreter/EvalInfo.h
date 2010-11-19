//------------------------------------------------------------------------------
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id: ASTTransformVisitor.h 36608 2010-11-11 18:21:02Z vvassilev $
// author:  Vassil Vassilev <vasil.georgiev.vasilev@cern.ch>
//------------------------------------------------------------------------------

#include <set>

namespace clang {
   class Stmt;
   class DeclRefExpr;
}

namespace cling {
   class EvalInfo {      
   private:
      typedef clang::Stmt Stmt;
      typedef clang::DeclRefExpr DeclRefExpr;
      
      Stmt *m_newStmt; // the new/old node
      std::set<DeclRefExpr*> m_variables; // the DeclRefs

   public:
      bool IsEvalNeeded; // whether to emit the Eval call or not
 
      EvalInfo(Stmt *stmt, bool needed)
         :m_newStmt(stmt), IsEvalNeeded(needed) {};

      Stmt *getNewStmt() const { return m_newStmt; }
      void setNewStmt(Stmt *stmt) { m_newStmt = stmt; } 
      
      const std::set<DeclRefExpr*> &getVariables() const { return m_variables; }
      void addVariable(DeclRefExpr *var) { 
         m_variables.insert(var);
      }
   };

} //end cling
