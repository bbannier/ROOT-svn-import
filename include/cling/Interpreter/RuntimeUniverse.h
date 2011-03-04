//------------------------------------------------------------------------------
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Vassil Vassilev <vasil.georgiev.vasilev@cern.ch>
//------------------------------------------------------------------------------
#ifndef __cling__
#error "This file must not be included by compiled programs."
#endif
#ifdef CLING_RUNTIME_UNIVERSE_H
#error "CLING_RUNTIME_UNIVERSE_H Must only include once."
#endif

#define CLING_RUNTIME_UNIVERSE_H

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS // needed by System/DataTypes.h
#endif

#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS // needed by System/DataTypes.h
#endif

#include <stdio.h>
#include "cling/Interpreter/Interpreter.h"
#include "cling/Interpreter/ValuePrinter.h"

namespace cling {
  namespace runtime {
    // Stores the declarations of variables, which are going to be available
    // only during runtime
    Interpreter* gCling = 0;
    namespace internal {
      // Annotates that these variables shouldn't be used unless you know
      // what you're doing
      
      // Uncomment next line when figure out how to remove explicit template
      // specialization
      //template<typename T> class TypeExtractionHelper{};
      
      template<typename T>
      T EvaluateProxyT(const char* expr, void* varaddr[], clang::DeclContext* DC ) {
        llvm::GenericValue result(gCling->EvaluateWithContext(expr, varaddr, DC));
        //FIXME: we should return T calculated from result
        //printf("%s", expr);
        return T();
      }
    }
  } // end namespace runtime
} // end namespace cling

using namespace cling::runtime;

