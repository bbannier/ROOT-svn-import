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
  void printValue(llvm::raw_ostream& o, const void* const p,
                  TY* const u, int flags, const char* tname);

  class ValuePrinter {
  public:
    
    enum DumperFlags {
      kIsObj = 1,
      kIsConst = 2
    };

  protected:
    template <typename TY>
    struct TypedPrinter {
      TypedPrinter(llvm::raw_ostream& o, const TY& t, int isConst) {
        std::string n;
        printValue(o, (TY*)&t, (TY*)&t, kIsObj | isConst, TypeName<TY>::get(n));
      }
    };
    
    template <typename TY>
    struct TypedPrinter<TY*> {
      TypedPrinter(llvm::raw_ostream& o, const TY* t, int isConst) {
        std::string n;
        printValue(o, (TY*)t, (TY*)t, /*isConst*/0, TypeName<TY*>::get(n));
      }
    };

  public:
    template <typename T>
    ValuePrinter(llvm::raw_ostream& o, const T& t) {
      // give temporaries a defined lifetime
      TypedPrinter<T>(o, t, kIsConst);
    }
    
    template <typename T>
    ValuePrinter(llvm::raw_ostream& o, T& t) {
      // give temporaries a defined lifetime
      TypedPrinter<T>(o, t, 0);
    }
    
    template <typename T>
    struct TypeName {
      static const char* get(std::string& n) {
        // Get the type name. This function is always compiled with clang,
        // thus the layout of its name is fixed.
        n = __PRETTY_FUNCTION__;
        n.erase(0,49);
        n.erase(n.length() - 21, std::string::npos);
        size_t l = n.length();
        if (n[l - 1] == ' ') n.erase(l - 1);
        return n.c_str();
      }
    };
  };

  // Allows to map const T to T, because CanStream<T>
  // is the same whether T is const or not.
  template <typename T> struct NoConst { typedef T Type; };
  template <typename T> struct NoConst<T const> { typedef T Type; };

  // CanStream<T>::get is 0 if a T object cannot be streamed to
  // llvm::raw_ostream, 1 otherwise. This can be specialized for user
  // types defining thier own streaming operators.
  template<typename T> struct CanStream { enum {get = 0}; };
  template<> struct CanStream<bool> { enum {get = 1}; };
  template<> struct CanStream<char> { enum {get = 1}; };
  template<> struct CanStream<unsigned char> { enum {get = 1}; };
  template<> struct CanStream<short> { enum {get = 1}; };
  template<> struct CanStream<unsigned short> { enum {get = 1}; };
  template<> struct CanStream<int> { enum {get = 1}; };
  template<> struct CanStream<unsigned int> { enum {get = 1}; };
  template<> struct CanStream<long> { enum {get = 1}; };
  template<> struct CanStream<unsigned long> { enum {get = 1}; };
  template<> struct CanStream<long long> { enum {get = 1}; };
  template<> struct CanStream<unsigned long long> { enum {get = 1}; };
  template<> struct CanStream<float> { enum {get = 1}; };
  template<> struct CanStream<double> { enum {get = 1}; };
  template<> struct CanStream<long double> { enum {get = 1}; };
  // char* is often an arbitrary memory address with ptr arith,
  // so don't print as text:
  template<> struct CanStream<char*> { enum {get = 0}; };
  template<> struct CanStream<const char*> { enum {get = 1}; };
  // aka string literals:
  template<int N> struct CanStream<char[N]> { enum {get = 1}; };

  // Can be reimplemented to stream an object (not a pointer!) of
  // type T.
  template <typename T, bool CANSTREAM>
  struct StreamObject {
    StreamObject(llvm::raw_ostream& o, const T& v) {
      o << "@" << (void*)&v << '\n';
    }
  };
  template <typename T>
  struct StreamObject<T,true> {
    StreamObject(llvm::raw_ostream& o, const T& v) {
      o << v << '\n'; }
  };
  template<>
  struct StreamObject<const char*, true> {
    StreamObject(llvm::raw_ostream& o, const char* v) {
    // Needed to surround strings with '"'
    o << '"' << v << "\"\n"; }
  };
  template<int N>
  struct StreamObject<char[N], true> {
    StreamObject(llvm::raw_ostream& o, const char* v) {
      // Needed to surround strings with '"'
      o << '"' << v << "\"\n"; }
  };
  
  // Can be re-implemented to print type-specific details, e.g. as
  //   template <typename ACTUAL>
  //   void dumpPtr(llvm::raw_ostream& o, const clang::Decl* a,
  //                ACTUAL* ap, int flags, const char* tname);
  template <typename TY>
  inline
  void printValue(llvm::raw_ostream& o, const void* const t,
                  TY* const u, int flags, const char* tname) {
    o << "(" << tname;
    if (flags & ValuePrinter::kIsConst) {
      o << " const";
    }
    o << ") ";
    if (flags & ValuePrinter::kIsObj) {
      StreamObject<TY, CanStream<TY>::get
      || CanStream<typename NoConst<TY>::Type>::get>(o, *u);
    } else {
      o << t << '\n';
    }
    o.flush();
  }

}

#endif // CLING_VALUEPRINTER_H
