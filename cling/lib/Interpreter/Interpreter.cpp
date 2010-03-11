//------------------------------------------------------------------------------
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Lukasz Janyst <ljanyst@cern.ch>
//------------------------------------------------------------------------------

#include "cling/Interpreter/Interpreter.h"

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
#include "clang/Lex/LexDiagnostic.h"
#include "clang/Lex/MacroInfo.h"
#include "clang/Lex/PPCallbacks.h"
#include "clang/Lex/Preprocessor.h"
//#include "clang/lib/Sema/Sema.h"
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

#include "Diagnostics.h"
#include "ParseEnvironment.h"
#include "Visitors.h"
#include "ClangUtils.h"

#include <cstdio>
#include <iostream>
#include <map>
#include <stack>
#include <string>
#include <utility>
#include <vector>

#include <limits.h>
#include <stdint.h>

static const char* fake_argv[] = { "clang", "-x", "c++", 0 };
static const int fake_argc = (sizeof(fake_argv) / sizeof(const char*)) - 1;

#if 0
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
   llvm::Module::global_iterator dst_iter = dst->global_begin();
   llvm::Module::global_iterator dst_end = dst->global_end();
   for (; dst_iter != dst_end; ++dst_iter) {
      // Find the same variable (by name) in the source module.
      llvm::GlobalVariable* src_gv =
         src->getGlobalVariable(dst_iter->getName());
      // Skip it if there is none.
      if (!src_gv) {
         continue;
      }
      // Get the mapping from the execution engine for the source
      // global variable and create a new mapping to the same
      // address for the destination global variable.  Now they
      // share the same allocated memory (and so have the same value).
      // FIXME: We should compare src var and dst var types here!
      void* p = ee->getPointerToGlobal(src_gv);
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

static
void
appendStatement(std::string& processedCode, const std::string& stmt)
{
   static unsigned int exprCount = 0;
   llvm::raw_string_ostream strcode(processedCode);
   // FIXME: The clang code generator does not yet properly handle anonymous namespace
   // (i.e. the initialization are not run if we use the anonimous namespace.
#ifdef CLING_USE_ANONYMOUS_NAMESPACE
   strcode << "namespace {\n   ";
#endif
   strcode << "static int __cling_anon" << exprCount << "() {   ";
   strcode << stmt << " return 0;  };\n";
#ifdef CLING_USE_ANONYMOUS_NAMESPACE
   strcode << "   ";
#endif
   strcode << "int __cling_exec" << exprCount << " = __cling_anon" << exprCount << "();";
#ifdef CLING_USE_ANONYMOUS_NAMESPACE
   strcode << '}';
#endif
   strcode << '\n';
   ++exprCount;
}

static
llvm::sys::Path
GetExecutablePath(const char* Argv0, bool CanonicalPrefixes)
{
   if (!CanonicalPrefixes) {
      return llvm::sys::Path(Argv0);
   }
   // This just needs to be some symbol in the binary; C++ doesn't
   // allow taking the address of ::main however.
   void* P = (void*)(intptr_t) GetExecutablePath;
   return llvm::sys::Path::GetMainExecutable(Argv0, P);
}

} // unnamed namespace
#endif // 0

namespace cling {

//
// MacroDetector
//

class MacroDetector : public clang::PPCallbacks {
private:
   const clang::CompilerInstance& m_CI;
   unsigned int m_minpos;
   std::vector<std::string> m_macros;

public:

   MacroDetector(const clang::CompilerInstance& CI, unsigned int minpos)
         : clang::PPCallbacks(), m_CI(CI), m_minpos(minpos) {
   }

   ~MacroDetector();

   std::vector<std::string>&
   getMacrosVector() {
      return m_macros;
   }

   void
   MacroDefined(const clang::IdentifierInfo* II, const clang::MacroInfo* MI);

   void
   MacroUndefined(const clang::IdentifierInfo* II, const clang::MacroInfo* MI);
};

MacroDetector::~MacroDetector()
{
}

void
MacroDetector::MacroDefined(const clang::IdentifierInfo* II,
                            const clang::MacroInfo* MI)
{
   if (MI->isBuiltinMacro()) {
      return;
   }
   clang::SourceManager& SM = m_CI.getSourceManager();
   clang::FileID mainFileID = SM.getMainFileID();
   if (SM.getFileID(MI->getDefinitionLoc()) != mainFileID) {
      return;
   }
   clang::SourceLocation SLoc = SM.getInstantiationLoc(
                                   MI->getDefinitionLoc());
   clang::SourceLocation ELoc = SM.getInstantiationLoc(
                                   MI->getDefinitionEndLoc());
   unsigned start = SM.getFileOffset(SLoc);
   if (start < m_minpos) {
      return;
   }
   unsigned end = SM.getFileOffset(ELoc);
   const clang::LangOptions& LO = m_CI.getLangOpts();
   end += clang::Lexer::MeasureTokenLength(ELoc, SM, LO);
   std::pair<const char*, const char*> buf = SM.getBufferData(mainFileID);
   std::string str(buf.first + start, end - start);
   m_macros.push_back("#define " + str);
}

void
MacroDetector::MacroUndefined(const clang::IdentifierInfo* II,
                              const clang::MacroInfo* MI)
{
   if (MI->isBuiltinMacro()) {
      return;
   }
   clang::SourceManager& SM = m_CI.getSourceManager();
   clang::FileID mainFileID = SM.getMainFileID();
   if (SM.getFileID(MI->getDefinitionLoc()) != mainFileID) {
      return;
   }
   clang::SourceLocation SLoc = SM.getInstantiationLoc(
                                   MI->getDefinitionLoc());
   clang::SourceLocation ELoc = SM.getInstantiationLoc(
                                   MI->getDefinitionEndLoc());
   unsigned start = SM.getFileOffset(SLoc);
   if (start < m_minpos) {
      return;
   }
   unsigned end = SM.getFileOffset(ELoc);
   const clang::LangOptions& LO = m_CI.getLangOpts();
   end += clang::Lexer::MeasureTokenLength(ELoc, SM, LO);
   std::pair<const char*, const char*> buf = SM.getBufferData(mainFileID);
   std::string str(buf.first + start, end - start);
   std::string::size_type pos = str.find(' ');
   if (pos != std::string::npos) {
      str = str.substr(0, pos);
   }
   m_macros.push_back("#undef " + str);
}

//
//  Interpreter
//

//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------
Interpreter::Interpreter()
{
   m_globalDeclarations = "#include <stdio.h>\n";
   m_llvm_context = 0;
   m_engine = 0;
   m_prev_module = 0;
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
   // Note: Engine takes ownership of the module.
   llvm::EngineBuilder builder(m_prev_module);
   std::string errMsg;
   builder.setErrorStr(&errMsg);
   builder.setEngineKind(llvm::EngineKind::JIT);
   m_engine = builder.create();
   if (!m_engine) {
      std::cerr << "Error: Unable to create the execution engine!\n";
      std::cerr << errMsg << '\n';
   }
}

//---------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------
Interpreter::~Interpreter()
{
   delete m_prev_module;
   m_prev_module = 0;
   delete m_engine;
   m_engine = 0;
   delete m_llvm_context;
   m_llvm_context = 0;
   // Shutdown the llvm library.
   llvm::llvm_shutdown();
}

//---------------------------------------------------------------------------
// Note: Used by MetaProcessor.
Interpreter::InputType
Interpreter::analyzeInput(const std::string& contextSource,
      const std::string& line, int& indentLevel,
      std::vector<clang::FunctionDecl*>* fds)
{
   // Check if there is an explicitation continuation character.
   if (line.length() > 1 && line[line.length() - 2] == '\\') {
      indentLevel = 1;
      return Incomplete;
   }
   //
   //  Setup a compiler instance to work with.
   //
   clang::CompilerInstance CI;
   //bool first_time = true;
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
         return Incomplete;
      }
      DiagsBuffer.FlushDiagnostics(CI.getDiagnostics());
      if (CI.getDiagnostics().getNumErrors()) {
         CI.takeLLVMContext();
         return Incomplete;
      }
   }
   CI.setTarget(clang::TargetInfo::CreateTargetInfo(CI.getDiagnostics(),
      CI.getTargetOpts()));
   if (!CI.hasTarget()) {
      CI.takeLLVMContext();
      return Incomplete;
   }
   CI.getTarget().setForcedLangOptions(CI.getLangOpts());
   CI.createFileManager();





   llvm::MemoryBuffer* buffer =
      llvm::MemoryBuffer::getMemBufferCopy(&*line.begin(), &*line.end(),
         "CLING");
   CI.getSourceManager().createMainFileIDForMemBuffer(buffer);
   if (CI.getSourceManager().getMainFileID().isInvalid()) {
      return Incomplete;
   }
   clang::Token lastTok;
   bool tokWasDo = false;
   int stackSize = analyzeTokens(CI.getPreprocessor(), lastTok,
      indentLevel, tokWasDo);
   if (stackSize < 0) {
      return TopLevel;
   }
   // tokWasDo is used for do { ... } while (...); loops
   if (
      lastTok.is(clang::tok::semi) ||
      (
         lastTok.is(clang::tok::r_brace) &&
         !tokWasDo
      )
   ) {
      if (stackSize > 0) {
         return Incomplete;
      }
      // Setting this ensures "foo();" is not a valid top-level declaration.
      //diag.setDiagnosticMapping(clang::diag::ext_missing_type_specifier,
      //                          clang::diag::MAP_ERROR);
      std::string src = contextSource + buffer->getBuffer().str();
      struct : public clang::ASTConsumer {
         bool hadIncludedDecls;
         unsigned pos;
         unsigned maxPos;
         clang::SourceManager* sm;
         std::vector<clang::FunctionDecl*> fds;
         void HandleTopLevelDecl(clang::DeclGroupRef D) {
            for (
               clang::DeclGroupRef::iterator I = D.begin(), E = D.end();
               I != E;
               ++I
            ) {
               clang::FunctionDecl* FD = dyn_cast<clang::FunctionDecl>(*I);
               if (FD) {
                  clang::SourceLocation Loc = FD->getTypeSpecStartLoc();
                  if (!Loc.isValid()) {
                     continue;
                  }
                  if (sm->isFromMainFile(Loc)) {
                     unsigned offset =
                        sm->getFileOffset(sm->getInstantiationLoc(Loc));
                     if (offset >= pos) {
                        fds.push_back(FD);
                     }
                  }
                  else {
                     while (!sm->isFromMainFile(Loc)) {
                        const clang::SrcMgr::SLocEntry& Entry =
                           sm->getSLocEntry(
                              sm->getFileID(sm->getSpellingLoc(Loc)));
                        if (!Entry.isFile()) {
                           break;
                        }
                        Loc = Entry.getFile().getIncludeLoc();
                     }
                     unsigned offset = sm->getFileOffset(Loc);
                     if (offset >= pos) {
                        hadIncludedDecls = true;
                     }
                  }
               }
            }
         }
      } consumer;
      consumer.hadIncludedDecls = false;
      consumer.pos = contextSource.length();
      consumer.maxPos = consumer.pos + buffer->getBuffer().size();
      consumer.sm = &CI.getSourceManager();
      buffer = llvm::MemoryBuffer::getMemBufferCopy(&*line.begin(),
         &*line.end(), "CLING");
      CI.getSourceManager().createMainFileIDForMemBuffer(buffer);
      clang::ParseAST(CI.getPreprocessor(), &consumer, CI.getASTContext());
#if 0
      if (
         CI.getDiagnostics().hadError(
            clang::diag::err_unterminated_block_comment)
      ) {
         return Incomplete;
      }
#endif // 0
      if (
         !CI.getDiagnostics().getNumErrors() &&
         (
            !consumer.fds.empty() ||
            consumer.hadIncludedDecls
         )
      ) {
         if (!consumer.fds.empty()) {
            fds->swap(consumer.fds);
         }
         return TopLevel;
      }
      return Stmt;
   }
   return Incomplete;
}

//---------------------------------------------------------------------------
// Note: Used only by analyzeInput().
int Interpreter::analyzeTokens(clang::Preprocessor& PP,
   clang::Token& lastTok, int& indentLevel, bool& tokWasDo)
{
   int result;
   std::stack<std::pair<clang::Token, clang::Token> > S; // Tok, PrevTok
   indentLevel = 0;
   PP.EnterMainSourceFile();
   clang::Token Tok;
   PP.Lex(Tok);
   while (Tok.isNot(clang::tok::eof)) {
      if (Tok.is(clang::tok::l_square)) {
         S.push(std::make_pair(Tok, lastTok));
      }
      else if (Tok.is(clang::tok::l_paren)) {
         S.push(std::make_pair(Tok, lastTok));
      }
      else if (Tok.is(clang::tok::l_brace)) {
         S.push(std::make_pair(Tok, lastTok));
         indentLevel++;
      }
      else if (Tok.is(clang::tok::r_square)) {
         if (S.empty() || S.top().first.isNot(clang::tok::l_square)) {
            std::cout << "Unmatched [\n";
            return -1;
         }
         tokWasDo = false;
         S.pop();
      }
      else if (Tok.is(clang::tok::r_paren)) {
         if (S.empty() || S.top().first.isNot(clang::tok::l_paren)) {
            std::cout << "Unmatched (\n";
            return -1;
         }
         tokWasDo = false;
         S.pop();
      }
      else if (Tok.is(clang::tok::r_brace)) {
         if (S.empty() || S.top().first.isNot(clang::tok::l_brace)) {
            std::cout << "Unmatched {\n";
            return -1;
         }
         tokWasDo = S.top().second.is(clang::tok::kw_do);
         S.pop();
         indentLevel--;
      }
      lastTok = Tok;
      PP.Lex(Tok);
   }
   result = S.size();
   // TODO: We need to properly account for indent-level for blocks that do not
   //       have braces... such as:
   //
   //       if (X)
   //         Y;
   //
   // TODO: Do-while without braces doesn't work, e.g.:
   //
   //       do
   //         foo();
   //       while (bar());
   //
   // Both of the above could be solved by some kind of rewriter-pass that would
   // insert implicit braces (or simply a more involved analysis).
   // Also try to match preprocessor conditionals...
   if (result == 0) {
      clang::Lexer Lexer(PP.getSourceManager().getMainFileID(),
         0, // FIXME: which const llvm::MemoryBuffer*?
         PP.getSourceManager(), PP.getLangOptions());
      Lexer.LexFromRawLexer(Tok);
      while (Tok.isNot(clang::tok::eof)) {
         if (Tok.is(clang::tok::hash)) {
            Lexer.LexFromRawLexer(Tok);
            if (clang::IdentifierInfo *II = PP.LookUpIdentifierInfo(Tok)) {
               switch (II->getPPKeywordID()) {
                  case clang::tok::pp_if:
                  case clang::tok::pp_ifdef:
                  case clang::tok::pp_ifndef:
                     result++;
                     break;
                  case clang::tok::pp_endif:
                     if (result == 0)
                        return -1; // Nesting error.
                     result--;
                     break;
                  default:
                     break;
               }
            }
         }
         Lexer.LexFromRawLexer(Tok);
      }
   }
   return result;
}

#if 0
//---------------------------------------------------------------------------
// Compile the buffer and link it to all the modules known to the
// compiler but do not add it to the list
//
// Note: Used by Metaprocessor.
//---------------------------------------------------------------------------
llvm::Module*
Interpreter::linkSource(const std::string& source, std::string* errMsg)
{
   static const std::string code_prefix =
      "#include <stdio.h>\nextern \"C\" int imain(int argc, char** argv) {\n";
   static const std::string code_suffix = ";\nreturn 0; } ";
   // check whether we have a preprocessor statement:
   size_t posHash = 0;
   while (isspace(source[posHash]))
      ++posHash;
   if (source[posHash] != '#')
      posHash = std::string::npos;
   ParseEnvironment *pEnv = 0;
   if (posHash != std::string::npos) {
      pEnv = parseSource(source);
      m_globalDeclarations.append(source + '\n');
   }
   else {
      std::vector<std::string> statements;
      std::string wrapped(splitInput(source, statements));
      if (wrapped.length()) {
         pEnv = parseSource(wrapped);
      }
   }
   if (!pEnv) {
      return 0;
   }
   clang::TranslationUnitDecl* tu =
      pEnv->getASTContext()->getTranslationUnitDecl();
   llvm::Module* module = compile(tu);
   llvm::Module* result = linkModule(module, errMsg);
   delete tu;
   delete module;
   return result;
}
#endif // 0

void
Interpreter::processLine(const std::string& input_line)
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
         fprintf(stderr, "Setting mapping for: %s to %lx\n",
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

#if 0
//---------------------------------------------------------------------------
// Note: Used only by linkSource().
std::string Interpreter::splitInput(const std::string& input,
                                    std::vector<std::string>& statements)
{
   std::string src = m_globalDeclarations;
   unsigned int newpos = src.length();
   src += "void __cling_internal() {\n";
   src += input;
   src += "\n}\n";
   fprintf(stderr, "code:\n%s\n", src.c_str());
   clang::Diagnostic diag(&m_compiler->getDiagnosticClient());
   diag.setSuppressSystemWarnings(true);
   std::vector<clang::Stmt*> stmts;
   MacroDetector* macros = new MacroDetector(m_compiler->getLangOpts(),
         newpos, 0);
   ParseEnvironment pEnv(m_compiler->getLangOpts(), m_compiler->getTarget(),
                         &diag, &m_compiler->getFileManager(), 0, m_inclPaths, macros);
   clang::SourceManager* sm = pEnv.getSourceManager();
   macros->setSourceManager(sm);
   StmtSplitter splitter(src, *sm, m_compiler->getLangOpts(), &stmts);
   FunctionBodyConsumer<StmtSplitter> consumer(&splitter, "__cling_internal");
   llvm::MemoryBuffer* buffer =
      llvm::MemoryBuffer::getMemBufferCopy(&*src.begin(), &*src.end(),
                                           "CLING");
   pEnv.getSourceManager()->createMainFileIDForMemBuffer(buffer);
   clang::ParseAST(*pEnv.getPreprocessor(), &consumer, *pEnv.getASTContext());
   statements.clear();
   std::string newGlobalDecls;
   std::string processedCode;
   if (diag.hasErrorOccurred()) {
      return "";
   }
   clang::QualType QT;
   for (unsigned i = 0; i < stmts.size(); i++) {
      SrcRange range =
         getStmtRangeWithSemicolon(stmts[i], *sm, m_compiler->getLangOpts());
      std::string s = src.substr(range.first, range.second - range.first);
      if (const clang::Expr *E = dyn_cast<clang::Expr>(stmts[i])) {
         QT = E->getType();
         appendStatement(processedCode, s);
      }
      else if (
         const clang::DeclStmt* DS = dyn_cast<clang::DeclStmt>(stmts[i])
      ) {
         if (!handleDeclStmt(DS, &pEnv, src, newGlobalDecls, processedCode)) {
            newGlobalDecls.append("extern " + s + '\n');
            processedCode.append(s + '\n');
         }
      }
      else {
         processedCode.append(s + '\n');
      }
      statements.push_back(s);
   }
   processedCode = m_globalDeclarations + processedCode;
   m_globalDeclarations.append(newGlobalDecls + '\n');
   return processedCode;
}

//---------------------------------------------------------------------------
// Parse memory buffer
//
// Note: Used only by linkSource().
//---------------------------------------------------------------------------
ParseEnvironment*
Interpreter::parseSource(const std::string& source)
{
   llvm::MemoryBuffer* buff =
      llvm::MemoryBuffer::getMemBufferCopy(&*source.begin(), &*source.end(),
                                           "CLING");
   clang::SourceManager* srcMgr = new clang::SourceManager();
   if (buff) {
      srcMgr->createMainFileIDForMemBuffer(buff);
   }
   if (srcMgr->getMainFileID().isInvalid()) {
      return 0;
   }
   clang::Diagnostic diag(&m_compiler->getDiagnosticClient());
   diag.setSuppressSystemWarnings(true);
   ParseEnvironment* pEnv = new ParseEnvironment(m_compiler->getLangOpts(),
         m_compiler->getTarget(), &m_compiler->getDiagnostics(),
         &m_compiler->getFileManager(), &m_compiler->getSourceManager(),
         m_inclPaths);
   llvm::raw_stdout_ostream out;
   clang::ASTConsumer* dummyConsumer = clang::CreateASTPrinter(&out);
   clang::Sema sema(*pEnv->getPreprocessor(), *pEnv->getASTContext(),
                    *dummyConsumer);
   clang::Parser p(*pEnv->getPreprocessor(), sema);
   pEnv->getPreprocessor()->EnterMainSourceFile();
   p.ParseTranslationUnit();
   return pEnv;
}

//----------------------------------------------------------------------------
// Compile the translation unit
//----------------------------------------------------------------------------
llvm::Module*
Interpreter::compile(clang::TranslationUnitDecl* tu)
{
   if (!tu) {
      return 0;
   }
   clang::Diagnostic diag(&m_compiler->getDiagnosticClient());
   diag.setSuppressSystemWarnings(true);
   llvm::OwningPtr<clang::CodeGenerator> codeGen;
   clang::CodeGenOptions options;
   codeGen.reset(CreateLLVMCodeGen(diag, "SOME NAME [Interpreter::compile()]",
                                   options, m_compiler->getLLVMContext()));
   codeGen->Initialize(tu->getASTContext());
   clang::TranslationUnitDecl::decl_iterator iter = tu->decls_begin();
   clang::TranslationUnitDecl::decl_iterator iter_end = tu->decls_end();
   for (; iter != iter_end; ++iter) {
      codeGen->HandleTopLevelDecl(clang::DeclGroupRef(*it));
   }
   codeGen->HandleTranslationUnit(tu->getASTContext());
   llvm::Module* module = codeGen->ReleaseModule();
   return module;
}

//---------------------------------------------------------------------------
// Link the module to all the modules known to the compiler but do
// not add it to the list
//---------------------------------------------------------------------------
llvm::Module*
Interpreter::linkModule(llvm::Module* module, std::string* errMsg)
{
   if (!m_module) {
      llvm::Linker linker("executable", llvm::CloneModule(module));
      m_module = linker.releaseModule();
      return m_module;
   }
   llvm::Linker linker("executable", llvm::CloneModule(m_module));
   if (linker.LinkInModule(llvm::CloneModule(module), errMsg)) {
      return 0;
   }
   m_module = linker.releaseModule();
   return m_module;
}

//---------------------------------------------------------------------------
// Compile the filename and link it to all the modules known to the
// compiler but do not add it to the list
//---------------------------------------------------------------------------
llvm::Module*
Interpreter::linkFile(const std::string& fileName, std::string* errMsg)
{
   ParseEnvironment* pEnv = parseFile(fileName);
   clang::TranslationUnitDecl* tu =
      pEnv->getASTContext()->getTranslationUnitDecl();
   llvm::Module* module = compile(tu);
   llvm::Module* result = linkModule(module, errMsg);
   delete pEnv;
   delete module;
   return result;
}

//---------------------------------------------------------------------------
// Parse file
//---------------------------------------------------------------------------
ParseEnvironment*
Interpreter::parseFile(const std::string& fileName)
{
   clang::SourceManager* srcMgr = new clang::SourceManager();
   const clang::FileEntry *file =
      m_compiler->getFileManager().getFile(fileName);
   if (file) {
      srcMgr->createMainFileID(file, clang::SourceLocation());
   }
   if (srcMgr->getMainFileID().isInvalid()) {
      return 0;
   }
   clang::Diagnostic diag(&m_compiler->getDiagnosticClient());
   diag.setSuppressSystemWarnings(true);
   ParseEnvironment* pEnv = new ParseEnvironment(m_compiler->getLangOpts(),
         m_compiler->getTarget(), &m_compiler->getDiagnostics(),
         &m_compiler->getFileManager(), &m_compiler->getSourceManager(),
         m_inclPaths);
   llvm::raw_stdout_ostream out;
   clang::ASTConsumer* dummyConsumer = clang::CreateASTPrinter(&out);
   clang::Sema sema(*pEnv->getPreprocessor(), *pEnv->getASTContext(),
                    *dummyConsumer);
   clang::Parser p(*pEnv->getPreprocessor(), sema);
   pEnv->getPreprocessor()->EnterMainSourceFile();
   p.ParseTranslationUnit();
   return pEnv;
}

//----------------------------------------------------------------------------
// Extract the function declarations
//----------------------------------------------------------------------------
std::vector<clang::Decl*>
Interpreter::extractDeclarations(clang::TranslationUnitDecl* tu)
{
   std::vector<clang::Decl*> vect;
   if (!tu)
      return vect;
   std::set<clang::Decl*> decls_before;
   std::vector<std::pair<clang::Decl*, const clang::ASTContext*> >::iterator
   dit, ditend = m_decls.end();
   for (dit = m_decls.begin(); dit != ditend; ++dit)
      decls_before.insert(dit->first);
   //-------------------------------------------------------------------------
   // Loop over the declarations
   //-------------------------------------------------------------------------
   clang::ASTContext& astContext = tu->getASTContext();
   //const clang::SourceManager& srcMgr = astContext.getSourceManager();
   for (clang::TranslationUnitDecl::decl_iterator it = tu->decls_begin(),
         itend = tu->decls_end(); it != itend; ++it) {
      if (it->getKind() == clang::Decl::Function) {
         clang::FunctionDecl* decl = static_cast<clang::FunctionDecl*>(*it);
         if (decl && decls_before.find(decl) == decls_before.end()) {
            vect.push_back(decl);
            m_decls.push_back(std::make_pair(*it, &astContext));
         }
      }
   }
   return vect;
}

//----------------------------------------------------------------------------
// Insert the implicit declarations to the translation unit
//----------------------------------------------------------------------------
void
Interpreter::insertDeclarations(clang::TranslationUnitDecl* tu,
                                clang::Sema* sema)
{
   clang::ASTContext& astContext = tu->getASTContext();
   clang::IdentifierTable&      table    = astContext.Idents;
   clang::DeclarationNameTable& declTab  = astContext.DeclarationNames;
   std::vector<std::pair<clang::Decl*, const clang::ASTContext*> >::iterator it;
   for (it = m_decls.begin(); it != m_decls.end(); ++it) {
      if (it->first->getKind() == clang::Decl::Function) {
         clang::FunctionDecl* func = static_cast<clang::FunctionDecl*>(it->first);
         if (func) {
            clang::IdentifierInfo&       id       = table.get(std::string(func->getNameAsString()));
            clang::DeclarationName       dName    = declTab.getIdentifier(&id);
            clang::FunctionDecl* decl = clang::FunctionDecl::Create(astContext,
                                        tu,
                                        func->getLocation(),
                                        dName,
                                        typeCopy(func->getType(), *it->second, astContext),
                                        0 /*DeclInfo*/);
            tu->addDecl(decl);
            sema->IdResolver.AddDecl(decl);
            if (sema->TUScope)
               sema->TUScope->AddDecl(clang::Action::DeclPtrTy::make(decl));
         }
      }
   }
}

//----------------------------------------------------------------------------
// Dump the translation unit
//----------------------------------------------------------------------------
void
Interpreter::dumpTU(clang::DeclContext* dc)
{
   for (clang::DeclContext::decl_iterator it = dc->decls_begin(),
         itE = dc->decls_end(); it != itE; ++it) {
      clang::Stmt* body = (*it) ? (*it)->getBody() : 0;
      if (body) {
         std::cerr << "--- AST ---" << std::endl;
         body->dumpAll();
         std::cerr << std::endl;
      }
   }
}

//-----------------------------------------------------------------------------
// Copy given type to the target AST context using serializers - grr ugly :)
//-----------------------------------------------------------------------------
clang::QualType Interpreter::typeCopy(clang::QualType source,
                                      const clang::ASTContext& sourceContext,
                                      clang::ASTContext& targetContext)
{
   const clang::BuiltinType*       bt;
   const clang::FunctionType*      ft1;
   const clang::FunctionProtoType* ft2;
   //--------------------------------------------------------------------------
   // Deal with a builtin type
   //--------------------------------------------------------------------------
   if ((bt = source.getTypePtr()->getAs<clang::BuiltinType>())) {
      return clang::QualType(bt->getCanonicalTypeInternal().getUnqualifiedType().getTypePtr()
                             /*was: targetContext.getBuiltinType( bt->getKind() ).getTypePtr()*/,
                             source.getCVRQualifiers());
   }
   //--------------------------------------------------------------------------
   // Deal with a pointer type
   //--------------------------------------------------------------------------
   else if (source.getTypePtr()->isPointerType()) {
      const clang::PointerType* pt = source.getTypePtr()->getAs<clang::PointerType>();
      clang::QualType pointee = typeCopy(pt->getPointeeType(),
                                         sourceContext,
                                         targetContext);
      clang::QualType result = targetContext.getPointerType(pointee);
      // FIXME: which source qualifiers? Maybe local? or fast?
      result.setLocalFastQualifiers(source.getLocalFastQualifiers());
      return result;
   }
   //--------------------------------------------------------------------------
   // Deal with a function type
   //--------------------------------------------------------------------------
   else if (source.getTypePtr()->isFunctionType()) {
      ft1  = static_cast<clang::FunctionType*>(source.getTypePtr());
      //-----------------------------------------------------------------------
      // No parameters
      //-----------------------------------------------------------------------
      if (ft1->getTypeClass() != clang::Type::FunctionProto)
         return targetContext.getFunctionNoProtoType(typeCopy(ft1->getResultType(),
                sourceContext,
                targetContext));
      ft2  = static_cast<clang::FunctionProtoType*>(source.getTypePtr());
      //-----------------------------------------------------------------------
      // We have some parameters
      //-----------------------------------------------------------------------
      std::vector<clang::QualType> args;
      clang::FunctionProtoType::arg_type_iterator it;
      for (it = ft2->arg_type_begin(); it != ft2->arg_type_end(); ++it)
         args.push_back(typeCopy(*it, sourceContext, targetContext));
      return targetContext.getFunctionType(typeCopy(ft2->getResultType(),
                                           sourceContext,
                                           targetContext),
                                           &args.front(), args.size(),
                                           ft2->isVariadic(),
                                           ft2->getTypeQuals());
   }
   assert("Unable to convert type");
   return source;
}

static llvm::ExecutionEngine* engine = 0;

//---------------------------------------------------------------------------
// Load an interpreter on a Module, making sure to run the global variable
// initialization.
//---------------------------------------------------------------------------
bool
Interpreter::loadModule(llvm::Module *module)
{
   //---------------------------------------------------------------------------
   // Create the execution engine
   //---------------------------------------------------------------------------
   llvm::EngineBuilder builder(module);
   std::string errMsg;
   builder.setErrorStr(&errMsg);
   builder.setEngineKind(llvm::EngineKind::JIT);
   if (!engine) {
      engine = builder.create();
   }
   if (!engine) {
      std::cout << "[!] Unable to create the execution engine! (" << errMsg << ")" << std::endl;
      return false;
   }
   static bool firsttime = true;
   if (firsttime) {
      // Run static constructors.
      engine->runStaticConstructorsDestructors(false);
      firsttime = false;
   }
   else {
      engine->runStaticConstructorsDestructors(module, false);
   }
   return true;
}

//---------------------------------------------------------------------------
// Call the Interpreter on a Module
//---------------------------------------------------------------------------
int
Interpreter::executeModuleMain(llvm::Module *module, const std::string& name)
{
   if (loadModule(module)) {
      llvm::Function* func(module->getFunction(name));
      if (!func) {
         // try C++:
         for (llvm::Module::iterator iFunc = module->begin(),
               eFunc = module->end(); iFunc != eFunc; ++iFunc) {
            // The function name matching is a complete hack.
            //  I don't know how to demangle etc.
            if (iFunc->hasName() && strstr(iFunc->getNameStr().c_str(), name.c_str())) {
               if (iFunc->getArgumentList().empty())
                  func = &(*iFunc);
            }
         }
      }
      if (!func) {
         std::cerr << "[!] Cannot find the entry function "
                   << name << "!" << std::endl;
         return 1;
      }
      std::vector<std::string> params;
      return engine->runFunctionAsMain(func,  params, 0);
   }
   else {
      return 1;
   }
}

//---------------------------------------------------------------------------
// Call the Interpreter on a File
//---------------------------------------------------------------------------
int
Interpreter::executeFile(const std::string& filename,
                         const std::string& funcname)
{
   llvm::Module* module = linkFile(filename);
   if (!module) {
      std::cerr << "[!] Errors occured while parsing file " << filename << "!" << std::endl;
      return 1;
   }
   std::string myfuncname(funcname);
   if (funcname == "()") {
      size_t posSlash = filename.find_last_of('/');
      ++posSlash; // npos to 0, good!
      myfuncname = filename.substr(posSlash);
      size_t posDot = myfuncname.find('.');
      if (posDot != std::string::npos) {
         myfuncname.erase(posDot);
      }
   }
   return executeModuleMain(module, myfuncname);
}

// Generate a variable declaration (like "int i;") for the specified type
// and variable name. Works for non-trivial types like function pointers.
std::string genVarDecl(const clang::PrintingPolicy& PP,
                       const clang::QualType& type,
                       const std::string& vName)
{
   std::string str = vName;
   type.getUnqualifiedType().getAsStringInternal(str, PP);
   return str;
}

bool
Interpreter::handleDeclStmt(const clang::DeclStmt* DS, ParseEnvironment* pEnv,
                            const std::string& src, std::string& globalDecls,
                            std::string& processedCode)
{
   std::vector<std::string> stmts;
   clang::SourceManager *sm = pEnv->getSourceManager();
   clang::ASTContext *context = pEnv->getASTContext();
   for (clang::DeclStmt::const_decl_iterator D = DS->decl_begin(),
         E = DS->decl_end(); D != E; ++D) {
      if (const clang::VarDecl *VD = dyn_cast<clang::VarDecl>(*D)) {
         std::string decl = genVarDecl(clang::PrintingPolicy(m_compiler->getLangOpts()),
                                       VD->getType(), VD->getNameAsCString());
         const clang::Expr *I = VD->getInit();
         if (I && 0 == dyn_cast<clang::CXXConstructExpr>(I)) {
            SrcRange range = getStmtRange(I, *sm, m_compiler->getLangOpts());
            if (I->isConstantInitializer(*context)) {
               // Keep the whole thing in the global context.
               processedCode.append(decl + "=" + src.substr(range.first, range.second - range.first) + ";\n");
            }
            else if (const clang::InitListExpr *ILE = dyn_cast<clang::InitListExpr>(I)) {
               // If it's an InitListExpr like {'a','b','c'}, but with non-constant
               // initializers, then split it up into x[0] = 'a'; x[1] = 'b'; and
               // so forth, which would go in the function body, while making the
               // declaration global.
               unsigned numInits = ILE->getNumInits();
               for (unsigned i = 0; i < numInits; i++) {
                  std::string stmt;
                  llvm::raw_string_ostream stmtstream(stmt);
                  stmtstream << VD->getNameAsCString() << "[" << i << "] = ";
                  range = getStmtRange(ILE->getInit(i), *sm, m_compiler->getLangOpts());
                  stmtstream << src.substr(range.first, range.second - range.first) << ";";
                  stmts.push_back(stmtstream.str());
               }
               processedCode.append(decl + ";\n");
            }
            else {
               std::string stmt(VD->getNameAsCString());
               stmt += " = " + src.substr(range.first, range.second - range.first) + ";";
               stmts.push_back(stmt);
               processedCode.append(decl + ";\n");
            }
         }
         else {
            // Just add it as a definition without an initializer.
            processedCode.append(decl + ";\n");
         }
         globalDecls.append("extern " + decl + ";\n");
//         }  if (const clang::RecordDecl *rec = dyn_cast<clang::RecordDecl>(*D)) {
      }
      else {
         clang::SourceLocation SLoc = sm->getInstantiationLoc((*D)->getLocStart());
         clang::SourceLocation ELoc = sm->getInstantiationLoc((*D)->getLocEnd());
         SrcRange range = getRangeWithSemicolon(SLoc, ELoc, *sm, m_compiler->getLangOpts());
         std::string decl = src.substr(range.first, range.second - range.first);
         processedCode.append(decl + ";\n");
         globalDecls.append(decl + ";\n");
      }
   }
   if (stmts.size() > 0) {
      std::string initstmts;
      llvm::raw_string_ostream stmtstream(initstmts);
      for (unsigned i = 0; i < stmts.size(); ++i) {
         stmtstream << stmts[i] << 'n';
      }
      appendStatement(processedCode, stmtstream.str());
   }
   return true;
}

//---------------------------------------------------------------------
// Add an entry to the interpreter's include path
//---------------------------------------------------------------------
void
Interpreter::addIncludePath(const llvm::StringRef& dir)
{
   if (!m_inclPaths) {
      m_inclPaths = new std::vector<std::string>;
   }
   m_inclPaths->push_back(dir);
}

//---------------------------------------------------------------------------
// Add a translation unit
//---------------------------------------------------------------------------
bool
Interpreter::addUnit(const std::string& fileName)
{
   if (fileName.empty()) {
      return false;
   }
   std::map<UnitID_t, UnitInfo_t>::iterator it;
   it = m_units.find(fileName);
   if (it != m_units.end()) {
      return false;
   }
   ParseEnvironment* pEnv = parseFile(fileName);
   clang::TranslationUnitDecl* tu =
      pEnv->getASTContext()->getTranslationUnitDecl();
   bool res = addUnit(fileName, tu);
   delete pEnv;
   return res;
}

//----------------------------------------------------------------------------
// Add the translation unit
//----------------------------------------------------------------------------
bool
Interpreter::addUnit(const UnitID_t& id, clang::TranslationUnitDecl* tu)
{
   if (!tu) {
      return false;
   }
   UnitInfo_t uinfo;
   uinfo.ast = tu;
   uinfo.decls = extractDeclarations(tu);
   uinfo.module = compile(tu);
   if (!uinfo.module) {
      delete tu;
      return false;
   }
   llvm::Module* module = linkModule(uinfo.module);
   if (!module) {
      delete tu;
      delete module;
      return false;
   }
   m_units[id] = uinfo;
   return true;
}

//---------------------------------------------------------------------------
// Add a translation unit
//---------------------------------------------------------------------------
bool
Interpreter::addUnit(const llvm::MemoryBuffer* buffer, UnitID_t& id)
{
   return false;
}

//---------------------------------------------------------------------------
// Remove the translation unit
//---------------------------------------------------------------------------
bool
Interpreter::removeUnit(const UnitID_t& id)
{
   return false;
}

//---------------------------------------------------------------------------
// Get a compiled  module linking together all translation units
//---------------------------------------------------------------------------
llvm::Module*
Interpreter::getModule()
{
   return m_module;
}
#endif // 0

} // namespace cling

