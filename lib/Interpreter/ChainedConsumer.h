//------------------------------------------------------------------------------
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Axel Naumann <axel@cern.ch>
//------------------------------------------------------------------------------

#ifndef CLING_CHAINED_CONSUMER_H
#define CLING_CHAINED_CONSUMER_H

#include "llvm/ADT/OwningPtr.h"
#include "clang/Sema/SemaConsumer.h"

#include <bitset>

namespace clang {
  class ASTContext;
}

namespace cling {

  class ChainedMutationListener;
  class ChainedDeserializationListener;

  class ChainedConsumer: public clang::SemaConsumer {

  public:
    enum EConsumerIndex {
      kDeclExtractor,
      kValuePrinterSynthesizer,
      kASTDumper,
      kPCHGenerator,
      kCodeGenerator,
      kConsumersCount
    };
    ChainedConsumer();
    virtual ~ChainedConsumer();

    // ASTConsumer
    virtual void Initialize(clang::ASTContext& Context);
    virtual void HandleTopLevelDecl(clang::DeclGroupRef D);
    virtual void HandleInterestingDecl(clang::DeclGroupRef D);
    virtual void HandleTranslationUnit(clang::ASTContext& Ctx);
    virtual void HandleTagDeclDefinition(clang::TagDecl* D);
    virtual void CompleteTentativeDefinition(clang::VarDecl* D);
    virtual void HandleVTable(clang::CXXRecordDecl* RD, bool DefinitionRequired);
    virtual clang::ASTMutationListener* GetASTMutationListener();
    virtual clang::ASTDeserializationListener* GetASTDeserializationListener();
    virtual void PrintStats();
    
    // SemaConsumer
    virtual void InitializeSema(clang::Sema& S);
    virtual void ForgetSema();

    // Transaction Support
    void StartTransaction() { m_InTransaction = true; }
    void FinishTransaction();
    bool IsInTransaction() { return m_InTransaction; }
    
    void Add(EConsumerIndex I, clang::ASTConsumer* C);
    clang::ASTConsumer** getConsumers() { 
      return Consumers; 
    }

    bool Exists(EConsumerIndex I) {
      return Consumers[I] != 0;
    }

    clang::ASTConsumer* getConsumer(EConsumerIndex I) {
      return Consumers[I];
    }

    bool EnableConsumer(EConsumerIndex I) {
      assert(Exists(I) && "Cannot disable. Consumer not set!");
      bool PrevousState = Enabled[I];
      Enabled.set(I);
      return PrevousState;
    }

    bool DisableConsumer(EConsumerIndex I) {
      assert(Exists(I) && "Cannot disable. Consumer not set!");
      assert(I != kCodeGenerator && "You shouldn't disable codegen!");
      bool PrevousState = Enabled[I];
      Enabled.reset(I);
      return PrevousState;
    }

    void RestorePreviousState(EConsumerIndex I, bool Previous) {
      assert(Exists(I) && "Cannot disable. Consumer not set!");
      Enabled.set(I, Previous);
    }

    bool IsConsumerEnabled(EConsumerIndex I) {
      return Enabled[I];
    }

  private:
    clang::ASTConsumer* Consumers[kConsumersCount]; // owns them
    std::bitset<kConsumersCount> Enabled;
    llvm::OwningPtr<ChainedMutationListener> MutationListener;
    llvm::OwningPtr<ChainedDeserializationListener> DeserializationListener;
    bool m_InTransaction;
    clang::ASTContext* m_Context;
  };
} // namespace cling

#endif
