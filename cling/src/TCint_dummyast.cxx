#include "clang/AST/ASTContext.h"
#include "llvm/ADT/OwningPtr.h"
#include <llvm/System/Host.h>
#include <llvm/System/Path.h>
#include "clang/Lex/Preprocessor.h"
#include "clang/Lex/HeaderSearch.h"
#include "clang/Frontend/InitPreprocessor.h"
#include <clang/Frontend/InitHeaderSearch.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/Basic/LangOptions.h>
#include <clang/Basic/FileManager.h>
#include <clang/Basic/Version.h>
#include <llvm/Target/TargetSelect.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/FormattedStream.h>
#include <clang/Sema/ParseAST.h>
#include "clang/AST/ASTConsumer.h"

clang::ASTContext* ClingGimmeAnAST() {
   // Insert the environment so that the code input
   // to insure all the appropriate symbols are defined.

   static clang::ASTContext* sASTContext = 0;
   if (!sASTContext) {
      std::string src="class MyClass{};";

      clang::LangOptions langInfo;
      //langInfo.C99         = 1;
      langInfo.HexFloats   = 1;
      langInfo.BCPLComment = 1; // Only for C99/C++.
      langInfo.Digraphs    = 1; // C94, C99, C++.
      langInfo.CPlusPlus   = 1;
      langInfo.CPlusPlus0x = 1;
      langInfo.CXXOperatorNames = 1;
      langInfo.Bool = 1;

      langInfo.NeXTRuntime = 1;
      langInfo.NoInline = 1;
   
      langInfo.Exceptions = 1;
      langInfo.GNUMode = 1;
      langInfo.NoInline = 1;
      langInfo.GNUInline = 1;
      langInfo.DollarIdents = 1;
 
      llvm::OwningPtr<clang::TextDiagnosticPrinter>
         diagPrinter(new clang::TextDiagnosticPrinter( llvm::errs() ));
      diagPrinter->setLangOptions(&langInfo);
      clang::Diagnostic diag(diagPrinter.get());
      llvm::OwningPtr<clang::SourceManager> sm(new clang::SourceManager);
      llvm::OwningPtr<clang::FileManager> fileMgr(new clang::FileManager);
      llvm::OwningPtr<clang::HeaderSearch> headerInfo(new clang::HeaderSearch(*fileMgr.get()));
   
      // target:
      llvm::InitializeNativeTarget();
      llvm::OwningPtr<clang::TargetInfo>
         target(clang::TargetInfo::CreateTargetInfo(llvm::sys::getHostTriple()));
      {
         llvm::StringMap<bool> Features;
         target->getDefaultFeatures("", Features);
         target->HandleTargetFeatures(Features);
      }
      target->getDefaultLangOptions(langInfo);

      clang::InitHeaderSearch hiInit(*headerInfo.get());
      hiInit.AddDefaultEnvVarPaths(langInfo);
      hiInit.AddDefaultSystemIncludePaths(langInfo, target->getTriple());
      llvm::sys::Path clangIncl(LLVM_LIBDIR, strlen(LLVM_LIBDIR));
      clangIncl.appendComponent("clang");
      clangIncl.appendComponent(CLANG_VERSION_STRING);
      clangIncl.appendComponent("include");
      hiInit.AddPath( clangIncl.c_str(), clang::InitHeaderSearch::System,
                      true, false, false, true /*ignore sysroot*/);
      hiInit.Realize();

      llvm::OwningPtr<clang::Preprocessor> preproc(new clang::Preprocessor(diag, langInfo, *target.get(), *sm.get(), *headerInfo));
      clang::PreprocessorInitOptions ppOptions;
      InitializePreprocessor(*preproc.get(), ppOptions);
      preproc->getBuiltinInfo().InitializeBuiltins(preproc->getIdentifierTable(),
                                                preproc->getLangOptions().NoBuiltin);

      sASTContext = new clang::ASTContext(langInfo,
                                          *sm.get(),
                                          *target.get(),
                                          preproc->getIdentifierTable(),
                                          preproc->getSelectorTable(),
                                          preproc->getBuiltinInfo());

      llvm::MemoryBuffer* buffer = llvm::MemoryBuffer::getMemBufferCopy(&*src.begin(),
                                                                        &*src.end(),
                                                                        "TCint_dummyast" );
      sm->createMainFileIDForMemBuffer( buffer );

      clang::ASTConsumer consumer;
      clang::ParseAST( *preproc.get(), &consumer, *sASTContext);
   }
   return sASTContext;
}
