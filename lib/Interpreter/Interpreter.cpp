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
#include "clang/Basic/DiagnosticIDs.h"
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
#include "clang/Sema/Template.h"
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
#include "cling/Interpreter/Diagnostics.h"
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

  const char* DynamicExprInfo::getExpr() {
    if (!m_Cache) {
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
      m_Cache = exprStr.c_str();
    }

    return m_Cache;
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
  
  int Interpreter::processLine(const std::string& input_line) {
    //
    //  Transform the input line to implement cint
    //  command line semantics (declarations are global),
    //  and compile to produce a module.
    //
    
    std::string wrapped;
    std::string stmtFunc;
    std::string functName;
    if (strncmp(input_line.c_str(),"#include ",strlen("#include ")) != 0 &&
        strncmp(input_line.c_str(),"extern ",strlen("extern ")) != 0) {
      //
      //  Wrap input into a function along with
      //  the saved global declarations.
      //
      InputValidator::Result ValidatorResult = m_InputValidator->validate(input_line);
      if (ValidatorResult != InputValidator::kValid) {
          fprintf(stderr, "Bad input, dude! That's a code %d\n", ValidatorResult);
        return 0;
      }
      functName = createUniqueName();
      wrapped = "void " + functName + "() {\n ";
      wrapped += input_line;
      wrapped += ";\n}";

    }
    else {
      wrapped = input_line;
    }
      DiagnosticPrinter& Diag = (DiagnosticPrinter&)getCI()->getDiagnosticClient();
      // disable that warning when using the prompt
      unsigned warningID = DiagnosticIDs::getIdFromName("warn_unused_expr");
      Diag.ignoreWarning(warningID);
      int result = handleLine(wrapped, functName);
      Diag.removeIgnoredWarning(warningID);
      return result;
  }
  
  std::string Interpreter::createUniqueName()
  {
    // Create an unique name
    
    std::ostringstream swrappername;
    swrappername << "__cling_Un1Qu3" << m_UniqueCounter++;
    return swrappername.str();
  }
  
  
  int Interpreter::handleLine(const std::string& input, 
                              std::string& FunctionName) {
    // if we are using the preprocessor
    if (input.c_str()[0] == '#') {
      return m_IncrParser->parse(input) != 0;
    }
    
    // else Disable codegen
    m_IncrParser->removeConsumer(IncrementalParser::kCodeGenerator);
    CompilerInstance* CI = m_IncrParser->parse(input);
    if (!CI) {
      fprintf(stderr, "Cannot compile string!\n");
      return 0;
    }
    FunctionDecl* TopLevelFD 
      = dyn_cast<FunctionDecl>(m_IncrParser->getLastTopLevelDecl());
    ASTContext& Ctx(getCI()->getASTContext());
    Sema& TheSema(getCI()->getSema());
    llvm::SmallVector<Decl*, 4> TouchedDecls;
    
    if (TopLevelFD) {
      CompoundStmt* CS = dyn_cast<CompoundStmt>(TopLevelFD->getBody());
      assert(CS && "Function body not a CompoundStmt?");
      DeclContext* DC = TopLevelFD->getDeclContext();
      Scope* S = TheSema.getScopeForContext(DC);
      CompoundStmt::body_iterator I;
      llvm::SmallVector<Stmt*, 4> Stmts;
      DC->removeDecl(TopLevelFD);
      S->RemoveDecl(TopLevelFD);
      for (I = CS->body_begin(); I != CS->body_end(); ++I) {
        DeclStmt* DS = dyn_cast<DeclStmt>(*I);
        if (!DS) {
          Stmts.push_back(*I);
          continue;
        }
        
        for (DeclStmt::decl_iterator J = DS->decl_begin();
             J != DS->decl_end(); ++J) {
          Decl* D = dyn_cast<Decl>(*J);
          if (D) {
            D->setDeclContext(DC);
            D->setLexicalDeclContext(DC); //FIXME: Watch out
          }
          
          if (VarDecl* VD = dyn_cast<VarDecl>(D)) {
            VD->setStorageClass(SC_None);
            VD->setStorageClassAsWritten(SC_None);
            // reset the linkage to External
            VD->ClearLinkageCache();
            DC->addDecl(VD);
          }

          TouchedDecls.push_back(D);
        }
      }
      // Remove the empty wrappers, i.e those which contain only decls
      if (Stmts.size()) {
        CS->setStmts(Ctx, Stmts.data(), Stmts.size());
        DC->addDecl(TopLevelFD);
        S->AddDecl(TopLevelFD);
        TouchedDecls.push_back(TopLevelFD);
      }
      else
        // tell exec engine not to run the function
        TopLevelFD = 0;
      
      // attach the value printer
      attachValuePrinter(CS);
    }
    // if it is not function decl
    else {
      TouchedDecls.push_back(m_IncrParser->getLastTopLevelDecl());
    }
    // resume the code gen
    m_IncrParser->addConsumer(IncrementalParser::kCodeGenerator,
                              m_ExecutionContext->getCodeGenerator());
    // TouchedDecls.push_back(LookupDecl("cling").LookupDecl("valuePrinterInternal").LookupDecl("PrintValue"));
    // TouchedDecls.push_back(LookupDecl("cling").LookupDecl("valuePrinterInternal").LookupDecl("printValue"));
    DeclGroupRef DGR = DeclGroupRef::Create(Ctx, TouchedDecls.data(), TouchedDecls.size());
    // collect the references that are being used
    m_ExecutionContext->getCodeGenerator()->HandleTopLevelDecl(DGR);
    TheSema.PerformPendingInstantiations();
    
    // Process any TopLevelDecls generated by #pragma weak.
    for (llvm::SmallVector<clang::Decl*,2>::iterator
           I = getCI()->getSema().WeakTopLevelDecls().begin(),
           E = getCI()->getSema().WeakTopLevelDecls().end(); I != E; ++I) {
      m_ExecutionContext->getCodeGenerator()->HandleTopLevelDecl(clang::DeclGroupRef(*I));
    }

    // generate code for the delta
    m_ExecutionContext->getCodeGenerator()->HandleTranslationUnit(Ctx);
    //
    //  Run it using the JIT.
    //
    if (TopLevelFD) {
      if (!TopLevelFD->isExternC()) {
        FunctionName = "";
        llvm::raw_string_ostream RawStr(FunctionName);
        MangleContext* Mangle = Ctx.createMangleContext();
        Mangle->mangleName(TopLevelFD, RawStr);
      }
      m_ExecutionContext->executeFunction(FunctionName);
    }

    return 1;

  }

  
  CompilerInstance* Interpreter::compileFile(const std::string& filename,
                                             const std::string* trailcode/*=0*/) {
    std::string code;
    code += "#include \"" + filename + "\"\n";
    if (trailcode) code += *trailcode;
    return m_IncrParser->parse(code);
  }

  void Interpreter::loadValuePrinter() {
    if (!m_ValuePrinterEnabled) {
      processLine("#include \"cling/Interpreter/Interpreter.h\"");
      processLine("#include \"cling/Interpreter/ValuePrinter.h\"");
      processLine("#include \"cling/Interpreter/Value.h\"");
      m_ValuePrinterEnabled = true;
    }
  }

  void Interpreter::attachValuePrinter(CompoundStmt* CS) {
    for (CompoundStmt::body_iterator I = CS->body_begin();
         I != CS->body_end(); ++I) {
      if ((CS->body_end() - CS->body_begin() == 1) || 
          (((I+1) != CS->body_end()) && 
           !isa<NullStmt>(*(I + 1)))) {
        if (Expr* To = dyn_cast<Expr>(*I)) {
          loadValuePrinter();
          *I = constructValuePrinter(To);
        }
      }
    }
  }

  // We need to artificially create:
  // cling::valuePrinterInternal::PrintValue(gCling->getValuePrinterStream(), 0, i);

  // So we need the following AST:
  // (CallExpr 0x2fdc4b8 'void'
  //   (ImplicitCastExpr 0x2fdc4a0 'void (*)(llvm::raw_ostream &, int, const int &)' <FunctionToPointerDecay>
  //     (DeclRefExpr 0x2fdc460 'void (llvm::raw_ostream &, int, const int &)' lvalue Function 0x2fd1b50 'PrintValue' 'void (llvm::raw_ostream &, int, const int &)' (FunctionTemplate 0x23b51c0 'PrintValue')))
  //   (CXXMemberCallExpr 0x2fdc388 'llvm::raw_ostream':'class llvm::raw_ostream' lvalue
  //     (MemberExpr 0x2fdc350 '<bound member function type>' ->getValuePrinterStream 0x235ae10
  //       (ImplicitCastExpr 0x2fdc3b0 'const class cling::Interpreter *' <NoOp>
  //         (ImplicitCastExpr 0x2fdc338 'class cling::Interpreter *' <LValueToRValue>
  //           (DeclRefExpr 0x2fdc310 'class cling::Interpreter *' lvalue Var 0x1aa89c0 'gCling' 'class cling::Interpreter *')))))
  //   (IntegerLiteral 0x2fdc3c8 'int' 0)
  //   (ImplicitCastExpr 0x2fdc4f8 'const int':'const int' lvalue <NoOp>
  //     (DeclRefExpr 0x2fdc3f0 'int' lvalue Var 0x2fd1420 'i' 'int')))
  // We need to emit the AST:
  Expr* Interpreter::constructValuePrinter(Expr* To) {
    // 1. Get the flags
    QualType QT = To->getType();
    if (!QT.isNull() && QT->isVoidType()) {
      return 0;
    } else {
      int Flags = 0;
      enum DumperFlags {
        kIsPtr = 1,
        kIsConst = 2,
        kIsPolymorphic = 4
      };
      
      if (To->isRValue()) Flags |= kIsConst;
      if (QT.isConstant(getCI()->getASTContext()) || QT.isLocalConstQualified()) {
        Flags |= kIsConst;
      }
      if (QT->isPointerType()) {
        // treat arrary-to-pointer decay as array:
        QualType PQT = QT->getPointeeType();
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
      // 2. Call gCling->getValuePrinterStream()
      // 2.1. Find gCling
      Sema& TheSema(getCI()->getSema());
      ASTContext& Ctx(getCI()->getASTContext());
      SourceLocation NoSLoc = SourceLocation();
      VarDecl* VD = dyn_cast<VarDecl>(LookupDecl("cling").LookupDecl("runtime").
                                      LookupDecl("gCling").getSingleDecl());
      assert(VD && "gCling not found!");
      CXXRecordDecl* RD = dyn_cast<CXXRecordDecl>(LookupDecl("cling").
                                                  LookupDecl("Interpreter").
                                                  getSingleDecl()
                                                  );
      QualType RDTy = Ctx.getPointerType(Ctx.getTypeDeclType(RD));
      // 2.2 Find getValuePrinterStream()
      CXXMethodDecl* getValPrinterDecl
        = LookupDecl("getValuePrinterStream", RD).getAs<CXXMethodDecl>();
      assert(getValPrinterDecl && "Decl not found!");
      
      // 2.3 Build a DeclRefExpr, which holds the object
      DeclRefExpr* MemberExprBase = TheSema.BuildDeclRefExpr(VD, RDTy, 
                                                             VK_LValue, NoSLoc
                                                        ).takeAs<DeclRefExpr>();
      // 2.3.1. Implicit cast to RValue
      // MemberExprBase = TheSema.ImpCastExprToType(MemberExprBase, RDTy, 
      //                                            CK_LValueToRValue, LValue).take();
      // MemberExprBase = TheSema.ImpCastExprToType(MemberExprBase, RDTy.addConst(), 
      //                                            CK_NoOp, RValue).take();
      // 2.4 Create a MemberExpr to getMemory from its declaration.
      CXXScopeSpec SS;
      LookupResult MemberLookup(TheSema, getValPrinterDecl->getDeclName(), 
                                NoSLoc, Sema::LookupMemberName);
      // Add the declaration as if doesn't exist. Skips the Lookup, because
      // we have the declaration already so just add it in
      MemberLookup.addDecl(getValPrinterDecl, AS_public);
      MemberLookup.resolveKind();
      Expr* MemberExpr = TheSema.BuildMemberReferenceExpr(MemberExprBase,
                                                          RDTy,
                                                          NoSLoc,
                                                          /*IsArrow=*/true,
                                                          SS,
                                                    /*FirstQualifierInScope=*/0,
                                                          MemberLookup,
                                                          /*TemplateArgs=*/0
                                                          ).take();
      // 2.5 Build the gCling->getValuePrinterStream()
      Scope* S = TheSema.getScopeForContext(TheSema.CurContext);
      Expr* TheInnerCall = TheSema.ActOnCallExpr(S, MemberExpr, NoSLoc,
                                                 MultiExprArg(), NoSLoc).take();

      // 3. Build the final Find cling::valuePrinterInternal::PrintValue call
      // 3.1. Find cling::valuePrinterInternal::PrintValue
      TemplateDecl* TD = dyn_cast<TemplateDecl>(LookupDecl("cling").
                                             LookupDecl("valuePrinterInternal").
                                                LookupDecl("PrintValue").
                                                getSingleDecl());
      // 3.2. Instantiate the TemplateDecl
      FunctionDecl* TDecl = dyn_cast<FunctionDecl>(TD->getTemplatedDecl());
      
      assert(TDecl && "The PrintValue function not found!");

      // Set up new context for the new FunctionDecl
      DeclContext* PrevContext = TheSema.CurContext;      
      TheSema.CurContext = TDecl->getDeclContext();
      
      // Create template arguments
      Sema::InstantiatingTemplate Inst(TheSema, NoSLoc, TDecl);
      // Only the last argument is templated
      TemplateArgument Arg(To->getType());
      TemplateArgumentList TemplateArgs(TemplateArgumentList::OnStack, &Arg, 1U);
      
      // Substitute the declaration of the templated function, with the 
      // specified template argument
      Decl* D = TheSema.SubstDecl(TDecl, 
                                  TDecl->getDeclContext(), 
                                  MultiLevelTemplateArgumentList(TemplateArgs));
      
      FunctionDecl* FD = dyn_cast<FunctionDecl>(D);
      // Creates new body of the substituted declaration
      TheSema.InstantiateFunctionDefinition(FD->getLocation(), FD, true, true);

      m_IncrParser->addConsumer(IncrementalParser::kCodeGenerator,
                                m_ExecutionContext->getCodeGenerator());

      DeclGroupRef DGR(FD);
      // DeclGroupRef DGR1(cast<NamespaceDecl>(FD->getDeclContext()));
      m_ExecutionContext->getCodeGenerator()->HandleTopLevelDecl(DGR);
      // m_ExecutionContext->getCodeGenerator()->HandleTopLevelDecl(DGR1);
      // // generate code for the delta

      m_ExecutionContext->getCodeGenerator()->HandleTopLevelDecl(DGR);
      TheSema.PerformPendingInstantiations();
      
      // Process any TopLevelDecls generated by #pragma weak.
      for (llvm::SmallVector<clang::Decl*,2>::iterator
             I = getCI()->getSema().WeakTopLevelDecls().begin(),
             E = getCI()->getSema().WeakTopLevelDecls().end(); I != E; ++I) {
        m_ExecutionContext->getCodeGenerator()->HandleTopLevelDecl(clang::DeclGroupRef(*I));
      }

      m_ExecutionContext->getCodeGenerator()->HandleTranslationUnit(Ctx);

      TheSema.CurContext = PrevContext;

      // 3.3. Build DeclRefExpr from the found decl
      const FunctionProtoType* FPT = FD->getType()->getAs<FunctionProtoType>();
      FunctionProtoType::ExtProtoInfo EPI = FPT->getExtProtoInfo();
      QualType FnTy = Ctx.getFunctionType(FD->getResultType(),
                                          FPT->arg_type_begin(),
                                          FPT->getNumArgs(),
                                          EPI);
      DeclRefExpr* DRE = TheSema.BuildDeclRefExpr(FD,
                                                  FnTy,
                                                  VK_RValue,
                                                  NoSLoc
                                                  ).takeAs<DeclRefExpr>();
      
      // 3.4. Prepare the params

      // 3.4.1. Create IntegerLiteral, holding the flags
      const llvm::APInt Val(Ctx.getTypeSize(Ctx.IntTy), Flags);
      
      Expr* FlagsIL = IntegerLiteral::Create(Ctx,Val, Ctx.IntTy, NoSLoc);
      
      ASTOwningVector<Expr*> CallArgs(TheSema);
      CallArgs.push_back(TheInnerCall);
      CallArgs.push_back(FlagsIL);
      CallArgs.push_back(To);

      S = TheSema.getScopeForContext(TheSema.CurContext);
      Expr* Result = TheSema.ActOnCallExpr(S, DRE, NoSLoc, 
                                           move_arg(CallArgs), NoSLoc).take();

      return Result;
    }

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
  
  Value Interpreter::Evaluate(const char* expr, DeclContext* DC) {
    // Execute and get the result
    Value Result;

    // Wrap the expression
    const std::string ExprStr(expr);
    std::string WrapperName = createUniqueName();
    std::string Wrapper = "void " + WrapperName + " () {\n";
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
    // resume the code gen
    m_IncrParser->addConsumer(IncrementalParser::kCodeGenerator,
                              m_ExecutionContext->getCodeGenerator());
    DeclGroupRef DGR(TopLevelFD);
    // collect the references that are being used
    m_ExecutionContext->getCodeGenerator()->HandleTopLevelDecl(DGR);
    // generate code for the delta
    m_ExecutionContext->getCodeGenerator()->HandleTranslationUnit(Context);
 
    // get the result
    llvm::GenericValue val;
    if (!TopLevelFD->isExternC()) {
        WrapperName = "";
        llvm::raw_string_ostream RawStr(WrapperName);
        MangleContext* Mangle = Context.createMangleContext();
        Mangle->mangleName(TopLevelFD, RawStr);
    }
    m_ExecutionContext->executeFunction(WrapperName, &val);

    return Value(val, RetTy.getTypePtrOrNull());
  }

  void Interpreter::setCallbacks(InterpreterCallbacks* C) {
    m_IncrParser->getOrCreateTransformer(this)->AttachDynIDHandler();
    enableDynamicLookup(true);
    m_IncrParser->getTransformer()->SetCallbacks(C);
  }

  void Interpreter::enableDynamicLookup(bool value /*=true*/) {
    m_IncrParser->enableDynamicLookup(value);
  }

  bool Interpreter::isDynamicLookupEnabled() {
    return m_IncrParser->isDynamicLookupEnabled();
  }

  void Interpreter::enablePrintAST(bool print /*=true*/) {
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
