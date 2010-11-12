//------------------------------------------------------------------------------
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Axel Naumann <axel@cern.ch>
//------------------------------------------------------------------------------

#ifndef CLING_INPUT_VALIDATOR_H
#define CLING_INPUT_VALIDATOR_H

#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/OwningPtr.h"

namespace clang {
  class CompilerInstance;
  class Preprocessor;
}

namespace cling {
  class InputValidator {
  public:
    enum Result {
      kUnknown, // initialized
      kMismatch, // e.g. { ( }
      kParseError, // e.g. %$#
      kUnbalanced, // e.g. { ( )
      kValid, // e.g. { ( ) }
      kNumResults
    };
    
    InputValidator(clang::CompilerInstance* CI);
    ~InputValidator();
    
    Result validate(llvm::StringRef code);
  
  private:
    llvm::OwningPtr<clang::CompilerInstance> m_CI;
    clang::Preprocessor* m_PP;
  };
}
#endif // CLING_INPUT_VALIDATOR_H
