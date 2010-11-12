//------------------------------------------------------------------------------
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id: DependentNodesTransform.cpp 36608 2010-11-11 18:21:02Z vvassilev $
// author:  Vassil Vassilev <vasil.georgiev.vasilev@cern.ch>
//------------------------------------------------------------------------------

#include "DependentNodesTransform.h"
#include "ASTTransformVisitor.h"

namespace cling {
   void DependentNodesTransform::TransformNodes(Sema *Sema){
      ASTTransformVisitor transformer(Sema);
      transformer.Visit(Sema->getASTContext().getTranslationUnitDecl());
   }
}//end cling
