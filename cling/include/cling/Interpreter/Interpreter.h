//--------------------------------------------------------------------*- C++ -*-
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Lukasz Janyst <ljanyst@cern.ch>
//------------------------------------------------------------------------------

#ifndef CLING_COMPILER_H
#define CLING_COMPILER_H

#include "llvm/System/Path.h"

#include <string>
#include <vector>

//------------------------------------------------------------------------------
// Forward declarations from LLVM
//------------------------------------------------------------------------------
namespace llvm {
class Module;
class LLVMContext;
class ExecutionEngine;
}

//------------------------------------------------------------------------------
// Forward declarations from CLANG
//------------------------------------------------------------------------------
namespace clang {
class ASTConsumer;
class CompilerInstance;
class FunctionDecl;
class Preprocessor;
class Token;
class Stmt;
}

namespace cling {

//---------------------------------------------------------------------------
//! Class for managing many translation units supporting automatic
//! forward declarations and linking
//---------------------------------------------------------------------------
class Interpreter {
public:

   //---------------------------------------------------------------------
   //! Mark the type of statement found in the command line
   //---------------------------------------------------------------------
   enum InputType { Incomplete, TopLevel, Stmt };

   //---------------------------------------------------------------------
   //! Constructor
   //---------------------------------------------------------------------
   Interpreter(const char* llvmdir = 0);

   //---------------------------------------------------------------------
   //! Destructor
   //---------------------------------------------------------------------
   virtual ~Interpreter();

   InputType analyzeInput(const std::string& contextSource,
                          const std::string& line,
                          int& indentLevel,
                          std::vector<clang::FunctionDecl*> *fds);

   void processLine(const std::string& input_line);
   int loadFile(const std::string& filename,
                const std::string* trailcode = 0);
   int executeFile(const std::string& filename);

   bool setPrintAST(bool print = true) {
      bool prev = m_printAST;
      m_printAST = print;
      return prev;
   }

   clang::CompilerInstance* getCI();

private:
   std::string m_globalDeclarations; // All global var decls seen.
   llvm::LLVMContext* m_llvm_context; // We own, our types.
   clang::CompilerInstance* m_CI; // We own, our compiler instance.
   llvm::ExecutionEngine* m_engine; // We own, our JIT.
   llvm::Module* m_prev_module; // We do *not* own, m_engine owns it.
   unsigned long long m_numCallWrappers; // number of generated call wrappers
   bool m_printAST; // whether to print the AST to be processed

private:

   int analyzeTokens(clang::Preprocessor& PP, clang::Token& lastTok,
                     int& indentLevel, bool& tokWasDo);

   llvm::Module* makeModuleFromCommandLine(const std::string& input_line);
   void createWrappedSrc(const std::string& src, std::string& wrapped);

   clang::CompilerInstance* createStatementList(const std::string& srcCode,
         std::vector<clang::Stmt*>& stmts);

   clang::CompilerInstance* createCI(const char* llvmdir = 0);
   clang::ASTConsumer* maybeGenerateASTPrinter() const;
   clang::CompilerInstance* compileString(const std::string& srcCode);
   clang::CompilerInstance* compileFile(const std::string& filename,
                                        const std::string* trailcode = 0);

   llvm::Module* doCodegen(clang::CompilerInstance* CI,
                           const std::string& filename);

   int verifyModule(llvm::Module* m);
   void printModule(llvm::Module* m);
   void executeCommandLine();
   void executeFunction(const std::string& funcname);

   llvm::sys::Path findDynamicLibrary(const std::string& filename,
                                      bool addPrefix = true,
                                      bool addSuffix = true) const;

};

} // namespace cling

#endif // CLING_COMPILER_H
