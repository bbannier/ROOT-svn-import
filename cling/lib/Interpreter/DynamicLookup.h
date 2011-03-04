//------------------------------------------------------------------------------
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id: ASTTransformVisitor.cpp 36608 2010-11-11 18:21:02Z vvassilev $
// author:  Vassil Vassilev <vasil.georgiev.vasilev@cern.ch>
//------------------------------------------------------------------------------

#ifndef CLING_DYNAMIC_LOOKUP_H
#define CLING_DYNAMIC_LOOKUP_H

#include "clang/Sema/ExternalSemaSource.h"
namespace clang {
  class Sema;
}

namespace cling {
  class DynamicIDHandler : public clang::ExternalSemaSource {
  public:
    DynamicIDHandler(clang::Sema* Sema) : m_Sema(Sema){}
    ~DynamicIDHandler();
    
    // Override this to provide last resort lookup for failed unqualified lookups
    virtual bool LookupUnqualified(clang::LookupResult &R, clang::Scope *S);
    //Remove the fake dependent declarations
    void RemoveFakeDecls();
  private:
    llvm::SmallVector<clang::Decl*, 8> m_FakeDecls;
    clang::Sema* m_Sema;
  };
} // end namespace cling
#endif // CLING_DYNAMIC_LOOKUP_H
