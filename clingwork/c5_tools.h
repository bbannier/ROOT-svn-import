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
#include "clang/lib/Sema/Sema.h"
#include "clang/Parse/Parser.h"
#include "clang/Sema/ParseAST.h"
#include "llvm/ADT/OwningPtr.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/Linker.h"
#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
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

#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include <limits.h>
#include <stdint.h>

//----------------------------------------------------------------------------

class MacroDetector : public clang::PPCallbacks
{
private:
  const clang::CompilerInstance& m_CI;
  //const clang::LangOptions& m_options;
  unsigned int m_minpos;
  //clang::SourceManager* m_srcMgr;
  std::vector<std::string> m_macros;

public:

  MacroDetector(const clang::CompilerInstance& CI, unsigned int minpos)
    : clang::PPCallbacks(), m_CI(CI), m_minpos(minpos)
  {
  }

  ~MacroDetector();

  std::vector<std::string>& getMacrosVector()
  {
    return m_macros;
  }

  void MacroDefined(const clang::IdentifierInfo* II,
    const clang::MacroInfo* MI);

  void MacroUndefined(const clang::IdentifierInfo* II,
    const clang::MacroInfo* MI);
};

//----------------------------------------------------------------------------

class StmtSplitter : public clang::StmtVisitor<StmtSplitter>
{
private:

  std::vector<clang::Stmt*>& m_vec_stmt;

public:

  StmtSplitter(std::vector<clang::Stmt*>& vec_stmt)
      : m_vec_stmt(vec_stmt)
  {
  }

  ~StmtSplitter();

  void VisitStmt(clang::Stmt *S)
  {
    m_vec_stmt.push_back(S);
  }

  void VisitDeclStmt(clang::DeclStmt *S)
  {
    m_vec_stmt.push_back(S);
  }

  void VisitChildren(clang::Stmt* S);

};

//----------------------------------------------------------------------------

class FunctionBodyConsumer : public clang::ASTConsumer
{
private:
  StmtSplitter& m_visitor;
  std::string m_funcName;

public:
  FunctionBodyConsumer(StmtSplitter& visitor, const char* funcName);
  ~FunctionBodyConsumer();
  void HandleTopLevelDecl(clang::DeclGroupRef D);
};

