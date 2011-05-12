//------------------------------------------------------------------------------
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Vassil Vassilev <vasil.georgiev.vasilev@cern.ch>
//------------------------------------------------------------------------------

#ifndef CLING_INTERPRETER_CALLBACKS_H
#define CLING_INTERPRETER_CALLBACKS_H

#include "clang/Sema/Lookup.h"

namespace cling {
  /// \brief  This interface provides a way to observe the actions of the 
  /// interpreter as it does its thing.  Clients can define their hooks here to
  /// implement interpreter level tools.
  class InterpreterCallbacks {
  private:
    Interpreter* m_Interpreter;
  public:
    InterpreterCallbacks(Interpreter* interp) : m_Interpreter(interp){}
    virtual ~InterpreterCallbacks() {
      delete m_Interpreter;
      m_Interpreter = 0;
    }

    /// \brief This callback is invoked whenever the interpreter needs to
    /// resolve the type and the adress of an object, which has been marked for
    /// delayed evaluation from the interpreter's dynamic lookup extension
    virtual bool LookupObject(clang::LookupResult& R, clang::Scope* S) {
      // Only for demo resolve all unknown objects to gCling
      clang::NamedDecl* D = m_Interpreter->LookupDecl("cling").LookupDecl("runtime").LookupDecl("gCling");
      assert (D && "gCling not found!");
      R.addDecl(D);
      return true;
    }
  };
} // end namespace cling

#endif // CLING_INTERPRETER_CALLBACKS_H
