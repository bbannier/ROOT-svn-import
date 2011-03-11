//------------------------------------------------------------------------------
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Vassil Vassilev <vasil.georgiev.vasilev@cern.ch>
//------------------------------------------------------------------------------

namespace llvm {
  class GenericValue;
}
namespace clang {
  class Type;
}

namespace cling {
  // The class represents a llvm::GenericValue with its corresponding 
  // clang::Type. There are two use-cases for that:
  // 1. Expression evaluation: we need to know the type of the GenericValue
  // that we have gotten from the JIT
  // 2. Value printer: needs to know the type in order to skip the printing of
  // void types
  class Value {
  public:
    llvm::GenericValue value;
    clang::Type* type;

    Value() : value(0), type(0) {}
    ~Value() {}
  };
} // end namespace cling
