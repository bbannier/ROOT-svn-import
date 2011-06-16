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

      DC->removeDecl(TopLevelFD);
      S->RemoveDecl(TopLevelFD);

      for (I = CS->body_begin(); I != CS->body_end(); ++I) {
        DeclStmt* DS = dyn_cast<DeclStmt>(*I);
        if (!DS) {
          Stmts.push_back(*I);
          continue;
        }
        
        for (DeclStmt::decl_iterator J = DS->decl_begin();
             J != DS->decl_end(); ++J) {
          NamedDecl* ND = dyn_cast<NamedDecl>(*J);
          if (ND) {
            DeclContext* OldDC = ND->getDeclContext();
            Scope* OldS = m_Sema->getScopeForContext(OldDC);
            OldDC->removeDecl(ND);
            if (OldS)
              OldS->RemoveDecl(ND);
            
            ND->setDeclContext(DC);
            ND->setLexicalDeclContext(DC); //FIXME: Watch out
          }
          
          if (VarDecl* VD = dyn_cast<VarDecl>(ND)) {
            VD->setStorageClass(SC_None);
            VD->setStorageClassAsWritten(SC_None);

            // if we want to print the result of the initializer of int i = 5
            if (VD->hasInit()) {
              Expr* DRE = m_Sema->BuildDeclRefExpr(VD, 
                                                   VD->getType(),
                                                   VK_LValue,
                                                   SourceLocation()
                                                   ).take();
              Stmts.push_back(DRE);
            }
          }

          assert(ND && "NamedDecl expected!");

          // force recalc of the linkage (to external)
          ND->ClearLinkageCache();

          TouchedDecls.push_back(ND);
        }

      }
      // Insert the extracted declarations before the wrapper
      for (unsigned i = 0; i < TouchedDecls.size(); ++i) {
        DC->addDecl(TouchedDecls[i]);
        S->AddDecl(TouchedDecls[i]);
        m_Sema->Consumer.HandleTopLevelDecl(DeclGroupRef(TouchedDecls[i]));
      }

      // Add the wrapper even though it is empty. The ValuePrinterSynthesizer
      // take care of it
      CS->setStmts(*m_Context, Stmts.data(), Stmts.size());
      DC->addDecl(TopLevelFD);
      S->AddDecl(TopLevelFD);
    }
  }


} // namespace cling
