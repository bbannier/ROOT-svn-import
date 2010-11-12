//------------------------------------------------------------------------------
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id: Interpreter.cpp 36507 2010-11-04 21:42:17Z axel $
// author:  Axel Naumann <axel@cern.ch>
//------------------------------------------------------------------------------

#include "InputValidator.h"

#include "llvm/Support/MemoryBuffer.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Frontend/CompilerInstance.h"

#include <stack>

cling::InputValidator::InputValidator(clang::CompilerInstance* CI):
m_CI(CI), m_PP(0) {
  CI->createPreprocessor();
  m_PP = &CI->getPreprocessor();
  llvm::MemoryBuffer* MemoryBuffer
    = llvm::MemoryBuffer::getMemBuffer("//cling InputSanitizer");
  CI->getSourceManager().createMainFileIDForMemBuffer(MemoryBuffer);
  m_PP->getBuiltinInfo().InitializeBuiltins(m_PP->getIdentifierTable(),
                                            m_PP->getLangOptions().NoBuiltin);
  m_PP->EnterMainSourceFile();  
}

cling::InputValidator::~InputValidator() {
  m_CI->takeLLVMContext(); // Don't take down the context with the CI.
}

cling::InputValidator::Result
cling::InputValidator::validate(llvm::StringRef code) {
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
  
  if (m_CI->getDiagnostics().getNumErrors()) {
    return kParseError;
  }
  
  if (!parenStack.empty()) return kUnbalanced;
  return kValid;
}
