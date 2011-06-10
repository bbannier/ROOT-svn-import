//------------------------------------------------------------------------------
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Axel Naumann <axel@cern.ch>
//------------------------------------------------------------------------------

#ifndef CLING_INCREMENTAL_PARSER_H
#define CLING_INCREMENTAL_PARSER_H

#include "clang/Basic/SourceLocation.h"
#include "llvm/ADT/OwningPtr.h"
#include "llvm/ADT/StringRef.h"
#include <vector>

#include "ChainedASTConsumer.h"

namespace clang {
  class CompilerInstance;
  class Decl;
  class Parser;
  class Sema;
  class ASTConsumer;
  class PragmaNamespace;
  class SourceLocation;
  class FileID;
  class PCHGenerator;
}

namespace cling {
  class MutableMemoryBuffer;
  class ChainedASTConsumer;
  class DynamicExprTransformer;
  class Interpreter;
  class CIFactory;
  
  class IncrementalParser {
  public:
    IncrementalParser(Interpreter* interp, clang::PragmaNamespace* Pragma,
                      int argc, const char* const *argv, const char* llvmdir);
    ~IncrementalParser();
    void Initialize(const char* startupPCH);
    clang::CompilerInstance* getCI() const { return m_CI.get(); }
    clang::Parser* getParser() const { return m_Parser.get(); }
    clang::CompilerInstance* parse(llvm::StringRef src);
    clang::CompilerInstance* CompileLineFromPrompt(llvm::StringRef input);
    clang::CompilerInstance* CompilePreprocessed(llvm::StringRef input);
    clang::CompilerInstance* Compile(llvm::StringRef input);

    MutableMemoryBuffer* getCurBuffer() {
      return m_MemoryBuffer.back();
    }

    void enableDynamicLookup(bool value = true);
    bool isDynamicLookupEnabled() { 
      return m_DynamicLookupEnabled; 
    }
    DynamicExprTransformer* getTransformer() const { return m_Transformer.get(); }
    DynamicExprTransformer* getOrCreateTransformer(Interpreter* interp);
    
    void emptyLastFunction();
    clang::Decl* getLastTopLevelDecl() const { return m_LastTopLevelDecl; }
    clang::Decl* getFirstTopLevelDecl() const { return m_FirstTopLevelDecl; }
    
    void addConsumer(ChainedASTConsumer::EConsumerIndex I, clang::ASTConsumer* consumer);
    void removeConsumer(ChainedASTConsumer::EConsumerIndex I);

    bool usingStartupPCH() const { return m_UsingStartupPCH; }
    void writeStartupPCH();
  private:
    void loadStartupPCH(const char* filename);

    llvm::OwningPtr<clang::CompilerInstance> m_CI; // compiler instance.
    llvm::OwningPtr<clang::Parser> m_Parser; // parser (incremental)
    bool m_DynamicLookupEnabled; // enable/disable dynamic scope
    llvm::OwningPtr<DynamicExprTransformer> m_Transformer; // delayed id lookup
    std::vector<MutableMemoryBuffer*> m_MemoryBuffer; // One buffer for each command line, owner by the source file manager
    clang::FileID m_MBFileID; // file ID of the memory buffer
    ChainedASTConsumer* m_Consumer; // CI owns it
    clang::Decl* m_LastTopLevelDecl; // last top level decl after most recent call to parse()
    clang::Decl* m_FirstTopLevelDecl; // first top level decl
    bool m_UsingStartupPCH; // Whether the interpreter is using a PCH file to accelerate its startup
    std::string m_StartupPCHFilename; // Set when writing the PCH
    llvm::OwningPtr<clang::PCHGenerator> m_StartupPCHGenerator; // Startup PCH generator
  };
}
#endif // CLING_INCREMENTAL_PARSER_H
