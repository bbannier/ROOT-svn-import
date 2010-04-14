//--------------------------------------------------------------------*- C++ -*-
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Lukasz Janyst <ljanyst@cern.ch>
//------------------------------------------------------------------------------

#ifndef CLING_COMPILER_H
#define CLING_COMPILER_H

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclGroup.h"
#include "clang/AST/Decl.h"
#include "clang/AST/ExprCXX.h"
#include "clang/AST/Expr.h"
#include "clang/AST/Stmt.h"
#include "clang/AST/StmtVisitor.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/IdentifierTable.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/Version.h"
#include "clang/CodeGen/ModuleBuilder.h"
#include "clang/Driver/Arg.h"
#include "clang/Driver/ArgList.h"
#include "clang/Driver/CC1Options.h"
#include "clang/Driver/DriverDiagnostic.h"
#include "clang/Driver/OptTable.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/CompilerInvocation.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/FrontendDiagnostic.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Frontend/HeaderSearchOptions.h"
#include "clang/Frontend/TextDiagnosticBuffer.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Lex/HeaderSearch.h"
#include "clang/Lex/MacroInfo.h"
#include "clang/Lex/PPCallbacks.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Parse/Parser.h"
#include "clang/Sema/ParseAST.h"
#include "llvm/ADT/OwningPtr.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/Assembly/PrintModulePass.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/Function.h"
#include "llvm/GlobalVariable.h"
#include "llvm/Linker.h"
#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
#include "llvm/PassManager.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/System/DynamicLibrary.h"
#include "llvm/System/Path.h"
#include "llvm/System/Process.h"
#include "llvm/System/Signals.h"
#include "llvm/Target/TargetSelect.h"
#include "llvm/Transforms/Utils/Cloning.h"

#include <clang/AST/Type.h>
#include <llvm/ADT/OwningPtr.h>

#include <string>
#include <map>
#include <vector>
#include <utility>

//------------------------------------------------------------------------------
// Forward declarations from LLVM
//------------------------------------------------------------------------------
namespace llvm {
class MemoryBuffer;
class Module;
class LLVMContext;
class StringRef;
}

//------------------------------------------------------------------------------
// Forward declarations from CLANG
//------------------------------------------------------------------------------
namespace clang {
class CompilerInstance;
class TranslationUnitDecl;
class DeclContext;
class Decl;
class SourceManager;
class Sema;
class ASTContext;
class FunctionDecl;
class Preprocessor;
class Token;
class DeclStmt;
class LangOptions;
}

namespace cling {
class ParseEnvironment;

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
   int loadFile(const std::string& filename);
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
   clang::CompilerInstance* compileFile(const std::string& filename);

   llvm::Module* doCodegen(clang::CompilerInstance* CI,
                           const std::string& filename);

   int verifyModule(llvm::Module* m);
   void printModule(llvm::Module* m);
   void executeCommandLine();
   void executeFunction(const std::string& funcname);

};

} // namespace cling

#endif // CLING_COMPILER_H
