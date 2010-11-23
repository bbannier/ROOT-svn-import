//--------------------------------------------------------------------*- C++ -*-
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id: Interpreter.h 34643 2010-07-28 20:41:05Z russo $
// author:  Axel Naumann <axel@cern.ch>
//------------------------------------------------------------------------------

#include "cling/Interpreter/CIBuilder.h"

#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/Version.h"
#include "clang/Frontend/TextDiagnosticBuffer.h"

#include "llvm/Target/TargetSelect.h"
#include "llvm/LLVMContext.h"


namespace cling {
//
//  Dummy function so we can use dladdr to find the executable path.
//
void locate_cling_executable()
{
}

}

cling::CIBuilder::CIBuilder(int argc, const char* const *argv, const char* llvmdir):
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

cling::CIBuilder::~CIBuilder()
{
   // Destruct the instance builder
}

bool
cling::CIBuilder::createDiagnostics(clang::CompilerInstance* &CI) const
{
   // Create diagnostics. Deletes and zeroes CI in case of problems.
   CI->createDiagnostics(m_argc - 1, m_argv + 1);
   if (!CI->hasDiagnostics()) {
      CI->takeLLVMContext();
      delete CI;
      CI = 0;
      return false;
   }
   return true;
}

clang::CompilerInstance*
cling::CIBuilder::createCI() const {
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
      clang::TextDiagnosticBuffer *DiagsBuffer = new clang::TextDiagnosticBuffer();
      clang::DiagnosticOptions DiagOpts;
      clang::DiagnosticClient *Client = new clang::DiagnosticClient();
      llvm::IntrusiveRefCntPtr<clang::Diagnostic> Diags = clang::CompilerInstance::createDiagnostics(DiagOpts, 0, 0, Client);


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
      if (!createDiagnostics(CI)) {
         return 0;
      }
      // Output the buffered error messages now.
      DiagsBuffer->FlushDiagnostics(CI->getDiagnostics());
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
