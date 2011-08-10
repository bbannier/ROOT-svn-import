//------------------------------------------------------------------------------
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Lukasz Janyst <ljanyst@cern.ch>
//------------------------------------------------------------------------------

#include "cling/Interpreter/Interpreter.h"

#include "DynamicLookup.h"
#include "ExecutionContext.h"
#include "IncrementalParser.h"
#include "InputValidator.h"

#include "cling/Interpreter/CIFactory.h"
#include "cling/Interpreter/InterpreterCallbacks.h"
#include "cling/Interpreter/Value.h"

#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/Mangle.h"
#include "clang/AST/DeclarationName.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/CodeGen/ModuleBuilder.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/Utils.h"
#include "clang/Lex/Pragma.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Sema/Sema.h"

#include "llvm/Linker.h"
#include "llvm/Support/DynamicLibrary.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/raw_os_ostream.h"

#include <cstdio>
#include <iostream>
#include <sstream>

using namespace clang;

static bool tryLinker(const std::string& filename,
                      const cling::InvocationOptions& Opts,
                      llvm::Module* module) {
  assert(module && "Module must exist for linking!");
  llvm::Linker L("cling", module, llvm::Linker::QuietWarnings
                 | llvm::Linker::QuietErrors);
  for (std::vector<llvm::sys::Path>::const_iterator I
         = Opts.LibSearchPath.begin(), E = Opts.LibSearchPath.end(); I != E;
       ++I) {
    L.addPath(*I);
  }
  L.addSystemPaths();
  bool Native = true;
  if (L.LinkInLibrary(filename, Native)) {
    // that didn't work, try bitcode:
    llvm::sys::Path FilePath(filename);
    std::string Magic;
    if (!FilePath.getMagicNumber(Magic, 64)) {
      // filename doesn't exist...
      L.releaseModule();
      return false;
    }
    if (llvm::sys::IdentifyFileType(Magic.c_str(), 64)
        == llvm::sys::Bitcode_FileType) {
      // We are promised a bitcode file, complain if it fails
      L.setFlags(0);
      if (L.LinkInFile(llvm::sys::Path(filename), Native)) {
        L.releaseModule();
        return false;
      }
    } else {
      // Nothing the linker can handle
      L.releaseModule();
      return false;
    }
  } else if (Native) {
    // native shared library, load it!
    llvm::sys::Path SoFile = L.FindLib(filename);
    assert(!SoFile.isEmpty() && "We know the shared lib exists but can't find it back!");
    std::string errMsg;
    bool err =
      llvm::sys::DynamicLibrary::LoadLibraryPermanently(SoFile.str().c_str(), &errMsg);
    if (err) {
      fprintf(stderr,
              "Interpreter::loadFile: Could not load shared library!\n");
      fprintf(stderr, "%s\n", errMsg.c_str());
      L.releaseModule();
      return false;
    }
  }
  L.releaseModule();
  return true;
}

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

  const char* DynamicExprInfo::getExpr() {
    std::string exprStr(m_Template);
    int i = 0;
    size_t found;

    while ((found = exprStr.find("@")) && (found != std::string::npos)) { 
      std::stringstream address;
      address << m_Addresses[i];
      exprStr = exprStr.insert(found + 1, address.str());
      exprStr = exprStr.erase(found, 1);
      ++i;    
    }

    return exprStr.c_str();
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
  m_ValuePrinterEnabled(false),
  m_LastDump(0)
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
    m_ExecutionContext.reset(new ExecutionContext());

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

    if (getCI()->getLangOpts().CPlusPlus) {
       // Set up the gCling variable - even if we use PCH ('this' is different)
       processLine("#include \"cling/Interpreter/ValuePrinter.h\"\n");
       std::stringstream initializer;
       initializer << "gCling=(cling::Interpreter*)" << (long)this <<";";
       processLine(initializer.str());
    }

    handleFrontendOptions();
  }
  
  //---------------------------------------------------------------------------
  // Destructor
  //---------------------------------------------------------------------------
  Interpreter::~Interpreter()
  {
    //llvm::Module* module = m_IncrParser->GetCodeGenerator()->GetModule();
    //m_ExecutionContext->runStaticDestructorsOnce(module);
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
            RunFunction(II->getName());
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

  void Interpreter::DumpIncludePath() {
    const HeaderSearchOptions Opts(getCI()->getHeaderSearchOpts());
    std::vector<std::string> Res;
    if (Opts.Sysroot != "/") {
      Res.push_back("-isysroot");
      Res.push_back(Opts.Sysroot);
    }
    
    /// User specified include entries.
    for (unsigned i = 0, e = Opts.UserEntries.size(); i != e; ++i) {
      const HeaderSearchOptions::Entry &E = Opts.UserEntries[i];
      if (E.IsFramework && (E.Group != frontend::Angled || !E.IsUserSupplied))
        llvm::report_fatal_error("Invalid option set!");
      if (E.IsUserSupplied) {
        if (E.Group == frontend::After) {
          Res.push_back("-idirafter");
        } else if (E.Group == frontend::Quoted) {
          Res.push_back("-iquote");
        } else if (E.Group == frontend::System) {
          Res.push_back("-isystem");
        } else if (E.Group == frontend::CXXSystem) {
          Res.push_back("-cxx-isystem");
        } else {
          assert(E.Group == frontend::Angled && "Invalid group!");
          Res.push_back(E.IsFramework ? "-F" : "-I");
        }
      } else {
        if (E.Group != frontend::Angled && E.Group != frontend::System)
          llvm::report_fatal_error("Invalid option set!");
        Res.push_back(E.Group == frontend::Angled ? "-iwithprefixbefore" :
                      "-iwithprefix");
      }
      Res.push_back(E.Path);
    }
    
    if (!Opts.EnvIncPath.empty()) {
      // FIXME: Provide an option for this, and move env detection to driver.
      llvm::report_fatal_error("Not yet implemented!");
    }
    if (!Opts.CEnvIncPath.empty()) {
      // FIXME: Provide an option for this, and move env detection to driver.
      llvm::report_fatal_error("Not yet implemented!");
    }
    if (!Opts.ObjCEnvIncPath.empty()) {
      // FIXME: Provide an option for this, and move env detection to driver.
      llvm::report_fatal_error("Not yet implemented!");
    }
    if (!Opts.CXXEnvIncPath.empty()) {
      // FIXME: Provide an option for this, and move env detection to driver.
      llvm::report_fatal_error("Not yet implemented!");
    }
    if (!Opts.ObjCXXEnvIncPath.empty()) {
      // FIXME: Provide an option for this, and move env detection to driver.
      llvm::report_fatal_error("Not yet implemented!");
    }
    if (!Opts.ResourceDir.empty()) {
      Res.push_back("-resource-dir");
      Res.push_back(Opts.ResourceDir);
    }
    if (!Opts.UseStandardIncludes)
      Res.push_back("-nostdinc");
    if (!Opts.UseStandardCXXIncludes)
      Res.push_back("-nostdinc++");
    if (Opts.Verbose)
      Res.push_back("-v");

    // print'em all
    for (unsigned i = 0; i < Res.size(); ++i) {
      llvm::errs() << Res[i] <<"\n";
    }

  }
  
  CompilerInstance* Interpreter::getCI() const {
    return m_IncrParser->getCI();
  }
  
  bool Interpreter::processLine(const std::string& input_line) {
    //
    //  Transform the input line to implement cint
    //  command line semantics (declarations are global),
    //  and compile to produce a module.
    //
    
    std::string wrapped = input_line;
    std::string stmtFunc;
    std::string functName;
    if (strncmp(input_line.c_str(),"#",strlen("#")) != 0 &&
        strncmp(input_line.c_str(),"extern ",strlen("extern ")) != 0) {
      //
      //  Wrap input into a function along with
      //  the saved global declarations.
      //
      // InputValidator::Result ValidatorResult = m_InputValidator->validate(input_line);
      // if (ValidatorResult != InputValidator::kValid) {
      //     fprintf(stderr, "Bad input, dude! That's a code %d\n", ValidatorResult);
      //   return 0;
      // }
      WrapInput(wrapped, functName);
    }

    Diagnostic& Diag = getCI()->getDiagnostics();
    // Disable warnings which doesn't make sense when using the prompt
    // This gets reset with the clang::Diagnostics().Reset()
    Diag.setDiagnosticMapping(DiagnosticIDs::getIdFromName("warn_unused_expr"),
                              clang::diag::MAP_IGNORE, SourceLocation());
    Diag.setDiagnosticMapping(DiagnosticIDs::getIdFromName("warn_unused_call"),
                              clang::diag::MAP_IGNORE, SourceLocation());
    bool result = handleLine(wrapped, functName);
    return result;
  }

  void Interpreter::WrapInput(std::string& input, std::string& fname) {
    fname = createUniqueName();
    input.insert(0, "void " + fname + "() {\n ");
    input.append("\n;\n}");
  }

  bool Interpreter::RunFunction(llvm::StringRef fname, llvm::GenericValue* res) {
    if (getCI()->getDiagnostics().hasErrorOccurred())
      return false;

    std::string mangledNameIfNeeded;
    FunctionDecl* FD = cast_or_null<FunctionDecl>(LookupDecl(fname).
                                                  getSingleDecl()
                                                  );
    if (FD) {
      if (!FD->isExternC()) {
        llvm::raw_string_ostream RawStr(mangledNameIfNeeded);
        llvm::OwningPtr<MangleContext> 
          Mangle(getCI()->getASTContext().createMangleContext());
        Mangle->mangleName(FD, RawStr);
        RawStr.flush();
        fname = mangledNameIfNeeded;
      }
      m_ExecutionContext->executeFunction(fname, res);
      return true;
    }

    return false;
  }

  std::string Interpreter::createUniqueName()
  {
    // Create an unique name
    
    std::ostringstream swrappername;
    swrappername << "__cling_Un1Qu3" << m_UniqueCounter++;
    return swrappername.str();
  }
  
  
  bool Interpreter::handleLine(llvm::StringRef input, 
                               llvm::StringRef FunctionName) {
    // if we are using the preprocessor
    if (input[0] == '#') {
      return (m_IncrParser->CompileAsIs(input));
    }

    m_IncrParser->CompileLineFromPrompt(input);
    //
    //  Run it using the JIT.
    //
    RunFunction(FunctionName);

    return 1;

  }
  
  bool
  Interpreter::loadFile(const std::string& filename,
                        const std::string* trailcode /*=0*/,
                        bool allowSharedLib /*=true*/)
  {
    if (allowSharedLib) {
      llvm::Module* module = m_IncrParser->GetCodeGenerator()->GetModule();
      if (module) {
        if (tryLinker(filename, getOptions(), module))
          return 0;
        if (filename.compare(0, 3, "lib") == 0) {
          // starts with "lib", try without (the llvm::Linker forces
          // a "lib" in front, which makes it liblib...
          if (tryLinker(filename.substr(3, std::string::npos),
                        getOptions(), module))
            return 0;
        }
      }
    }
    
    std::string code;
    code += "#include \"" + filename + "\"\n";
    if (trailcode) code += *trailcode;
    return (m_IncrParser->CompileAsIs(code));
  }
  
  bool
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
    
    std::string func;
    std::string wrapper = pairFuncExt.first.str()+"("+pairFileArgs.second.str();
    WrapInput(wrapper, func);

    if (loadFile(pairFileArgs.first, &wrapper)) {
      return RunFunction(func);
    }
    return false;
  }

  QualType Interpreter::getQualType(llvm::StringRef type) {
     std::string className = createUniqueName();
     QualType Result;
     CompilerInstance* CI;

     // template<typename T> class dummy{}; 
     std::string templatedClass = "template<typename T> class " + className + "{};\n";
     CI  = m_IncrParser->CompileAsIs(templatedClass);
     Decl *templatedClassDecl = 0;
     if (CI)
        templatedClassDecl = m_IncrParser->getLastTopLevelDecl();

     //template <> dummy<DeclContext*> {};
     std::string explicitSpecialization = "template<> class " + className + "<" + type.str()  + "*>{};\n";
     CI = m_IncrParser->CompileAsIs(explicitSpecialization);
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
  
  Value Interpreter::Evaluate(const char* expr, DeclContext* DC) {
    assert(DC && "DeclContext cannot be null!");

    Sema& S = getCI()->getSema();
    assert(S.ExternalSource && "No ExternalSource set!");
    static_cast<DynamicIDHandler*>(S.ExternalSource)->Callbacks->setEnabled();

    // Execute and get the result
    Value Result;

    // Wrap the expression
    std::string WrapperName;
    std::string Wrapper = expr;
    WrapInput(Wrapper, WrapperName);
    
    // Set up the declaration context
    DeclContext* CurContext;
    CurContext = m_IncrParser->getCI()->getSema().CurContext;
    m_IncrParser->getCI()->getSema().CurContext = DC;

    llvm::SmallVector<clang::DeclGroupRef, 4> DGRs;
    m_IncrParser->Parse(Wrapper, DGRs);

    assert((DGRs.size() || DGRs.size() > 2) && "Only FunctionDecl expected!");

    m_IncrParser->getCI()->getSema().CurContext = CurContext;
    // get the Type
    FunctionDecl* TopLevelFD 
      = dyn_cast<FunctionDecl>(DGRs.front().getSingleDecl());
    CurContext = m_IncrParser->getCI()->getSema().CurContext;
    m_IncrParser->getCI()->getSema().CurContext = TopLevelFD;
    ASTContext& Context(getCI()->getASTContext());
    QualType RetTy;
    if (Stmt* S = TopLevelFD->getBody())
      if (CompoundStmt* CS = dyn_cast<CompoundStmt>(S))
        if (Expr* E = dyn_cast<Expr>(CS->body_back())) {
          RetTy = E->getType();
          // Change the void function's return type
          FunctionProtoType::ExtProtoInfo EPI;
          QualType FuncTy = Context.getFunctionType(RetTy,
                                                    /*ArgArray*/0,
                                                    /*NumArgs*/0,
                                                    EPI);
          TopLevelFD->setType(FuncTy);
          // add return stmt
          Stmt* RetS = getCI()->getSema().ActOnReturnStmt(SourceLocation(), E).take();
          CS->setStmts(Context, &RetS, 1);
        }
    m_IncrParser->getCI()->getSema().CurContext = CurContext;

    // FIXME: Finish the transaction in better way
    m_IncrParser->CompileAsIs("");
 
    // get the result
    llvm::GenericValue val;
    RunFunction(WrapperName, &val);

    return Value(val, RetTy.getTypePtrOrNull());
  }

  void Interpreter::setCallbacks(InterpreterCallbacks* C) {
    Sema& S = getCI()->getSema();
    assert(S.ExternalSource && "No ExternalSource set!");
    static_cast<DynamicIDHandler*>(S.ExternalSource)->Callbacks = C;
  }
      

  void Interpreter::enableDynamicLookup(bool value /*=true*/) {
    m_IncrParser->enableDynamicLookup(value);
  }

  bool Interpreter::isDynamicLookupEnabled() {
    return m_IncrParser->isDynamicLookupEnabled();
  }

  void Interpreter::enablePrintAST(bool print /*=true*/) {
    m_IncrParser->enablePrintAST(print);
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
      // First Decl to print
      Decl *FD = m_IncrParser->getFirstTopLevelDecl();
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

  void Interpreter::runStaticInitializersOnce() const {
    // Forward to ExecutionContext; should not be called by
    // anyone except for IncrementalParser.
    llvm::Module* module = m_IncrParser->GetCodeGenerator()->GetModule();
    m_ExecutionContext->runStaticInitializersOnce(module);
  }
  
} // namespace cling
