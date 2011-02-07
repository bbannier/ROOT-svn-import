//------------------------------------------------------------------------------
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id: Interpreter.cpp 36507 2010-11-04 21:42:17Z axel $
// author:  Axel Naumann <axel@cern.ch>
//------------------------------------------------------------------------------

#ifndef CLING_CHAINED_AST_CONSUMER_H
#define CLING_CHAINED_AST_CONSUMER_H

#include "llvm/ADT/SmallVector.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/DeclGroup.h"

namespace clang {
   class ASTContext;  
}

namespace cling {
  class ChainedASTConsumer: public clang::ASTConsumer {
  public:
    ChainedASTConsumer() {}
    virtual ~ChainedASTConsumer();
    
#define CAC_DECL(WHAT, ARGS, PARAM) \
void WHAT ARGS { \
for (llvm::SmallVector<clang::ASTConsumer*,2>::iterator i = Consumers.begin(), \
e = Consumers.end(); i != e; ++i) (*i)->WHAT PARAM; \
}
    CAC_DECL(Initialize,(clang::ASTContext &Context),(Context));
    CAC_DECL(HandleTopLevelDecl,(clang::DeclGroupRef D),(D));
    CAC_DECL(HandleInterestingDecl,(clang::DeclGroupRef D),(D));
    CAC_DECL(HandleTranslationUnit,(clang::ASTContext &Ctx),(Ctx));
    CAC_DECL(HandleTagDeclDefinition,(clang::TagDecl *D),(D));
    CAC_DECL(CompleteTentativeDefinition,(clang::VarDecl *D),(D));
    CAC_DECL(HandleVTable,(clang::CXXRecordDecl *RD, bool DefinitionRequired), \
             (RD, DefinitionRequired));
    CAC_DECL(PrintStats,(),());
#undef CAC_DECL

    clang::ASTMutationListener *GetASTMutationListener() {
      return Consumers.empty() ? 0 : Consumers[0]->GetASTMutationListener();
    }
    clang::ASTDeserializationListener *GetASTDeserializationListener() {
      return Consumers.empty() ? 0 : Consumers[0]->GetASTDeserializationListener();
    }

    llvm::SmallVector<clang::ASTConsumer*,2> Consumers;
  };
} // namespace cling

#endif
