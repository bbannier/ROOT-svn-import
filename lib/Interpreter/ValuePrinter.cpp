//--------------------------------------------------------------------*- C++ -*-
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id: ValuePrinter.h 39950 2011-06-24 15:42:26Z vvassilev $
// author:  Vassil Vassilev <vasil.georgiev.vasilev@cern.ch>
//------------------------------------------------------------------------------

#include "cling/Interpreter/ValuePrinter.h"
#include "cling/Interpreter/ValuePrinterInfo.h"

#include "clang/AST/Expr.h"
#include "clang/AST/Type.h"

namespace cling {
  void StreamChar(llvm::raw_ostream& o, char v) {
    o << '"' << v << "\"\n";
  }

  void StreamCharPtr(llvm::raw_ostream& o, const char* const v) {
    o << '"';
    const char* p = v;
    for (;*p && p - v < 128; ++p) {
      o << *p;
    }
    if (*p) o << "\"...\n";
    else o << "\"\n";
  }

  void StreamInt(llvm::raw_ostream& o, int v) {
    o << v << "\n";
  }

  void StreamRef(llvm::raw_ostream& o, const void* v) {
    // TODO: Print the object members.
    o <<"&" << v << "\n";
  }
  
  void StreamObj(llvm::raw_ostream& o, const void* v) {
    o << "@" << v << "\n";
  }

  void StreamValue(llvm::raw_ostream& o, const void* const p, clang::QualType Ty) {
    if (Ty->isCharType())
      StreamChar(o, *(char*)p);
    else if (Ty->isIntegerType())
      StreamInt(o, *(int*)p);
    else if (Ty->isReferenceType())
      StreamRef(o, p);
    else if (Ty->isPointerType()) {
      clang::QualType PointeeTy = Ty->getPointeeType();
      if (PointeeTy->isCharType())
        StreamCharPtr(o, (const char*)p);
    }
    else
      StreamObj(o, p);
  }
  
  void printValueDefault(llvm::raw_ostream& o, const void* const p,
                         const ValuePrinterInfo& PVI) {
    clang::Expr* E = PVI.m_Expr;
    o << "(";
    o << PVI.m_TypeName;
    if (E->isRValue()) // show the user that the var cannot be changed
      o << " const";
    o << ") ";
    StreamValue(o, p, E->getType());
    
  }
} // end namespace cling
