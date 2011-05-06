//--------------------------------------------------------------------*- C++ -*-
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Axel Naumann <axel@cern.ch>
//------------------------------------------------------------------------------

#ifndef CLING_CIFACTORY_H
#define CLING_CIFACTORY_H

#include "clang/Frontend/CompilerInstance.h"

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Path.h"

namespace llvm {
  class LLVMContext;
  class MemoryBuffer;
}

namespace clang {
  class PragmaNamespace;
}

namespace cling {
  class CIFactory {
  public:
    // TODO: Add overload that takes file not MemoryBuffer
    static clang::CompilerInstance* createCI(llvm::StringRef code,
                                             int argc,
                                             const char* const *argv,
                                             const char* llvmdir,
                                             llvm::LLVMContext* llvm_context = 0);

    static clang::CompilerInstance* createCI(llvm::MemoryBuffer* buffer, 
                                             clang::PragmaNamespace* Pragma, 
                                             int argc,
                                             const char* const *argv,
                                             const char* llvmdir,
                                             llvm::LLVMContext* llvm_context = 0);
  private:
    //---------------------------------------------------------------------
    //! Constructor
    //---------------------------------------------------------------------
    CIFactory() {}
    ~CIFactory() {}
  };
} // namespace cling
#endif // CLING_CIFACTORY_H
