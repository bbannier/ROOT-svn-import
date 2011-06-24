//--------------------------------------------------------------------*- C++ -*-
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Axel Naumann <axel@cern.ch>
//------------------------------------------------------------------------------

#ifndef CLING_VALUEPRINTER_H
#define CLING_VALUEPRINTER_H


#include <string>
#include "llvm/Support/raw_ostream.h"

#include "cling/Interpreter/ValuePrinterInfo.h"

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
                  TY* const u, ValuePrinterInfo VPI);


  namespace valuePrinterInternal {
    int debugLevel = 0;
    const char* DebugPrefix() { return "cling::ValuePrinter: "; }

    // Allows to map const T to T, because CanStream<T>
    // is the same whether T is const or not.
    template <typename T> struct NonConst { typedef T Type; };
    template <typename T> struct NonConst<T const> { typedef T Type; };

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
    // strings:
    template<> struct CanStream<char*> { enum {get = 1}; };
    template<> struct CanStream<const char*> { enum {get = 1}; };
    // aka string literals:
    template<int N> struct CanStream<char[N]> { enum {get = 1}; };
    // aka string literals:
    template<typename T, int N> struct CanStream<T[N]> { enum {get = CanStream<T>::get}; };

    template <typename T>
    struct TypeName {
      static const char* get(std::string& n) {
        // Get the type name. This function is always compiled with clang,
        // thus the layout of its name is fixed.
        n = __PRETTY_FUNCTION__;
        n.erase(0,57);
        n.erase(n.length() - 21, std::string::npos);
        size_t l = n.length();
        if (n[l - 1] == ' ') n.erase(l - 1);
        return n.c_str();
      }
    };


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
    std::string TypeName;
    valuePrinterInternal::TypeName<T>::get(TypeName);
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
      if (valuePrinterInternal::debugLevel > 1)
        o << valuePrinterInternal::DebugPrefix() << __PRETTY_FUNCTION__ << '\n';

      o << "@" << (void*)&v << '\n';
    }
  };
  template <typename T, bool CANSTREAM>
  struct StreamObject<T*, CANSTREAM> {
    StreamObject(llvm::raw_ostream& o, const T* v, int) {
      if (valuePrinterInternal::debugLevel > 1)
        o << valuePrinterInternal::DebugPrefix() << __PRETTY_FUNCTION__ << '\n';

      o << (const void*)v << '\n';
    }
  };
  template <typename T>
  struct StreamObject<T,true> {
    StreamObject(llvm::raw_ostream& o, const T& v, int) {
      if (valuePrinterInternal::debugLevel > 1)
        o << valuePrinterInternal::DebugPrefix() << __PRETTY_FUNCTION__ << '\n';

      o << v << '\n'; }
  };
  template <typename T, int N>
  struct StreamObject<T[N], true> {
    StreamObject(llvm::raw_ostream& o, const T* v, int) {
      if (valuePrinterInternal::debugLevel > 1)
        o << valuePrinterInternal::DebugPrefix() << __PRETTY_FUNCTION__ << '\n';

      for (int i = 0; i < N; ++i)
        StreamObject(o, v[i], 0);
    }
  };
  template<>
  struct StreamObject<char*, true> {
    StreamObject(llvm::raw_ostream& o, const char* v, int) {
      // Needed to surround strings with '"', only print 128 chars:
      if (valuePrinterInternal::debugLevel > 1)
        o << valuePrinterInternal::DebugPrefix() << __PRETTY_FUNCTION__ << '\n';

      o << '"';
      const char* p = v;
      for (;*p && p - v < 128; ++p) {
        o << *p;
      }
      if (*p) o << "\"...\n";
      else o << "\"\n";
    }
  };
  template<>
  struct StreamObject<const char*, true>: public StreamObject<char*, true> {
    StreamObject(llvm::raw_ostream& o, const char* v, int flags):
      StreamObject<char*, true>(o, v, flags) {
      if (valuePrinterInternal::debugLevel > 1)
        o << valuePrinterInternal::DebugPrefix() << __PRETTY_FUNCTION__ << '\n';
    }
  };

  template<int N>
  struct StreamObject<char[N], true> {
    StreamObject(llvm::raw_ostream& o, char* v, int) {
    if (valuePrinterInternal::debugLevel > 1)
      o << valuePrinterInternal::DebugPrefix() << __PRETTY_FUNCTION__ << '\n';

    // Needed to surround strings with '"'
    o << '"' << v << "\"\n"; }
  };
  
  // Can be re-implemented to print type-specific details, e.g. as
  //   template <typename ACTUAL>
  //   void dumpPtr(llvm::raw_ostream& o, const clang::Decl* a,
  //                ACTUAL* ap, int flags, const char* tname);
  template <typename TY>
  void printValue(llvm::raw_ostream& o, const void* const t,
                  TY* const u, ValuePrinterInfo PVI) {
    typedef typename valuePrinterInternal::NonConst<TY>::Type NonConstType;

    if (valuePrinterInternal::debugLevel > 0)
      o << valuePrinterInternal::DebugPrefix() << __PRETTY_FUNCTION__
        << ": t=" << t << " u=" << (void*) u << " flags=" << PVI.m_Flags << " tname=" << PVI.m_TypeName.c_str() << '\n';

    if (valuePrinterInternal::debugLevel > 1)
      o << valuePrinterInternal::DebugPrefix()
        << "StreamObject<NonConstType*, " << valuePrinterInternal::CanStream<NonConstType*>::get
        << ">, StreamObject<NonConstType, " << valuePrinterInternal::CanStream<NonConstType>::get << ">\n";

    o << "(" << PVI.m_TypeName.c_str();
    if (PVI.m_Flags & ValuePrinterInfo::VPI_Const) {
      o << " const";
    }
    o << ") ";
    if (PVI.m_Flags & ValuePrinterInfo::VPI_Ptr) {
      StreamObject<NonConstType*, valuePrinterInternal::CanStream<NonConstType*>::get>(o, (NonConstType*)u, PVI.m_Flags);
    } else {
      StreamObject<NonConstType, valuePrinterInternal::CanStream<NonConstType>::get>(o, *(NonConstType*)u, PVI.m_Flags);
    }

  }

}

#endif // CLING_VALUEPRINTER_H
