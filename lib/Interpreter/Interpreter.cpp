//------------------------------------------------------------------------------
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Lukasz Janyst <ljanyst@cern.ch>
//------------------------------------------------------------------------------

#include "cling/Interpreter/Interpreter.h"

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclGroup.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ExprCXX.h"
#include "clang/AST/Mangle.h"
#include "clang/AST/Stmt.h"
#include "clang/AST/Type.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/CodeGen/ModuleBuilder.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/Utils.h"
#include "clang/Lex/HeaderSearch.h"
#include "clang/Lex/Pragma.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Sema/Scope.h"
#include "clang/Sema/Lookup.h"
#include "llvm/Constants.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/DynamicLibrary.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/raw_os_ostream.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "Visitors.h"
#include "ClangUtils.h"
#include "DynamicLookup.h"
#include "ExecutionContext.h"
#include "IncrementalParser.h"
#include "InputValidator.h"
#include "cling/Interpreter/InvocationOptions.h"
#include "cling/Interpreter/InterpreterCallbacks.h"
#include "cling/Interpreter/CIFactory.h"
#include "cling/Interpreter/Value.h"

#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using namespace clang;

namespace {
  static
  llvm::sys::Path
  findDynamicLibrary(const std::string& filename,
                     const cling::InvocationOptions& Opts,
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
    
    std::vector<llvm::sys::Path> LibPaths(Opts.LibSearchPath.begin(),
                                          Opts.LibSearchPath.end());
    std::vector<llvm::sys::Path> SysLibPaths;
    llvm::sys::Path::GetSystemLibraryPaths(SysLibPaths);
    LibPaths.insert(LibPaths.end(), SysLibPaths.begin(), SysLibPaths.end());
    for (unsigned i = 0; i < LibPaths.size(); ++i) {
      llvm::sys::Path FullPath(LibPaths[i]);
      FullPath.appendComponent(filename);
      if (FullPath.isDynamicLibrary())
        return FullPath;
    }
    
    if (addPrefix) {
      static const std::string prefix("lib");
      llvm::sys::Path found = findDynamicLibrary(prefix + filename, Opts,
                                                 false, addSuffix);
      if (found.isDynamicLibrary())
        return found;
    }
    
    if (addSuffix) {
      llvm::sys::Path found
      = findDynamicLibrary(filename + llvm::sys::Path::GetDLLSuffix().str(),
                           Opts, false, false);
      if (found.isDynamicLibrary())
        return found;
    }
    
    return llvm::sys::Path();
  }
  
}
namespace {
   class ASTTLDPrinter : public ASTConsumer {
      llvm::raw_ostream &Out;
      bool Dump;

   public:
      ASTTLDPrinter(llvm::raw_ostream* o = NULL, bool Dump = false)
  : Out(o? *o : llvm::outs()), Dump(Dump) { }

      virtual void HandleTopLevelDecl(DeclGroupRef D) {
         for (DeclGroupRef::iterator I = D.begin(), E = D.end(); I != E; ++I)
            HandleTopLevelSingleDecl(*I);
      }
      
      void HandleTopLevelSingleDecl(Decl *D) {
         PrintingPolicy Policy = D->getASTContext().PrintingPolicy;
         Policy.Dump = Dump;

         if (isa<FunctionDecl>(D) || isa<ObjCMethodDecl>(D)) {
            D->dump();
            
            if (Stmt *Body = D->getBody()) {
               llvm::errs() << "DeclStmts:---------------------------------\n";
               Body->dump();
               llvm::errs() << "End DeclStmts:-----------------------------\n\n\n\n";
            }
         }
      }
   };
} // end anonymous namespace

namespace cling {

  Interpreter::NamedDeclResult::NamedDeclResult(llvm::StringRef Decl, 
                                                Interpreter* interp, 
                                                DeclContext* Within)
    : m_Interpreter(interp),
      m_Context(m_Interpreter->getCI()->getASTContext()),
      m_CurDeclContext(Within),
      m_Result(0)
  {
    LookupDecl(Decl);
  }

  Interpreter::NamedDeclResult&
  Interpreter::NamedDeclResult::LookupDecl(llvm::StringRef Decl) {
    DeclarationName Name(&m_Context.Idents.get(Decl));
    DeclContext::lookup_result Lookup = m_CurDeclContext->lookup(Name);
    // FIXME: We need to traverse over each found result in the pair in order to
    // solve possible ambiguities.
    if (Lookup.first != Lookup.second) {
      if (DeclContext* DC = dyn_cast<DeclContext>(*Lookup.first))
        m_CurDeclContext = DC;
      else
        m_CurDeclContext = (*Lookup.first)->getDeclContext();
      
      m_Result = (*Lookup.first);
    }
    else {
      // TODO: Find the template instantiations with using a wrapper (getQualType). 
        m_Result = 0;
    }

    return *this;
  }

  NamedDecl* Interpreter::NamedDeclResult::getSingleDecl() const {
    // TODO: Check whether it is only one decl if (end-begin == 1 )
    return dyn_cast<NamedDecl>(m_Result);
  }

  //
  //  Interpreter
  //
  
  //---------------------------------------------------------------------------
  // Constructor
  //---------------------------------------------------------------------------
   Interpreter::Interpreter(int argc, const char* const *argv,
                            const char* startupPCH /*= 0*/,
                            const char* llvmdir /*= 0*/):
  m_UniqueCounter(0),
  m_printAST(false),
  m_LastDump(0),
  m_ASTDumper(0)
  {
    m_PragmaHandler = new PragmaNamespace("cling");

    std::vector<unsigned> LeftoverArgsIdx;
    m_Opts = InvocationOptions::CreateFromArgs(argc, argv, LeftoverArgsIdx);
    std::vector<const char*> LeftoverArgs;

    // We do C++ by default:
    LeftoverArgs.push_back("-x");
    LeftoverArgs.push_back("c++");

    for (size_t I = 0, N = LeftoverArgsIdx.size(); I < N; ++I) {
      LeftoverArgs.push_back(argv[LeftoverArgsIdx[I]]);
    }
 
    m_IncrParser.reset(new IncrementalParser(this, &getPragmaHandler(),
                                             LeftoverArgs.size(), &LeftoverArgs[0],
                                             llvmdir));
    m_ExecutionContext.reset(new ExecutionContext(m_IncrParser->getCI()));
    m_IncrParser->addConsumer(IncrementalParser::kCodeGenerator,
                              m_ExecutionContext->getCodeGenerator());
    
    m_InputValidator.reset(new InputValidator(CIFactory::createCI("//cling InputSanitizer",
                                                                  LeftoverArgs.size(), &LeftoverArgs[0],
                                                                  llvmdir)));

    m_ValuePrintStream.reset(new llvm::raw_os_ostream(std::cout));

    // Allow the interpreter to find itself.
    // OBJ first: if it exists it should be more up to date
    AddIncludePath(CLING_SRCDIR_INCL);
    AddIncludePath(CLING_INSTDIR_INCL);

    // Warm them up
    m_IncrParser->Initialize(startupPCH);
    if (m_IncrParser->usingStartupPCH()) {
      processStartupPCH();
    }

    // Set up the gCling variable - even if we use PCH ('this' is different)
    std::stringstream initializer;
    initializer << "gCling=(cling::Interpreter*)" << this <<";\n";    
    processLine(initializer.str());

    handleFrontendOptions();
  }
  
  //---------------------------------------------------------------------------
  // Destructor
  //---------------------------------------------------------------------------
  Interpreter::~Interpreter()
  {
    //delete m_prev_module;
    //m_prev_module = 0; // Don't do this, the engine does it.
    //delete m_IncrASTParser;
    //m_IncrASTParser = 0;
  }
   
  const char* Interpreter::getVersion() const {
    return "$Id$";
  }

  void Interpreter::writeStartupPCH() {
    m_IncrParser->writeStartupPCH();
  }

  void Interpreter::handleFrontendOptions() {
    if (m_Opts.ShowVersion) {
      llvm::outs() << getVersion() << '\n';
    }
    if (m_Opts.Help) {
      m_Opts.PrintHelp();
    }
  }

  void Interpreter::processStartupPCH() {
    clang::TranslationUnitDecl* TU = m_IncrParser->getCI()->getASTContext().getTranslationUnitDecl();
    for (clang::DeclContext::decl_iterator D = TU->decls_begin(),
           E = TU->decls_end(); D != E; ++D) {
      // That's probably overestimating
      ++m_UniqueCounter;
      const clang::FunctionDecl* F = dyn_cast<const clang::FunctionDecl>(*D);
      if (F) {
        clang::DeclarationName N = F->getDeclName();
        if (N.isIdentifier()) {
          clang::IdentifierInfo* II = N.getAsIdentifierInfo();
          if (II->getName().find("__cling_Un1Qu3") == 0) {
            m_ExecutionContext->executeFunction(II->getName());
          }
        }
      }
    }
  }
   
  void Interpreter::AddIncludePath(const char *incpath)
  {
    // Add the given path to the list of directories in which the interpreter
    // looks for include files. Only one path item can be specified at a
    // time, i.e. "path1:path2" is not supported.
      
    CompilerInstance* CI = getCI();
    HeaderSearchOptions& headerOpts = CI->getHeaderSearchOpts();
    const bool IsUserSupplied = false;
    const bool IsFramework = false;
    const bool IsSysRootRelative = true;
    headerOpts.AddPath (incpath, frontend::Angled, IsUserSupplied, IsFramework, IsSysRootRelative);
      
    Preprocessor& PP = CI->getPreprocessor();
    ApplyHeaderSearchOptions(PP.getHeaderSearchInfo(), headerOpts,
                                    PP.getLangOptions(),
                                    PP.getTargetInfo().getTriple());
      
  }
  
  CompilerInstance* Interpreter::getCI() const {
    return m_IncrParser->getCI();
  }
  
  int Interpreter::processLine(const std::string& input_line) {
    //
    //  Transform the input line to implement cint
    //  command line semantics (declarations are global),
    //  and compile to produce a module.
    //
    
    std::string wrapped;
    std::string stmtFunc;
    if (strncmp(input_line.c_str(),"#include ",strlen("#include ")) != 0) {
      //
      //  Wrap input into a function along with
      //  the saved global declarations.
      //
      InputValidator::Result ValidatorResult = m_InputValidator->validate(input_line);
      if (ValidatorResult != InputValidator::kValid) {
          fprintf(stderr, "Bad input, dude! That's a code %d\n", ValidatorResult);
        return 0;
      }

      createWrappedSrc(input_line, wrapped, stmtFunc);
      if (!wrapped.size()) {
         return 0;
      }
    } else {
      wrapped = input_line;
    }
    
    //
    //  Send the wrapped code through the
    //  frontend to produce a translation unit.
    //
    CompilerInstance* CI = m_IncrParser->parse(wrapped);

    if (!CI) {
      return 0;
    }
    // Note: We have a valid compiler instance at this point.
    TranslationUnitDecl* tu =
      CI->getASTContext().getTranslationUnitDecl();
    if (!tu) { // Parse failed, return.
      fprintf(stderr, "Wrapped parse failed, no translation unit!\n");
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
    stmtFunc = createUniqueName();
    std::string stmtVsDeclFunc = stmtFunc + "_stmt_vs_decl";
    std::vector<Stmt*> stmts;
    CompilerInstance* CI = 0;
    bool haveSemicolon = false;
    MapTy& Map = m_IncrParser->getTransformer()->getSubstSymbolMap(); // delayed id substitutions
    {
      size_t endsrc = src.length();
      while (endsrc && isspace(src[endsrc - 1])) --endsrc;
      haveSemicolon = src[endsrc - 1] == ';';

      std::string nonTUsrc = "void " + stmtVsDeclFunc + "() {\n" + src + ";}";
      // Create an ASTConsumer for this frontend run which
      // will produce a list of statements seen.
      StmtSplitter splitter(stmts);
      FunctionBodyConsumer* consumer =
        new FunctionBodyConsumer(splitter, stmtVsDeclFunc.c_str());

      //Diagnostic& Diag = m_IncrASTParser->getCI()->getDiagnostics();
      //bool prevDiagSupp = Diag.getSuppressAllDiagnostics();
      //Diag.setSuppressAllDiagnostics(true);
      // fprintf(stderr,"nonTUsrc=%s\n",nonTUsrc.c_str());
      m_IncrParser->addConsumer(IncrementalParser::kFunctionBodyConsumer,
                                consumer);
      CI = m_IncrParser->parse(nonTUsrc);
      m_IncrParser->removeConsumer(IncrementalParser::kFunctionBodyConsumer);
      //Diag.setSuppressAllDiagnostics(prevDiagSupp);

      if (!CI) {
        wrapped.clear();
        return;
      }
    }
    
    //
    //  Rewrite the source code to support cint command
    //  line semantics.  We must move variable declarations
    //  to the global namespace and change the code so that
    //  the new global variables are used.
    //
    std::string wrapped_globals;
    std::string wrapped_stmts;
    std::string finalStmtStr; // last statement for value printer
    const Expr* finalExpr = 0;
    {
      SourceManager& SM = CI->getSourceManager();
      const LangOptions& LO = CI->getLangOpts();
      std::vector<Stmt*>::iterator stmt_iter = stmts.begin();
      std::vector<Stmt*>::iterator stmt_end = stmts.end();

      for (; stmt_iter != stmt_end; ++stmt_iter) {
        Stmt* cur_stmt = *stmt_iter;
        
        if (dyn_cast<NullStmt>(cur_stmt)) continue;

        if (!finalStmtStr.empty()) {
           wrapped_stmts.append(finalStmtStr + '\n');
        }

        //const llvm::MemoryBuffer* MB = SM.getBuffer(SM.getFileID(cur_stmt->getLocStart()));
        //const llvm::MemoryBuffer* MB = SM.getBuffer(SM.getMainFileID());
        const llvm::MemoryBuffer* MB = (const llvm::MemoryBuffer*)m_IncrParser->getCurBuffer();
        const char* buffer = MB->getBufferStart();
        std::string stmt_string;
        {
          std::pair<unsigned, unsigned> r = getStmtRangeWithSemicolon(cur_stmt, SM, LO);
          // if the node was marked as artificially dependent the source locations 
          // would be wrong. Recalculate them.
          MapTy::const_iterator It = Map.find(cur_stmt);
          if (It != Map.end()) {
            if (!It->second)
              continue;
            r = getStmtRangeWithSemicolon(It->second, SM, LO);                
          }

          stmt_string = std::string(buffer + r.first, r.second - r.first);
          //fprintf(stderr, "stmt: %s\n", stmt_string.c_str());
        }
        //
        //  Handle expression statements.
        //
        {
          const Expr* expr = dyn_cast<Expr>(cur_stmt);
          if (expr) {
            //fprintf(stderr, "have expr stmt.\n");
            finalStmtStr = stmt_string;
            finalExpr = expr;

             continue;
          }
        }
        //
        //  Handle everything that is not a declaration statement.
        //
        const DeclStmt* DS = dyn_cast<DeclStmt>(cur_stmt);
        if (!DS) {
          //fprintf(stderr, "not expr, not declaration.\n");
          finalStmtStr = stmt_string;
          finalExpr = 0;
          continue;
        }
        //
        //  Loop over each declarator in the declaration statement.
        //
        DeclStmt::const_decl_iterator D = DS->decl_begin();
        DeclStmt::const_decl_iterator E = DS->decl_end();
        for (; D != E; ++D) {
          //
          //  Handle everything that is not a variable declarator.
          //
          const VarDecl* VD = dyn_cast<VarDecl>(*D);
          if (!VD) {
            if (DS->isSingleDecl()) {
              //fprintf(stderr, "decl, not var decl, single decl.\n");
              wrapped_globals.append(stmt_string + '\n');
              continue;
            }
            //fprintf(stderr, "decl, not var decl, not single decl.\n");
            SourceLocation SLoc = SM.getInstantiationLoc((*D)->getLocStart());
            SourceLocation ELoc = SM.getInstantiationLoc((*D)->getLocEnd());
            std::pair<unsigned, unsigned> r 
              = getRangeWithSemicolon(SLoc, ELoc, SM, LO);
            std::string decl = std::string(buffer + r.first, r.second - r.first);
            wrapped_globals.append(decl + ";\n");
            continue;
          }
          //
          //  Handle a variable declarator.
          //
          std::string decl = VD->getNameAsString();
          // FIXME: Probably should not remove the qualifiers!
          VD->getType().getUnqualifiedType().
          getAsStringInternal(decl, PrintingPolicy(LO));
          const Expr* I = VD->getInit();
          //
          //  Handle variable declarators with no initializer
          //  or with an initializer that is a constructor call.
          //
          if (!I || dyn_cast<CXXConstructExpr>(I)) {
            if (!I) {
              //fprintf(stderr, "var decl, no init.\n");
            }
            else {
              //fprintf(stderr, "var decl, init is constructor.\n");
            }
            wrapped_globals.append(decl + ";\n"); // FIXME: wrong for constructor
            continue;
          }
          //
          //  Handle variable declarators with a constant initializer.
          //
          if (I->isConstantInitializer(CI->getASTContext(), false)) {
            //fprintf(stderr, "var decl, init is const.\n");
            std::pair<unsigned, unsigned> r = getStmtRange(I, SM, LO);
            // Here, we copy! the initializer expression into the stmt wrapper.
            // The other copy is on the globals' decl scope.
            // But that's fine, we don't have to worry about side effects of the
            // initializer because we know it's a constant initializer.

            finalStmtStr = std::string(buffer + r.first, r.second - r.first);
            finalExpr = I;
            wrapped_globals.append(decl + " = " + finalStmtStr + ";\n");
            continue;
          }
          //
          //  Handle variable declarators whose initializer is not a list.
          //
          const InitListExpr* ILE = dyn_cast<InitListExpr>(I);
          if (!ILE) {
            //fprintf(stderr, "var decl, init is not list.\n");
            std::pair<unsigned, unsigned> r = getStmtRange(I, SM, LO);
            // if the node was marked as artificially dependent the source locations 
            // would be wrong. Recalculate them.
            MapTy::const_iterator It = Map.find(const_cast<Expr*>(I));
            if (It != Map.end()) {
              if (!It->second)
                continue;
              r = getStmtRange(It->second, SM, LO);                
            }

            finalStmtStr = std::string(VD->getName())  + " = " +
               std::string(buffer + r.first, r.second - r.first) + ";";
            finalExpr = I;
            wrapped_globals.append(decl + ";\n");
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
            stm << std::string(buffer + r.first, r.second - r.first) << ";\n";
            finalStmtStr = stm.str();
            finalExpr = ILE->getInit(j);
          }
          wrapped_globals.append(decl + ";\n");
        }
      }
      if (finalExpr) {
        // Users don't care about implicit casts (e.g. from InitExpr)
        const ImplicitCastExpr* ICE = dyn_cast<ImplicitCastExpr>(finalExpr);
        if (ICE) {
          finalExpr = ICE->getSubExprAsWritten();
        }
      }

      {
        size_t lenFinalStmtStr = finalStmtStr.length();
        while (lenFinalStmtStr && isspace(finalStmtStr[lenFinalStmtStr - 1])) {
          --lenFinalStmtStr;
        }
        if (lenFinalStmtStr && finalStmtStr[lenFinalStmtStr - 1] == ';') {
          finalStmtStr.erase(lenFinalStmtStr - 1);
        }
        while (lenFinalStmtStr && isspace(finalStmtStr[lenFinalStmtStr - 1])) {
          --lenFinalStmtStr;
        }
      }

      haveStatements = !finalStmtStr.empty();
      if (haveStatements) {
        std::stringstream sstr_stmt;
        sstr_stmt << "extern \"C\" void " << stmtFunc << "() {\n"
                  << wrapped_stmts;
        if (!haveSemicolon && finalExpr) {
          QualType QT = finalExpr->getType();
          if (!QT.isNull() && QT->isVoidType()) {
            sstr_stmt << finalStmtStr << ";}\n";
          } else {
            int Flags = 0;
            enum DumperFlags {
              kIsPtr = 1,
              kIsConst = 2,
              kIsPolymorphic
            };

            if (finalExpr->isRValue()) Flags |= kIsConst;

            if (!QT.isNull()) {
              if (QT.isConstant(CI->getASTContext()) || QT.isLocalConstQualified()) {
                Flags |= kIsConst;
              }
              const PointerType* PT = dyn_cast<PointerType>(QT.getTypePtr());
              if (PT) {
                // treat arrary-to-pointer decay as array:
                QualType PQT = PT->getPointeeType();
                const Type* PTT = PQT.getTypePtr();
                if (!PTT || !PTT->isArrayType()) {
                  Flags |= kIsPtr;
                  const RecordType* RT = dyn_cast<RecordType>(QT.getTypePtr());
                  if (RT) {
                    RecordDecl* RD = RT->getDecl();
                    if (RD) {
                      CXXRecordDecl* CRD = dyn_cast<CXXRecordDecl>(RD);
                      if (CRD && CRD->isPolymorphic()) {
                        Flags |= kIsPolymorphic;
                      }
                    }
                  }
                }
              }
            }
            
            sstr_stmt << "cling::valuePrinterInternal::PrintValue(((cling::Interpreter*)"
                      << (void*)this << ")->getValuePrinterStream()," << Flags << ","
                      << finalStmtStr << ");}\n";
          }
        } else {
          sstr_stmt << finalStmtStr << ";}\n";
        }
        wrapped_stmts = sstr_stmt.str();
      } else {
        stmtFunc = "";
      }
    }
    //
    //fprintf(stderr, "wrapped_globals:\n%s\n", wrapped_globals.c_str());
    //fprintf(stderr, "wrapped_stmts:\n%s\n", wrapped_stmts.c_str());
    wrapped += wrapped_globals + wrapped_stmts;

    //CI->clearOutputFiles(/*EraseFiles=*/CI->getDiagnostics().getNumErrors());
    //CI->getDiagnosticClient().EndSourceFile();
    unsigned err_count = CI->getDiagnosticClient().getNumErrors();
    // reset diag client
    if (err_count) {
      wrapped.clear();
      return;
    }

    // clear the DenseMap
    Map.clear();
  }
  
  CompilerInstance* Interpreter::compileFile(const std::string& filename,
                                             const std::string* trailcode/*=0*/) {
    std::string code;
    code += "#include \"" + filename + "\"\n";
    if (trailcode) code += *trailcode;
    return m_IncrParser->parse(code);
  }
  
  static bool tryLoadSharedLib(const std::string& filename,
                               const InvocationOptions& Opts) {
    llvm::sys::Path DynLib = findDynamicLibrary(filename, Opts);
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
    if (allowSharedLib && tryLoadSharedLib(filename, getOptions()))
      return 0;
    
    CompilerInstance* CI = compileFile(filename, trailcode);
    if (!CI) {
      return 1;
    }
    
    m_ExecutionContext->runCodeGen();
    return 0;
  }
  
  int
  Interpreter::executeFile(const std::string& fileWithArgs)
  {
    // Look for start of parameters:

    typedef std::pair<llvm::StringRef,llvm::StringRef> StringRefPair;

    StringRefPair pairFileArgs = llvm::StringRef(fileWithArgs).split('(');
    if (pairFileArgs.second.empty()) {
      pairFileArgs.second = ")";
    }
    StringRefPair pairPathFile = pairFileArgs.first.rsplit('/');
    if (pairPathFile.second.empty()) {
       pairPathFile.second = pairPathFile.first;
    }
    StringRefPair pairFuncExt = pairPathFile.second.rsplit('.');

    //fprintf(stderr, "funcname: %s\n", pairFuncExt.first.data());
    
    std::string func = createUniqueName();
    std::string wrapper = "extern \"C\" void " + func;
    wrapper += "(){\n" + pairFuncExt.first.str() + "(" + pairFileArgs.second.str() + ";\n}";
    int err = loadFile(pairFileArgs.first, &wrapper);
    if (err) {
      return err;
    }
    m_ExecutionContext->executeFunction(func);
    return 0;
  }

  QualType Interpreter::getQualType(llvm::StringRef type) {
     std::string className = createUniqueName();
     QualType Result;
     CompilerInstance* CI;

     // template<typename T> class dummy{}; 
     std::string templatedClass = "template<typename T> class " + className + "{};\n";
     CI  = m_IncrParser->parse(templatedClass);
     Decl *templatedClassDecl = 0;
     if (CI)
        templatedClassDecl = m_IncrParser->getLastTopLevelDecl();

     //template <> dummy<DeclContext*> {};
     std::string explicitSpecialization = "template<> class " + className + "<" + type.str()  + "*>{};\n";
     CI = m_IncrParser->parse(explicitSpecialization);
     if (CI) {
        if (ClassTemplateSpecializationDecl* D = dyn_cast<ClassTemplateSpecializationDecl>(m_IncrParser->getLastTopLevelDecl())) {
           Result = D->getTemplateArgs()[0].getAsType();

           // TODO: Remove the fake Decls
           // We couldn't remove the template specialization and leave only the
           // template
           /*Scope *S = CI->getSema().getScopeForContext(CI->getSema().getASTContext().getTranslationUnitDecl());
           S->RemoveDecl(D);
           //D->getDeclContext()->removeDecl(D);
           if (templatedClassDecl) {
              templatedClassDecl->getDeclContext()->removeDecl(templatedClassDecl);
              S->RemoveDecl(templatedClassDecl);
              }*/

           return Result;
        }
     }

     fprintf(stderr, "Cannot find the type:%s\n", type.data());
     return Result;
  }

  Interpreter::NamedDeclResult Interpreter::LookupDecl(llvm::StringRef Decl, 
                                                       DeclContext* Within) {
    if (!Within)
      Within = getCI()->getASTContext().getTranslationUnitDecl();
    return Interpreter::NamedDeclResult(Decl, this, Within);
  }

  void Interpreter::installLazyFunctionCreator(void* (*fp)(const std::string&)) {
    m_ExecutionContext->installLazyFunctionCreator(fp);
  }
  
  
  // Implements the interpretation of the unknown symbols. 
  Value Interpreter::EvaluateWithContext(const char* expr,
                                         void* varaddr[],
                                         DeclContext* DC) {
    std::string exprStr(expr);
    int i = 0;
    size_t found;
    while ((found = exprStr.find("@")) && (found != std::string::npos)) { 
      std::stringstream address;
      address << varaddr[i];
      exprStr = exprStr.insert(found + 1, address.str());
      exprStr = exprStr.erase(found, 1);
      ++i;    
    }
    printf("The expression that is going to be escaped is: %s\n", exprStr.c_str());
    printf("\n");
    return Evaluate(exprStr.c_str(), DC);
  }
  
  Value Interpreter::Evaluate(const char* expr, DeclContext* DC) {
    // Execute and get the result
    Value Result;

    // Wrap the expression
    const std::string ExprStr(expr);
    std::string WrapperName = createUniqueName();
    std::string Wrapper = "void " + WrapperName + " () {\n";
    //expr = "gCling->getVersion()";
    Wrapper += expr;
    Wrapper += ";\n}";
    
    // Set up the declaration context
    DeclContext* CurContext;
    CurContext = m_IncrParser->getCI()->getSema().CurContext;
    m_IncrParser->getCI()->getSema().CurContext = DC;

    // Temporary stop the code gen
    m_IncrParser->removeConsumer(IncrementalParser::kCodeGenerator);

    CompilerInstance* CI = m_IncrParser->parse(Wrapper);
    if (!CI) {
      fprintf(stderr, "Cannot compile string!\n");
    }

    m_IncrParser->getCI()->getSema().CurContext = CurContext;
    // get the Type
    FunctionDecl* TopLevelFD 
      = dyn_cast<FunctionDecl>(m_IncrParser->getLastTopLevelDecl());
    CurContext = m_IncrParser->getCI()->getSema().CurContext;
    m_IncrParser->getCI()->getSema().CurContext = TopLevelFD;
    QualType RetTy;
    if (Stmt* S = TopLevelFD->getBody())
      if (CompoundStmt* CS = dyn_cast<CompoundStmt>(S))
        if (Expr* E = dyn_cast<Expr>(CS->body_back())) {
          RetTy = E->getType();
          // Change the void function's return type
          FunctionProtoType::ExtProtoInfo EPI;
          QualType FuncTy
            = getCI()->getASTContext().getFunctionType(RetTy,
                                                       /*ArgArray*/0,
                                                       /*NumArgs*/0,
                                                       EPI);

          TopLevelFD->setType(FuncTy);
          // add return stmt
          Stmt* RetS = getCI()->getSema().ActOnReturnStmt(SourceLocation(), E).take();
          CS->setStmts(getCI()->getASTContext(), &RetS, 1);
        }
    m_IncrParser->getCI()->getSema().CurContext = CurContext;
    // resume the code gen
    m_IncrParser->addConsumer(IncrementalParser::kCodeGenerator,
                              m_ExecutionContext->getCodeGenerator());
    m_ExecutionContext->getCodeGenerator()->HandleTopLevelDecl(DeclGroupRef(TopLevelFD));

    // get the result
    llvm::GenericValue val;
    if (!isInCLinkageSpecification(TopLevelFD)) {
        WrapperName = "";
        llvm::raw_string_ostream RawStr(WrapperName);
        MangleContext* Mangle = getCI()->getASTContext().createMangleContext();
        Mangle->mangleName(TopLevelFD, RawStr);
    }
    m_ExecutionContext->executeFunction(WrapperName, &val);

    return Value(val, RetTy.getTypePtrOrNull());
  }

  bool Interpreter::isInCLinkageSpecification(const Decl *D) {
    D = D->getCanonicalDecl();
    for (const DeclContext *DC = D->getDeclContext();
         !DC->isTranslationUnit(); DC = DC->getParent()) {
      if (const LinkageSpecDecl *Linkage = dyn_cast<LinkageSpecDecl>(DC))
        return Linkage->getLanguage() == LinkageSpecDecl::lang_c;
    }
    
    return false;
  }

  void Interpreter::setRuntimeCallbacks(bool Enabled /*=true*/) {
    InterpreterCallbacks* C = 0;
    if (Enabled)
      C = new InterpreterCallbacks(this);

    m_IncrParser->getTransformer()->SetRuntimeCallbacks(C);
  }

  bool Interpreter::setDynamicLookup(bool value /*=true*/){
    bool prev = m_IncrParser->getEnabled();
    m_IncrParser->setEnabled(value);
    return prev;
  }

  void Interpreter::setPrintAST(bool print /*=true*/) {
    if (print) {
      if (!m_ASTDumper)
        m_ASTDumper = new ASTTLDPrinter();
      m_IncrParser->addConsumer(IncrementalParser::kASTDumper, m_ASTDumper);
    }
    else
      m_IncrParser->removeConsumer(IncrementalParser::kASTDumper);
    m_printAST = !m_printAST;
  }
  
  
  void Interpreter::dumpAST(bool showAST, int last) {
    Decl* D = m_LastDump;
    int oldPolicy = m_IncrParser->getCI()->getASTContext().PrintingPolicy.Dump;
    
    if (!D && last == -1 ) {
      fprintf(stderr, "No last dump found! Assuming ALL \n");
      last = 0;
      showAST = false;        
    }
    
    m_IncrParser->getCI()->getASTContext().PrintingPolicy.Dump = showAST;
    
    if (last == -1) {
      while ((D = D->getNextDeclInContext())) {
        D->dump();
      }
    }
    else if (last == 0) {
      m_IncrParser->getCI()->getASTContext().getTranslationUnitDecl()->dump();
    } else {
      Decl *FD = m_IncrParser->getFirstTopLevelDecl(); // First Decl to print
      Decl *LD = FD;
      
      // FD and LD are first
      
      Decl *NextLD = 0;
      for (int i = 1; i < last; ++i) {
        NextLD = LD->getNextDeclInContext();
        if (NextLD) {
          LD = NextLD;
        }
      }
      
      // LD is last Decls after FD: [FD x y z LD a b c d]
      
      while ((NextLD = LD->getNextDeclInContext())) {
        // LD not yet at end: move window
        FD = FD->getNextDeclInContext();
        LD = NextLD;
      }
      
      // Now LD is == getLastDeclinContext(), and FD is last decls before
      // LD is last Decls after FD: [x y z a FD b c LD]
      
      while (FD) {
        FD->dump();
        fprintf(stderr, "\n"); // New line for every decl
        FD = FD->getNextDeclInContext();
      }        
    }
    
    m_LastDump = m_IncrParser->getLastTopLevelDecl();     
    m_IncrParser->getCI()->getASTContext().PrintingPolicy.Dump = oldPolicy;
  }
  
} // namespace cling
