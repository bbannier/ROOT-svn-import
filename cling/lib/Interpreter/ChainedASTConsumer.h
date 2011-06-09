//------------------------------------------------------------------------------
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Axel Naumann <axel@cern.ch>
//------------------------------------------------------------------------------

#ifndef CLING_CHAINED_AST_CONSUMER_H
#define CLING_CHAINED_AST_CONSUMER_H

#include "llvm/ADT/OwningPtr.h"
#include "clang/Sema/SemaConsumer.h"

#include <bitset>

namespace clang {
  class ASTContext;
}

namespace cling {

  class ChainedASTMutationListener;
  class ChainedASTDeserializationListener;

  class ChainedASTConsumer: public clang::SemaConsumer {

  public:
    // Copy of IncrementalParser::EConsumerIndex!
    enum EConsumerIndex {
      kCodeGenerator,
      kDeclExtractor,
      kValuePrinterSynthesizer,
      kASTDumper,
      kPCHGenerator,
      kFunctionBodyConsumer,
      kConsumersCount
    };
    ChainedASTConsumer();
    virtual ~ChainedASTConsumer();

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

  private:
    clang::ASTConsumer* Consumers[kConsumersCount]; // owns them
    std::bitset<kConsumersCount> Enabled;
    llvm::OwningPtr<ChainedASTMutationListener> MutationListener;
    llvm::OwningPtr<ChainedASTDeserializationListener> DeserializationListener;
  };
} // namespace cling

#endif
