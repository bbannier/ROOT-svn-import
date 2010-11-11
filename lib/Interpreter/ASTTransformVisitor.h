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

using namespace clang;

// Ideally the visitor should traverse the dependent nodes, which actially are 
// the language extensions. For example File::Open("MyFile"); h->Draw() is not valid C++ call
// if h->Draw() is defined in MyFile. In that case we need to skip Sema diagnostics, so the 
// h->Draw() is marked as dependent node. That requires the ASTTransformVisitor to find all
// dependent nodes and escape them to the interpreter, using pre-defined Eval function.
class ASTTransformVisitor : public DeclVisitor<ASTTransformVisitor>,
                            public StmtVisitor<ASTTransformVisitor, Stmt*> {

private:
   FunctionTemplateDecl *EvalTemplate;

public:

   clang::Sema *SemaPtr; // Sema is needed

   typedef DeclVisitor<ASTTransformVisitor> BaseDeclVisitor;
   typedef StmtVisitor<ASTTransformVisitor, Stmt*> BaseStmtVisitor;

   FunctionTemplateDecl *getEvalTemplate(){ return EvalTemplate; }
   void setEvalTemplate(FunctionTemplateDecl *FTD){ EvalTemplate = FTD; }

   //region Constructors

   ASTTransformVisitor(Sema *SemaPtr) : SemaPtr(SemaPtr){};

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

   Stmt *VisitStmt(Stmt *Node);
   Stmt *VisitCompoundStmt(CompoundStmt *S);
   Stmt *VisitImplicitCastExpr(ImplicitCastExpr *ICE);
   Stmt *VisitDeclRefExpr(DeclRefExpr *DRE);
   Stmt *VisitCallExpr(CallExpr *CE);
   Stmt *VisitBinaryOperator(BinaryOperator *binOp);

   //endregion

};
