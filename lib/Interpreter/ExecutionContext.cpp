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

#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclGroup.h"
#include "clang/CodeGen/ModuleBuilder.h"

#include "cling/Interpreter/CIBuilder.h"
#include "cling/Interpreter/Interpreter.h"

#include <cstdio>
#include <iostream>
#include <utility>

using namespace cling;

namespace {

//-------------------------------------------------------------------------
// Copy the execution engine memory mappings for the global
// variables in the source module to the destination module.
//-------------------------------------------------------------------------
static
void
copyGlobalMappings(llvm::ExecutionEngine* ee, llvm::Module* src,
                   llvm::Module* dst)
{
   // Loop over all the global variables in the destination module.
   std::string new_global_name;
   llvm::Module::global_iterator dst_iter = dst->global_begin();
   llvm::Module::global_iterator dst_end = dst->global_end();
   for (; dst_iter != dst_end; ++dst_iter) {
      new_global_name = dst_iter->getName();
      if (new_global_name.size() > 1) {
         if (new_global_name.substr(0, 5) == "llvm.") {
            continue;
         }
         if (new_global_name[0] == '_') {
            if (
               (new_global_name[1] == '_') ||
               std::isupper(new_global_name[1])
            ) {
               continue;
            }
         }
         if (new_global_name[0] == '.') {
            continue;
         }
      }
      //fprintf(stderr, "Destination module has global: %s\n",
      //        new_global_name.c_str());
      //fprintf(stderr, "Search source module for global var: %s\n",
      //        dst_iter->getName().data());
      // Find the same variable (by name) in the source module.
      llvm::GlobalVariable* src_gv =
         src->getGlobalVariable(dst_iter->getName());
      if (!src_gv) { // no such global in prev module
         continue; // skip it
      }
      // Get the mapping from the execution engine for the source
      // global variable and create a new mapping to the same
      // address for the destination global variable.  Now they
      // share the same allocated memory (and so have the same value).
      // FIXME: We should compare src var and dst var types here!
      void* p = ee->getPointerToGlobal(src_gv);
      //fprintf(stderr, "Setting mapping for: %s to %lx\n",
      //   dst_iter->getName().data(), (unsigned long) p);
      // And duplicate it for the destination module.
      ee->addGlobalMapping(&*dst_iter, p);
   }
   // This example block copies the global variable and the mapping.
   //GlobalVariable* src_gv = &*src_global_iter;
   //void* p = ee->getPointerToGlobal(src_gv);
   //string name = src_gv->getName();
   //// New global variable is owned by destination module.
   //GlobalVariable* dst_gv = new GlobalVariable(
   //  *dest_module, // Module&
   //  src_gv->getType(), // const Type*
   //  src_gv->isConstant(), // bool, isConstant
   //  src_gv->getLinkage(), // LinkageTypes
   //  src_gv->getInitializer(), // Constant*, Initializer
   //  "" // const Twine&, Name
   //);
   //dst_gv->copyAttributesFrom(src_gv);
   //++src_global_iter;
   //src_gv->eraseFromParent();
   //dst_gv->setName(name);
   //ee->addGlobalMapping(dst_gv, p);
}

} // unnamed namespace


ExecutionContext::ExecutionContext(Interpreter& Interp):
   m_prev_module(0),
   m_Interpreter(&Interp)
{
   m_CI.reset(m_Interpreter->createCI());
   m_prev_module
      = new llvm::Module("_Clang_first",
                         *m_Interpreter->getCIBuilder().getLLVMContext());
   //
   //  Create an execution engine to use.
   //
   // Note: Engine takes ownership of the module.
   llvm::EngineBuilder builder(m_prev_module);
   std::string errMsg;
   builder.setErrorStr(&errMsg);
   builder.setEngineKind(llvm::EngineKind::JIT);
   m_engine.reset(builder.create());
   if (!m_engine) {
      std::cerr << "Error: Unable to create the execution engine!\n";
      std::cerr << errMsg << '\n';
   }
}
ExecutionContext::~ExecutionContext()
{
   m_CI->takeLLVMContext(); // Don't take down the context with the CI.
}


void
ExecutionContext::executeFunction(llvm::StringRef funcname)
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
   std::vector<llvm::GenericValue> args;
   llvm::GenericValue ret = m_engine->runFunction(f, args);
   //
   //fprintf(stderr, "Finished running generated code with JIT.\n");
   //
   // Print the result.
   //llvm::outs() << "Result: " << ret.IntVal << "\n";
   // Run global destruction.
   //m_engine->runStaticConstructorsDestructors(true);
   m_engine->freeMachineCodeForFunction(f);
}


bool
ExecutionContext::startCodegen(clang::CompilerInstance* CI,
                            const std::string& filename)
{
   // CodeGen start: parse old AST

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
  return true;
}

bool
ExecutionContext::getModuleFromCodegen()
{
   llvm::Module* m = m_codeGen->ReleaseModule();
  // Remove Codegen, it's once per start / getModuleFromCodeGen() pair
  m_codeGen.reset(0);

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

   for (unsigned mi = m_posInitGlobals.first, e = m_engine->modules_size();
        mi != e; ++mi) {
      const llvm::Module* module = m_engine->modules_at(mi);
      llvm::GlobalVariable *GV = module->getNamedGlobal("llvm.global_ctors");
      if (!GV) continue;
      llvm::ConstantArray *InitList = dyn_cast<llvm::ConstantArray>(GV->getInitializer());
      m_posInitGlobals.first = mi;
      if (!InitList) continue;
      for (unsigned i = m_posInitGlobals.second, e = InitList->getNumOperands(); i != e; ++i) {
         m_posInitGlobals.second = 0;
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
         m_posInitGlobals.second = i + 1;
      }
   }
   return true;
}


void
ExecutionContext::useModule(llvm::Module*& m)
{
   // Use a new module, replacing the existing one.
   // Transfers global mappings before replacement.
   // Note: we take ownership of the module m!

   //
   //  Transfer global mappings from previous module.
   //
   copyGlobalMappings(m_engine.get(), m_prev_module, m);

   //  All done with previous module, delete it.
   //
   {
      bool ok = m_engine->removeModule(m_prev_module);
      if (!ok) {
         fprintf(stderr, "Previous module not found in execution engine!\n");
      }
      delete m_prev_module;
      m_prev_module = m;
   }
   //
   //  Give new module to the execution engine.
   //
   m_engine->addModule(m); // Note: The engine takes ownership of the module.
   runNewStaticConstructorsDestructors();
   m = 0;
}


void
ExecutionContext::installLazyFunctionCreator(LazyFunctionCreatorFunc_t fp)
{
   m_engine->InstallLazyFunctionCreator(fp);
}

