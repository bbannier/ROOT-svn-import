//--------------------------------------------------------------------*- C++ -*-
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Vassil Vassilev <vasil.georgiev.vasilev@cern.ch>
//------------------------------------------------------------------------------

#ifndef CLING_AST_UTILS_H
#define CLING_AST_UTILS_H

#include "clang/AST/Type.h"

namespace clang {
  class Expr;
  class Sema;
}

namespace cling {
  class Synthesize {
  public:
    static clang::Expr* CStyleCastPtrExpr(clang::Sema* S, 
                                          clang::QualType Ty, uint64_t Ptr);
  };
} // end namespace cling
#endif // CLING_AST_UTILS_H
