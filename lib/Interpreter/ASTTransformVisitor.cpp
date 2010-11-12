//------------------------------------------------------------------------------
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id: ASTTransformVisitor.cpp 36608 2010-11-11 18:21:02Z vvassilev $
// author:  Vassil Vassilev <vasil.georgiev.vasilev@cern.ch>
//------------------------------------------------------------------------------

#include "ASTTransformVisitor.h"

namespace cling {

   //region DeclVisitor

   void ASTTransformVisitor::Visit(Decl *D) {
      BaseDeclVisitor::Visit(D);
   }
   
   void ASTTransformVisitor::VisitDeclaratorDecl(DeclaratorDecl *D) {
      BaseDeclVisitor::VisitDeclaratorDecl(D);
      //     if (TypeSourceInfo *TInfo = D->getTypeSourceInfo())
      //       Visit(TInfo->getTypeLoc());
   }
   
   void ASTTransformVisitor::VisitFunctionDecl(FunctionDecl *D) {
      BaseDeclVisitor::VisitFunctionDecl(D);
      if (D->isThisDeclarationADefinition()) {
         Stmt *Old = D->getBody();
         Stmt *New = Visit(Old);
         if (Old != New)
            D->setBody(New);
      }
   }
   
   void ASTTransformVisitor::VisitFunctionTemplateDecl(FunctionTemplateDecl *D) {
      if (D->getName().compare("Eval") == 0) {
         NamespaceDecl *ND = dyn_cast<NamespaceDecl>(D->getDeclContext());
         if (ND && ND->getName().compare("cling") == 0) {
            setEvalTemplate(D);
         }
      }
   }
   
   void ASTTransformVisitor::VisitObjCMethodDecl(ObjCMethodDecl *D) {
      BaseDeclVisitor::VisitObjCMethodDecl(D);
      if (D->getBody())
         Visit(D->getBody());
   }
   
   void ASTTransformVisitor::VisitBlockDecl(BlockDecl *D) {
      BaseDeclVisitor::VisitBlockDecl(D);
      Visit(D->getBody());
   }
   
   void ASTTransformVisitor::VisitVarDecl(VarDecl *D) {
      BaseDeclVisitor::VisitVarDecl(D);
      if (Expr *Init = D->getInit())
         Visit(Init);
   }
   
   void ASTTransformVisitor::VisitDecl(Decl *D) {
      if (isa<FunctionDecl>(D) || isa<ObjCMethodDecl>(D) || isa<BlockDecl>(D))
         return;
      
      if (DeclContext *DC = dyn_cast<DeclContext>(D))
         static_cast<ASTTransformVisitor*>(this)->VisitDeclContext(DC);
   }
   
   void ASTTransformVisitor::VisitDeclContext(DeclContext *DC) {
      for (DeclContext::decl_iterator
              I = DC->decls_begin(), E = DC->decls_end(); I != E; ++I)
         Visit(*I);
   }
   
   //endregion
   
   //region StmtVisitor
   
   Stmt *ASTTransformVisitor::VisitStmt(Stmt *Node) {
      for (Stmt::child_iterator
              I = Node->child_begin(), E = Node->child_end(); I != E; ++I)
         if (*I)
            *I = Visit(*I);
      
      return Node;
   }
   
   Stmt *ASTTransformVisitor::VisitCompoundStmt(CompoundStmt *S) {
      for (CompoundStmt::body_iterator
              I = S->body_begin(), E = S->body_end(); I != E; ++I) {
         *I = Visit(*I);
      }
      return S;
   }
   
   Stmt *ASTTransformVisitor::VisitImplicitCastExpr(ImplicitCastExpr *ICE) {
      return ICE;
   }
   
   Stmt *ASTTransformVisitor::VisitDeclRefExpr(DeclRefExpr *DRE) {
      return DRE;
   }
   
   Stmt *ASTTransformVisitor::VisitCallExpr(CallExpr *CE) {
      
      //      if (!CE->isTypeDependent()) {
      // setPrf(CE);
      //      }
      
      //      for (CallExpr::child_iterator
      //              I = CE->child_begin(), E = CE->child_end(); I != E; ++I) {
      //         *I = Visit(*I);
      //      } 
      
      return CE;
   }
   
   // Here is the test Eval function specialization. Here the CallExpr to the function
   // is created.
   Stmt *ASTTransformVisitor::VisitBinaryOperator(BinaryOperator *binOp) {
      Stmt *rhs = Visit(binOp->getRHS());
      Stmt *lhs = Visit(binOp->getLHS());
      if (CallExpr *CE = dyn_cast<CallExpr>(lhs)){
         if (CE->isValueDependent() || CE->isTypeDependent()) {
            if (FunctionTemplateDecl *FTD = getEvalTemplate()) {
               DeclContext *PrevContext = SemaPtr->CurContext;
               Decl *D;
               
               Sema::InstantiatingTemplate Inst(*SemaPtr, SourceLocation(), FTD);
               SemaPtr->CurContext = FTD->getDeclContext();
               
               if (Expr *lhsEx = dyn_cast<Expr>(lhs)) {
                  QualType lhsTy = lhsEx->getType();
                  //TemplateArgument Arg(lhsTy);
                  TemplateArgument Arg(SemaPtr->getASTContext().IntTy);
                  TemplateArgumentList TemplateArgs(TemplateArgumentList::OnStack, &Arg, 1U);
                  
                  D = SemaPtr->SubstDecl(FTD, FTD->getDeclContext(), MultiLevelTemplateArgumentList(TemplateArgs));
                  SemaPtr->CurContext = PrevContext;

                  FTD = dyn_cast<FunctionTemplateDecl>(D);
                  FunctionDecl *Fn = FTD->getTemplatedDecl();
                  
                  DeclRefExpr *DRE = DeclRefExpr::Create(*&SemaPtr->getASTContext(), NULL, SourceRange(), Fn, Fn->getLocation(), Fn->getType(), NULL);
                  ASTContext *Ctx = &SemaPtr->getASTContext();
                  CallExpr *EvalCall = new (Ctx) CallExpr(*Ctx, DRE, 0, 0, Fn->getType(), SourceLocation());
                  return SemaPtr->ActOnCallExpr(SemaPtr->getScopeForContext(SemaPtr->CurContext), DRE, SourceLocation(), MultiExprArg(), SourceLocation()).get();
               }

            }
         }
      }
      
      if (IntegerLiteral *IL = dyn_cast<IntegerLiteral>(rhs)) {
         
      }
      
      
      // ASTContext *c = &SemaPtr->getASTContext();
      // const char *str = "Transform World!";
      
      // QualType constCharArray = c->getConstantArrayType(c->getConstType(c->CharTy), llvm::APInt(32, 16U), ArrayType::Normal, 0);
      // StringLiteral *SL = StringLiteral::Create(*c, &*str, sizeof(str), false, constCharArray, SourceLocation());
      
      // QualType charType = c->getPointerType(c->getConstType(c->CharTy));
      // ImplicitCastExpr *cast = ImplicitCastExpr::Create(*c, charType, CK_ArrayToPointerDecay, SL, 0, VK_RValue);
      
      // CallExpr *theCall = new (c) CallExpr(*c, getPrf()->getCallee(), (Expr**)&cast, 1U, c->VoidPtrTy, SourceLocation());
      // return theCall;
      return binOp;
   }

//endregion

}//end cling
