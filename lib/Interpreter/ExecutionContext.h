//--------------------------------------------------------------------*- C++ -*-
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Axel naumann <ljanyst@cern.ch>
//------------------------------------------------------------------------------

#ifndef CLING_EXECUTIONCONTEXT_H
#define CLING_EXECUTIONCONTEXT_H

#include "llvm/ADT/OwningPtr.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/System/Path.h"

namespace llvm {
class Module;
class ExecutionEngine;
}

namespace clang {
class CompilerInstance;
}

namespace cling {
class Interpreter;

class ExecutionContext {
public:
   typedef void* (*LazyFunctionCreatorFunc_t)(const std::string&);

   ExecutionContext(Interpreter& Interp);
   ~ExecutionContext();

   clang::CompilerInstance* getCI() { return m_CI.get(); }

   void useModule(llvm::Module*& m);

   void installLazyFunctionCreator(LazyFunctionCreatorFunc_t fp);

   bool doCodegen(clang::CompilerInstance* CI,
                  const std::string& filename);

   void executeFunction(llvm::StringRef funcname);

private:
   int verifyModule(llvm::Module* m);
   void printModule(llvm::Module* m);
   bool runNewStaticConstructorsDestructors();

   llvm::OwningPtr<clang::CompilerInstance> m_CI; // compiler instance for growing AST.
   llvm::OwningPtr<llvm::ExecutionEngine> m_engine; // We own, our JIT.
   llvm::Module* m_prev_module; // We do *not* own, m_engine owns it.
   std::pair<unsigned,unsigned> m_posInitGlobals; // position (module idx, global idx) of next global to be initialized in m_ASTCI's AST

   Interpreter* m_Interpreter;
};
}
#endif // CLING_EXECUTIONCONTEXT_H

