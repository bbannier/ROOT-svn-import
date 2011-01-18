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
      
   public: // members
      void *gCling; //Pointer to the Interpreter object
      clang::Sema *SemaPtr;
      Decl *CurrentDecl;
      
   public: // types
      
      typedef DeclVisitor<ASTTransformVisitor> BaseDeclVisitor;
      typedef StmtVisitor<ASTTransformVisitor, EvalInfo> BaseStmtVisitor;

      using BaseStmtVisitor::Visit;

   public:
      
      //region Constructors
      
      explicit ASTTransformVisitor()
         : EvalDecl(0), gCling(0), SemaPtr(0), CurrentDecl(0){}
      
      ASTTransformVisitor(void* gCling, Sema *SemaPtr)
         : EvalDecl(0), gCling(gCling),  SemaPtr(SemaPtr), CurrentDecl(0){}

      //endregion
      
      //region Destructor

      ~ASTTransformVisitor() {
         // delete EvalDecl;
         // EvalDecl = 0;
         // delete SemaPtr;
         // SemaPtr = 0;
         // delete CurrentDecl;
         // CurrentDecl = 0;
         // delete gCling;
         // gCling = 0;
      }
      
      
      //endregion

      FunctionDecl *getEvalDecl() { return EvalDecl; }
      void setEvalDecl(FunctionDecl *FDecl) { if (!EvalDecl) EvalDecl = FDecl; }
      MapTy &getSubstSymbolMap() { return m_SubstSymbolMap; }
      
      //region DeclVisitor
      
      void Visit(Decl *D);
      void VisitFunctionDecl(FunctionDecl *D);
      void VisitTemplateDecl(TemplateDecl *D); 
      
      void VisitDecl(Decl *D);
      void VisitDeclContext(DeclContext *DC);

      //endregion

      //region StmtVisitor

      EvalInfo VisitStmt(Stmt *Node);
      EvalInfo VisitExpr(Expr *Node);
      // EvalInfo VisitCompoundStmt(CompoundStmt *S);
      EvalInfo VisitCallExpr(CallExpr *E);
      // EvalInfo VisitImplicitCastExpr(ImplicitCastExpr *ICE);
      EvalInfo VisitDeclRefExpr(DeclRefExpr *DRE);
      EvalInfo VisitBinaryOperator(BinaryOperator *binOp);
      EvalInfo VisitDependentScopeDeclRefExpr(DependentScopeDeclRefExpr *Node);

      //endregion

      //region EvalBuilder

      Expr *SubstituteUnknownSymbol(const QualType InstTy, Expr *SubTree);
      CallExpr *BuildEvalCallExpr(QualType type, Expr *SubTree);
      Expr *BuildEvalCharArg(QualType ToType, Expr *SubTree);
      bool IsArtificiallyDependent(Expr *Node);
      bool ShouldVisit(Decl *D);
      //endregion
      
   };
   
}//end cling
