//------------------------------------------------------------------------------
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Axel Naumann <axel@cern.ch>
//------------------------------------------------------------------------------

#include "InputValidator.h"

#include "llvm/Support/MemoryBuffer.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Frontend/CompilerInstance.h"

#include <stack>

namespace cling {
  InputValidator::InputValidator(clang::CompilerInstance* CI):
    m_CI(CI), m_PP(0) {
    m_PP = &CI->getPreprocessor();
  }
  
  InputValidator::~InputValidator() {}

  InputValidator::Result
  InputValidator::validate(llvm::StringRef code) {
    m_CI->getDiagnosticClient().BeginSourceFile(m_CI->getLangOpts(), m_PP);
    llvm::MemoryBuffer* MemoryBuffer(llvm::MemoryBuffer::getMemBuffer(code));
    clang::FileID FID = m_CI->getSourceManager()
      .createFileIDForMemBuffer(MemoryBuffer);
    m_PP->EnterSourceFile(FID, 0, clang::SourceLocation());
    
    clang::Token Tok;
    std::stack<int> parenStack;
    do {
      m_PP->Lex(Tok);
      int kind = (int)Tok.getKind();
      if (kind >= (int)clang::tok::l_square
          && kind <= (int)clang::tok::r_brace) {
        kind -= (int)clang::tok::l_square;
        if (kind % 2) {
          // closing the right one?
          if (parenStack.empty()) return kMismatch;
          int prev = parenStack.top();
          if (prev != kind - 1) return kMismatch;
          parenStack.pop();
        } else {
          parenStack.push(kind);
        }
      }
    } while (Tok.isNot(clang::tok::eof));
    
    if (m_CI->getDiagnostics().hasErrorOccurred()) {
      return kParseError;
    }
    
    if (!parenStack.empty()) return kUnbalanced;
    return kValid;
  }
} // end namespace cling
