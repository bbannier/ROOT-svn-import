//------------------------------------------------------------------------------
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id: DependentNodesTransform.h 36608 2010-11-11 18:21:02Z vvassilev $
// author:  Vassil Vassilev <vasil.georgiev.vasilev@cern.ch>
//------------------------------------------------------------------------------

#include "clang/Sema/Sema.h"

using namespace clang;
namespace cling {
   // This class is only interface between ASTTrasnformVisitor and the Interpreter. 
   // Perhaps it should be removed.
   class DependentNodesTransform {

   public:
      DependentNodesTransform(){}
      ~DependentNodesTransform(){}
      
      void TransformNodes(Sema *Sema);
   };
}//end cling
