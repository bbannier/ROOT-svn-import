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

   // Ideally the visitor should traverse the dependent nodes, which actially are 
   // the language extensions. For example File::Open("MyFile"); h->Draw() is not valid C++ call
   // if h->Draw() is defined in MyFile. In that case we need to skip Sema diagnostics, so the 
   // h->Draw() is marked as dependent node. That requires the ASTTransformVisitor to find all
   // dependent nodes and escape them to the interpreter, using pre-defined Eval function.
   class ASTTransformVisitor : public DeclVisitor<ASTTransformVisitor>,
                               public StmtVisitor<ASTTransformVisitor, EvalInfo> {
      
   private:
      FunctionDecl *EvalDecl;
      
   public:
      
      clang::Sema *SemaPtr; // Sema is needed
      
      typedef DeclVisitor<ASTTransformVisitor> BaseDeclVisitor;
      typedef StmtVisitor<ASTTransformVisitor, EvalInfo> BaseStmtVisitor;
      
      FunctionDecl *getEvalDecl(){ return EvalDecl; }
      void setEvalDecl(FunctionDecl *FDecl){ EvalDecl = FDecl; }
      
      //region Constructors
      ASTTransformVisitor(Sema *SemaPtr) : SemaPtr(SemaPtr), CurrentDecl(0){};
      Decl *CurrentDecl;
      
      //endregion
      
      using BaseStmtVisitor::Visit;
      
      //region DeclVisitor
      
      void Visit(Decl *D);
      void VisitDeclaratorDecl(DeclaratorDecl *D);
      void VisitFunctionDecl(FunctionDecl *D);
      void VisitFunctionTemplateDecl(FunctionTemplateDecl *D); 
      void VisitObjCMethodDecl(ObjCMethodDecl *D);
      void VisitBlockDecl(BlockDecl *D);
      void VisitVarDecl(VarDecl *D);
      void VisitDecl(Decl *D);
      void VisitDeclContext(DeclContext *DC);

      //endregion

      //region StmtVisitor

      EvalInfo VisitStmt(Stmt *Node);
      EvalInfo VisitCompoundStmt(CompoundStmt *S);
      EvalInfo VisitImplicitCastExpr(ImplicitCastExpr *ICE);
      EvalInfo VisitDeclRefExpr(DeclRefExpr *DRE);
      EvalInfo VisitCallExpr(CallExpr *CE);
      EvalInfo VisitBinaryOperator(BinaryOperator *binOp);
      
      //endregion
      
   };
   
}//end cling
