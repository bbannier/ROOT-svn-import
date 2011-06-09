//------------------------------------------------------------------------------
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Vassil Vassilev <vasil.georgiev.vasilev@cern.ch>
//------------------------------------------------------------------------------

#include "ChainedASTConsumer.h"

#include "clang/AST/ASTMutationListener.h"
#include "clang/AST/DeclGroup.h"
#include "clang/Serialization/ASTDeserializationListener.h"

using namespace clang;

namespace cling {

  class ChainedASTDeserializationListener: public ASTDeserializationListener {
  public:
    // Does NOT take ownership of the elements in L.
    ChainedASTDeserializationListener(const std::vector<ASTDeserializationListener*>& L, const std::bitset<ChainedASTConsumer::kConsumersCount>& E);
    virtual ~ChainedASTDeserializationListener(){}

    virtual void ReaderInitialized(ASTReader* Reader);
    virtual void IdentifierRead(serialization::IdentID ID,
                                IdentifierInfo* II);
    virtual void TypeRead(serialization::TypeIdx Idx, QualType T);
    virtual void DeclRead(serialization::DeclID ID, const Decl* D);
    virtual void SelectorRead(serialization::SelectorID iD, Selector Sel);
    virtual void MacroDefinitionRead(serialization::MacroID, 
                                     MacroDefinition* MD);
  private:
    std::vector<ASTDeserializationListener*> Listeners;
    const std::bitset<ChainedASTConsumer::kConsumersCount> Enabled;
  };
  ChainedASTDeserializationListener::ChainedASTDeserializationListener(const std::vector<ASTDeserializationListener*>& L, const std::bitset<ChainedASTConsumer::kConsumersCount>& E)
    : Listeners(L), Enabled(E) {
  }
  
  void ChainedASTDeserializationListener::ReaderInitialized(ASTReader* Reader) {
    for (size_t i = 0, e = Listeners.size(); i != e; ++i)
      Listeners[i]->ReaderInitialized(Reader);
  }
  
  void ChainedASTDeserializationListener::IdentifierRead(serialization::IdentID ID,
                                                         IdentifierInfo* II) {
    for (size_t i = 0, e = Listeners.size(); i != e; ++i)
      if (Enabled[i])
        Listeners[i]->IdentifierRead(ID, II);
  }
  
  void ChainedASTDeserializationListener::TypeRead(serialization::TypeIdx Idx,
                                                   QualType T) {
    for (size_t i = 0, e = Listeners.size(); i != e; ++i)
      if (Enabled[i])
        Listeners[i]->TypeRead(Idx, T);
  }
  
  void ChainedASTDeserializationListener::DeclRead(serialization::DeclID ID,
                                                   const Decl* D) {
    for (size_t i = 0, e = Listeners.size(); i != e; ++i)
      if (Enabled[i])
        Listeners[i]->DeclRead(ID, D);
  }
  
  void ChainedASTDeserializationListener::SelectorRead(serialization::SelectorID ID,
                                                       Selector Sel) {
    for (size_t i = 0, e = Listeners.size(); i != e; ++i)
      if (Enabled[i])
        Listeners[i]->SelectorRead(ID, Sel);
  }
  
  void ChainedASTDeserializationListener::MacroDefinitionRead(serialization::MacroID ID,
                                                              MacroDefinition* MD) {
    for (size_t i = 0, e = Listeners.size(); i != e; ++i)
      if (Enabled[i])
        Listeners[i]->MacroDefinitionRead(ID, MD);
  }
  
  // This ASTMutationListener forwards its notifications to a set of
  // child listeners.
  class ChainedASTMutationListener : public ASTMutationListener {
  public:
   // Does NOT take ownership of the elements in L.
    ChainedASTMutationListener(const std::vector<ASTMutationListener*>& L,
                               const std::bitset<ChainedASTConsumer::kConsumersCount>& E);
    virtual ~ChainedASTMutationListener(){};

    virtual void CompletedTagDefinition(const TagDecl* D);
    virtual void AddedVisibleDecl(const DeclContext* DC, const Decl* D);
    virtual void AddedCXXImplicitMember(const CXXRecordDecl* RD, const Decl* D);
    virtual void AddedCXXTemplateSpecialization(const ClassTemplateDecl* TD,
                                                const ClassTemplateSpecializationDecl* D);
    virtual void AddedCXXTemplateSpecialization(const FunctionTemplateDecl* TD,
                                                const FunctionDecl* D);
    virtual void CompletedImplicitDefinition(const FunctionDecl* D);
    virtual void StaticDataMemberInstantiated(const VarDecl* D);
  private:
    std::vector<ASTMutationListener*> Listeners;
    const std::bitset<ChainedASTConsumer::kConsumersCount> Enabled;
  };
  
  ChainedASTMutationListener::ChainedASTMutationListener(const std::vector<ASTMutationListener*>& L, const std::bitset<ChainedASTConsumer::kConsumersCount>& E)
    : Listeners(L), Enabled(E) {
  }
  
  void ChainedASTMutationListener::CompletedTagDefinition(const TagDecl* D) {
    for (size_t i = 0, e = Listeners.size(); i != e; ++i)
      if (Enabled[i])
        Listeners[i]->CompletedTagDefinition(D);
  }
  
  void ChainedASTMutationListener::AddedVisibleDecl(const DeclContext* DC,
                                                    const Decl* D) {
    for (size_t i = 0, e = Listeners.size(); i != e; ++i)
      if (Enabled[i])
        Listeners[i]->AddedVisibleDecl(DC, D);
  }
  
  void ChainedASTMutationListener::AddedCXXImplicitMember(const CXXRecordDecl* RD, 
                                                          const Decl* D) {
    for (size_t i = 0, e = Listeners.size(); i != e; ++i)
      if (Enabled[i])
        Listeners[i]->AddedCXXImplicitMember(RD, D);
  }
  void ChainedASTMutationListener::AddedCXXTemplateSpecialization(const ClassTemplateDecl* TD,
                                                                  const ClassTemplateSpecializationDecl* D) {
    for (size_t i = 0, e = Listeners.size(); i != e; ++i)
      if (Enabled[i])
        Listeners[i]->AddedCXXTemplateSpecialization(TD, D);
  }
  void ChainedASTMutationListener::AddedCXXTemplateSpecialization(const FunctionTemplateDecl* TD,
                                                                  const FunctionDecl* D) {
    for (size_t i = 0, e = Listeners.size(); i != e; ++i)
      if (Enabled[i])
        Listeners[i]->AddedCXXTemplateSpecialization(TD, D);
  }
  void ChainedASTMutationListener::CompletedImplicitDefinition(const FunctionDecl* D) {
    for (size_t i = 0, e = Listeners.size(); i != e; ++i)
      if (Enabled[i])
        Listeners[i]->CompletedImplicitDefinition(D);
  }
  void ChainedASTMutationListener::StaticDataMemberInstantiated(const VarDecl* D) {
    for (size_t i = 0, e = Listeners.size(); i != e; ++i)
      if (Enabled[i])
        Listeners[i]->StaticDataMemberInstantiated(D);
  }

  ChainedASTConsumer::ChainedASTConsumer()
   :  MutationListener(0), DeserializationListener(0) { 
    for (size_t i = 0; i < kConsumersCount; ++i) {
      Consumers[i] = 0;
      Enabled[i] = false;
    }
  }

  ChainedASTConsumer::~ChainedASTConsumer() {
    for (size_t i = 0; i < kConsumersCount; ++i)
      if (Exists((EConsumerIndex)i))
        delete Consumers[i];
  }
  
  void ChainedASTConsumer::Initialize(ASTContext& Context) {
    for (size_t i = 0; i < kConsumersCount; ++i)
      if (Exists((EConsumerIndex)i))
        Consumers[i]->Initialize(Context);
  }
  
  void ChainedASTConsumer::HandleTopLevelDecl(DeclGroupRef D) {
    for (size_t i = 0; i < kConsumersCount; ++i)
      if (Enabled[i])
        Consumers[i]->HandleTopLevelDecl(D);
  }
  
  void ChainedASTConsumer::HandleInterestingDecl(DeclGroupRef D) {
    for (size_t i = 0; i < kConsumersCount; ++i)
      if (Enabled[i])
        Consumers[i]->HandleInterestingDecl(D);
  }
  
  void ChainedASTConsumer::HandleTranslationUnit(ASTContext& Ctx) {
    for (size_t i = 0; i < kConsumersCount; ++i)
      if (Enabled[i] && i != kPCHGenerator)
        Consumers[i]->HandleTranslationUnit(Ctx);
  }
  
  void ChainedASTConsumer::HandleTagDeclDefinition(TagDecl* D) {
    for (size_t i = 0; i < kConsumersCount; ++i)
      if (Enabled[i])
        Consumers[i]->HandleTagDeclDefinition(D);
  }
  
  void ChainedASTConsumer::CompleteTentativeDefinition(VarDecl* D) {
    for (size_t i = 0; i < kConsumersCount; ++i)
      if (Enabled[i])
        Consumers[i]->CompleteTentativeDefinition(D);
  }
  
  void ChainedASTConsumer::HandleVTable(CXXRecordDecl* RD, bool DefinitionRequired) {
    for (size_t i = 0; i < kConsumersCount; ++i)
      if (Enabled[i])
        Consumers[i]->HandleVTable(RD, DefinitionRequired);
  }
  
  ASTMutationListener* ChainedASTConsumer::GetASTMutationListener() {
    return MutationListener.get();
  }
  
  ASTDeserializationListener* ChainedASTConsumer::GetASTDeserializationListener() {
    return DeserializationListener.get();
  }
  
  void ChainedASTConsumer::PrintStats() {
    for (size_t i = 0; i < kConsumersCount; ++i)
      if (Exists((EConsumerIndex)i))
        Consumers[i]->PrintStats();
  }
  
  void ChainedASTConsumer::InitializeSema(Sema& S) {
    for (size_t i = 0; i < kConsumersCount; ++i)
      if (Exists((EConsumerIndex)i))
        if (SemaConsumer* SC = dyn_cast<SemaConsumer>(Consumers[i]))
          SC->InitializeSema(S);
  }
  
  void ChainedASTConsumer::ForgetSema() {
    for (size_t i = 0; i < kConsumersCount; ++i)
      if (SemaConsumer* SC = dyn_cast<SemaConsumer>(Consumers[i]))
        if (Exists((EConsumerIndex)i))
          SC->ForgetSema();
  }
  
  void ChainedASTConsumer::Add(EConsumerIndex I, clang::ASTConsumer* C) {
    assert(!Exists(I) && "Consumer already registered at this index!");
    Consumers[I] = C;
    Enabled[I] = true;
    // Collect the mutation listeners and deserialization listeners of all
    // children, and create a multiplex listener each if so.
    // TODO: Do it in smarter way. Do not recalculate on every addition.
    std::vector<ASTMutationListener*> mutationListeners;
    std::vector<ASTDeserializationListener*> serializationListeners;
    for (size_t i = 0; i < kConsumersCount; ++i) {
      if (Exists((EConsumerIndex)i)) {
        ASTMutationListener* mutationListener =
          Consumers[i]->GetASTMutationListener();
        if (mutationListener)
          mutationListeners.push_back(mutationListener);
        ASTDeserializationListener* serializationListener =
          Consumers[i]->GetASTDeserializationListener();
        if (serializationListener)
          serializationListeners.push_back(serializationListener);
      }
    }
    if (mutationListeners.size()) {
      MutationListener.reset(new ChainedASTMutationListener(mutationListeners, Enabled));
    }
    if (serializationListeners.size()) {
      DeserializationListener.reset(new ChainedASTDeserializationListener(serializationListeners, Enabled));
    }
    
  }
  
} // namespace cling

