//------------------------------------------------------------------------------
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Axel Naumann <axel@cern.ch>
//------------------------------------------------------------------------------

#ifndef CLING_CHAINED_AST_CONSUMER_H
#define CLING_CHAINED_AST_CONSUMER_H

#include "llvm/ADT/SmallVector.h"
#include "clang/Sema/SemaConsumer.h"
#include "clang/AST/DeclGroup.h"

namespace clang {
   class ASTContext;
}

namespace cling {
  class ChainedASTConsumer: public clang::SemaConsumer {
  public:
    // Copy of IncrementalParser::EConsumerIndex!
    enum EConsumerIndex {
      kCodeGenerator,
      kDeclExtractor,
      kASTDumper,
      kPCHGenerator,
      kFunctionBodyConsumer,
      kNConsumers
    };
    ChainedASTConsumer(): Consumers() {}
    virtual ~ChainedASTConsumer();
    
#define CAC_DECL(WHAT, ARGS, PARAM) \
    void WHAT ARGS { for (int i = 0; i < kNConsumers; ++i) \
      if (Consumers[i]) Consumers[i]->WHAT PARAM; }
    CAC_DECL(Initialize,(clang::ASTContext &Context),(Context));
    CAC_DECL(HandleTopLevelDecl,(clang::DeclGroupRef D),(D));
    CAC_DECL(HandleInterestingDecl,(clang::DeclGroupRef D),(D));
    CAC_DECL(HandleTagDeclDefinition,(clang::TagDecl *D),(D));
    CAC_DECL(CompleteTentativeDefinition,(clang::VarDecl *D),(D));
    CAC_DECL(HandleVTable,(clang::CXXRecordDecl *RD, bool DefinitionRequired), \
             (RD, DefinitionRequired));
    CAC_DECL(PrintStats,(),());
#undef CAC_DECL

    void HandleTranslationUnit(clang::ASTContext &Ctx) {
      for (int i = 0; i < kNConsumers; ++i)
        if (i != kPCHGenerator && Consumers[i]) Consumers[i]->HandleTranslationUnit(Ctx);
    }

#define CSC_DECL(WHAT, ARGS, PARAM)                                     \
    void WHAT ARGS {                                                    \
      for (int i = 0; i < kNConsumers; ++i) {                           \
        if (Consumers[i]) {                                             \
          clang::SemaConsumer* SC =dyn_cast<clang::SemaConsumer>(Consumers[i]);\
          if (SC) SC->WHAT PARAM;                                       \
      } } }
    CSC_DECL(InitializeSema,(clang::Sema &S),(S));
    CSC_DECL(ForgetSema,(),());
#undef CSC_DECL

    clang::ASTMutationListener *GetASTMutationListener() {
      for (int i = 0; i < kNConsumers; ++i) {
        if (Consumers[i] && Consumers[i]->GetASTMutationListener()) {
          return Consumers[i]->GetASTMutationListener();
        }
      }
      return 0;
    }
    clang::ASTDeserializationListener *GetASTDeserializationListener() {
      for (int i = 0; i < kNConsumers; ++i) {
        if (Consumers[i] && Consumers[i]->GetASTDeserializationListener()) {
          return Consumers[i]->GetASTDeserializationListener();
        }
      }
      return 0;
    }

    void add(EConsumerIndex I, clang::ASTConsumer* C) {
      assert(!Consumers[I] && "Consumer already registered at this index!");
      Consumers[I] = C;
    }

    clang::ASTConsumer* Consumers[kNConsumers];
  };
} // namespace cling

#endif
