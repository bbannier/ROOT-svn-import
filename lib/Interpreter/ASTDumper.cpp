//------------------------------------------------------------------------------
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Vassil Vassilev <vasil.georgiev.vasilev@cern.ch>
//------------------------------------------------------------------------------

#include "ASTDumper.h"

#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclObjC.h"
#include "clang/AST/PrettyPrinter.h"
#include "clang/AST/Stmt.h"

using namespace clang;

namespace cling {

  ASTDumper::~ASTDumper() {}

  void ASTDumper::HandleTopLevelDecl(DeclGroupRef D) {
      for (DeclGroupRef::iterator I = D.begin(), E = D.end(); I != E; ++I)
        HandleTopLevelSingleDecl(*I);
    }
    
  void ASTDumper::HandleTopLevelSingleDecl(Decl* D) {
    PrintingPolicy Policy = D->getASTContext().PrintingPolicy;
    Policy.Dump = Dump;

    if (isa<FunctionDecl>(D) || isa<ObjCMethodDecl>(D)) {
      D->dump();
      
      if (Stmt* Body = D->getBody()) {
        llvm::errs() << "DeclStmts:---------------------------------\n";
        Body->dump();
        llvm::errs() << "End DeclStmts:-----------------------------\n\n\n\n";
      }
    }
  }
} // namespace cling
