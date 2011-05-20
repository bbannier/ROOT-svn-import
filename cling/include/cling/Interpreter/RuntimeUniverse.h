//------------------------------------------------------------------------------
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Vassil Vassilev <vasil.georgiev.vasilev@cern.ch>
//------------------------------------------------------------------------------
#ifndef __CLING__
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

namespace cling {

  class Interpreter;

  /// \brief Used to stores the declarations, which are going to be 
  /// available only at runtime. These are cling runtime builtins
  namespace runtime {

    /// \brief The interpreter provides itself as a builtin, i.e. it
    /// interprets itself. This is particularly important for implementing
    /// the dynamic scopes and the runtime bindings
    Interpreter* gCling = 0;

  } // end namespace runtime
} // end namespace cling

using namespace cling::runtime;
