//------------------------------------------------------------------------------
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Lukasz Janyst <ljanyst@cern.ch>
//------------------------------------------------------------------------------

#include "cling/Interpreter/Interpreter.h"

#include "cling/Interpreter/CIBuilder.h"

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclGroup.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclGroup.h"
#include "clang/AST/ExprCXX.h"
#include "clang/AST/Expr.h"
#include "clang/AST/Stmt.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Lex/HeaderSearch.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Parse/Parser.h"
#include "clang/Parse/ParseAST.h"
#include "clang/Sema/SemaConsumer.h"
//#include "../../clang/lib/Sema/Sema.h"
#include "llvm/Constants.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/System/DynamicLibrary.h"
#include "llvm/System/Path.h"

#include "Visitors.h"
#include "ClangUtils.h"
#include "ExecutionContext.h"

#include "DependentNodesTransform.h"

#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

static const char* fake_argv[] = { "clang", "-x", "c++", "-D__CLING__", 0 };
static const int fake_argc = (sizeof(fake_argv) / sizeof(const char*)) - 1;

namespace {
static
llvm::sys::Path
findDynamicLibrary(const std::string& filename,
                   bool addPrefix = true,
                   bool addSuffix = true)
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
      llvm::sys::Path found
         = findDynamicLibrary(filename + llvm::sys::Path::GetDLLSuffix().str(),
                              false, false);
      if (found.isDynamicLibrary())
         return found;
   }

   return llvm::sys::Path();
}


}

namespace cling {

//
//  Interpreter
//

//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------
Interpreter::Interpreter(const char* llvmdir /*= 0*/):
   m_CIBuilder(0),
   m_CI(0),
   m_UniqueCounter(0),
  m_printAST(false)
{
   m_CIBuilder.reset(new CIBuilder(fake_argc, fake_argv, llvmdir));
   m_CI.reset(createCI());

   m_ExecutionContext.reset(new ExecutionContext(*this));
   compileString("#include <stdio.h>\n");
}

//---------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------
Interpreter::~Interpreter()
{
   //delete m_prev_module;
   //m_prev_module = 0; // Don't do this, the engine does it.

   m_CI->takeLLVMContext(); // Don't take down the context with the CI.
}

clang::CompilerInstance*
Interpreter::createCI()
{
   return m_CIBuilder->createCI();
}

int
Interpreter::processLine(const std::string& input_line)
{
   //
   //  Transform the input line to implement cint
   //  command line semantics (declarations are global),
   //  and compile to produce a module.
   //

   //
   //  Wrap input into a function along with
   //  the saved global declarations.
   //
   //fprintf(stderr, "input_line:\n%s\n", src.c_str());
   std::string wrapped;
   std::string stmtFunc;
   createWrappedSrc(input_line, wrapped, stmtFunc);
   if (!wrapped.size()) {
      return 0;
   }
   //
   //  Send the wrapped code through the
   //  frontend to produce a translation unit.
   //
   clang::CompilerInstance* CI = compileString(wrapped);
   if (!CI) {
      return 0;
   }
   // Note: We have a valid compiler instance at this point.
   clang::TranslationUnitDecl* tu =
      CI->getASTContext().getTranslationUnitDecl();
   if (!tu) { // Parse failed, return.
      fprintf(stderr, "Wrapped parse failed, no translation unit!\n");
      return 0;
   }
   //
   //  Send the translation unit through the
   //  llvm code generator to make a module.
   //
   if (!m_ExecutionContext->doCodegen(CI, "Interpreter::processLine() input")) {
      fprintf(stderr, "Module creation failed!\n");
      return 0;
   }
   //
   //  Run it using the JIT.
   //
   if (!stmtFunc.empty())
      m_ExecutionContext->executeFunction(stmtFunc);
   return 1;
}

std::string Interpreter::createUniqueName()
{
   // Create an unique name

   std::ostringstream swrappername;
   swrappername << "__cling_Un1Qu3" << m_UniqueCounter++;
   return swrappername.str();
}



void
   Interpreter::createWrappedSrc(const std::string& src, std::string& wrapped,
                                 std::string& stmtFunc)
{
   bool haveStatements = false;
   std::string nonTUsrc = "void __cling__nonTUsrc() {" + src + ";}";
   std::vector<clang::Stmt*> stmts;
   clang::CompilerInstance* CI = createStatementList(nonTUsrc, stmts);
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
      std::vector<clang::Stmt*>::iterator stmt_iter = stmts.begin();
      std::vector<clang::Stmt*>::iterator stmt_end = stmts.end();
      for (; stmt_iter != stmt_end; ++stmt_iter) {
         clang::Stmt* cur_stmt = *stmt_iter;
         std::string stmt_string;
         {
            std::pair<unsigned, unsigned> r =
               getStmtRangeWithSemicolon(cur_stmt, SM, LO);
            stmt_string = nonTUsrc.substr(r.first, r.second - r.first);
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
               std::string decl = nonTUsrc.substr(r.first, r.second - r.first);
               wrapped_globals.append(decl + ";\n");
               held_globals.append(decl + ";\n");
               continue;
            }
            //
            //  Handle a variable declarator.
            //
            std::string decl = VD->getNameAsString();
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
            if (I->isConstantInitializer(CI->getASTContext(), false)) {
               //fprintf(stderr, "var decl, init is const.\n");
               std::pair<unsigned, unsigned> r = getStmtRange(I, SM, LO);
               wrapped_globals.append(decl + " = " +
                                      nonTUsrc.substr(r.first, r.second - r.first) + ";\n");
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
                                    nonTUsrc.substr(r.first, r.second - r.first) + ";\n");
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
               stm << VD->getNameAsString() << "[" << j << "] = ";
               std::pair<unsigned, unsigned> r =
                  getStmtRange(ILE->getInit(j), SM, LO);
               stm << nonTUsrc.substr(r.first, r.second - r.first) << ";\n";
               wrapped_stmts.append(stm.str());
            }
            wrapped_globals.append(decl + ";\n");
            held_globals.append(decl + ";\n");
         }
      }
      haveStatements = !wrapped_stmts.empty();
      if (haveStatements) {
         stmtFunc = createUniqueName();
         wrapped_stmts = "extern \"C\" void " + stmtFunc + "() {\n" + wrapped_stmts;
         wrapped_stmts += "\n}";
      } else {
         stmtFunc = "";
      }
   }
   //
   //fprintf(stderr, "m_globalDeclarations:\n%s\n",
   //   m_globalDeclarations.c_str());
   //fprintf(stderr, "held_globals:\n%s\n", held_globals.c_str());
   //fprintf(stderr, "---\n");
   //fprintf(stderr, "wrapped_globals:\n%s\n", wrapped_globals.c_str());
   //fprintf(stderr, "wrapped_stmts:\n%s\n", wrapped_stmts.c_str());
   wrapped += wrapped_globals + wrapped_stmts;
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
      return;
   }
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
   CI->getDiagnosticClient().BeginSourceFile(CI->getLangOpts(), &PP);
   //CI->createASTContext();
   CI->setASTContext(new clang::ASTContext(CI->getLangOpts(),
      PP.getSourceManager(), CI->getTarget(), PP.getIdentifierTable(),
      PP.getSelectorTable(), PP.getBuiltinInfo(), 0));
   // Create an ASTConsumer for this frontend run which
   // will produce a list of statements seen.
   StmtSplitter splitter(stmts);
   FunctionBodyConsumer* consumer =
      new FunctionBodyConsumer(splitter, "__cling__nonTUsrc");
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

clang::ASTConsumer*
Interpreter::maybeGenerateASTPrinter() const
{
   if (m_printAST) {
      return clang::CreateASTDumper();
   }
   return new clang::ASTConsumer();
}

namespace {
   class MutableMemoryBuffer: public llvm::MemoryBuffer {
      std::string m_FileID;
      size_t m_Alloc;
   protected:
      void maybeRealloc(llvm::StringRef code, size_t oldlen) {
         size_t applen = code.size();
         char* B = 0;
         if (oldlen) {
            B = const_cast<char*>(getBufferStart());
            assert(!B[oldlen] && "old buffer is not 0 terminated!");
            // B + oldlen points to trailing '\0'
         }
         size_t newlen = oldlen + applen + 1;
         if (newlen > m_Alloc) {
            m_Alloc += 64*1024;
            B = (char*)realloc(B, m_Alloc);
         }
         memcpy(B + oldlen, code.data(), applen);
         B[newlen - 1] = 0;
         init(B, B + newlen - 1);
      }
      
   public:
      MutableMemoryBuffer(llvm::StringRef Code, llvm::StringRef Name)
         : MemoryBuffer(), m_FileID(Name), m_Alloc(0) {
         maybeRealloc(Code, 0);
      }

      virtual ~MutableMemoryBuffer() {
         free((void*)getBufferStart());
      }

      void append(llvm::StringRef code) {
         assert(getBufferSize() && "buffer is empty!");
         maybeRealloc(code, getBufferSize());
      }
      virtual const char *getBufferIdentifier() const {
         return m_FileID.c_str();
      }
   };
}


clang::CompilerInstance*
Interpreter::compileString(const std::string& argCode)
{
   clang::CompilerInstance* CI = m_ExecutionContext->getCI();
   if (!CI) {
      return 0;
   }

   static clang::Sema* S = 0;
   static clang::Parser* P = 0;
   static clang::Lexer* L = 0;
   static MutableMemoryBuffer* MMB = 0;
   if (!S) {
      CI->createPreprocessor();
   }

   clang::Preprocessor& PP = CI->getPreprocessor();
   clang::ASTConsumer *Consumer = 0;

   std::string endOfInputName = "__cling_EndOfInput_" + createUniqueName();
   std::string srcCode(argCode);
   srcCode += "\nvoid " + endOfInputName + "(){};\n";

   if (!S) {
      CI->getDiagnosticClient().BeginSourceFile(CI->getLangOpts(), &PP);
      clang::ASTContext *Ctx = new clang::ASTContext(CI->getLangOpts(),
         PP.getSourceManager(), CI->getTarget(), PP.getIdentifierTable(),
         PP.getSelectorTable(), PP.getBuiltinInfo(), 0);
      CI->setASTContext(Ctx);
      CI->setASTConsumer(maybeGenerateASTPrinter());
      Consumer = &CI->getASTConsumer();
      PP.getBuiltinInfo().InitializeBuiltins(PP.getIdentifierTable(),
                                             PP.getLangOptions().NoBuiltin);
      //llvm::MemoryBuffer* SB =
      //   llvm::MemoryBuffer::getMemBufferCopy(srcCode, "CLING");
      MMB = new MutableMemoryBuffer(srcCode, "CLING");
      if (!MMB) {
         fprintf(stderr, "Interpreter::compileString: Failed to create memory "
                 "buffer!\n");
         ///*reuseCI*/CI->takeLLVMContext();
         ///*reuseCI*/delete CI;
         ///*reuseCI*/CI = 0;
         return 0;
      }

      CI->getSourceManager().clearIDTables();
      CI->getSourceManager().createMainFileIDForMemBuffer(MMB);
      if (CI->getSourceManager().getMainFileID().isInvalid()) {
         fprintf(stderr, "Interpreter::compileString: Failed to create main "
                 "file id!\n");
         ///*reuseCI*/CI->takeLLVMContext();
         ///*reuseCI*/delete CI;
         ///*reuseCI*/CI = 0;
         return 0;
      }

      bool CompleteTranslationUnit = false;
      clang::CodeCompleteConsumer *CompletionConsumer = 0;
      S = new clang::Sema(PP, *Ctx, *Consumer, CompleteTranslationUnit, CompletionConsumer);
      PP.EnterMainSourceFile();
      L = static_cast<clang::Lexer*>(PP.getTopmostLexer());

      // Initialize the parser.
      P = new clang::Parser(PP, *S);
      P->Initialize();

      Consumer->Initialize(*Ctx);

      if (clang::SemaConsumer *SC = dyn_cast<clang::SemaConsumer>(Consumer))
         SC->InitializeSema(*S);
   } else {
      MMB->append(srcCode);
   }

   L->updateBufferEnd(MMB->getBufferEnd());
   // BEGIN REPLACEMENT clang::ParseAST(PP, &CI->getASTConsumer(), CI->getASTContext());

   if (!Consumer) Consumer = &CI->getASTConsumer();
   clang::Parser::DeclGroupPtrTy ADecl;
   while (!P->ParseTopLevelDecl(ADecl)) {  // Not end of file.
      // If we got a null return and something *was* parsed, ignore it.  This
      // is due to a top-level semicolon, an action override, or a parse error
      // skipping something.
      if (ADecl) {
         clang::DeclGroupRef DGR = ADecl.getAsVal<clang::DeclGroupRef>();
         Consumer->HandleTopLevelDecl(DGR);
         if (DGR.isSingleDecl()) {
            clang::FunctionDecl* FD = dyn_cast<clang::FunctionDecl>(DGR.getSingleDecl());
            if (FD && FD->getNameAsString() == endOfInputName)
               break;
         }
      }
   };

   // Process any TopLevelDecls generated by #pragma weak.
   for (llvm::SmallVector<clang::Decl*,2>::iterator
           I = S->WeakTopLevelDecls().begin(),
           E = S->WeakTopLevelDecls().end(); I != E; ++I)
      Consumer->HandleTopLevelDecl(clang::DeclGroupRef(*I));

   // Here we are substituting the dependent nodes with Cling invocations.
   DependentNodesTransform* transformer = new DependentNodesTransform();
   transformer->TransformNodes(S);
   delete transformer;

   clang::ASTContext *Ctx = &CI->getASTContext();
   Consumer->HandleTranslationUnit(*Ctx);

   //if (SemaConsumer *SC = dyn_cast<SemaConsumer>(Consumer))
   //   SC->ForgetSema();

   // END REPLACEMENT clang::ParseAST(PP, &CI->getASTConsumer(), CI->getASTContext());


   //CI->setASTConsumer(0);
   //if (CI->hasPreprocessor()) {
   //   CI->getPreprocessor().EndSourceFile();
   //}
   //CI->clearOutputFiles(/*EraseFiles=*/CI->getDiagnostics().getNumErrors());
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
Interpreter::compileFile(const std::string& filename,
                         const std::string* trailcode /*=0*/)
{
   std::string code;
   code += "#include \"" + filename + "\"\n";
   if (trailcode) code += *trailcode;
   return compileString(code);
}

static
bool tryLoadSharedLib(const std::string& filename)
{
   llvm::sys::Path DynLib = findDynamicLibrary(filename);
   if (!DynLib.isDynamicLibrary())
      return false;

   std::string errMsg;
   bool err =
      llvm::sys::DynamicLibrary::LoadLibraryPermanently(DynLib.str().c_str(), &errMsg);
   if (err) {
      //llvm::errs() << "Could not load shared library: " << errMsg << '\n';
      fprintf(stderr,
              "Interpreter::loadFile: Could not load shared library!\n");
      fprintf(stderr, "%s\n", errMsg.c_str());
      return false;
   }
   return true;
}

int
Interpreter::loadFile(const std::string& filename,
                      const std::string* trailcode /*=0*/,
                      bool allowSharedLib /*=true*/)
{
   if (allowSharedLib && tryLoadSharedLib(filename))
      return 0;

   clang::CompilerInstance* CI = compileFile(filename, trailcode);
   if (!CI) {
      return 1;
   }
   
   if (!m_ExecutionContext->doCodegen(CI, filename)) {
      fprintf(stderr, "Error: Backend did not create a module!\n");
      return 1;
   }
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

   std::string args("()"); // arguments with enclosing '(', ')'
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

   std::string func = createUniqueName();
   std::string wrapper = "extern \"C\" void " + func;
   wrapper += "(){\n" + funcname + args + ";\n}";
   int err = loadFile(filename.substr(0, endFileName), &wrapper);
   if (err) {
      return err;
   }
   m_ExecutionContext->executeFunction(func);
   return 0;
}


} // namespace cling

