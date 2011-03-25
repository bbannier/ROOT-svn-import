//------------------------------------------------------------------------------
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Vassil Vassilev <vasil.georgiev.vasilev@cern.ch>
//------------------------------------------------------------------------------

#ifndef CLING_DYNAMIC_LOOKUP_H
#define CLING_DYNAMIC_LOOKUP_H

#include "llvm/ADT/OwningPtr.h"

#include "clang/AST/StmtVisitor.h"
#include "clang/AST/DeclVisitor.h"
#include "clang/Sema/Ownership.h"
#include "clang/Sema/ExternalSemaSource.h"

namespace clang {
  class Sema;
}

namespace cling {
  class DynamicIDHandler : public clang::ExternalSemaSource {
  public:
    DynamicIDHandler(clang::Sema* Sema) : m_Sema(Sema){}
    ~DynamicIDHandler();
    
    // Override this to provide last resort lookup for failed unqualified lookups
    virtual bool LookupUnqualified(clang::LookupResult &R, clang::Scope *S);

    // Check whether the failed lookup is marked as artificially dependent.
    // The difference comes from the DeclContext if the DeclContext is marked
    // as dependent then we don't need to handle the failed lookup
    bool IsDynamicLookup (clang::LookupResult& R, clang::Scope* S);

    // Remove the fake dependent declarations
    void RemoveFakeDecls();
  private:
    llvm::SmallVector<clang::Decl*, 8> m_FakeDecls;
    clang::Sema* m_Sema;
  };
} // end namespace cling

namespace cling {
  // The DynamicExprTransformer needs to have information about the nodes
  // it visits in order to escape properly the unknown symbols. Walking up 
  // it needs to know not only the node, which is being returned from the 
  // visited subnode, but it needs information from its subnode if the 
  // subnode can handle the dependent symbol itself or it wants delegate it
  // to its parent.
  // Ideally when given subnode has enough information to handle the unknown
  // symbol it should do it instead of delegating to the parent. This limits
  // the size of the expressions/statements being escaped.
  class EvalInfo {
  private:      
    //clang::Stmt *m_newStmt; // the new/old node
  public:
    EvalInfo(clang::Stmt *S, bool needed) : IsEvalNeeded(needed) {
      Stmts.push_back(S);
    }

    EvalInfo(llvm::SmallVector<clang::Stmt*, 2> stmts, bool needed) : Stmts(stmts), IsEvalNeeded(needed) { }

    //:m_newStmt(S), IsEvalNeeded(needed) {};
    llvm::SmallVector<clang::Stmt*, 2> Stmts;
    bool IsEvalNeeded; // whether to emit the Eval call or not
    clang::Stmt *Stmt() { return Stmts[0]; }
    bool isMultiStmt() { return Stmts.size() > 1; }
    unsigned StmtCount() { return Stmts.size(); }
    //clang::Stmt *getNewStmt() const { return m_newStmt; }
    //void setNewStmt(clang::Stmt *S) { m_newStmt = S; } 
  };
  
} //end namespace cling

namespace cling {
  class Interpreter;
  class DynamicIDHandler;
  class EvalInfo;
  
  typedef llvm::DenseMap<clang::Stmt*, clang::Stmt*> MapTy;
  
  // Ideally the visitor should traverse the dependent nodes, which actially are 
  // the language extensions. For example File::Open("MyFile"); h->Draw() is not valid C++ call
  // if h->Draw() is defined in MyFile. In that case we need to skip Sema diagnostics, so the 
  // h->Draw() is marked as dependent node. That requires the DynamicExprTransformer to find all
  // dependent nodes and escape them to the interpreter, using pre-defined Eval function.
  class DynamicExprTransformer : public clang::DeclVisitor<DynamicExprTransformer>,
                                 public clang::StmtVisitor<DynamicExprTransformer, EvalInfo> {
    
  private: // members
    clang::FunctionDecl* m_EvalDecl;
    llvm::OwningPtr<DynamicIDHandler> m_DynIDHandler;
    MapTy m_SubstSymbolMap;
    /* 
       Specifies the unknown symbol surrounding
       Example: int a; ...; h->Draw(a); -> Eval(gCling, "*(int*)@", {&a});
       m_EvalExpressionBuf holds the types of the variables.
       m_Environment holds the refs from which runtime addresses are built.
    */
    std::string m_EvalExpressionBuf;
    llvm::SmallVector<clang::DeclRefExpr*, 64> m_Environment;
    clang::DeclContext* m_CurDeclContext; // We need it for Evaluate()
    clang::QualType m_DeclContextType; // Used for building Eval args
    clang::VarDecl* classA;
  public: // members
    clang::Sema* m_Sema;
    
  public: // types
    
    typedef clang::DeclVisitor<DynamicExprTransformer> BaseDeclVisitor;
    typedef clang::StmtVisitor<DynamicExprTransformer, EvalInfo> BaseStmtVisitor;
    
    using BaseStmtVisitor::Visit;
    
  public:
    
    //Constructors
    DynamicExprTransformer();      
    DynamicExprTransformer(clang::Sema* Sema);
    
    // Destructors
    ~DynamicExprTransformer() { }
    
    void Initialize();
    clang::FunctionDecl *getEvalDecl() { return m_EvalDecl; }
    void setEvalDecl(clang::FunctionDecl *FDecl) { if (!m_EvalDecl) m_EvalDecl = FDecl; }
    MapTy &getSubstSymbolMap() { return m_SubstSymbolMap; }
    
    // DeclVisitor      
    void Visit(clang::Decl *D);
    void VisitFunctionDecl(clang::FunctionDecl *D);
    void VisitDecl(clang::Decl *D);
    void VisitDeclContext(clang::DeclContext *DC);
    
    // StmtVisitor
    EvalInfo VisitDeclStmt(clang::DeclStmt *Node);
    EvalInfo VisitStmt(clang::Stmt *Node);
    EvalInfo VisitExpr(clang::Expr *Node);
    EvalInfo VisitCompoundStmt(clang::CompoundStmt *Node);
    EvalInfo VisitCallExpr(clang::CallExpr *E);
    EvalInfo VisitDeclRefExpr(clang::DeclRefExpr *DRE);
    EvalInfo VisitDependentScopeDeclRefExpr(clang::DependentScopeDeclRefExpr *Node);
    
    // EvalBuilder
    clang::Expr *SubstituteUnknownSymbol(const clang::QualType InstTy, clang::Expr *SubTree);
    clang::CallExpr *BuildEvalCallExpr(clang::QualType type, clang::Expr *SubTree, clang::ASTOwningVector<clang::Expr*> &CallArgs);
    void BuildEvalEnvironment(clang::Expr *SubTree);
    void BuildEvalArgs(clang::ASTOwningVector<clang::Expr*> &Result);
    clang::Expr *BuildEvalArg0(clang::ASTContext &C);
    clang::Expr *BuildEvalArg1(clang::ASTContext &C);
    clang::Expr *BuildEvalArg2(clang::ASTContext &C);
    
    // Helper
    bool IsArtificiallyDependent(clang::Expr *Node);
    bool ShouldVisit(clang::Decl *D);
    clang::FunctionDecl* LookupForEvaluateProxyT();
    bool GetChildren(llvm::SmallVector<clang::Stmt*, 32> &Stmts, clang::Stmt *Node);
  };
} // end namespace cling
#endif // CLING_DYNAMIC_LOOKUP_H
