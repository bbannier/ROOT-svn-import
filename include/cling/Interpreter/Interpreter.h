//--------------------------------------------------------------------*- C++ -*-
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Lukasz Janyst <ljanyst@cern.ch>
//------------------------------------------------------------------------------

#ifndef CLING_COMPILER_H
#define CLING_COMPILER_H

#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ADT/OwningPtr.h"

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

class CIBuilder;
class ExecutionContext;

//---------------------------------------------------------------------------
//! Class for managing many translation units supporting automatic
//! forward declarations and linking
//---------------------------------------------------------------------------
class Interpreter {
public:

   //---------------------------------------------------------------------
   //! Constructor
   //---------------------------------------------------------------------
   Interpreter(const char* llvmdir = 0);

   //---------------------------------------------------------------------
   //! Destructor
   //---------------------------------------------------------------------
   virtual ~Interpreter();

   int processLine(const std::string& input_line);

   int loadFile(const std::string& filename,
                const std::string* trailcode = 0,
                bool allowSharedLib = true);

   int executeFile(const std::string& filename);


   bool setPrintAST(bool print = true) {
      bool prev = m_printAST;
      m_printAST = print;
      return prev;
   }

   clang::CompilerInstance* getCI() const { return m_CI.get(); }
   clang::CompilerInstance* createCI();
   CIBuilder& getCIBuilder() const { return *m_CIBuilder.get(); }

private:
   llvm::OwningPtr<cling::CIBuilder> m_CIBuilder; // our compiler intsance builder
   llvm::OwningPtr<clang::CompilerInstance> m_CI; // compiler instance.
   llvm::OwningPtr<ExecutionContext> m_ExecutionContext; // compiler instance.
   unsigned long long m_UniqueCounter; // number of generated call wrappers
   bool m_printAST; // whether to print the AST to be processed

private:

   void createWrappedSrc(const std::string& src, std::string& wrapped,
                         bool& haveStatements);

   std::string createUniqueName();

   clang::CompilerInstance* createStatementList(const std::string& srcCode,
         std::vector<clang::Stmt*>& stmts);

   clang::ASTConsumer* maybeGenerateASTPrinter() const;
   clang::CompilerInstance* compileString(const std::string& srcCode);
   clang::CompilerInstance* compileFile(const std::string& filename,
                                        const std::string* trailcode = 0);

};

} // namespace cling

#endif // CLING_COMPILER_H
