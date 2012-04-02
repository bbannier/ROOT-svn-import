// vim: ts=2 sw=2 foldmethod=indent
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclGroup.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclBase.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclFriend.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/ExprCXX.h"
#include "clang/AST/Expr.h"
#include "clang/AST/PrettyPrinter.h"
#include "clang/AST/Stmt.h"
#include "clang/AST/Type.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/IdentifierTable.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/Specifiers.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/TokenKinds.h"
#include "clang/Basic/Version.h"
#include "clang/CodeGen/ModuleBuilder.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/TextDiagnosticBuffer.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Lex/HeaderSearch.h"
#include "clang/Lex/MacroInfo.h"
#include "clang/Lex/PPCallbacks.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Parse/ParseAST.h"
#define private public
#include "clang/Parse/Parser.h"
#undef public
#include "clang/Sema/DeclSpec.h"
#include "clang/Sema/Sema.h"
#include "clang/Sema/Lookup.h"
#include "clang/Sema/Ownership.h"
#include "clang/Sema/Scope.h"
#include "llvm/ADT/OwningPtr.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/Assembly/PrintModulePass.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/GlobalVariable.h"
#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
#include "llvm/PassManager.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/DynamicLibrary.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetSelect.h"

#include <cstdio>
#include <iostream>
#include <sstream>
#include <stack>
#include <string>
#include <utility>
#include <vector>

// Function Table
void init(const char* llvmdir);
void cleanup();
clang::CompilerInstance* createCI(const char* llvmdir);
clang::CompilerInstance* getCI();

// Local Variables
static const char* llvm_install_dir = "/local2/russo/llvm";
//static const char* fake_argv[] = { "clang", "-x", "c++", "-fexceptions", "-D__CLING__", "-isysroot", "/local2/russo/root_debug/include", 0 };
static const char* fake_argv[] = { "clang", "-x", "c++", "-fexceptions", "-D__CLING__", 0 };
static const int fake_argc = (sizeof(fake_argv) / sizeof(const char*)) - 1;

static llvm::LLVMContext* m_llvm_context = 0; // We own, our types.
static clang::CompilerInstance* m_CI = 0; // We own, our compiler instance.
//static llvm::Module* m_prev_module = 0; // We do *not* own, m_engine owns it.

//
// Functions
//

void init(const char* llvmdir)
{
  //
  //  Initialize the llvm library.
  //
  llvm::InitializeAllTargets();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmPrinters();
  llvm::InitializeAllAsmParsers();
  //
  //  Create a llvm context to use.
  //
  //m_llvm_context = &llvm::getGlobalContext();
  m_llvm_context = new llvm::LLVMContext;
  m_CI = createCI(llvmdir);
  //m_prev_module = new llvm::Module("_Clang_first", *m_llvm_context);
}

void cleanup()
{
  //delete m_prev_module;
  //m_prev_module = 0; // Don't do this, the engine does it.
  //m_CI->takeLLVMContext(); // Don't take down the context with the CI.
  delete m_CI;
  m_CI = 0;
  delete m_llvm_context;
  m_llvm_context = 0;
  // Shutdown the llvm library.
  llvm::llvm_shutdown();
}

clang::CompilerInstance*
createCI(const char* llvmdir)
{
  //
  //  Create and setup a compiler instance.
  //
  clang::CompilerInstance* CI = new clang::CompilerInstance();
  llvm::IntrusiveRefCntPtr<clang::DiagnosticIDs> DiagID(
    new clang::DiagnosticIDs());
  //bool first_time = true;
  //CI->setLLVMContext(m_llvm_context);
  {
    //
    //  Buffer the error messages while we process
    //  the compiler options.
    //
    clang::TextDiagnosticBuffer* DiagsBuffer = new clang::TextDiagnosticBuffer();
    // Diags takes ownership of DiagsBuffer
    clang::DiagnosticsEngine Diags(DiagID, DiagsBuffer);
    clang::CompilerInvocation::CreateFromArgs(CI->getInvocation(),
        fake_argv + 1, fake_argv + fake_argc, Diags);
    if (
      CI->getHeaderSearchOpts().UseBuiltinIncludes &&
      CI->getHeaderSearchOpts().ResourceDir.empty()
    ) {
      if (llvmdir) {
        llvm::sys::Path P(llvmdir);
        P.appendComponent("lib");
        P.appendComponent("clang");
        P.appendComponent(CLANG_VERSION_STRING);
        CI->getHeaderSearchOpts().ResourceDir = P.str();
      }
    }
    CI->createDiagnostics(fake_argc - 1, const_cast<char**>(fake_argv + 1));
    if (!CI->hasDiagnostics()) {
      //CI->takeLLVMContext();
      delete CI;
      CI = 0;
      return 0;
    }
    // Output the buffered error messages now.
    DiagsBuffer->FlushDiagnostics(CI->getDiagnostics());
    if (CI->getDiagnostics().getClient()->getNumErrors()) {
      //CI->takeLLVMContext();
      delete CI;
      CI = 0;
      return 0;
    }
  }
  CI->setTarget(clang::TargetInfo::CreateTargetInfo(CI->getDiagnostics(),
                CI->getTargetOpts()));
  if (!CI->hasTarget()) {
    //CI->takeLLVMContext();
    delete CI;
    CI = 0;
    return 0;
  }
  CI->getTarget().setForcedLangOptions(CI->getLangOpts());
  if (!CI->hasFileManager()) {
    CI->createFileManager();
  }
  if (!CI->hasSourceManager()) {
    CI->createSourceManager(CI->getFileManager());
  }
  //else {
  //  CI->getSourceManager().clearIDTables();
  //}
  //CI->createPreprocessor(); // Note: This single line takes almost all the time!
  return CI;
}

clang::CompilerInstance*
getCI()
{
  if (!m_CI) {
    return 0;
  }
  //m_CI->getSourceManager().clearIDTables();
  return m_CI;
}

int main(int argc, char** argv)
{
  init(llvm_install_dir);
  clang::CompilerInstance* CI = 0;
  CI = getCI();
  if (!CI) {
    return 0;
  }
  CI->createPreprocessor();
  clang::Preprocessor& PP = CI->getPreprocessor();
  CI->getDiagnosticClient().BeginSourceFile(CI->getLangOpts(), &PP);
  CI->createASTContext();
  //clang::ASTContext& Context = CI->getASTContext();
  //CI->getASTContext().setASTMutationListern(Consumer.GetASTMutationListener();
  CI->setASTConsumer(new clang::ASTConsumer());
  clang::ASTConsumer& Consumer = CI->getASTConsumer();
  //PP.getBuiltinInfo().InitializeBuiltins(PP.getIdentifierTable(),
  //  PP.getLangOptions());
  CI->createPCHExternalASTSource("it3.h.pch", true, false, 0);
  llvm::MemoryBuffer* SB = llvm::MemoryBuffer::getMemBufferCopy(argv[1],
    "lookup.name.file");
  const clang::FileEntry* File = CI->getFileManager().getVirtualFile(
    SB->getBufferIdentifier(), SB->getBufferSize(), 0);
  CI->getSourceManager().createMainFileID(File);
  CI->getSourceManager().overrideFileContents(File, SB);
  if (!CI->hasSema()) {
    CI->createSema(clang::TU_Complete, 0);
  }
  clang::Sema& Sema = CI->getSema();
  clang::Parser* P = new clang::Parser(PP, Sema);
  PP.EnterMainSourceFile();
  P->Initialize();
  Sema.Initialize();
  clang::Scope* TUScope = Sema.getCurScope();
  if (clang::ExternalASTSource*
    External = Sema.getASTContext().getExternalSource()) {
    External->StartTranslationUnit(&Consumer);
  }
  clang::TranslationUnitDecl* TU =
    CI->getASTContext().getTranslationUnitDecl();
  if (!TU) {
    fprintf(
      stderr
      , "astread: No translation unit decl in passed "
      "PCH file!\n"
    );
    return 0;
  }
  //TU->dump();
  clang::ExprResult Result(P->ParseExpression());
#if 0
  clang::PrintingPolicy Policy(CI->getASTContext().getPrintingPolicy());
  Policy.SuppressTagKeyword = true;
  //Policy.SuppressTag = true;
  Policy.SuppressUnwrittenScope = true;
  Policy.SuppressInitializers = true;
  Policy.AnonymousTagLocations = false;
  std::string proto;
  {
     bool first_time = true;
     while (P->getCurToken().isNot(clang::tok::eof)) {
        clang::TypeResult Res(P->ParseTypeName());
        if (Res.isUsable()) {
           clang::QualType QT(Res.get().get());
           if (first_time) {
              first_time = false;
           }
           else {
              proto += ',';
           }
           proto += QT.getCanonicalType().getAsString(Policy);
           fprintf(stderr, "%s\n", proto.c_str());
        }
        if (!P->getCurToken().is(clang::tok::comma)) {
           break;
        }
        P->ConsumeToken();
     }
  }
#endif // 0
  //
  //  All done.
  //
  cleanup();
  return 0;
}

