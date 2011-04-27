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

  /// \brief Provides last chance of recovery for clang semantic analysis.
  /// When the compiler doesn't find the symbol in its symbol table it asks
  /// its ExternalSemaSource to look for the symbol.
  ///
  /// In contrast to the compiler point of view, where these symbols must be 
  /// errors, the interpreter's point of view these symbols are to be 
  /// evaluated at runtime. For that reason the interpreter marks all unknown
  /// by the compiler symbols to be with delayed lookup (evaluation).
  /// One have to be carefull in the cases, in which the compiler expects that
  /// the lookup will fail!
  class DynamicIDHandler : public clang::ExternalSemaSource {
  public:
    DynamicIDHandler(clang::Sema* Sema);
    ~DynamicIDHandler();
    
    /// \brief Provides last resort lookup for failed unqualified lookups
    ///
    /// If there is failed looku, tell sema to create an artificial declaration
    /// which is of dependent type. So the lookup result is marked as dependent
    /// and the diagnostics are suppressed. After that is's an interpreter's 
    /// responsibility to fix all these fake declarations and lookups. 
    /// It is done by the DynamicExprTransformer
    ///
    /// @param[out] R The recovered symbol
    /// @param[in] S The scope in which the lookup failed
    virtual bool LookupUnqualified(clang::LookupResult &R, clang::Scope *S);

  private:
    clang::Sema* m_Sema;
    clang::ASTContext& m_Context;

    /// \brief Checks whether the failed lookup is not expected from the
    /// compiler to fail.
    ///
    /// @param[out] R The symbol to be checked.
    /// @param[in] S The scope, where the lookup failed.
    bool IsDynamicLookup (clang::LookupResult& R, clang::Scope* S);

  };
} // end namespace cling

namespace cling {
  typedef llvm::SmallVector<clang::Stmt*, 2> ASTNodes;
  // The DynamicExprTransformer needs to have information about the nodes
  // it visits in order to escape properly the unknown symbols. Walking up 
  // it needs to know not only the node, which is being returned from the 
  // visited subnode, but it needs information from its subnode if the 
  // subnode can handle the dependent symbol itself or it wants delegate it
  // to its parent.
  // Ideally when given subnode has enough information to handle the unknown
  // symbol it should do it instead of delegating to the parent. This limits
  // the size of the expressions/statements being escaped.
  class ASTNodeInfo {
  private:
    ASTNodes Nodes;
    bool forReplacement;
  public:
    ASTNodeInfo() : forReplacement(0){}
    ASTNodeInfo(clang::Stmt *S, bool needed) : forReplacement(needed) {
      Nodes.push_back(S);
    }

    bool isForReplacement() { return forReplacement; }
    void setForReplacement(bool val = true) { forReplacement = val; }
    bool hasSingleNode() { return Nodes.size() == 1; }
    clang::Stmt* getAsSingleNode() {
      assert(hasSingleNode() && "There is more than one node!");
      return Nodes[0];
    }
    ASTNodes getNodes() { return Nodes; }
    void addNode(clang::Stmt *Node) { Nodes.push_back(Node); }
    template <typename T> T* getAs() {
      return dyn_cast<T>(getAsSingleNode());
    }
    template <typename T> T* castTo() {
      T* Result = dyn_cast<T>(getAsSingleNode());
      assert(Result && "Cannot cast to type!");
      return Result;
    }
  };

} //end namespace cling

namespace cling {
  class Interpreter;
  class DynamicIDHandler;
  class ASTNodeInfo;
  
  typedef llvm::DenseMap<clang::Stmt*, clang::Stmt*> MapTy;
  
  // Ideally the visitor should traverse the dependent nodes, which actially are 
  // the language extensions. For example File::Open("MyFile"); h->Draw() is not valid C++ call
  // if h->Draw() is defined in MyFile. In that case we need to skip Sema diagnostics, so the 
  // h->Draw() is marked as dependent node. That requires the DynamicExprTransformer to find all
  // dependent nodes and escape them to the interpreter, using pre-defined Eval function.
  class DynamicExprTransformer : public clang::DeclVisitor<DynamicExprTransformer>,
                                 public clang::StmtVisitor<DynamicExprTransformer, ASTNodeInfo> {
    
  private:

    /// \brief Stores the declaration of the EvaluateProxyT function
    clang::FunctionDecl* m_EvalDecl;

    /// \brief Sema's external source, which provides last resort lookup
    llvm::OwningPtr<DynamicIDHandler> m_DynIDHandler;

    /// \brief Keeps track of the replacements being made. If an AST node is
    /// changed with another it should be added to the map (newNode->oldNode)
    MapTy m_SubstSymbolMap;

    /// \brief The unknown symbol surrounding environment, which has to be 
    /// included when replacing a node.
    ///
    /// For example:
    /// @code
    /// int a = 5;
    /// const char* b = dep->Symbol(a);
    /// @endcode
    /// This information is kept using the syntax: "dep->Symbol(*(int*)@)",
    /// where @ denotes that the runtime address the variable "a" is needed.
    std::string m_EvalExpressionBuf;

    /// \brief Stores the addresses of the variables that m_EvalExpressionBuf
    /// describes.
    llvm::SmallVector<clang::DeclRefExpr*, 64> m_Environment;

    /// \brief Stores the actual declaration context, in which declarations are
    /// being visited.
    clang::DeclContext* m_CurDeclContext; // We need it for Evaluate()

    /// \brief Stores pointer to cling, mainly used for declaration lookup
    Interpreter* m_Interpreter;

    /// \brief Sema, which is in the core of all the transformations
    clang::Sema* m_Sema;

    /// \brief The ASTContext
    clang::ASTContext& m_Context;
    
  public:
    
    typedef clang::DeclVisitor<DynamicExprTransformer> BaseDeclVisitor;
    typedef clang::StmtVisitor<DynamicExprTransformer, ASTNodeInfo> BaseStmtVisitor;
    
    using BaseStmtVisitor::Visit;
    
    //Constructors
    DynamicExprTransformer(Interpreter* interp, clang::Sema* Sema);
    
    // Destructors
    ~DynamicExprTransformer() { }
    
    void Initialize();
    MapTy &getSubstSymbolMap() { return m_SubstSymbolMap; }
    
    // DeclVisitor      
    void Visit(clang::Decl *D);
    void VisitFunctionDecl(clang::FunctionDecl *D);
    void VisitDecl(clang::Decl *D);
    void VisitDeclContext(clang::DeclContext *DC);
    
    // StmtVisitor
    ASTNodeInfo VisitStmt(clang::Stmt *Node);
    ASTNodeInfo VisitCompoundStmt(clang::CompoundStmt *Node);
    /// \brief Transforms a declaration with initializer of dependent type.
    /// If an object on the free store is being initialized we use the 
    /// EvaluateProxyT
    /// If an object on the stack is being initialized it is transformed into
    /// reference and an object on the free store is created in order to 
    /// avoid the copy constructors, which might be missing or private
    ///
    /// For example:
    /// @code
    /// int i = 5;
    /// MyClass my(dep->Symbol(i))
    /// @endcode
    /// where dep->Symbol() is of artificially dependent type it is being 
    /// transformed into:
    /// @code
    /// cling::runtime::internal::LifetimeHandler 
    /// __unique("dep->Sybmol(*(int*)@)",(void*[]){&i}, DC, "MyClass");
    /// MyClass &my(*(MyClass*)__unique.getMemory());
    /// @endcode
    ///
    /// Note: here our main priority is to preserve equivalent behavior. We have
    /// to clean the heap memory afterwords.
    ///
    ASTNodeInfo VisitDeclStmt(clang::DeclStmt* Node);
    ASTNodeInfo VisitExpr(clang::Expr* Node);
    ASTNodeInfo VisitBinaryOperator(clang::BinaryOperator* Node);
    ASTNodeInfo VisitCallExpr(clang::CallExpr* E);
    ASTNodeInfo VisitDeclRefExpr(clang::DeclRefExpr* DRE);
    ASTNodeInfo VisitDependentScopeDeclRefExpr(clang::DependentScopeDeclRefExpr* Node);
    void AttachDynIDHandler();
    void DetachDynIDHandler();
    
    // EvalBuilder
  protected:
    clang::FunctionDecl *getEvalDecl() { 
      assert(m_EvalDecl && "EvaluateProxyT not found!");
      return m_EvalDecl; 
    }
    clang::Expr* SubstituteUnknownSymbol(const clang::QualType InstTy, clang::Expr* SubTree);
    clang::CallExpr* BuildEvalCallExpr(clang::QualType type, clang::Expr* SubTree, clang::ASTOwningVector<clang::Expr*>& CallArgs);
    void BuildEvalEnvironment(clang::Expr* SubTree);
    void BuildEvalArgs(clang::ASTOwningVector<clang::Expr*>& Result);
    clang::Expr* BuildEvalArg0();
    clang::Expr* BuildEvalArg1();
    clang::Expr* BuildEvalArg2();

    clang::Expr* ConstructllvmStringRefExpr(const char* Str);

    // Helper
    bool IsArtificiallyDependent(clang::Expr *Node);
    bool ShouldVisit(clang::Decl *D);
    bool GetChildren(ASTNodes& Children, clang::Stmt *Node);
  };
} // end namespace cling
#endif // CLING_DYNAMIC_LOOKUP_H
