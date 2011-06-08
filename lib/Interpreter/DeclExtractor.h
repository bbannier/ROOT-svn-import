//------------------------------------------------------------------------------
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Vassil Vassilev <vasil.georgiev.vasilev@cern.ch>
//------------------------------------------------------------------------------

#ifndef CLING_DECL_EXTRACTOR_H
#define CLING_DECL_EXTRACTOR_H

#include "clang/Sema/SemaConsumer.h"

namespace clang {
  class Decl;
}

namespace cling {
  class DeclExtractor : public clang::SemaConsumer {

  private:
    clang::ASTContext* m_Context;
    clang::Sema* m_Sema;

  public:
    DeclExtractor();
    virtual ~DeclExtractor();
    void Initialize(clang::ASTContext& Ctx);
    void InitializeSema(clang::Sema& S);
    void HandleTopLevelDecl(clang::DeclGroupRef DGR);
    void ForgetSema();

  private:
    void ExtractDecl(clang::Decl* D);    
  };

} // namespace cling

#endif // CLING_DECL_EXTRACTOR_H
