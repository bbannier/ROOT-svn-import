//------------------------------------------------------------------------------
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Vassil Vassilev <vasil.georgiev.vasilev@cern.ch>
//------------------------------------------------------------------------------

#ifndef CLING_VALUE_PRINTER_SYNTHESIZER_H
#define CLING_VALUE_PRINTER_SYNTHESIZER_H

#include "clang/Sema/SemaConsumer.h"

namespace clang {
  class Expr;
  class CompoundStmt;
}

namespace cling {
  class Interpreter;

  class ValuePrinterSynthesizer : public clang::SemaConsumer {

  private:
    clang::ASTContext* m_Context;
    clang::Sema* m_Sema;
    Interpreter* m_Interpreter;

public:
    ValuePrinterSynthesizer(Interpreter* Interp);
    virtual ~ValuePrinterSynthesizer();
    void Initialize(clang::ASTContext& Ctx);
    void InitializeSema(clang::Sema& S);
    void HandleTopLevelDecl(clang::DeclGroupRef DGR);
    void ForgetSema();

  private:
    clang::Expr* SynthesizeVP(clang::Expr* E);
    unsigned ClearNullStmts(clang::CompoundStmt* CS);
  };

} // namespace cling

#endif // CLING_DECL_EXTRACTOR_H
