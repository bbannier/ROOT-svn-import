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

#include "ChainedASTConsumer.h"

static const char* fake_argv[] = { "clang", "-x", "c++", "-D__CLING__", "-I.", 0 };
static const int fake_argc = (sizeof(fake_argv) / sizeof(const char*)) - 1;

namespace cling {
  //
  //  Dummy function so we can use dladdr to find the executable path.
  //
  void locate_cling_executable()
  {
  }

  clang::CompilerInstance* CIFactory::createCI(llvm::StringRef code) {
    return createCI(llvm::MemoryBuffer::getMemBuffer(code), 0, 0);
  }

  clang::CompilerInstance* CIFactory::createCI(llvm::MemoryBuffer* buffer, 
                                               clang::PragmaNamespace* Pragma, 
                                               const char* llvmdir) {
    return createCI(buffer, Pragma, fake_argc, fake_argv, llvmdir, new llvm::LLVMContext() );

  }

  clang::CompilerInstance* CIFactory::createCI(llvm::StringRef code,
                                               int argc,
                                               const char* const *argv,
                                               const char* llvmdir) {
    return createCI(llvm::MemoryBuffer::getMemBuffer(code), 0, argc, argv, llvmdir, new llvm::LLVMContext());
  }


  
  clang::CompilerInstance* CIFactory::createCI(llvm::MemoryBuffer* buffer, 
                                               clang::PragmaNamespace* Pragma, 
                                               int argc, 
                                               const char* const *argv,
                                               const char* llvmdir,
                                               llvm::LLVMContext* llvm_context){
    if (!Pragma) {
      Pragma = new clang::PragmaNamespace("cling");
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
      resource_path =
        clang::CompilerInvocation::GetResourcesPath
        ("cling", (void*)(intptr_t) locate_cling_executable);
    }


    // Create and setup a compiler instance.
    clang::CompilerInstance* CI = new clang::CompilerInstance();
    {
      //
      //  Buffer the error messages while we process
      //  the compiler options.
      //
      clang::DiagnosticOptions DiagOpts;
      DiagOpts.ShowColors = 1;
      clang::DiagnosticClient *Client = new cling::DiagnosticPrinter();
      llvm::IntrusiveRefCntPtr<clang::Diagnostic> Diags = clang::CompilerInstance::createDiagnostics(DiagOpts, 0, 0, Client);
      CI->setDiagnostics(Diags.getPtr());
      
      clang::CompilerInvocation::CreateFromArgs
        (CI->getInvocation(),
         (argv + 1),
         (argv + argc),
         *Diags
         );
      if (CI->getHeaderSearchOpts().UseBuiltinIncludes &&
          CI->getHeaderSearchOpts().ResourceDir.empty()) {
        CI->getHeaderSearchOpts().ResourceDir = resource_path.str();
      }
      if (CI->getDiagnostics().hasErrorOccurred()) {
        delete CI;
        CI = 0;
        return 0;
      }
    }
    CI->setTarget(clang::TargetInfo::CreateTargetInfo(CI->getDiagnostics(),
                                                      CI->getTargetOpts()));
    if (!CI->hasTarget()) {
      delete CI;
      CI = 0;
      return 0;
    }
    CI->getTarget().setForcedLangOptions(CI->getLangOpts());
    
    // Set up source and file managers
    CI->createFileManager();
    CI->createSourceManager(CI->getFileManager());
    
    // Set up the memory buffer
    if (buffer)
      CI->getSourceManager().createMainFileIDForMemBuffer(buffer);
    
    // Set up the preprocessor
    CI->createPreprocessor();
    clang::Preprocessor& PP = CI->getPreprocessor();
    PP.AddPragmaHandler(Pragma);
    PP.getBuiltinInfo().InitializeBuiltins(PP.getIdentifierTable(),
                                           PP.getLangOptions());
    /*NoBuiltins = */ //true);
    
    // Set up the ASTContext
    clang::ASTContext *Ctx = new clang::ASTContext(CI->getLangOpts(),
                                                   PP.getSourceManager(), CI->getTarget(), PP.getIdentifierTable(),
                                                   PP.getSelectorTable(), PP.getBuiltinInfo(), 0);
    CI->setASTContext(Ctx);
    //CI->getSourceManager().clearIDTables(); //do we really need it?
    
    // Set up the ASTConsumers
    ChainedASTConsumer* Consumer = new ChainedASTConsumer();
    Consumer->Initialize(*Ctx);
    CI->setASTConsumer(Consumer);
    
    
    // Set up Sema
    bool CompleteTranslationUnit = false;
    clang::CodeCompleteConsumer* CCC = 0;
    CI->createSema(CompleteTranslationUnit, CCC);
    
    
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
    
    return CI;
  }  
  // Pin this vtable to this file.
  ChainedASTConsumer::~ChainedASTConsumer() {}
} // end namespace
