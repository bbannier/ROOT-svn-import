//--------------------------------------------------------------------*- C++ -*-
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id: Interpreter.h 34643 2010-07-28 20:41:05Z russo $
// author:  Axel Naumann <axel@cern.ch>
//------------------------------------------------------------------------------

#include "cling/Interpreter/CIBuilder.h"

#include "cling/Interpreter/Diagnostics.h"

#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/Version.h"

#include "llvm/Target/TargetSelect.h"
#include "llvm/LLVMContext.h"

#include "ChainedASTConsumer.h"


namespace cling {
   //
   //  Dummy function so we can use dladdr to find the executable path.
   //
   void locate_cling_executable()
   {
   }
   
   CIBuilder::CIBuilder(int argc, const char* const *argv, const char* llvmdir):
      m_argc(argc),
      m_argv(argv),
      m_llvm_context(0)
   {
      // Create an instance builder, passing the llvmdir and arguments.
      
      //
      //  Initialize the llvm library.
      //
      llvm::InitializeAllTargets();
      llvm::InitializeAllAsmPrinters();
      m_llvm_context.reset(new llvm::LLVMContext);
      
      if (llvmdir) {
         m_resource_path = llvmdir;
         m_resource_path.appendComponent("lib");
         m_resource_path.appendComponent("clang");
         m_resource_path.appendComponent(CLANG_VERSION_STRING);
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
         m_resource_path =
            clang::CompilerInvocation::GetResourcesPath
            ("cling", (void*)(intptr_t) locate_cling_executable);
      }
   }
   
   CIBuilder::~CIBuilder()
   {
      // Destruct the instance builder
   }
   
   clang::CompilerInstance*
   CIBuilder::createCI() const {
      //
      // Create and setup a compiler instance.
      //
      clang::CompilerInstance* CI = new clang::CompilerInstance();
      CI->setLLVMContext(m_llvm_context.get());
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
             (m_argv + 1),
             (m_argv + m_argc),
             *Diags
             );
         if (CI->getHeaderSearchOpts().UseBuiltinIncludes &&
             CI->getHeaderSearchOpts().ResourceDir.empty()) {
            CI->getHeaderSearchOpts().ResourceDir = m_resource_path.str();
         }
         if (CI->getDiagnostics().hasErrorOccurred()) {
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
      CI->createSourceManager(CI->getFileManager());
      //CI->createSourceManager();
      //CI->createPreprocessor(); // Note: This single line takes almost all the time!
      
      return CI;
   }
   
   // Pin this vtable to this file.
   ChainedASTConsumer::~ChainedASTConsumer() {}
} // end namespace
