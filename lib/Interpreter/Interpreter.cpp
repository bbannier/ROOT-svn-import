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
#include "llvm/MC/MCAsmInfo.h"
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
#include "llvm/Target/Mangler.h"
#include "llvm/Target/SubtargetFeature.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetSelect.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Transforms/Utils/Cloning.h"

#include "Diagnostics.h"
#include "ParseEnvironment.h"
#include "Visitors.h"
#include "ClangUtils.h"

#include <cstdio>
#include <iostream>
#include <map>
#include <sstream>
#include <stack>
#include <string>
#include <utility>
#include <vector>

#include <limits.h>
#include <stdint.h>

static const char* fake_argv[] = { "clang", "-x", "c++", "-D__CLING__", 0 };
static const int fake_argc = (sizeof(fake_argv) / sizeof(const char*)) - 1;

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

#if 0
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
#endif // 0

} // unnamed namespace

namespace cling {

//
//  Dummy function so we can use dladdr to find the executable path.
//
void locate_cling_executable()
{
}

//
// MacroDetector
//

class MacroDetector : public clang::PPCallbacks {
private:
   const clang::CompilerInstance& m_CI;
   unsigned int m_minpos;
   std::vector<std::string> m_macros;
   clang::FileID m_InterpreterFile; // file included by the stub, identified by MacroDefined(), FileChanged()
   unsigned int m_inclLevel; // level of inclusions below m_InterpreterFile

   // Whether the CPP interpreter file tag was found,
   // the file is parsed, or has been parsed
   enum {
      kNotSeenYet,
      kIsComingUp,
      kIsActive,
      kHasPassed
   } m_InterpreterFileStatus;

public:

   MacroDetector(const clang::CompilerInstance& CI, unsigned int minpos)
      : clang::PPCallbacks(), m_CI(CI), m_minpos(minpos),
        m_inclLevel(0),
        m_InterpreterFileStatus(kNotSeenYet) {
   }

   virtual ~MacroDetector();

   std::vector<std::string>&
   getMacrosVector() {
      return m_macros;
   }

   void
   MacroDefined(const clang::IdentifierInfo* II, const clang::MacroInfo* MI);

   void
   MacroUndefined(const clang::IdentifierInfo* II, const clang::MacroInfo* MI);

   void
   FileChanged(clang::SourceLocation Loc, clang::PPCallbacks::FileChangeReason Reason,
               clang::SrcMgr::CharacteristicKind FileType);
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
   if (SM.getFileID(MI->getDefinitionLoc()) == mainFileID) {
      if (II && II->getLength() == 35
          && II->getName() == "__CLING__MAIN_FILE_INCLUSION_MARKER") {
         assert(m_InterpreterFileStatus == kNotSeenYet && "Inclusion-buffer already found the interpreter file!");
         m_InterpreterFileStatus = kIsComingUp;
      } else if (II && II->getLength() == 25
          && II->getName() == "__CLING__MAIN_FILE_MARKER") {
         assert(m_InterpreterFileStatus == kNotSeenYet && "Inclusion-buffer already found the interpreter file!");
         m_InterpreterFileStatus = kIsActive;
      }
      return;
   }

   if (m_InterpreterFileStatus != kIsActive || m_InterpreterFile.isInvalid()
       || SM.getFileID(MI->getDefinitionLoc()) != m_InterpreterFile)
      return;

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
   std::pair<const char*, const char*> buf = std::make_pair(
      SM.getBuffer(m_InterpreterFile)->getBufferStart(),
      SM.getBuffer(m_InterpreterFile)->getBufferEnd());
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
   if (SM.getFileID(MI->getDefinitionLoc()) == mainFileID) {
      if (II
          && ((II->getLength() == 35
               && II->getName() == "__CLING__MAIN_FILE_INCLUSION_MARKER")
              || (II->getLength() == 25
                  && II->getName() == "__CLING__MAIN_FILE_MARKER"))) {
         assert(m_InterpreterFileStatus == kIsActive && "Interpreter file is not active!");
         m_InterpreterFileStatus = kHasPassed;
      }
      return;
   }

   if (m_InterpreterFileStatus != kIsActive || m_InterpreterFile.isInvalid()
       || SM.getFileID(MI->getDefinitionLoc()) != m_InterpreterFile)
      return;

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
   std::pair<const char*, const char*> buf = std::make_pair(
      SM.getBuffer(m_InterpreterFile)->getBufferStart(),
      SM.getBuffer(m_InterpreterFile)->getBufferEnd());
   std::string str(buf.first + start, end - start);
   std::string::size_type pos = str.find(' ');
   if (pos != std::string::npos) {
      str = str.substr(0, pos);
   }
   m_macros.push_back("#undef " + str);
}

void
MacroDetector::FileChanged(clang::SourceLocation Loc, clang::PPCallbacks::FileChangeReason Reason,
                           clang::SrcMgr::CharacteristicKind /*FileType*/)
{
   // This callback is invoked whenever a source file is entered or exited.
   // The SourceLocation indicates the new location, and EnteringFile indicates
   // whether this is because we are entering a new include'd file (when true)
   // or whether we're exiting one because we ran off the end (when false).
   // It extracts the main file ID - the one requested by the interpreter
   // and not the string buffer used to prepent the previous global declarations
   // and to append the execution suffix.

   clang::SourceManager& SM = m_CI.getSourceManager();
   if (m_InterpreterFileStatus == kIsComingUp) {
      m_InterpreterFile = SM.getFileID(Loc);
      m_InterpreterFileStatus = kIsActive;
   } else if (m_InterpreterFileStatus == kIsActive) {
      if (Reason == EnterFile) {
         ++m_inclLevel;
         if (m_inclLevel == 1) {
            clang::FileID inclFileID = SM.getFileID(Loc);
            if (!inclFileID.isInvalid()) {
               const clang::FileEntry* FE = SM.getFileEntryForID(inclFileID);
               if (FE) {
                  std::string filename = FE->getName();
                  m_macros.push_back("#include \"" + filename + "\"");
               }
            }
         }
      } else if (Reason == ExitFile) {
         --m_inclLevel;
      }
   }
}

//
//  Interpreter
//

//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------
Interpreter::Interpreter(const char* llvmdir /*= 0*/):
   m_llvm_context(0),
   m_CI(0),
   m_engine(0),
   m_prev_module(0),
   m_numCallWrappers(0),
   m_printAST(false)
{
   m_globalDeclarations = "#include <stdio.h>\n";
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
   m_CI = createCI(llvmdir);
   m_prev_module = new llvm::Module("_Clang_first", *m_llvm_context);
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
   //delete m_prev_module;
   //m_prev_module = 0; // Don't do this, the engine does it.
   delete m_engine;
   m_engine = 0;
   m_CI->takeLLVMContext(); // Don't take down the context with the CI.
   delete m_CI;
   m_CI = 0;
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
   clang::CompilerInstance* CI = getCI();
   if (!CI) {
      return Incomplete;
   }
   CI->createPreprocessor();
   llvm::MemoryBuffer* buffer =
      llvm::MemoryBuffer::getMemBufferCopy(line, "CLING");
   CI->getSourceManager().clearIDTables();
   CI->getSourceManager().createMainFileIDForMemBuffer(buffer);
   if (CI->getSourceManager().getMainFileID().isInvalid()) {
      ///*reuseCI*/CI->takeLLVMContext();
      ///*reuseCI*/delete CI;
      ///*reuseCI*/CI = 0;
      return Incomplete;
   }
   clang::Token lastTok;
   bool tokWasDo = false;
   int stackSize = analyzeTokens(CI->getPreprocessor(), lastTok,
                                 indentLevel, tokWasDo);
   ///*reuseCI*/CI->takeLLVMContext();
   ///*reuseCI*/delete CI;
   ///*reuseCI*/CI = 0;
   if (stackSize < 0) {
      return TopLevel;
   }
   // tokWasDo is used for do { ... } while (...); loops
   if (
      !lastTok.is(clang::tok::semi) &&
      (
         !lastTok.is(clang::tok::r_brace) ||
         tokWasDo
      )
   ) {
      return Incomplete;
   }
   if (stackSize > 0) {
      return Incomplete;
   }
   CI = getCI();
   if (!CI) {
      return TopLevel;
   }
   CI->createPreprocessor();
   clang::Preprocessor& PP = CI->getPreprocessor();
   // Setting this ensures "foo();" is not a valid top-level declaration.
   //diag.setDiagnosticMapping(clang::diag::ext_missing_type_specifier,
   //                          clang::diag::MAP_ERROR);
   CI->getDiagnosticClient().BeginSourceFile(CI->getLangOpts(), &PP);
   //CI->createASTContext();
   CI->setASTContext(new clang::ASTContext(CI->getLangOpts(),
      PP.getSourceManager(), CI->getTarget(), PP.getIdentifierTable(),
      PP.getSelectorTable(), PP.getBuiltinInfo(), false, 0));

   CI->setASTConsumer(maybeGenerateASTPrinter());
   PP.getBuiltinInfo().InitializeBuiltins(PP.getIdentifierTable(),
                                          PP.getLangOptions().NoBuiltin);
   //std::string src = contextSource + buffer->getBuffer().str();
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
   consumer.sm = &CI->getSourceManager();
   buffer = llvm::MemoryBuffer::getMemBufferCopy(line, "CLING");
   if (!buffer) {
      ///*reuseCI*/CI->takeLLVMContext();
      ///*reuseCI*/delete CI;
      ///*reuseCI*/CI = 0;
      return TopLevel;
   }
   CI->getSourceManager().clearIDTables();
   CI->getSourceManager().createMainFileIDForMemBuffer(buffer);
   if (CI->getSourceManager().getMainFileID().isInvalid()) {
      ///*reuseCI*/CI->takeLLVMContext();
      ///*reuseCI*/delete CI;
      ///*reuseCI*/CI = 0;
      return TopLevel;
   }
   clang::ParseAST(PP, &CI->getASTConsumer(), CI->getASTContext());
   //CI->setASTConsumer(0);  // We may use the consumer below.
   //CI->setASTContext(0);  // We may use the consumer below.
   if (CI->hasPreprocessor()) {
      CI->getPreprocessor().EndSourceFile();
   }
   CI->clearOutputFiles(/*EraseFiles=*/CI->getDiagnostics().getNumErrors());
   CI->getDiagnosticClient().EndSourceFile();
#if 0
   if (
      CI->getDiagnostics().hadError(
         clang::diag::err_unterminated_block_comment)
   ) {
      ///*reuseCI*/CI->takeLLVMContext();
      ///*reuseCI*/delete CI;
      ///*reuseCI*/CI = 0;
      return Incomplete;
   }
#endif // 0
   if (
      !CI->getDiagnostics().getNumErrors() &&
      (
         !consumer.fds.empty() ||
         consumer.hadIncludedDecls
      )
   ) {
      if (!consumer.fds.empty()) {
         fds->swap(consumer.fds);
      }
      ///*reuseCI*/CI->takeLLVMContext();
      ///*reuseCI*/delete CI;
      ///*reuseCI*/CI = 0;
      return TopLevel;
   }
   ///*reuseCI*/CI->takeLLVMContext();
   ///*reuseCI*/delete CI;
   ///*reuseCI*/CI = 0;
   return Stmt;
}

//---------------------------------------------------------------------------
// Note: Used only by analyzeInput().
int Interpreter::analyzeTokens(clang::Preprocessor& PP,
                               clang::Token& lastTok, int& indentLevel,
                               bool& tokWasDo)
{
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
   int result = S.size();
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
                         PP.getSourceManager().getBuffer(
                            PP.getSourceManager().getMainFileID()),
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
                     ++result;
                     break;
                  case clang::tok::pp_endif:
                     if (result == 0)
                        return -1; // Nesting error.
                     --result;
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

void
Interpreter::processLine(const std::string& input_line)
{
   //
   //  Transform the input line to implement cint
   //  command line semantics (declarations are global),
   //  and compile to produce a module.
   //
   llvm::Module* m = makeModuleFromCommandLine(input_line);
   if (!m) {
      return;
   }
   //
   //  Transfer global mappings from previous module.
   //
   copyGlobalMappings(m_engine, m_prev_module, m);
   //
   //  All done with previous module, delete it.
   //
   {
      bool ok = m_engine->removeModule(m_prev_module);
      if (!ok) {
         //fprintf(stderr, "Previous module not found in execution engine!\n");
      }
      delete m_prev_module;
      m_prev_module = 0;
   }
   //
   //  Give new module to the execution engine.
   //
   m_engine->addModule(m); // Note: The engine takes ownership of the module.
   //
   //  Run it using the JIT.
   //
   executeCommandLine();
   //
   //  All done, save module to transfer mappings
   //  on the next run.
   //
   m_prev_module = m;
}

llvm::Module*
Interpreter::makeModuleFromCommandLine(const std::string& input_line)
{
   //
   //  Check to see if input is a preprocessor directive.
   //
   // FIXME: Not good enough, need to use lexer here! (comments, extern vs. CPP etc)
   bool nothingToCompile = true;
   {
      std::string::size_type pos = input_line.find_first_not_of(" \t\n");
      if ((pos != std::string::npos)
          && (input_line[pos] != '#')
          && (input_line.compare(pos, 7, "extern ") != 0)) {
         nothingToCompile = false;
      }
   }
   //
   //  If it is a preprocessor directive, just add it to the
   //  collection of global declarations,
   //  otherwise we must rewrite the code, then compile.
   //
   if (nothingToCompile) {
      m_globalDeclarations.append(input_line);
      m_globalDeclarations.append("\n");
      return 0;
   }

   clang::CompilerInstance* CI = 0;
   //
   //  Wrap input into a function along with
   //  the saved global declarations.
   //
   std::string src(m_globalDeclarations);
   src += "void __cling_internal() {\n";
   src += "#define __CLING__MAIN_FILE_MARKER __cling__prompt\n";
   src += input_line;
   src += "#undef __CLING__MAIN_FILE_MARKER\n";
   src += "\n} // end __cling_internal()\n";
   //fprintf(stderr, "input_line:\n%s\n", src.c_str());
   std::string wrapped;
   createWrappedSrc(src, wrapped);
   if (!wrapped.size()) {
      return 0;
   }
   //
   //  Send the wrapped code through the
   //  frontend to produce a translation unit.
   //
   CI = compileString(wrapped);
   if (!CI) {
      return 0;
   }
   ///**/   ///*reuseCI*/CI->takeLLVMContext();
   ///**/   ///*reuseCI*/delete CI;
   ///**/   ///*reuseCI*/CI = 0;
   ///**/   return 0;
   // Note: We have a valid compiler instance at this point.
   clang::TranslationUnitDecl* tu =
      CI->getASTContext().getTranslationUnitDecl();
   if (!tu) { // Parse failed, return.
      fprintf(stderr, "Wrapped parse failed, no translation unit!\n");
      ///*reuseCI*/CI->takeLLVMContext();
      ///*reuseCI*/delete CI;
      ///*reuseCI*/CI = 0;
      return 0;
   }
   //
   //  Send the translation unit through the
   //  llvm code generator to make a module.
   //
   llvm::Module* m = doCodegen(CI, "CLING");
   if (!m) {
      fprintf(stderr, "Module creation failed!\n");
      ///*reuseCI*/CI->takeLLVMContext();
      ///*reuseCI*/delete CI;
      ///*reuseCI*/CI = 0;
      return 0;
   }
   //
   //  All done with the compiler instance,
   //  get rid of it.
   //
   ///*reuseCI*/CI->takeLLVMContext();
   ///*reuseCI*/delete CI;
   ///*reuseCI*/CI = 0;
   //printModule(m);
   return m;
}

void
Interpreter::createWrappedSrc(const std::string& src, std::string& wrapped)
{
   std::vector<clang::Stmt*> stmts;
   clang::CompilerInstance* CI = createStatementList(src, stmts);
   if (!CI) {
      wrapped.clear();
      return;
   }
   //
   //  Rewrite the source code to support cint command
   //  line semantics.  We must move variable declarations
   //  to the global namespace and change the code so that
   //  the new global variables are used.
   //
   std::string held_globals;
   std::string wrapped_globals;
   std::string wrapped_stmts;
   {
      clang::SourceManager& SM = CI->getSourceManager();
      const clang::LangOptions& LO = CI->getLangOpts();
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
            //fprintf(stderr, "stmt: %s\n", stmt_string.c_str());
         }
         //
         //  Handle expression statements.
         //
         {
            const clang::Expr* expr = dyn_cast<clang::Expr>(cur_stmt);
            if (expr) {
               //fprintf(stderr, "have expr stmt.\n");
               wrapped_stmts.append(stmt_string + '\n');
               continue;
            }
         }
         //
         //  Handle everything that is not a declaration statement.
         //
         const clang::DeclStmt* DS = dyn_cast<clang::DeclStmt>(cur_stmt);
         if (!DS) {
            //fprintf(stderr, "not expr, not declaration.\n");
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
               if (DS->isSingleDecl()) {
                  //fprintf(stderr, "decl, not var decl, single decl.\n");
                  wrapped_globals.append(stmt_string + '\n');
                  held_globals.append(stmt_string + '\n');
                  continue;
               }
               //fprintf(stderr, "decl, not var decl, not single decl.\n");
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
                  //fprintf(stderr, "var decl, no init.\n");
               }
               else {
                  //fprintf(stderr, "var decl, init is constructor.\n");
               }
               wrapped_globals.append(decl + ";\n"); // FIXME: wrong for constructor
               held_globals.append(decl + ";\n");
               continue;
            }
            //
            //  Handle variable declarators with a constant initializer.
            //
            if (I->isConstantInitializer(CI->getASTContext())) {
               //fprintf(stderr, "var decl, init is const.\n");
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
               //fprintf(stderr, "var decl, init is not list.\n");
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
            //fprintf(stderr, "var decl, init is list.\n");
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
   //fprintf(stderr, "m_globalDeclarations:\n%s\n",
   //   m_globalDeclarations.c_str());
   //fprintf(stderr, "held_globals:\n%s\n", held_globals.c_str());
   //fprintf(stderr, "---\n");
   //fprintf(stderr, "wrapped_globals:\n%s\n", wrapped_globals.c_str());
   //fprintf(stderr, "wrapped_stmts:\n%s\n", wrapped_stmts.c_str());
   wrapped += m_globalDeclarations + wrapped_globals + wrapped_stmts;
   // Accumulate the held global declarations for the next run.
   m_globalDeclarations.append(held_globals + "\n");
   //
   //  Shutdown parse.
   //
   CI->setASTConsumer(0);
   CI->setASTContext(0);
   //if (CI->hasPreprocessor()) {
   //   CI->getPreprocessor().EndSourceFile();
   //}
   //CI->clearOutputFiles(/*EraseFiles=*/CI->getDiagnostics().getNumErrors());
   //CI->getDiagnosticClient().EndSourceFile();
   unsigned err_count = CI->getDiagnostics().getNumErrors();
   if (err_count) {
      wrapped.clear();
      ///*reuseCI*/CI->takeLLVMContext();
      ///*reuseCI*/delete CI;
      ///*reuseCI*/CI = 0;
      return;
   }
   ///*reuseCI*/CI->takeLLVMContext();
   ///*reuseCI*/delete CI;
   ///*reuseCI*/CI = 0;
}

clang::CompilerInstance*
Interpreter::createStatementList(const std::string& srcCode,
                                 std::vector<clang::Stmt*>& stmts)
{
   clang::CompilerInstance* CI = getCI();
   if (!CI) {
      return 0;
   }
   CI->createPreprocessor();
   clang::Preprocessor& PP = CI->getPreprocessor();
   PP.addPPCallbacks(new MacroDetector(*CI, m_globalDeclarations.size()));
   CI->getDiagnosticClient().BeginSourceFile(CI->getLangOpts(), &PP);
   //CI->createASTContext();
   CI->setASTContext(new clang::ASTContext(CI->getLangOpts(),
      PP.getSourceManager(), CI->getTarget(), PP.getIdentifierTable(),
      PP.getSelectorTable(), PP.getBuiltinInfo(), false, 0));
   // Create an ASTConsumer for this frontend run which
   // will produce a list of statements seen.
   StmtSplitter splitter(stmts);
   FunctionBodyConsumer* consumer =
      new FunctionBodyConsumer(splitter, "__cling_internal");
   CI->setASTConsumer(consumer);
   PP.getBuiltinInfo().InitializeBuiltins(PP.getIdentifierTable(),
                                          PP.getLangOptions().NoBuiltin);
   llvm::MemoryBuffer* SB =
      llvm::MemoryBuffer::getMemBufferCopy(srcCode, "CLING");
   if (!SB) {
      fprintf(stderr, "Interpreter::createStatementList: Failed to create "
                      "memory buffer!\n");
      ///*reuseCI*/CI->takeLLVMContext();
      ///*reuseCI*/delete CI;
      ///*reuseCI*/CI = 0;
      return 0;
   }
   CI->getSourceManager().clearIDTables();
   CI->getSourceManager().createMainFileIDForMemBuffer(SB);
   if (CI->getSourceManager().getMainFileID().isInvalid()) {
      fprintf(stderr, "Interpreter::createStatementList: Failed to create "
                      "main file id!\n");
      ///*reuseCI*/CI->takeLLVMContext();
      ///*reuseCI*/delete CI;
      ///*reuseCI*/CI = 0;
      return 0;
   }
   clang::ParseAST(PP, &CI->getASTConsumer(), CI->getASTContext());
   //CI->setASTConsumer(0); // We still need these later.
   //CI->setASTContext(0); // We still need these later.
   if (CI->hasPreprocessor()) {
      CI->getPreprocessor().EndSourceFile();
   }
   CI->clearOutputFiles(/*EraseFiles=*/CI->getDiagnostics().getNumErrors());
   CI->getDiagnosticClient().EndSourceFile();
   unsigned err_count = CI->getDiagnostics().getNumErrors();
   if (err_count) {
      fprintf(stderr, "Interpreter::createStatementList: Parse failed!\n");
      CI->setASTConsumer(0);
      CI->setASTContext(0);
      ///*reuseCI*/CI->takeLLVMContext();
      ///*reuseCI*/delete CI;
      ///*reuseCI*/CI = 0;
      return 0;
   }
   return CI;
}

clang::CompilerInstance*
Interpreter::createCI(const char* llvmdir /*=0*/)
{
   //
   //  Create and setup a compiler instance.
   //
   clang::CompilerInstance* CI = new clang::CompilerInstance();
   //bool first_time = true;
   CI->setLLVMContext(m_llvm_context);
   {
      //
      //  Buffer the error messages while we process
      //  the compiler options.
      //
      clang::TextDiagnosticBuffer DiagsBuffer;
      clang::Diagnostic Diags(&DiagsBuffer);
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
         } else {
            // FIXME: The first arg really does need to be argv[0] on FreeBSD.
            //
            // Note: The second arg is not used for Apple, FreeBSD, Linux,
            //       or cygwin, and can only be used on systems which support
            //       the use of dladdr().
            //
            // Note: On linux and cygwin this uses /proc/self/exe to find the path.
            //
            // Note: On Apple it uses _NSGetExecutablePath().
            //
            // Note: On FreeBSD it uses getprogpath().
            //
            // Note: Otherwise it uses dladdr().
            //
            CI->getHeaderSearchOpts().ResourceDir =
               clang::CompilerInvocation::GetResourcesPath("cling",
                                                           (void*)(intptr_t) locate_cling_executable);
         }
      }
      CI->createDiagnostics(fake_argc - 1, const_cast<char**>(fake_argv + 1));
      if (!CI->hasDiagnostics()) {
         CI->takeLLVMContext();
         delete CI;
         CI = 0;
         return 0;
      }
      // Output the buffered error messages now.
      DiagsBuffer.FlushDiagnostics(CI->getDiagnostics());
      if (CI->getDiagnostics().getNumErrors()) {
         CI->takeLLVMContext();
         delete CI;
         CI = 0;
         return 0;
      }
   }
   CI->setTarget(clang::TargetInfo::CreateTargetInfo(CI->getDiagnostics(),
                 CI->getTargetOpts()));
   if (!CI->hasTarget()) {
      CI->takeLLVMContext();
      delete CI;
      CI = 0;
      return 0;
   }
   CI->getTarget().setForcedLangOptions(CI->getLangOpts());
   CI->createFileManager();
   //
   //  If we are managing a permanent CI,
   //  the code looks like this:
   //
   //if (first_time) {
   //   CI->createSourceManager();
   //   first_time = false;
   //}
   //else {
   //   CI->getSourceManager().clearIDTables();
   //}
   CI->createSourceManager();
   //CI->createPreprocessor(); // Note: This single line takes almost all the time!
   return CI;
}

clang::CompilerInstance*
Interpreter::getCI()
{
   if (!m_CI) {
      return 0;
   }
   m_CI->createDiagnostics(fake_argc - 1, const_cast<char**>(fake_argv + 1));
   if (!m_CI->hasDiagnostics()) {
      m_CI->takeLLVMContext();
      delete m_CI;
      m_CI = 0;
      return 0;
   }
   //m_CI->getSourceManager().clearIDTables();
   return m_CI;
}

clang::ASTConsumer*
Interpreter::maybeGenerateASTPrinter() const
{
   if (m_printAST) {
      return clang::CreateASTPrinter(&llvm::outs());
   }
   return new clang::ASTConsumer();
}

clang::CompilerInstance*
Interpreter::compileString(const std::string& srcCode)
{
   clang::CompilerInstance* CI = getCI();
   if (!CI) {
      return 0;
   }
   CI->createPreprocessor();
   clang::Preprocessor& PP = CI->getPreprocessor();
   CI->getDiagnosticClient().BeginSourceFile(CI->getLangOpts(), &PP);
   //CI->createASTContext();
   CI->setASTContext(new clang::ASTContext(CI->getLangOpts(),
      PP.getSourceManager(), CI->getTarget(), PP.getIdentifierTable(),
      PP.getSelectorTable(), PP.getBuiltinInfo(), false, 0));
   CI->setASTConsumer(maybeGenerateASTPrinter());
   PP.getBuiltinInfo().InitializeBuiltins(PP.getIdentifierTable(),
                                          PP.getLangOptions().NoBuiltin);
   llvm::MemoryBuffer* SB =
      llvm::MemoryBuffer::getMemBufferCopy(srcCode, "CLING");
   if (!SB) {
      fprintf(stderr, "Interpreter::compileString: Failed to create memory "
                      "buffer!\n");
      ///*reuseCI*/CI->takeLLVMContext();
      ///*reuseCI*/delete CI;
      ///*reuseCI*/CI = 0;
      return 0;
   }
   CI->getSourceManager().clearIDTables();
   CI->getSourceManager().createMainFileIDForMemBuffer(SB);
   if (CI->getSourceManager().getMainFileID().isInvalid()) {
      fprintf(stderr, "Interpreter::compileString: Failed to create main "
                      "file id!\n");
      ///*reuseCI*/CI->takeLLVMContext();
      ///*reuseCI*/delete CI;
      ///*reuseCI*/CI = 0;
      return 0;
   }
   clang::ParseAST(PP, &CI->getASTConsumer(), CI->getASTContext());
   CI->setASTConsumer(0);
   //CI->setASTContext(0); // Caller still needs this.
   if (CI->hasPreprocessor()) {
      CI->getPreprocessor().EndSourceFile();
   }
   CI->clearOutputFiles(/*EraseFiles=*/CI->getDiagnostics().getNumErrors());
   CI->getDiagnosticClient().EndSourceFile();
   unsigned err_count = CI->getDiagnostics().getNumErrors();
   if (err_count) {
      fprintf(stderr, "Interpreter::compileString: Parse failed!\n");
      ///*reuseCI*/CI->takeLLVMContext();
      ///*reuseCI*/delete CI;
      ///*reuseCI*/CI = 0;
      return 0;
   }
   return CI;
}

clang::CompilerInstance*
Interpreter::compileFile(const std::string& filename, const std::string* trailcode /*=0*/)
{
   std::string code(m_globalDeclarations);
   code += "#define __CLING__MAIN_FILE_INCLUSION_MARKER \"" + filename + "\"\n";
   code += "#include \"" + filename + "\"\n";
   code += "#undef __CLING__MAIN_FILE_INCLUSION_MARKER\n";
   if (trailcode) code += *trailcode;
   return compileString(code);
}

llvm::Module*
Interpreter::doCodegen(clang::CompilerInstance* CI, const std::string& filename)
{
   clang::TranslationUnitDecl* tu =
      CI->getASTContext().getTranslationUnitDecl();
   if (!tu) {
      fprintf(
           stderr
         , "Interpreter::doCodegen: No translation unit decl in passed "
           "ASTContext!\n"
      );
      return 0;
   }
   llvm::OwningPtr<clang::CodeGenerator> codeGen(
      CreateLLVMCodeGen(CI->getDiagnostics(), filename, CI->getCodeGenOpts(),
                        CI->getLLVMContext()));
   codeGen->Initialize(CI->getASTContext());
   clang::TranslationUnitDecl::decl_iterator iter = tu->decls_begin();
   clang::TranslationUnitDecl::decl_iterator iter_end = tu->decls_end();
   //fprintf(stderr, "Running code generation.\n");
   for (; iter != iter_end; ++iter) {
      codeGen->HandleTopLevelDecl(clang::DeclGroupRef(*iter));
   }
   codeGen->HandleTranslationUnit(CI->getASTContext());
   //fprintf(stderr, "Finished code generation.\n");
   llvm::Module* m = codeGen->ReleaseModule();
   if (!m) {
      fprintf(
           stderr
         , "Interpreter::doCodegen: Code generation did not create a module!\n"
      );
      return 0;
   }
   return m;
}

int
Interpreter::verifyModule(llvm::Module* m)
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
Interpreter::printModule(llvm::Module* m)
{
   //
   //  Print module LLVM code in human-readable form.
   //
   llvm::PassManager PM;
   PM.add(llvm::createPrintModulePass(&llvm::outs()));
   PM.run(*m);
}

void
Interpreter::executeCommandLine()
{
   //fprintf(stderr, "Running generated code with JIT.\n");
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
   if (!f) {
      fprintf(
           stderr
         , "Interpreter::executeCommandLine: Could not find the "
           "__cling_internal() function!\n"
      );
      return;
   }
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

llvm::sys::Path
Interpreter::findDynamicLibrary(const std::string& filename,
                                bool addPrefix /* = true */,
                                bool addSuffix /* = true */) const
{
   // Check wether filename is a dynamic library, either through absolute path
   // or in one of the system library paths.
   {
      llvm::sys::Path FullPath(filename);
      if (FullPath.isDynamicLibrary())
         return FullPath;
   }

   std::vector<llvm::sys::Path> LibPaths;
   llvm::sys::Path::GetSystemLibraryPaths(LibPaths);
   for (unsigned i = 0; i < LibPaths.size(); ++i) {
      llvm::sys::Path FullPath(LibPaths[i]);
      FullPath.appendComponent(filename);
      if (FullPath.isDynamicLibrary())
         return FullPath;
   }

   if (addPrefix) {
      static const std::string prefix("lib");
      llvm::sys::Path found = findDynamicLibrary(prefix + filename, false, addSuffix);
      if (found.isDynamicLibrary())
         return found;
   }

   if (addSuffix) {
      llvm::sys::Path found = findDynamicLibrary(filename + LTDL_SHLIB_EXT, false, false);
      if (found.isDynamicLibrary())
         return found;
   }

   return llvm::sys::Path();
}

int
Interpreter::loadFile(const std::string& filename, const std::string* trailcode /*=0*/)
{
   llvm::sys::Path DynLib = findDynamicLibrary(filename);
   if (DynLib.isDynamicLibrary()) {
      std::string errMsg;
      bool err =
         llvm::sys::DynamicLibrary::LoadLibraryPermanently(DynLib.str().c_str(), &errMsg);
      if (err) {
         //llvm::errs() << "Could not load shared library: " << errMsg << '\n';
         fprintf(stderr
                 , "Interpreter::loadFile: Could not load shared library!\n"
                 );
         fprintf(stderr, "%s\n", errMsg.c_str());
         return 1;
      }
      return 0;
   }
   clang::CompilerInstance* CI = compileFile(filename, trailcode);
   if (!CI) {
      return 1;
   }
   clang::TranslationUnitDecl* tu =
      CI->getASTContext().getTranslationUnitDecl();
   if (!tu) { // Parse failed, return.
      fprintf(
           stderr
         , "Interpreter::loadFile: No translation unit decl found!\n"
      );
      ///*reuseCI*/CI->takeLLVMContext();
      ///*reuseCI*/delete CI;
      ///*reuseCI*/CI = 0;
      return 1;
   }
   llvm::Module* m = doCodegen(CI, filename);
   if (!m) {
      //fprintf(stderr, "Error: Backend did not create a module!\n");
      ///*reuseCI*/CI->takeLLVMContext();
      ///*reuseCI*/delete CI;
      ///*reuseCI*/CI = 0;
      return 1;
   }
   //--
   //llvm::Linker linker("executable", llvm::CloneModule(m_prev_module));
   //if (linker.LinkInModule(m, errMsg)) {
   //   m = linker.releaseModule();
   //   delete m;
   //   m = 0;
   //   return 0;
   //}
   //m = linker.releaseModule();
   //
   //  Transfer global mappings from previous module.
   //
   //copyGlobalMappings(m_engine, m_prev_module, m);
   //
   //  All done with previous module, delete it.
   //
   //{
   //   bool ok = m_engine->removeModule(m_prev_module);
   //   if (!ok) {
   //      //fprintf(stderr, "Previous module not found in execution engine!\n");
   //   }
   //   delete m_prev_module;
   //   m_prev_module = 0;
   //}
   //--
   //
   //  Give new module to the execution engine.
   //
   m_engine->addModule(m); // Note: The engine takes ownership of the module.
   ///*reuseCI*/CI->takeLLVMContext();
   ///*reuseCI*/delete CI;
   ///*reuseCI*/CI = 0;
   return 0;
}

int
Interpreter::executeFile(const std::string& filename)
{
   std::string::size_type pos = filename.find_last_of('/');
   if (pos == std::string::npos) {
      pos = 0;
   }
   else {
      ++pos;
   }

   // Note: We are assuming the filename does not end in slash here.
   std::string funcname(filename, pos);
   std::string::size_type endFileName = std::string::npos;

   std::string args;
   pos = funcname.find_first_of('(');
   if (pos != std::string::npos) {
      std::string::size_type posParamsEnd = funcname.find_last_of(')');
      if (posParamsEnd != std::string::npos) {
         args = funcname.substr(pos, posParamsEnd - pos + 1);
         endFileName = filename.find_first_of('(');
      }
   }

   //fprintf(stderr, "funcname: %s\n", funcname.c_str());
   pos = funcname.find_last_of('.');
   if (pos != std::string::npos) {
      funcname.erase(pos);
      //fprintf(stderr, "funcname: %s\n", funcname.c_str());
   }

   std::ostringstream swrappername;
   swrappername << "__cling__internal_wrapper" << m_numCallWrappers++;
   std::string wrapper = "extern \"C\" void ";
   wrapper += swrappername.str() + "() {\n  " + funcname + "(" + args + ");\n}";
   int err = loadFile(filename.substr(0, endFileName), &wrapper);
   if (err) {
      return err;
   }
   executeFunction(swrappername.str());
   return 0;
}

void
Interpreter::executeFunction(const std::string& funcname)
{
   // Call an extern C function without arguments
   llvm::Function* f = m_engine->FindFunctionNamed(funcname.c_str());
   if (!f) {
      fprintf(
           stderr
         , "Interpreter::executeFunction: Could not find function named: %s\n"
         , funcname.c_str()
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

} // namespace cling

