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
      
      Stmt *newStmt; // the new/old node
      std::set<DeclRefExpr*> variables; // the DeclRefs

   public:
      bool IsEvalNeeded; // whether to emit the Eval call or not
 
      EvalInfo(Stmt *stmt, bool needed)
         :newStmt(stmt), IsEvalNeeded(needed) {};

      Stmt *getNewStmt() { return newStmt; }
      const Stmt *getNewStmt() const { return newStmt; }
      void setNewStmt(Stmt *stmt) { newStmt = stmt; } 
      
      const std::set<DeclRefExpr*> &getVariables() const { return variables; }
      void addVariable(DeclRefExpr *var) { 
         variables.insert(var);
      }
   };

} //end cling
