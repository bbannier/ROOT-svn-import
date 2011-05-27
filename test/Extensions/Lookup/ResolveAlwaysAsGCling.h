//------------------------------------------------------------------------------
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id: InterpreterCallbacks.h 39299 2011-05-20 12:53:30Z vvassilev $
// author:  Vassil Vassilev <vasil.georgiev.vasilev@cern.ch>
//------------------------------------------------------------------------------

#ifndef CLING_TEST_RESOLVE_ALWAYS_AS_GCLING
#define CLING_TEST_RESOLVE_ALWAYS_AS_GCLING

#include "cling/Interpreter/Interpreter.h"
#include "cling/Interpreter/InterpreterCallbacks.h"

#include "clang/Sema/Lookup.h"

namespace cling {
  namespace test {
    class ResolveAlwaysAsGClingCallback: public cling::InterpreterCallbacks {
    public:
      ResolveAlwaysAsGClingCallback(Interpreter* interp)
        : InterpreterCallbacks(interp) {}
      
      ~ResolveAlwaysAsGClingCallback(){}
      bool LookupObject(clang::LookupResult& R, clang::Scope* S) {
        // Only for demo resolve all unknown objects to gCling
        clang::NamedDecl* D = m_Interpreter->LookupDecl("cling").LookupDecl("runtime").LookupDecl("gCling");
        assert (D && "gCling not found!");
        R.addDecl(D);
        return true;
      }
    };
  } // end test
} // end cling

#endif //CLING_TEST_RESOLVE_ALWAYS_AS_GCLING
