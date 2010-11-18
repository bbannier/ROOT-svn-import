//------------------------------------------------------------------------------
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id: ASTTransformVisitor.h 36608 2010-11-11 18:21:02Z vvassilev $
// author:  Vassil Vassilev <vasil.georgiev.vasilev@cern.ch>
//------------------------------------------------------------------------------

#include <set>

#include "clang/AST/Stmt.h"

using namespace clang;

namespace cling {
   struct EvalInfo {
      EvalInfo(Stmt *stmt, bool needed, std::set<DeclRefExpr*> *vars)
         :NewStmt(stmt), IsEvalNeeded(needed), Variables(vars){};
      Stmt *NewStmt; // the new/old node
      bool IsEvalNeeded; // whether to emit the Eval call or not
      std::set<DeclRefExpr*> *Variables; // the DeclRefs
   };

} //end cling
