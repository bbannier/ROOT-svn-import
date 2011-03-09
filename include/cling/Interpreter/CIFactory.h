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
    //---------------------------------------------------------------------
    //! Constructor
    //---------------------------------------------------------------------
    CIFactory(int argc = 0, const char* const *argv = 0, const char* llvmdir = 0);
    ~CIFactory();
    
    clang::CompilerInstance* createCI(llvm::MemoryBuffer* buffer, clang::PragmaNamespace* Pragma = 0) const;
    clang::CompilerInstance* createCI(llvm::StringRef code, clang::PragmaNamespace* Pragma = 0) const;
    llvm::sys::Path getResourcePath(){return m_resource_path;}
    
  private:
    int         m_argc;
    const char* const *m_argv;
    llvm::sys::Path m_resource_path;
    mutable llvm::OwningPtr<llvm::LLVMContext> m_llvm_context; // We own, our types.
  };
} // namespace cling
#endif // CLING_CIFACTORY_H
