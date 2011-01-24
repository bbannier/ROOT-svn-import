//------------------------------------------------------------------------------
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id: ASTTransformVisitor.h 36608 2010-11-11 18:21:02Z vvassilev $
// author:  Vassil Vassilev <vasil.georgiev.vasilev@cern.ch>
//------------------------------------------------------------------------------

#include "clang/AST/DeclVisitor.h"
#include "clang/AST/StmtVisitor.h"

#include "clang/Sema/Sema.h"
#include "clang/Sema/SemaInternal.h"
#include "clang/Sema/Template.h"

#include "clang/AST/DeclTemplate.h"
#include "clang/AST/Stmt.h"

#include "llvm/Support/MemoryBuffer.h"

#include "EvalInfo.h"

using namespace clang;

namespace cling {

   typedef llvm::DenseMap<Stmt*, Stmt*> MapTy;

   // Ideally the visitor should traverse the dependent nodes, which actially are 
   // the language extensions. For example File::Open("MyFile"); h->Draw() is not valid C++ call
   // if h->Draw() is defined in MyFile. In that case we need to skip Sema diagnostics, so the 
   // h->Draw() is marked as dependent node. That requires the ASTTransformVisitor to find all
   // dependent nodes and escape them to the interpreter, using pre-defined Eval function.
   class ASTTransformVisitor : public DeclVisitor<ASTTransformVisitor>,
                               public StmtVisitor<ASTTransformVisitor, EvalInfo> {
      
   private: // members
      FunctionDecl *EvalDecl;
      MapTy m_SubstSymbolMap;
      /* 
         Specifies the unknown symbol surrounding
         Example: int a; ...; h->Draw(a); -> Eval(gCling, "*(int*)@", {&a});
         m_EvalExpressionBuf holds the types of the variables.
         m_Environment holds the refs from which runtime addresses are built.
      */
      std::string m_EvalExpressionBuf;
      llvm::SmallVector<DeclRefExpr*, 64> m_Environment;

   public: // members
      void *gCling; //Pointer to the Interpreter object
      clang::Sema *SemaPtr;
      Decl *CurrentDecl;
      
   public: // types
      
      typedef DeclVisitor<ASTTransformVisitor> BaseDeclVisitor;
      typedef StmtVisitor<ASTTransformVisitor, EvalInfo> BaseStmtVisitor;

      using BaseStmtVisitor::Visit;

   public:
      
      //Constructors
      explicit ASTTransformVisitor()
         : EvalDecl(0), gCling(0), SemaPtr(0), CurrentDecl(0){}      
      ASTTransformVisitor(void* gCling, Sema *SemaPtr)
         : EvalDecl(0), gCling(gCling), SemaPtr(SemaPtr), CurrentDecl(0){}
      
      // Destructors
      ~ASTTransformVisitor() { }

      FunctionDecl *getEvalDecl() { return EvalDecl; }
      void setEvalDecl(FunctionDecl *FDecl) { if (!EvalDecl) EvalDecl = FDecl; }
      MapTy &getSubstSymbolMap() { return m_SubstSymbolMap; }
      
      // DeclVisitor      
      void Visit(Decl *D);
      void VisitFunctionDecl(FunctionDecl *D);
      void VisitTemplateDecl(TemplateDecl *D); 
      void VisitDecl(Decl *D);
      void VisitDeclContext(DeclContext *DC);

      // StmtVisitor
      EvalInfo VisitStmt(Stmt *Node);
      EvalInfo VisitExpr(Expr *Node);
      EvalInfo VisitCallExpr(CallExpr *E);
      EvalInfo VisitDeclRefExpr(DeclRefExpr *DRE);
      EvalInfo VisitBinaryOperator(BinaryOperator *binOp);
      EvalInfo VisitDependentScopeDeclRefExpr(DependentScopeDeclRefExpr *Node);

      // EvalBuilder
      Expr *SubstituteUnknownSymbol(const QualType InstTy, Expr *SubTree);
      CallExpr *BuildEvalCallExpr(QualType type, Expr *SubTree, ASTOwningVector<Expr*> &CallArgs);
      void BuildEvalEnvironment(Expr *SubTree);
      void BuildEvalArgs(ASTOwningVector<Expr*> &Result);
      Expr *BuildEvalArg0(ASTContext &C);
      Expr *BuildEvalArg1(ASTContext &C);
      Expr *BuildEvalArg2(ASTContext &C);

      // Helper
      bool IsArtificiallyDependent(Expr *Node);
      bool ShouldVisit(Decl *D);
      
   };
   
}//end cling
