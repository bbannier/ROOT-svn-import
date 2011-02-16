//--------------------------------------------------------------------*- C++ -*-
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id: Interpreter.h 35384 2010-09-22 15:06:50Z axel $
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
#include "llvm/Support/DynamicLibrary.h"
#include "llvm/Target/TargetOptions.h"

#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclGroup.h"
#include "clang/CodeGen/ModuleBuilder.h"

#include "cling/Interpreter/CIBuilder.h"
#include "cling/Interpreter/Interpreter.h"

#include <cstdio>
#include <iostream>
#include <utility>
#include <cxxabi.h>

using namespace cling;

void EventListener::NotifyFunctionEmitted(const llvm::Function &F,
                                          void *Code, size_t Size,
                                          const JITEventListener::EmittedFunctionDetails &Details)
{
   std::cerr << "EventListener::NotifyFunctionEmitted: m_vec_functions.push_back("
             << F.getName().data() << "); Code @ " << Code << std::endl;
   m_vec_functions.push_back(const_cast<llvm::Function *>(&F));
}

void EventListener::CleanupList()
{
   m_vec_functions.clear(); 
}

void EventListener::UnregisterFunctionMapping(llvm::ExecutionEngine &engine)
{
   std::vector<llvm::Function *>::reverse_iterator it;

   for (it=m_vec_functions.rbegin(); it < m_vec_functions.rend(); it++) {
      llvm::Function *ff = (llvm::Function *)*it;
      std::cerr << "EventListener::UnregisterFunctionMapping: updateGlobalMapping("
                << ff->getName().data() << ", 0); Global @" << engine.getPointerToGlobalIfAvailable(ff) << std::endl;
      engine.freeMachineCodeForFunction(ff);
      engine.updateGlobalMapping(ff, 0);
      std::cerr << "Global after delete @" << engine.getPointerToGlobalIfAvailable(ff) << std::endl;
   }
   m_vec_functions.clear(); 
}


std::vector<std::string> ExecutionContext::m_vec_unresolved;
std::vector<ExecutionContext::LazyFunctionCreatorFunc_t> ExecutionContext::m_vec_lazy_function;

ExecutionContext::ExecutionContext(Interpreter& Interp):
   m_ee_module(0),
   m_module(0),
   m_posInitGlobals(0)
{
  // If not set, exception handling will not be turned on
  llvm::JITExceptionHandling = true;

   m_ee_module
      = new llvm::Module("_Clang_first",
                         *Interp.getCIBuilder().getLLVMContext());
   //
   //  Create an execution engine to use.
   //
   // Note: Engine takes ownership of the module.
   llvm::EngineBuilder builder(m_ee_module);
   builder.setOptLevel(llvm::CodeGenOpt::Less);
   std::string errMsg;
   builder.setErrorStr(&errMsg);
   builder.setEngineKind(llvm::EngineKind::JIT);
   m_engine.reset(builder.create());
   if (!m_engine) {
      std::cerr << "Error: Unable to create the execution engine!\n";
      std::cerr << errMsg << '\n';
   }

   // install lazy function
   m_engine->InstallLazyFunctionCreator(LazyFunctionCreator);

  // temporarily set m_module to run initializers:
  m_module = m_ee_module;
  runNewStaticConstructorsDestructors();
  m_module = 0;
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

void* ExecutionContext::LazyFunction(const std::string& mangled_name)
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
ExecutionContext::LazyFunctionCreator(const std::string& mangled_name)
{
   void *ret = 0;
   std::vector<LazyFunctionCreatorFunc_t>::iterator it;
  
   for (it=m_vec_lazy_function.begin(); it < m_vec_lazy_function.end(); it++) {
      ret = (void*)((LazyFunctionCreatorFunc_t)*it)(mangled_name);
      if (ret != 0) return ret;
   }
   return LazyFunction(mangled_name);
}

void
ExecutionContext::executeFunction(llvm::StringRef funcname, llvm::GenericValue* returnValue)
{

   // Call an extern C function without arguments
   llvm::Function* f = m_engine->FindFunctionNamed(funcname.data());
   if (!f) {
      fprintf(
           stderr
         , "ExecutionContext::executeFunction: Could not find function named: %s\n"
         , funcname.data()
      );
      return;
   }
   // register the listener
   m_engine->RegisterJITEventListener(&m_listener);
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
      m_listener.UnregisterFunctionMapping(getEngine());   
      m_engine->UnregisterJITEventListener(&m_listener);
      return;
   }
   // cleanup list and unregister our listener
   m_listener.CleanupList();
   m_engine->UnregisterJITEventListener(&m_listener);

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


bool
ExecutionContext::startCodegen(clang::CompilerInstance* CI,
                            const std::string& filename)
{
  // CodeGen start: parse old AST
  
  if (!m_codeGen.get()) {
    clang::TranslationUnitDecl* tu =
    CI->getASTContext().getTranslationUnitDecl();
    if (!tu) {
      fprintf(stderr,
              "ExecutionContext::startCodegen: No translation unit decl passed!\n");
      return false;
    }
    
    m_codeGen.reset(
                    CreateLLVMCodeGen(CI->getDiagnostics(), filename, CI->getCodeGenOpts(),
                                      CI->getLLVMContext()));
    m_codeGen->Initialize(CI->getASTContext());
    clang::TranslationUnitDecl::decl_iterator iter = tu->decls_begin();
    clang::TranslationUnitDecl::decl_iterator iter_end = tu->decls_end();
    //fprintf(stderr, "Running code generation.\n");
    for (; iter != iter_end; ++iter) {
      m_codeGen->HandleTopLevelDecl(clang::DeclGroupRef(*iter));
    }
  }
  return true;
}
  
bool
ExecutionContext::getModuleFromCodegen()
{
   llvm::Module* m = m_codeGen->GetModule();

   if (!m) {
      fprintf(stderr,
              "ExecutionContext::getModuleFromCodeGen: Code generation did not create a module!\n");
      return false;
   }

   //printModule(m);

   //
   //  Give new module to the execution engine.
   //
   useModule(m); // Note: The engine takes ownership of the module.

   return true;
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

bool
ExecutionContext::runNewStaticConstructorsDestructors()
{
  // incremental version of m_engine->runStaticConstructorsDestructors(false);
  
  if (!m_module) return true;
  llvm::GlobalVariable *GV = m_module->getNamedGlobal("llvm.global_ctors");
  if (!GV) return true;
  llvm::ConstantArray *InitList = dyn_cast<llvm::ConstantArray>(GV->getInitializer());
  if (!InitList) return true;
  for (unsigned i = m_posInitGlobals, e = InitList->getNumOperands(); i != e; ++i) {
    m_posInitGlobals = 0;
    if (llvm::ConstantStruct *CS = 
        dyn_cast<llvm::ConstantStruct>(InitList->getOperand(i))) {
      if (CS->getNumOperands() != 2)
        return false; // Not array of 2-element structs.
      
      llvm::Constant *FP = CS->getOperand(1);
      if (FP->isNullValue())
        break;  // Found a null terminator, exit.
      
      if (llvm::ConstantExpr *CE = dyn_cast<llvm::ConstantExpr>(FP))
        if (CE->isCast())
          FP = CE->getOperand(0);
      if (llvm::Function *F = dyn_cast<llvm::Function>(FP)) {
        // Execute the ctor/dtor function!
        m_engine->runFunction(F, std::vector<llvm::GenericValue>());
      }
    }
    m_posInitGlobals = i + 1;
  }
  return true;
}
  

void
ExecutionContext::useModule(llvm::Module* m)
{
   // Use a new module, replacing the existing one.
   // Transfers global mappings before replacement.
   // Note: we take ownership of the module m!

  if (!m_module) {
    m_engine->addModule(m); // Note: The engine takes ownership of the module.
    m_module = m;
  }
  runNewStaticConstructorsDestructors();
}


void
ExecutionContext::installLazyFunctionCreator(LazyFunctionCreatorFunc_t fp)
{
   m_vec_lazy_function.push_back(fp);
   //m_engine->InstallLazyFunctionCreator(fp);
}

