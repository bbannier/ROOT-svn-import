//--------------------------------------------------------------------*- C++ -*-
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Axel naumann <ljanyst@cern.ch>
//------------------------------------------------------------------------------

#ifndef CLING_EXECUTIONCONTEXT_H
#define CLING_EXECUTIONCONTEXT_H

#include "llvm/ADT/OwningPtr.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Path.h"

namespace llvm {
  class Module;
  class ExecutionEngine;
  class GenericValue;
}

namespace clang {
  class CompilerInstance;
  class CodeGenerator;
}

namespace cling {
  class Interpreter;
  class Value;
  
  class ExecutionContext {
  public:
    typedef void* (*LazyFunctionCreatorFunc_t)(const std::string&);
    
    ExecutionContext(clang::CompilerInstance* CI);
    ~ExecutionContext();
    
    llvm::ExecutionEngine& getEngine() { return *m_engine; }
    clang::CodeGenerator* getCodeGenerator() const { return m_codeGen.get(); }
    
    void installLazyFunctionCreator(LazyFunctionCreatorFunc_t fp);

    void runCodeGen();
    
    void executeFunction(llvm::StringRef function, 
                         llvm::GenericValue* returnValue = 0);
    
  private:
    static void* HandleMissingFunction(const std::string&);
    static void* NotifyLazyFunctionCreators(const std::string&);

    int verifyModule(llvm::Module* m);
    void printModule(llvm::Module* m);
    void InitializeBuilder();
    
    static std::vector<std::string> m_vec_unresolved;
    static std::vector<LazyFunctionCreatorFunc_t> m_vec_lazy_function;
    
    llvm::OwningPtr<clang::CodeGenerator> m_codeGen;
    clang::CompilerInstance* m_CI;
    llvm::ExecutionEngine* m_engine; // Owned by JIT
    llvm::Module* m_module; // IncrAST module, owned by m_engine.
    unsigned m_posInitGlobals; // position (global idx in out module) of next global to be initialized in m_ASTCI's AST
   };
} // end cling
#endif // CLING_EXECUTIONCONTEXT_H

