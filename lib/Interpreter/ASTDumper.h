//------------------------------------------------------------------------------
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Vassil Vassilev <vasil.georgiev.vasilev@cern.ch>
//------------------------------------------------------------------------------

#ifndef CLING_AST_DUMPER_H
#define CLING_AST_DUMPER_H

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/DeclGroup.h"

#include "llvm/Support/raw_os_ostream.h"

namespace cling {

  class ASTDumper : public clang::ASTConsumer {

  private:
    llvm::raw_ostream& Out;
    bool Dump;
    
  public:
    ASTDumper(llvm::raw_ostream* o = NULL, bool Dump = false)
      : Out(o? *o : llvm::outs()), Dump(Dump) { }
    virtual ~ASTDumper();
    
    virtual void HandleTopLevelDecl(clang::DeclGroupRef D);

  private:
    void HandleTopLevelSingleDecl(clang::Decl* D);
  };

} // namespace cling

#endif // CLING_AST_DUMPER_H
