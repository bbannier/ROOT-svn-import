//------------------------------------------------------------------------------
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Vassil Vassilev <vasil.georgiev.vasilev@cern.ch>
//------------------------------------------------------------------------------

#include "DeclExtractor.h"

#include "clang/AST/DeclGroup.h"
#include "clang/AST/Decl.h"
#include "clang/Sema/Scope.h"
#include "clang/Sema/Sema.h"

using namespace clang;
namespace cling {


  DeclExtractor::DeclExtractor() {

  }
  DeclExtractor::~DeclExtractor() {

  }

  void DeclExtractor::Initialize(clang::ASTContext& Ctx) {
    m_Context = &Ctx;
  }

  void DeclExtractor::InitializeSema(clang::Sema& S) {
    m_Sema = &S;
  }

  void DeclExtractor::HandleTopLevelDecl(clang::DeclGroupRef DGR) {
    for (DeclGroupRef::iterator I = DGR.begin(), E = DGR.end(); I != E; ++I)
      ExtractDecl(*I);
  }

  void DeclExtractor::ForgetSema() {
    m_Sema = 0;
  }

  void DeclExtractor::ExtractDecl(clang::Decl* D) {
    FunctionDecl* TopLevelFD = dyn_cast<FunctionDecl>(D);
    llvm::SmallVector<Decl*, 4> TouchedDecls;
    
    if (TopLevelFD) {
      CompoundStmt* CS = dyn_cast<CompoundStmt>(TopLevelFD->getBody());
      assert(CS && "Function body not a CompoundStmt?");
      DeclContext* DC = TopLevelFD->getDeclContext();
      Scope* S = m_Sema->getScopeForContext(DC);
      CompoundStmt::body_iterator I;
      llvm::SmallVector<Stmt*, 4> Stmts;
      for (I = CS->body_begin(); I != CS->body_end(); ++I) {
        DeclStmt* DS = dyn_cast<DeclStmt>(*I);
        if (!DS) {
          Stmts.push_back(*I);
          continue;
        }

        DC->removeDecl(TopLevelFD);
        S->RemoveDecl(TopLevelFD);
        
        for (DeclStmt::decl_iterator J = DS->decl_begin();
             J != DS->decl_end(); ++J) {
          Decl* D = dyn_cast<Decl>(*J);
          if (D) {
            D->setDeclContext(DC);
            D->setLexicalDeclContext(DC); //FIXME: Watch out
          }
          
          if (VarDecl* VD = dyn_cast<VarDecl>(D)) {
            VD->setStorageClass(SC_None);
            VD->setStorageClassAsWritten(SC_None);
            // reset the linkage to External
            VD->ClearLinkageCache();
            DC->addDecl(VD);
          }

          TouchedDecls.push_back(D);
        }

        DC->addDecl(TopLevelFD);
        S->AddDecl(TopLevelFD);
      }
      // Remove the empty wrappers, i.e those which contain only decls
      if (Stmts.size()) {
        CS->setStmts(*m_Context, Stmts.data(), Stmts.size());
        //TouchedDecls.push_back(TopLevelFD);
      }
      else
        // tell exec engine not to run the function
        TopLevelFD = 0;
    }

    // TODO: Should have better way of doing that
    for (unsigned i = 0; i < TouchedDecls.size(); ++i) {
      m_Sema->Consumer.HandleTopLevelDecl(DeclGroupRef(TouchedDecls[i]));
    }

  }


} // namespace cling
