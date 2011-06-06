//------------------------------------------------------------------------------
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id: InterpreterCallbacks.h 39299 2011-05-20 12:53:30Z vvassilev $
// author:  Vassil Vassilev <vasil.georgiev.vasilev@cern.ch>
//------------------------------------------------------------------------------

#ifndef CLING_TEST_SYMBOL_RESOLVER_CALLBACK
#define CLING_TEST_SYMBOL_RESOLVER_CALLBACK

#include "cling/Interpreter/Interpreter.h"
#include "cling/Interpreter/InterpreterCallbacks.h"

#include "clang/Sema/Lookup.h"

namespace cling {
  namespace test {
    extern "C" int printf(const char* fmt, ...);
    class TestProxy {
    public:
      TestProxy(){}
      int Draw(){ return 12; }
      const char* getVersion(){ return "Interpreter.cpp"; }

      int Add10(int num) { return num + 10;}
      void PrintString(std::string s) { printf("%s\n", s.c_str()); }
      void PrintArray(int a[], size_t size) {
        for (unsigned i = 0; i < size; ++i)
          printf("%i", a[i]);
      }
    };

    TestProxy* Tester = 0;

    class SymbolResolverCallback: public cling::InterpreterCallbacks {
    public:
      SymbolResolverCallback(Interpreter* interp)
        : InterpreterCallbacks(interp) {
        interp->processLine("cling::test::Tester = new cling::test::TestProxy();");
      }
      
      ~SymbolResolverCallback(){}
      bool LookupObject(clang::LookupResult& R, clang::Scope* S) {
        // Only for demo resolve all unknown objects to gCling
        clang::NamedDecl* D = m_Interpreter->LookupDecl("cling").LookupDecl("test").LookupDecl("Tester");
        assert (D && "Tester not found!");
        R.addDecl(D);
        return true;
      }
    };
  } // end test
} // end cling

#endif // CLING_TEST_SYMBOL_RESOLVER_CALLBACK
