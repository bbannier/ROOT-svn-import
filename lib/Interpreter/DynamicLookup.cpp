//------------------------------------------------------------------------------
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id: ASTTransformVisitor.cpp 36608 2010-11-11 18:21:02Z vvassilev $
// author:  Vassil Vassilev <vasil.georgiev.vasilev@cern.ch>
//------------------------------------------------------------------------------

#include "DynamicLookup.h"

#include "clang/Sema/Scope.h"
#include "clang/Sema/Lookup.h"


namespace clang {
  class LookupResult;
  class Scope;
}

using namespace clang;

namespace cling {
  extern "C" int printf(const char* fmt, ...);
  // pin the vtable to this file
  DynamicIDHandler::~DynamicIDHandler(){}

  // If there is failed lookup we tell sema to create artificial declaration
  // which is of dependent type. So the lookup result is marked as dependent
  // and the diagnostics are suppressed. After that is our responsibility to
  // fix all these fake declarations and lookups. It is done by the
  // DynamicExpressionTransformer (aka ASTTransformVisitor)
  bool DynamicIDHandler::LookupUnqualified(LookupResult &R, Scope *S) {
    if (R.getLookupKind() != Sema::LookupOrdinaryName) return false;
    if (R.isForRedeclaration()) return false;
    DeclarationName Name = R.getLookupName();
    IdentifierInfo *II = Name.getAsIdentifierInfo();
    SourceLocation NameLoc = R.getNameLoc();
    FunctionDecl *D = dyn_cast<FunctionDecl>(R.getSema().ImplicitlyDefineFunction(NameLoc, *II, S));
    if (D) { 
      BuiltinType *Ty = new BuiltinType(BuiltinType::Dependent);
      QualType QTy(Ty, 0);            
      D->setType(QTy);
      R.addDecl(D);
      // Mark this declaration for removal
      m_FakeDecls.push_back(D);
      
      // Say that we can handle the situation. Clang should try to recover
      return true;
    }
    // We cannot handle the situation. Give up
    return false;              
  }

  // Removes the implicitly created functions, which help to emulate the dynamic scopes
  void DynamicIDHandler::RemoveFakeDecls() {      
    Scope *S = m_Sema->getScopeForContext(m_Sema->getASTContext().getTranslationUnitDecl());
    for (unsigned int i = 0; i < m_FakeDecls.size(); ++i) {
      printf("\nI am about to remove:\n");
      m_FakeDecls[i]->dump();
      S->RemoveDecl(m_FakeDecls[i]);
    }
  }

} // end namespace cling
