//------------------------------------------------------------------------------
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Axel Naumann <axel@cern.ch>
//------------------------------------------------------------------------------

#ifndef CLING_INCREMENTAL_AST_PARSER_H
#define CLING_INCREMENTAL_AST_PARSER_H

#include "llvm/ADT/OwningPtr.h"
#include "llvm/ADT/StringRef.h"

namespace clang {
  class CompilerInstance;
  class Parser;
  class Sema;
  class ASTConsumer;
}

namespace cling {
  class MutableMemoryBuffer;
  class ChainedASTConsumer;

  class IncrementalASTParser {
  public:
    IncrementalASTParser(clang::CompilerInstance* CI,
                         clang::ASTConsumer* Consumer);
    ~IncrementalASTParser();
    
    clang::CompilerInstance* getCI() const { return m_CI.get(); }
    clang::CompilerInstance* parse(llvm::StringRef src,
                                   int nTopLevelDecls = 1,
                                   clang::ASTConsumer* Consumer = 0);
    
  private:
    llvm::OwningPtr<clang::CompilerInstance> m_CI; // compiler instance.
    llvm::OwningPtr<clang::Parser> m_Parser; // parser (incremental)
    llvm::OwningPtr<clang::Sema> m_Sema; // sema used for parsing
    llvm::OwningPtr<MutableMemoryBuffer> m_MemoryBuffer; // compiler instance.
    ChainedASTConsumer* m_Consumer; // CI owns it
    bool m_InterruptAtNextTopLevelDecl; // whether to stop parsing at the next top level decl 
  };
}
#endif // CLING_INCREMENTAL_AST_PARSER_H
