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

#include "c4_tools.h"

#include <cstdio>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include <limits.h>
#include <stdint.h>

//----------------------------------------------------------------------------

llvm::sys::Path
GetExecutablePath(const char* Argv0, bool CanonicalPrefixes)
{
  if (!CanonicalPrefixes) {
    return llvm::sys::Path(Argv0);
  }

  // This just needs to be some symbol in the binary; C++ doesn't
  // allow taking the address of ::main however.

  void* P = (void*) (intptr_t) GetExecutablePath;
  return llvm::sys::Path::GetMainExecutable(Argv0, P);
}

//----------------------------------------------------------------------------

void
LLVMErrorHandler(void* UserData, const std::string& Message)
{
  clang::Diagnostic& Diags = *static_cast<clang::Diagnostic*>(UserData);
  Diags.Report(clang::diag::err_fe_error_backend) << Message;
  exit(1);
}

//----------------------------------------------------------------------------

static
std::pair<unsigned, unsigned>
getStmtRange(const clang::Stmt* S,
  const clang::SourceManager& SM,
  const clang::LangOptions& LO)
{
  // Get the source range of the specified Stmt.
  clang::SourceLocation SLoc = SM.getInstantiationLoc(S->getLocStart());
  clang::SourceLocation ELoc = SM.getInstantiationLoc(S->getLocEnd());
  // This is necessary to get the correct range of function-like macros.
  if ((SLoc == ELoc) && S->getLocEnd().isMacroID()) {
    ELoc = SM.getInstantiationRange(S->getLocEnd()).second;
  }
  if (!SLoc.isValid() && !ELoc.isValid()) {
    return std::pair<unsigned, unsigned>(0, 0);
  }
  unsigned start = 0;
  unsigned end = 0;
  if (SLoc.isValid()) {
    start = SM.getFileOffset(SLoc);
  }
  if (ELoc.isValid()) {
    end = SM.getFileOffset(ELoc) +
      clang::Lexer::MeasureTokenLength(ELoc, SM, LO);
  }
  else if (SLoc.isValid()) {
    clang::SourceLocation Loc = SM.getInstantiationLoc(SLoc);
    std::pair<clang::FileID, unsigned> LocInfo = SM.getDecomposedLoc(Loc);
    std::pair<const char*, const char*> BD = SM.getBufferData(LocInfo.first);
    end = BD.second - BD.first;
  }
  return std::pair<unsigned, unsigned>(start, end);
}

//----------------------------------------------------------------------------

static
std::pair<unsigned, unsigned>
getRangeWithSemicolon(clang::SourceLocation SLoc,
  clang::SourceLocation ELoc,
  const clang::SourceManager& SM,
  const clang::LangOptions& LO)
{
  unsigned start = 0;
  unsigned end = 0;
  if (SLoc.isValid()) {
    start = SM.getFileOffset(SLoc);
  }
  if (ELoc.isValid()) {
    end = SM.getFileOffset(ELoc);
  }
  if (SLoc.isValid() && !ELoc.isValid()) {
    clang::SourceLocation Loc = SM.getInstantiationLoc(SLoc);
    std::pair<clang::FileID, unsigned> LocInfo = SM.getDecomposedLoc(Loc);
    std::pair<const char*, const char*> BD = SM.getBufferData(LocInfo.first);
    end = BD.second - BD.first;
  }
  else if (ELoc.isValid()) {
    clang::SourceLocation Loc = SM.getInstantiationLoc(ELoc);
    std::pair<clang::FileID, unsigned> LocInfo = SM.getDecomposedLoc(Loc);
    std::pair<const char*, const char*> BD = SM.getBufferData(LocInfo.first);
    const char* StrData = BD.first + LocInfo.second;
    clang::Lexer TheLexer(Loc, LO, BD.first, StrData, BD.second);
    clang::Token TheTok;
    TheLexer.LexFromRawLexer(TheTok);
    end += TheTok.getLength();
    // Check if we the source range did include the semicolon.
    if (TheTok.isNot(clang::tok::semi) && TheTok.isNot(clang::tok::r_brace)) {
      TheLexer.LexFromRawLexer(TheTok);
      if (TheTok.is(clang::tok::semi)) {
        end += TheTok.getLength();
      }
    }
  }
  return std::pair<unsigned, unsigned>(start, end);
}

//----------------------------------------------------------------------------

static
std::pair<unsigned, unsigned>
getStmtRangeWithSemicolon(const clang::Stmt* S,
  const clang::SourceManager& SM,
  const clang::LangOptions& LO)
{
  // Get the source range of the specified Stmt, ensuring that a semicolon is
  // included, if necessary - since the clang ranges do not guarantee this.
  clang::SourceLocation SLoc = SM.getInstantiationLoc(S->getLocStart());
  clang::SourceLocation ELoc = SM.getInstantiationLoc(S->getLocEnd());
  return getRangeWithSemicolon(SLoc, ELoc, SM, LO);
}
   
//----------------------------------------------------------------------------

static std::string m_globalDeclarations = "#include <stdio.h>\n";

static llvm::LLVMContext* m_llvm_context = 0; // We own.
static llvm::ExecutionEngine* m_engine = 0; // We own.
static llvm::Module* m_prev_module = 0; // We do *not* own.
static const char* fake_argv[] = { "clang", "-x", "c++", 0 };
static const int fake_argc = (sizeof(fake_argv) / sizeof(const char*)) - 1;

static
void processLine(const std::string& input_line)
{
  //
  //  Setup a compiler instance to work with.
  //
  clang::CompilerInstance CI;
  bool first_time = true;
  CI.setLLVMContext(m_llvm_context);
  {
    clang::TextDiagnosticBuffer DiagsBuffer;
    clang::Diagnostic Diags(&DiagsBuffer);
    clang::CompilerInvocation::CreateFromArgs(CI.getInvocation(),
      fake_argv + 1, fake_argv + fake_argc, Diags);
    if (
      CI.getHeaderSearchOpts().UseBuiltinIncludes &&
      CI.getHeaderSearchOpts().ResourceDir.empty()
    ) {
      //CI.getHeaderSearchOpts().ResourceDir =
      //  clang::CompilerInvocation::GetResourcesPath(argv[0],
      //    (void*) (intptr_t) GetExecutablePath);
      CI.getHeaderSearchOpts().ResourceDir =
        llvm::sys::Path("/local2/russo/llvm/lib/clang/1.1").str();
    }
    CI.createDiagnostics(fake_argc - 1, const_cast<char**>(fake_argv + 1));
    if (!CI.hasDiagnostics()) {
      CI.takeLLVMContext();
      return;
    }
    DiagsBuffer.FlushDiagnostics(CI.getDiagnostics());
    bool Success = false;
    if (CI.getDiagnostics().getNumErrors()) {
      CI.takeLLVMContext();
      return;
    }
  }
  CI.setTarget(clang::TargetInfo::CreateTargetInfo(CI.getDiagnostics(),
    CI.getTargetOpts()));
  if (!CI.hasTarget()) {
    CI.takeLLVMContext();
    return;
  }
  CI.getTarget().setForcedLangOptions(CI.getLangOpts());
  CI.createFileManager();
  //
  //  Check to see if input is a preprocessor directive.
  //
  std::string::size_type posHash = input_line.find_first_not_of(' ');
  if ((posHash != std::string::npos) && (input_line[posHash] != '#')) {
    posHash = std::string::npos;
  }
  //
  //  Run the input through the frontend.
  //
  if (posHash != std::string::npos) { // input is a preproc directive, do & ret
    // --
#if 0
    if (first_time) {
      CI.createSourceManager();
      first_time = false;
    }
    else {
      CI.getSourceManager().clearIDTables();
    }
    CI.createPreprocessor();
    clang::Preprocessor& PP = CI.getPreprocessor();
    CI.getDiagnosticClient().BeginSourceFile(CI.getLangOpts(), &PP);
    CI.createASTContext();
    //llvm::raw_stdout_ostream out;
    //clang::ASTConsumer* dummyConsumer = clang::CreateASTPrinter(&out);
    CI.setASTConsumer(new ASTConsumer());
    PP.getBuiltinInfo().InitializeBuiltins(PP.getIdentifierTable(),
      PP.getLangOptions().NoBuiltin);
    llvm::MemoryBuffer* SB = llvm::MemoryBuffer::getMemBufferCopy(
      &*input_line.begin(), &*input_line.end(), "CLING");
    if (!SB) {
      // FIXME: We need our own error code.
      CI.getDiagnostics().Report(clang::diag::err_fe_error_reading)
        << "could not create memory buffer";
      CI.takeLLVMContext();
      return;
    }
    CI.getSourceManager().createMainFileIDForMemBuffer(SB);
    if (CI.getSourceManager().getMainFileID().isInvalid()) {
      // FIXME: We need our own error code.
      CI.getDiagnostics().Report(clang::diag::err_fe_error_reading)
        << "<input string>";
      CI.takeLLVMContext();
      return;
    }
    clang::ParseAST(PP, &CI.getASTConsumer(), CI.getASTContext());
    //clang::Sema sema(PP, CI.getASTContext(), CI.getASTConsumer());
    //clang::Parser P(PP, sema);
    //PP.EnterMainSourceFile();
    //P.ParseTranslationUnit();
    CI.setASTConsumer(0);
    CI.setASTContext(0);
    CI.clearOutputFiles(/*EraseFiles=*/CI.getDiagnostics().getNumErrors());
    CI.getDiagnosticClient().EndSourceFile();
    unsigned err_count = CI.getDiagnostics().getNumErrors();
#endif // 0
    m_globalDeclarations.append(input_line);
    m_globalDeclarations.append("\n");
    CI.takeLLVMContext();
    return;
  }
  //
  //  Wrap input into a function along with the saved global declarations.
  //
  std::vector<clang::Stmt*> stmts;
  std::string src(m_globalDeclarations);
  {
    src += "void __cling_internal() {\n";
    src += input_line;
    src += "\n} // end __cling_internal()\n";
    fprintf(stderr, "input_line:\n%s\n", src.c_str());
    if (first_time) {
      CI.createSourceManager();
      first_time = false;
    }
    else {
      CI.getSourceManager().clearIDTables();
    }
    CI.createPreprocessor();
    clang::Preprocessor& PP = CI.getPreprocessor();
    PP.addPPCallbacks(new MacroDetector(CI, m_globalDeclarations.size()));
    CI.getDiagnosticClient().BeginSourceFile(CI.getLangOpts(), &PP);
    CI.createASTContext();
    // Create an ASTConsumer for this frontend run which
    // will produce a list of statements seen.
    StmtSplitter splitter(stmts);
    FunctionBodyConsumer* consumer =
      new FunctionBodyConsumer(splitter, "__cling_internal");
    CI.setASTConsumer(consumer);
    PP.getBuiltinInfo().InitializeBuiltins(PP.getIdentifierTable(),
      PP.getLangOptions().NoBuiltin);
    // Make our input the main source file for the frontend.
    {
      llvm::MemoryBuffer* SB = llvm::MemoryBuffer::getMemBufferCopy(
        &*src.begin(), &*src.end(), "CLING");
      if (!SB) {
        CI.getDiagnostics().Report(clang::diag::err_fe_error_reading)
          << "could not create memory buffer";
        CI.takeLLVMContext();
        return;
      }
      CI.getSourceManager().createMainFileIDForMemBuffer(SB);
      if (CI.getSourceManager().getMainFileID().isInvalid()) {
        CI.getDiagnostics().Report(clang::diag::err_fe_error_reading)
          << "<input string>";
        CI.takeLLVMContext();
        return;
      }
    }
    // Run the frontend to make the statement list.
    std::fprintf(stderr, "Parsing to make statement list\n");
    clang::ParseAST(PP, &CI.getASTConsumer(), CI.getASTContext());
    std::fprintf(stderr, "Statement list parse finished.\n");
    // Exit if error.
    if (CI.getDiagnostics().hasErrorOccurred()) {
      CI.setASTConsumer(0);
      CI.setASTContext(0);
      CI.clearOutputFiles(/*EraseFiles=*/CI.getDiagnostics().getNumErrors());
      CI.getDiagnosticClient().EndSourceFile();
      CI.takeLLVMContext();
      return;
    }
  }
  //
  //  Pull the global declarations out of the statement list, and
  //  wrap each individual statment into a function and create an
  //  initialized static global variable to call that function.
  //
  std::string held_globals;
  std::string wrapped_globals;
  std::string wrapped_stmts;
  {
    clang::SourceManager& SM = CI.getSourceManager();
    const clang::LangOptions& LO = CI.getLangOpts();
    wrapped_stmts += "void __cling_internal() {\n";
    std::vector<clang::Stmt*>::iterator stmt_iter = stmts.begin();
    std::vector<clang::Stmt*>::iterator stmt_end = stmts.end();
    for (; stmt_iter != stmt_end; ++stmt_iter) {
      clang::Stmt* cur_stmt = *stmt_iter;
      std::string stmt_string;
      {
        std::pair<unsigned, unsigned> r =
          getStmtRangeWithSemicolon(cur_stmt, SM, LO);
        stmt_string = src.substr(r.first, r.second - r.first);
        std::fprintf(stderr, "stmt: %s\n", stmt_string.c_str());
      }
      //
      //  Handle expression statements.
      //
      {
        const clang::Expr* expr = dyn_cast<clang::Expr>(cur_stmt);
        if (expr) {
          std::fprintf(stderr, "have expr stmt.\n");
          wrapped_stmts.append(stmt_string + '\n');
          continue;
        }
      }
      //
      //  Handle everything that is not a declaration statement.
      //
      const clang::DeclStmt* DS = dyn_cast<clang::DeclStmt>(cur_stmt);
      if (!DS) {
        std::fprintf(stderr, "not expr, not declaration.\n");
        wrapped_stmts.append(stmt_string + '\n');
        continue;
      }
      //
      //  Loop over each declarator in the declaration statement.
      //
      clang::DeclStmt::const_decl_iterator D = DS->decl_begin();
      clang::DeclStmt::const_decl_iterator E = DS->decl_end();
      for (; D != E; ++D) {
        //
        //  Handle everything that is not a variable declarator.
        //
        const clang::VarDecl* VD = dyn_cast<clang::VarDecl>(*D);
        if (!VD) {
          std::fprintf(stderr, "decl, not var decl.\n");
          clang::SourceLocation SLoc =
            SM.getInstantiationLoc((*D)->getLocStart());
          clang::SourceLocation ELoc =
            SM.getInstantiationLoc((*D)->getLocEnd());
          std::pair<unsigned, unsigned> r =
            getRangeWithSemicolon(SLoc, ELoc, SM, LO);
          std::string decl = src.substr(r.first, r.second - r.first);
          wrapped_globals.append(decl + ";\n");
          held_globals.append(decl + ";\n");
          continue;
        }
        //
        //  Handle a variable declarator.
        //
        std::string decl = VD->getNameAsCString();
        // FIXME: Probably should not remove the qualifiers!
        VD->getType().getUnqualifiedType().
          getAsStringInternal(decl, clang::PrintingPolicy(LO));
        const clang::Expr* I = VD->getInit();
        //
        //  Handle variable declarators with no initializer
        //  or with an initializer that is a constructor call.
        //
        if (!I || dyn_cast<clang::CXXConstructExpr>(I)) {
          if (!I) {
            std::fprintf(stderr, "var decl, no init.\n");
          }
          else {
            std::fprintf(stderr, "var decl, init is constructor.\n");
          }
          wrapped_globals.append(decl + ";\n"); // FIXME: wrong for constructor
          held_globals.append(decl + ";\n");
          continue;
        }
        //
        //  Handle variable declarators with a constant initializer.
        //
        if (I->isConstantInitializer(CI.getASTContext())) {
          std::fprintf(stderr, "var decl, init is const.\n");
          std::pair<unsigned, unsigned> r = getStmtRange(I, SM, LO);
          wrapped_globals.append(decl + " = " +
            src.substr(r.first, r.second - r.first) + ";\n");
          held_globals.append(decl + ";\n");
          continue;
        }
        //
        //  Handle variable declarators whose initializer is not a list.
        //
        const clang::InitListExpr* ILE = dyn_cast<clang::InitListExpr>(I);
        if (!ILE) {
          std::fprintf(stderr, "var decl, init is not list.\n");
          std::pair<unsigned, unsigned> r = getStmtRange(I, SM, LO);
          wrapped_stmts.append(std::string(VD->getName())  + " = " +
            src.substr(r.first, r.second - r.first) + ";\n");
          wrapped_globals.append(decl + ";\n");
          held_globals.append(decl + ";\n");
          continue;
        }
        //
        //  Handle variable declarators with an initializer list.
        //
        std::fprintf(stderr, "var decl, init is list.\n");
        unsigned numInits = ILE->getNumInits();
        for (unsigned j = 0; j < numInits; ++j) {
          std::string stmt;
          llvm::raw_string_ostream stm(stmt);
          stm << VD->getNameAsCString() << "[" << j << "] = ";
          std::pair<unsigned, unsigned> r =
            getStmtRange(ILE->getInit(j), SM, LO);
          stm << src.substr(r.first, r.second - r.first) << ";\n";
          wrapped_stmts.append(stm.str());
        }
        wrapped_globals.append(decl + ";\n");
        held_globals.append(decl + ";\n");
      }
    }
    wrapped_stmts += "\n} // end __cling_internal()\n";
  }
  //
  std::fprintf(stderr, "m_globalDeclarations:\n%s\n",
    m_globalDeclarations.c_str());
  std::fprintf(stderr, "held_globals:\n%s\n", held_globals.c_str());
  std::fprintf(stderr, "---\n");
  std::fprintf(stderr, "wrapped_globals:\n%s\n", wrapped_globals.c_str());
  std::fprintf(stderr, "wrapped_stmts:\n%s\n", wrapped_stmts.c_str());
  std::string wrapped;
  wrapped += m_globalDeclarations + wrapped_globals + wrapped_stmts;
  // Accumulate the held global declarations for the next run.
  m_globalDeclarations.append(held_globals + "\n");
  //
  //  Shutdown parse.
  //
  CI.setASTConsumer(0);
  CI.setASTContext(0);
  CI.clearOutputFiles(/*EraseFiles=*/CI.getDiagnostics().getNumErrors());
  CI.getDiagnosticClient().EndSourceFile();
  //
  //  Now send the wrapped code through the
  //  frontend to produce a translation unit.
  //
  clang::TranslationUnitDecl* tu = 0;
  {
    if (first_time) {
      CI.createSourceManager();
      first_time = false;
    }
    else {
      CI.getSourceManager().clearIDTables();
    }
    CI.createPreprocessor();
    clang::Preprocessor& PP = CI.getPreprocessor();
    CI.getDiagnosticClient().BeginSourceFile(CI.getLangOpts(), &PP);
    CI.createASTContext();
    //llvm::raw_stdout_ostream out;
    //clang::ASTConsumer* dummyConsumer = clang::CreateASTPrinter(&out);
    CI.setASTConsumer(new clang::ASTConsumer());
    PP.getBuiltinInfo().InitializeBuiltins(PP.getIdentifierTable(),
      PP.getLangOptions().NoBuiltin);
    llvm::MemoryBuffer* SB = llvm::MemoryBuffer::getMemBufferCopy(
      &*wrapped.begin(), &*wrapped.end(), "CLING");
    if (!SB) {
      CI.getDiagnostics().Report(clang::diag::err_fe_error_reading)
        << "could not create memory buffer";
      CI.takeLLVMContext();
      return;
    }
    CI.getSourceManager().createMainFileIDForMemBuffer(SB);
    if (CI.getSourceManager().getMainFileID().isInvalid()) {
      CI.getDiagnostics().Report(clang::diag::err_fe_error_reading)
        << "<input string>";
      CI.takeLLVMContext();
      return;
    }
    std::fprintf(stderr, "Parsing wrapped code to make translation unit.\n");
    clang::ParseAST(PP, &CI.getASTConsumer(), CI.getASTContext());
    std::fprintf(stderr, "Finished parsing wrapped code.\n");
    CI.clearOutputFiles(/*EraseFiles=*/CI.getDiagnostics().getNumErrors());
    CI.getDiagnosticClient().EndSourceFile();
    unsigned err_count = CI.getDiagnostics().getNumErrors();
    if (err_count) {
      CI.takeLLVMContext();
      return;
    }
    tu = CI.getASTContext().getTranslationUnitDecl();
    if (!tu) { // Parse failed, return.
      CI.takeLLVMContext();
      return;
    }
  }
  //
  //  Run the wrapped code through the llvm code generator to make a module.
  //
  llvm::Module* m = 0;
  {
    llvm::OwningPtr<clang::CodeGenerator> codeGen(
      CreateLLVMCodeGen(CI.getDiagnostics(), "<FAKE>", CI.getCodeGenOpts(),
      CI.getLLVMContext()));
    codeGen->Initialize(CI.getASTContext());
    clang::TranslationUnitDecl::decl_iterator iter = tu->decls_begin();
    clang::TranslationUnitDecl::decl_iterator iter_end = tu->decls_end();
    std::fprintf(stderr, "Running code generation.\n");
    for (; iter != iter_end; ++iter) {
      codeGen->HandleTopLevelDecl(clang::DeclGroupRef(*iter));
    }
    codeGen->HandleTranslationUnit(CI.getASTContext());
    std::fprintf(stderr, "Finished code generation.\n");
    m = codeGen->ReleaseModule();
    if (!m) {
      std::fprintf(stderr, "Error: Backend did not create a module!\n");
      CI.takeLLVMContext();
      return;
    }
  }
  //
  //  Verify generated module.
  //
  bool mod_has_errs = llvm::verifyModule(*m, llvm::PrintMessageAction);
  if (mod_has_errs) {
    CI.takeLLVMContext();
    return;
  }
  //
  //  Dump generated module.
  //
  //--llvm::PassManager PM;
  //--PM.add(llvm::createPrintModulePass(&llvm::outs()));
  //--PM.run(*m);
  //
  //  Transfer global mappings from previous module.
  //
  {
    llvm::Module::global_iterator iter = m->global_begin();
    llvm::Module::global_iterator end = m->global_end();
    for (; iter != end; ++iter) {
      fprintf(stderr, "Current module has global: %s\n",
        iter->getName().data());
      //if (iter->isDeclaration()) {
        fprintf(stderr, "Search previous module for global var: %s\n",
          iter->getName().data());
        llvm::GlobalVariable* gv =
          m_prev_module->getGlobalVariable(iter->getName());
        if (!gv) { // no such global in prev module
          continue; // skip it
        }
        // FIXME: Need to compare types here!
        // Get the mapping of the var in the prev module.
        void* p = m_engine->getPointerToGlobal(gv);
        fprintf(stderr, "Setting mapping for: %s to %x\n",
          iter->getName().data(), (unsigned long) p);
        // And duplicate it for the new module.
        m_engine->addGlobalMapping(&*iter, p);
      //}
    }
  }
  //
  //  All done with previous module, delete it.
  //
  {
     bool ok = m_engine->removeModule(m_prev_module);
     if (!ok) {
       fprintf(stderr, "Previous module not found in execution engine!\n");
     }
  }
  delete m_prev_module;
  m_prev_module = 0;
  //
  //  Give new module to the execution engine.
  //
  m_engine->addModule(m); // Note: The engine takes ownership of the module.
  //
  //  Run it using the JIT.
  //
  {
    std::fprintf(stderr, "Running generated code with JIT.\n");
    //
    //  Run global initialization.
    //
    m_engine->runStaticConstructorsDestructors(false);
    //
    //  Run the function __cling_internal().
    //
    // Create argument list for function.
    std::vector<llvm::GenericValue> args;
    //llvm::GenericValue arg1;
    //arg1.IntVal = llvm::APInt(32, 5);
    //args.push_back(arg1);
    llvm::Function* f = m_engine->FindFunctionNamed("_Z16__cling_internalv");
    llvm::GenericValue ret = m_engine->runFunction(f, args);
    //
    std::fprintf(stderr, "Finished running generated code with JIT.\n");
    //
    // Print the result.
    //llvm::outs() << "Result: " << ret.IntVal << "\n";
    // Run global destruction.
    //m_engine->runStaticConstructorsDestructors(true);
    m_engine->freeMachineCodeForFunction(f);
  }
  //
  //  All done, save module to transfer mappings
  //  on the next run.
  //
  m_prev_module = m;
  //
  //  Prevent the destruction of our context, we need to
  //  reuse it because it has all our types.
  //
  CI.takeLLVMContext();
}

int main(int argc, const char** argv)
{
  //
  //  Initialize the llvm library.
  //
  llvm::InitializeAllTargets();
  llvm::InitializeAllAsmPrinters();
  //
  //  Create an execution engine to use.
  //
  //m_llvm_context = &llvm::getGlobalContext();
  m_llvm_context = new llvm::LLVMContext;
  m_prev_module = new llvm::Module("first", *m_llvm_context);
  if (!m_prev_module) {
    std::fprintf(stderr, "Error: Unable to create the first module!\n");
    return 1;
  }
  std::string errMsg;
  {
    // Note: Engine takes ownership of the module.
    llvm::EngineBuilder builder(m_prev_module);
    builder.setErrorStr(&errMsg);
    builder.setEngineKind(llvm::EngineKind::JIT);
    m_engine = builder.create();
    if (!m_engine) {
      std::fprintf(stderr, "Error: Unable to create the execution engine!\n");
      std::fprintf(stderr, "%s\n", errMsg.c_str());
      return 1;
    }
  }
  //
  //  Take input and handle it.
  //
  std::string input_line;
  while (1) {
    std::getline(std::cin, input_line);
    processLine(input_line);
    input_line.clear();
  }
  llvm::llvm_shutdown();
  return 0;
}

