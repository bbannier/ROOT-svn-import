//------------------------------------------------------------------------------
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id: DynamicLookup.cpp 39556 2011-06-04 21:20:50Z vvassilev $
// author:  Vassil Vassilev <vasil.georgiev.vasilev@cern.ch>
//------------------------------------------------------------------------------

#ifndef CLING_VALUE_PRINTER_SYNTHESIZER_H
#define CLING_VALUE_PRINTER_SYNTHESIZER_H

#include "clang/Sema/SemaConsumer.h"

namespace clang {
  class Expr;
}

namespace cling {
  class Interpreter;

  class ValuePrinterSynthesizer : public clang::SemaConsumer {

  private:
    clang::ASTContext* m_Context;
    clang::Sema* m_Sema;
    Interpreter* m_Interpreter;
    bool IsValuePrinterLoaded;

public:
    ValuePrinterSynthesizer(Interpreter* Interp);
    virtual ~ValuePrinterSynthesizer();
    void Initialize(clang::ASTContext& Ctx);
    void InitializeSema(clang::Sema& S);
    void HandleTopLevelDecl(clang::DeclGroupRef DGR);
    void ForgetSema();

  private:
    clang::Expr* SynthesizeVP(clang::Expr* E);
    void LoadValuePrinter();
  };

} // namespace cling

#endif // CLING_DECL_EXTRACTOR_H
