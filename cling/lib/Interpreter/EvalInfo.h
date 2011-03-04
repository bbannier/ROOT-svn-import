//------------------------------------------------------------------------------
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Vassil Vassilev <vasil.georgiev.vasilev@cern.ch>
//------------------------------------------------------------------------------

#ifndef CLING_EVALINFO_H
#define CLING_EVALINFO_H

namespace clang {
   class Stmt;
}

namespace cling {
   // The DynamicExprTransformer needs to have information about the nodes
   // it visits in order to escape properly the unknown symbols. Walking up 
   // it needs to know not only the node, which is being returned from the 
   // visited subnode, but it needs information from its subnode if the 
   // subnode can handle the dependent symbol itself or it wants delegate it
   // to its parent.
   // Ideally when given subnode has enough information to handle the unknown
   // symbol it should do it instead of delegating to the parent. This limits
   // the size of the expressions/statements being escaped.
   class EvalInfo {
   private:      
      clang::Stmt *m_newStmt; // the new/old node
   public:
      bool IsEvalNeeded; // whether to emit the Eval call or not
 
      EvalInfo(clang::Stmt *S, bool needed)
         :m_newStmt(S), IsEvalNeeded(needed) {};

      clang::Stmt *getNewStmt() const { return m_newStmt; }
      void setNewStmt(clang::Stmt *S) { m_newStmt = S; } 
   };

} //end cling
#endif
