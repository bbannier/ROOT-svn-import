//--------------------------------------------------------------------*- C++ -*-
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Vassil Vassilev <vasil.georgiev.vasilev@cern.ch>
//------------------------------------------------------------------------------

#include "ASTUtils.h"

#include "clang/Sema/Sema.h"

using namespace clang;

namespace cling {
  Expr* Synthesize::CStyleCastPtrExpr(Sema* S, QualType Ty, uint64_t Ptr) {
    ASTContext& Ctx = S->getASTContext();
    if (!Ty->isPointerType())
      Ty = Ctx.getPointerType(Ty);
    TypeSourceInfo* TSI = Ctx.CreateTypeSourceInfo(Ty);
    const llvm::APInt Addr(8 * sizeof(void *), Ptr);
    
    Expr* Result = IntegerLiteral::Create(Ctx, Addr, Ctx.UnsignedLongTy, 
                                          SourceLocation());
    Result = S->BuildCStyleCastExpr(SourceLocation(), TSI, SourceLocation(),
                                         Result).take();
    assert(Result && "Cannot create CStyleCastPtrExpr");
    return Result;

  }
} // end namespace cling
