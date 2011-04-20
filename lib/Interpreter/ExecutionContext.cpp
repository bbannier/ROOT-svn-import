//--------------------------------------------------------------------*- C++ -*-
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Axel naumann <ljanyst@cern.ch>
//------------------------------------------------------------------------------

#include "ExecutionContext.h"

#include "llvm/Constants.h"
#include "llvm/GlobalVariable.h"
#include "llvm/Module.h"
#include "llvm/PassManager.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/Assembly/PrintModulePass.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/ExecutionEngine/JITEventListener.h"
#include "llvm/LLVMContext.h"
#include "llvm/Support/DynamicLibrary.h"
#include "llvm/Target/TargetOptions.h"

#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclGroup.h"
#include "clang/CodeGen/ModuleBuilder.h"

#include "cling/Interpreter/CIFactory.h"
#include "cling/Interpreter/Interpreter.h"
#include "cling/Interpreter/Value.h"

#include <cstdio>
#include <iostream>
#include <utility>
#include <cxxabi.h>

using namespace cling;

namespace {
  class JITtedFunctionCollector : public llvm::JITEventListener {
  private:
    std::vector<llvm::Function *> m_vec_functions;
    llvm::ExecutionEngine *m_engine; 
    
  public:
    JITtedFunctionCollector() { }
    virtual ~JITtedFunctionCollector() { }
    
    virtual void NotifyFunctionEmitted(const llvm::Function&, void *, size_t,
                                       const JITEventListener::EmittedFunctionDetails&);
    virtual void NotifyFreeingMachineCode(void *OldPtr) {}
    
    void CleanupList();
    void UnregisterFunctionMapping(llvm::ExecutionEngine&);
  };
}


void JITtedFunctionCollector::NotifyFunctionEmitted(const llvm::Function &F,
                                          void *Code, size_t Size,
                                          const JITEventListener::EmittedFunctionDetails &Details)
{
   //BB std::cerr << "JITtedFunctionCollector::NotifyFunctionEmitted: m_vec_functions.push_back("
   //BB           << F.getName().data() << "); Code @ " << Code << std::endl;
   m_vec_functions.push_back(const_cast<llvm::Function *>(&F));
}

void JITtedFunctionCollector::CleanupList()
{
   m_vec_functions.clear(); 
}

void JITtedFunctionCollector::UnregisterFunctionMapping(llvm::ExecutionEngine &engine)
{
   std::vector<llvm::Function *>::reverse_iterator it;

   for (it=m_vec_functions.rbegin(); it < m_vec_functions.rend(); it++) {
      llvm::Function *ff = (llvm::Function *)*it;
      //BB std::cerr << "JITtedFunctionCollector::UnregisterFunctionMapping: updateGlobalMapping("
      //BB          << ff->getName().data() << ", 0); Global @" << engine.getPointerToGlobalIfAvailable(ff) << std::endl;
      engine.freeMachineCodeForFunction(ff);
      engine.updateGlobalMapping(ff, 0);
      //BB std::cerr << "Global after delete @" << engine.getPointerToGlobalIfAvailable(ff) << std::endl;
   }
   m_vec_functions.clear(); 
}


std::vector<std::string> ExecutionContext::m_vec_unresolved;
std::vector<ExecutionContext::LazyFunctionCreatorFunc_t> ExecutionContext::m_vec_lazy_function;

ExecutionContext::ExecutionContext(clang::CompilerInstance* CI):
  m_CI(CI),
  m_engine(0),
  m_module(0),
  m_posInitGlobals(0)
{
  // If not set, exception handling will not be turned on
  llvm::JITExceptionHandling = true;
  m_codeGen.reset(CreateLLVMCodeGen(CI->getDiagnostics(), 
                                    "cling input",
                                    CI->getCodeGenOpts(), 
                                    * new llvm::LLVMContext())
                  );
  m_codeGen->Initialize(CI->getASTContext());
}

void
ExecutionContext::InitializeBuilder()
{
   //
   //  Create an execution engine to use.
   //
   // Note: Engine takes ownership of the module.
   if (m_engine) return;

   assert(m_codeGen && "Cannot initialize builder without module!");

   m_module = m_codeGen->GetModule();

   llvm::EngineBuilder builder(m_module);
   builder.setOptLevel(llvm::CodeGenOpt::Less);
   std::string errMsg;
   builder.setErrorStr(&errMsg);
   builder.setEngineKind(llvm::EngineKind::JIT);
   m_engine = builder.create();
   if (!m_engine) {
      std::cerr << "Error: Unable to create the execution engine!\n";
      std::cerr << errMsg << '\n';
   }
   //m_engine->addModule(m_module); // Note: The engine takes ownership of the module.

   // install lazy function
   m_engine->InstallLazyFunctionCreator(NotifyLazyFunctionCreators);
}

ExecutionContext::~ExecutionContext()
{
   if (m_codeGen)
      m_codeGen->ReleaseModule();
}

void unresolvedSymbol()
{
   // throw exception?
   std::cerr << "Error: calling unresolved symbol (should never happen)!"
             << std::endl;
}

void* ExecutionContext::HandleMissingFunction(const std::string& mangled_name)
{
   // Not found in the map, add the symbol in the list of unresolved symbols
   std::vector<std::string>::iterator it;
   it = find (m_vec_unresolved.begin(), m_vec_unresolved.end(), mangled_name);
   if (it == m_vec_unresolved.end())
      m_vec_unresolved.push_back(mangled_name);
   // Avoid "ISO C++ forbids casting between pointer-to-function and
   // pointer-to-object":
   return (void*)reinterpret_cast<size_t>(unresolvedSymbol);
}

void*
ExecutionContext::NotifyLazyFunctionCreators(const std::string& mangled_name)
{
   void *ret = 0;
   std::vector<LazyFunctionCreatorFunc_t>::iterator it;
  
   for (it=m_vec_lazy_function.begin(); it < m_vec_lazy_function.end(); it++) {
      ret = (void*)((LazyFunctionCreatorFunc_t)*it)(mangled_name);
      if (ret != 0) return ret;
   }
   return HandleMissingFunction(mangled_name);
}

void
ExecutionContext::executeFunction(llvm::StringRef funcname, 
                                  llvm::GenericValue* returnValue)
{
   // Call an extern C function without arguments
  runCodeGen();

   llvm::Function* f = m_engine->FindFunctionNamed(funcname.data());
   if (!f) {
      fprintf(
           stderr
         , "ExecutionContext::executeFunction: Could not find function named: %s\n"
         , funcname.data()
      );
      return;
   }
   JITtedFunctionCollector listener;
   // register the listener
   m_engine->RegisterJITEventListener(&listener);
   m_engine->getPointerToFunction(f);
   // check if there is any unresolved symbol in the list
   if (!m_vec_unresolved.empty()) {
      std::cerr << "ExecutionContext::executeFunction:" << std::endl;
      for (size_t i = 0, e = m_vec_unresolved.size(); i != e; ++i) {
         std::cerr << "Error: Symbol \'" << m_vec_unresolved[i] << 
                      "\' unresolved!" << std::endl;
         llvm::Function *ff = m_engine->FindFunctionNamed(m_vec_unresolved[i].c_str());
         if (ff) {
            m_engine->updateGlobalMapping(ff, 0);
            m_engine->freeMachineCodeForFunction(ff);
         }
         else {
            std::cerr << "Error: Canot find symbol \'" << m_vec_unresolved[i] << 
                         std::endl;
         }
      }
      m_vec_unresolved.clear();
      // cleanup functions
      listener.UnregisterFunctionMapping(getEngine());   
      m_engine->UnregisterJITEventListener(&listener);
      return;
   }
   // cleanup list and unregister our listener
   listener.CleanupList();
   m_engine->UnregisterJITEventListener(&listener);

   std::vector<llvm::GenericValue> args;
   llvm::GenericValue val;
   if (!returnValue)
      returnValue = &val;

   *returnValue = m_engine->runFunction(f, args);
   //
   //fprintf(stderr, "Finished running generated code with JIT.\n");
   //
   // Print the result.
   //llvm::outs() << "Result: " << ret.IntVal << "\n";
   m_engine->freeMachineCodeForFunction(f);
}


void
ExecutionContext::runCodeGen() {
  InitializeBuilder();
  assert(m_module && "Code generation did not create a module!");
  m_engine->runStaticConstructorsDestructors(false);
  llvm::GlobalVariable* gctors = m_module->getGlobalVariable("llvm.global_ctors", true);
   if (gctors) {
      gctors->dropAllReferences();
      gctors->eraseFromParent();
   }
}

int
ExecutionContext::verifyModule(llvm::Module* m)
{
   //
   //  Verify generated module.
   //
   bool mod_has_errs = llvm::verifyModule(*m, llvm::PrintMessageAction);
   if (mod_has_errs) {
      return 1;
   }
   return 0;
}

void
ExecutionContext::printModule(llvm::Module* m)
{
   //
   //  Print module LLVM code in human-readable form.
   //
   llvm::PassManager PM;
   PM.add(llvm::createPrintModulePass(&llvm::outs()));
   PM.run(*m);
}

void
ExecutionContext::installLazyFunctionCreator(LazyFunctionCreatorFunc_t fp)
{
   m_vec_lazy_function.push_back(fp);
   //m_engine->InstallLazyFunctionCreator(fp);
}

