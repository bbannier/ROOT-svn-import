//--------------------------------------------------------------------*- C++ -*-
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Axel Naumann <axel@cern.ch>
//------------------------------------------------------------------------------

#include "cling/Interpreter/CIFactory.h"

#include "cling/Interpreter/Diagnostics.h"

#include "clang/AST/ASTContext.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/Version.h"
#include "clang/Lex/Pragma.h"
#include "clang/Lex/Preprocessor.h"

#include "llvm/Target/TargetSelect.h"
#include "llvm/LLVMContext.h"
#include "llvm/Support/MemoryBuffer.h"

#include "ChainedConsumer.h"

using namespace clang;

namespace cling {
  //
  //  Dummy function so we can use dladdr to find the executable path.
  //
  void locate_cling_executable()
  {
  }

  CompilerInstance* CIFactory::createCI(llvm::StringRef code,
                                        int argc,
                                        const char* const *argv,
                                        const char* llvmdir) {
    return createCI(llvm::MemoryBuffer::getMemBuffer(code), 0, argc, argv, llvmdir);
  }

  
  CompilerInstance* CIFactory::createCI(llvm::MemoryBuffer* buffer, 
                                        PragmaNamespace* Pragma, 
                                        int argc, 
                                        const char* const *argv,
                                        const char* llvmdir){
    // main's argv[0] is skipped!

    if (!Pragma) {
      Pragma = new PragmaNamespace("cling");
    }

    // Create an instance builder, passing the llvmdir and arguments.
    //
    //  Initialize the llvm library.
    //
    llvm::InitializeAllTargets();
    llvm::InitializeAllAsmPrinters();
    llvm::sys::Path resource_path;
    if (llvmdir) {
      resource_path = llvmdir;
      resource_path.appendComponent("lib");
      resource_path.appendComponent("clang");
      resource_path.appendComponent(CLANG_VERSION_STRING);
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
      resource_path = CompilerInvocation::GetResourcesPath("cling",
                                       (void*)(intptr_t) locate_cling_executable
                                                           );
    }


    // Create and setup a compiler instance.
    CompilerInstance* CI = new CompilerInstance();
    {
      //
      //  Buffer the error messages while we process
      //  the compiler options.
      //
      DiagnosticOptions DiagOpts;
      DiagOpts.ShowColors = 1;
      DiagnosticClient *Client = new cling::DiagnosticPrinter();
      llvm::IntrusiveRefCntPtr<Diagnostic> Diags = CompilerInstance::createDiagnostics(DiagOpts, 0, 0, Client);
      CI->setDiagnostics(Diags.getPtr());
      
      CompilerInvocation::CreateFromArgs
        (CI->getInvocation(), argv, argv + argc, *Diags);
      // Set the language options, which cling needs
      SetClingCustomLangOpts(CI->getLangOpts());

      if (CI->getHeaderSearchOpts().UseBuiltinIncludes &&
          CI->getHeaderSearchOpts().ResourceDir.empty()) {
        CI->getHeaderSearchOpts().ResourceDir = resource_path.str();
      }
      CI->getInvocation().getPreprocessorOpts().addMacroDef("__CLING__");
      if (CI->getDiagnostics().hasErrorOccurred()) {
        delete CI;
        CI = 0;
        return 0;
      }
    }
    CI->setTarget(TargetInfo::CreateTargetInfo(CI->getDiagnostics(),
                                               CI->getTargetOpts()));
    if (!CI->hasTarget()) {
      delete CI;
      CI = 0;
      return 0;
    }
    CI->getTarget().setForcedLangOptions(CI->getLangOpts());
    SetClingTargetLangOpts(CI->getLangOpts(), CI->getTarget());
    
    // Set up source and file managers
    CI->createFileManager();
    CI->createSourceManager(CI->getFileManager());
    
    // Set up the memory buffer
    if (buffer)
      CI->getSourceManager().createMainFileIDForMemBuffer(buffer);
    
    // Set up the preprocessor
    CI->createPreprocessor();
    Preprocessor& PP = CI->getPreprocessor();
    PP.AddPragmaHandler(Pragma);
    PP.getBuiltinInfo().InitializeBuiltins(PP.getIdentifierTable(),
                                           PP.getLangOptions());
    /*NoBuiltins = */ //true);
    
    // Set up the ASTContext
    ASTContext *Ctx = new ASTContext(CI->getLangOpts(),
                                     PP.getSourceManager(), CI->getTarget(), PP.getIdentifierTable(),
                                     PP.getSelectorTable(), PP.getBuiltinInfo(), 0);
    CI->setASTContext(Ctx);
    //CI->getSourceManager().clearIDTables(); //do we really need it?
    
    // Set up the ASTConsumers
    CI->setASTConsumer(new ChainedConsumer());
    
    
    // Set up Sema
    bool CompleteTranslationUnit = false;
    CodeCompleteConsumer* CCC = 0;
    CI->createSema(CompleteTranslationUnit, CCC);

    return CI;
  }

  void CIFactory::SetClingCustomLangOpts(clang::LangOptions& Opts) {
    Opts.EmitAllDecls = 1;
  }

  void CIFactory::SetClingTargetLangOpts(clang::LangOptions& Opts,
                                         const clang::TargetInfo& Target) {
    if (Target.getTriple().getOS() == llvm::Triple::Win32) {
      Opts.Microsoft = 1;
      Opts.MSCVersion = 1300;
    }
  }
} // end namespace
