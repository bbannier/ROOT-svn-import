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
#include "llvm/ExecutionEngine/JITEventListener.h"

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

   class EventListener : public llvm::JITEventListener {
   private:
      std::vector<llvm::Function *> m_vec_functions;
      llvm::ExecutionEngine *m_engine; 
      
   public:
      EventListener() { }
      virtual ~EventListener() { }
      
      virtual void NotifyFunctionEmitted(const llvm::Function&, void *, size_t,
                                         const JITEventListener::EmittedFunctionDetails&);
      virtual void NotifyFreeingMachineCode(void *OldPtr) {}
      
      void CleanupList();
      void UnregisterFunctionMapping(llvm::ExecutionEngine&);
   };
   
   class ExecutionContext {
   public:
      typedef void* (*LazyFunctionCreatorFunc_t)(const std::string&);
      
      ExecutionContext(Interpreter& Interp);
      ~ExecutionContext();
      
      static void* LazyFunction(const std::string&);
      
      static void* LazyFunctionCreator(const std::string&);
      
      clang::CompilerInstance* getCI() { return m_CI.get(); }
      
      void useModule(llvm::Module* m);
      
      void installLazyFunctionCreator(LazyFunctionCreatorFunc_t fp);
      
      bool startCodegen(clang::CompilerInstance* CI,
                        const std::string& filename);
      bool getModuleFromCodegen();
      
      void executeFunction(llvm::StringRef function, llvm::GenericValue* returnValue = 0);
      
      llvm::ExecutionEngine& getEngine() {
         return *m_engine.get();
      }
      
      clang::CodeGenerator* getCodeGenerator() const {
         return m_codeGen.get();
      }
      
   private:
      
      EventListener m_listener;
      
      static std::vector<std::string> m_vec_unresolved;
      static std::vector<LazyFunctionCreatorFunc_t> m_vec_lazy_function;
      
      int verifyModule(llvm::Module* m);
      void printModule(llvm::Module* m);
      bool runNewStaticConstructorsDestructors();
      
      llvm::OwningPtr<clang::CompilerInstance> m_CI; // compiler instance for growing AST.
      llvm::OwningPtr<llvm::ExecutionEngine> m_engine; // We own, our JIT.
      llvm::OwningPtr<clang::CodeGenerator> m_codeGen;
      llvm::Module* m_ee_module; // ExeEngine init module, owned by m_engine.
      llvm::Module* m_module; // IncrAST module, owned by m_engine.
      unsigned m_posInitGlobals; // position (global idx in out module) of next global to be initialized in m_ASTCI's AST
   };
} // end cling
#endif // CLING_EXECUTIONCONTEXT_H

