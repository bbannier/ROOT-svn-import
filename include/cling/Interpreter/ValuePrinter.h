//--------------------------------------------------------------------*- C++ -*-
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Axel Naumann <axel@cern.ch>
//------------------------------------------------------------------------------

#ifndef CLING_VALUEPRINTER_H
#define CLING_VALUEPRINTER_H

#include "llvm/Support/raw_ostream.h"

#include "cling/Interpreter/ValuePrinterInfo.h"


namespace cling {
  
  // Can be re-implemented to print type-specific details, e.g. as
  //   template <typename ACTUAL>
  //   void dumpPtr(llvm::raw_ostream& o, const clang::Decl* a, ACTUAL* ac,
  //                int flags, const char* tname);
  template <typename TY>
  void printValue(llvm::raw_ostream& o, const void* const p,
                  TY* const u, const ValuePrinterInfo& VPI);

  void printValueDefault(llvm::raw_ostream& o, const void* const p, 
                         const ValuePrinterInfo& PVI);

  namespace valuePrinterInternal {
    template <typename T> struct NonConst { typedef T Type; };
    template <typename T> struct NonConst<T const> { typedef T Type; };

    template <typename T>
    struct ToNonConstPtr {
      typedef typename NonConst<T>::Type* Type;
      static Type get(const T& t) {return (Type)&t;}
    };
    
    template <typename T>
    struct ToNonConstPtr<T*> {
      typedef typename NonConst<T*>::Type Type;
      static Type get(const T* t) {return (Type)t;}
    };

    template <typename T>
    void PrintValue(llvm::raw_ostream& o, ValuePrinterInfo PVI, const T& value)
    {
      typename ToNonConstPtr<T>::Type V = ToNonConstPtr<T>::get(value);
      printValue(o, V, V, PVI);
      o.flush();
    }
    
  } // namespace valuePrinterInternal
  
  // Can be reimplemented to stream an object (not a pointer!) of
  // type T.
  template <typename T, bool CANSTREAM>
  struct StreamObject {
    StreamObject(llvm::raw_ostream& o, const T& v, int) {      
      o << "@" << (void*)&v << '\n';
    }
  };
  template <typename T, bool CANSTREAM>
  struct StreamObject<T*, CANSTREAM> {
    StreamObject(llvm::raw_ostream& o, const T* v, int) {
      o << (const void*)v << '\n';
    }
  };
  template <typename T>
  struct StreamObject<T,true> {
    StreamObject(llvm::raw_ostream& o, const T& v, int) {
      o << v << '\n'; }
  };
  template <typename T, int N>
  struct StreamObject<T[N], true> {
    StreamObject(llvm::raw_ostream& o, const T* v, int) {
      for (int i = 0; i < N; ++i)
        StreamObject(o, v[i], 0);
    }
  };
  template<int N>
  struct StreamObject<char[N], true> {
    StreamObject(llvm::raw_ostream& o, char* v, int) {

    // Needed to surround strings with '"'
    o << '"' << v << "\"\n"; }
  };
  
  // Can be re-implemented to print type-specific details, e.g. as
  //   template <typename ACTUAL>
  //   void dumpPtr(llvm::raw_ostream& o, const clang::Decl* a,
  //                ACTUAL* ap, int flags, const char* tname);
  template <typename TY>
  void printValue(llvm::raw_ostream& o, const void* const p,
                  TY* const u, const ValuePrinterInfo& PVI) {
    printValueDefault(o, p, PVI);
  }

}

#endif // CLING_VALUEPRINTER_H
