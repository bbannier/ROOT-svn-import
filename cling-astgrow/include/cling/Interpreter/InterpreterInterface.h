//--------------------------------------------------------------------*- C++ -*-
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Axel Naumann <axel@cern.ch>
//------------------------------------------------------------------------------

#ifndef CLING_INTERPRETERINTERFACE_H
#define CLING_INTERPRETERINTERFACE_H

namespace clang {
class CompilerInterface;
}

namespace cling {
class InterpreterInterface {
public:
   //---------------------------------------------------------------------
   //! Constructor
   //---------------------------------------------------------------------
   InterpreterInterface(const char* llvmdir = 0);
   ~InterpreterInterface();

   clang::CompilerInstance* createCI() const;

private:
   std::string m_llvmdir;
   mutable llvm::LLVMContext* m_llvm_context; // We own, our types.
};
}
