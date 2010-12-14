//--------------------------------------------------------------------*- C++ -*-
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Axel Naumann <axel@cern.ch>
//------------------------------------------------------------------------------

#ifndef CLING_VALUEPRINTER_H
#define CLING_VALUEPRINTER_H


#include <string>

namespace llvm {
  class raw_ostream;
}

namespace cling {
  
  // Can be re-implemented to print type-specific details, e.g. as
  //   template <typename ACTUAL>
  //   void dumpPtr(llvm::raw_ostream& o, const clang::Decl* a, ACTUAL* ac,
  //                int flags, const char* tname);
  template <typename TY>
  void printValue(llvm::raw_ostream& o, const void* const t, TY* u, int flags,
                  const char* tname);
  
  class ValuePrinter {
  public:
    
    enum DumperFlags {
      kIsObj = 1,
      kIsConst = 2
    };

  protected:
    template <typename TY>
    struct TypedPrinter {
      void operator()(llvm::raw_ostream& o, const TY& t) {
        std::string n;
        printValue(o, &t, &t, kIsObj | kIsConst, getTypeName(&t, n));
      }
      void operator()(llvm::raw_ostream& o, TY& t) {
        std::string n;
        printValue(o, &t, &t, kIsObj, getTypeName(&t, n));
      }
    };
    
    template <typename TY>
    struct TypedPrinter<TY*> {
      void operator()(llvm::raw_ostream& o, TY* t) {
        std::string n;
        printValue(o, t, t, 0, getTypeName(t, n));
      }
    };

  public:
    template <typename T>
    ValuePrinter(llvm::raw_ostream& o, const T& t) {
      // give temporaries a defined lifetime
      TypedPrinter<T> d; d(o, t);
    }
    
    template <typename T>
    ValuePrinter(llvm::raw_ostream& o, T& t) {
      // give temporaries a defined lifetime
      TypedPrinter<T> d; d(o, t);
    }
    
    template <typename T>
    static
    const char* getTypeName(T*,std::string& n) {
      // Get the type name. This function is always compiled with clang,
      // thus the layout of its name is fixed.
      n = __PRETTY_FUNCTION__;
      n.erase(0,24);
      n.erase(n.length() - 17, std::string::npos);
      size_t l = n.length();
      if (n[l - 1] == ' ') n.erase(l - 1);
      return n.c_str();
    }
  };

  // Can be re-implemented to print type-specific details, e.g. as
  //   template <typename ACTUAL>
  //   void dumpPtr(llvm::raw_ostream& o, const clang::Decl* a, ACTUAL* ac,
  //                int flags, const char* tname);
  template <typename TY>
  inline
  void printValue(llvm::raw_ostream& o, const void* const t, TY* u, int flags,
                  const char* tname) {
    if (flags & ValuePrinter::kIsConst) {
      o << "const ";
    }
    o << tname;
    if (!(flags & ValuePrinter::kIsObj)) {
      o << "*";
    }
    o << " @ " << t << '\n';
  }
  
}

#endif // CLING_VALUEPRINTER_H
