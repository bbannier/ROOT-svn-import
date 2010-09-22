//--------------------------------------------------------------------*- C++ -*-
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Axel Naumann <axel@cern.ch>
//------------------------------------------------------------------------------

#ifndef CLING_INPUTVALIDATOR_H
#define CLING_INPUTVALIDATOR_H

#include "llvm/ADT/StringRef.h"

#include <vector>

namespace clang {
class FunctionDecl;
class Preprocessor;
class Token;
}

namespace cling {

class Interpreter;

// Determine whether prompt input is complete.
// Note: Used by MetaProcessor.

class InputValidator {
public:
   //---------------------------------------------------------------------
   //! Mark the type of statement found in the command line
   //---------------------------------------------------------------------
   enum InputType { Incomplete, TopLevel, Stmt };

   InputValidator(Interpreter* CIB):
      m_Interpreter(CIB) {}

   InputType analyzeInput(llvm::StringRef contextSource,
                          llvm::StringRef line,
                          int& indentLevel,
                          std::vector<clang::FunctionDecl*> *fds);

protected:
   int analyzeTokens(clang::Preprocessor& PP, clang::Token& lastTok,
                     int& indentLevel, bool& tokWasDo);



private:
   Interpreter* m_Interpreter; // Interpreter that holds the AST
};

}

#endif // CLING_INPUTVALIDATOR_H
