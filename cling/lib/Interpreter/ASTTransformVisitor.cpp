//------------------------------------------------------------------------------
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id: ASTTransformVisitor.cpp 36608 2010-11-11 18:21:02Z vvassilev $
// author:  Vassil Vassilev <vasil.georgiev.vasilev@cern.ch>
//------------------------------------------------------------------------------

#include "ASTTransformVisitor.h"

namespace cling {

   //region DeclVisitor

   void ASTTransformVisitor::Visit(Decl *D) {
      Decl *PrevDecl = ASTTransformVisitor::CurrentDecl;
      ASTTransformVisitor::CurrentDecl = D;
      BaseDeclVisitor::Visit(D);
      ASTTransformVisitor::CurrentDecl = PrevDecl;
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
         Stmt *New = Visit(Old).getNewStmt();
         if (Old != New)
            D->setBody(New);
      }
   }
   
   void ASTTransformVisitor::VisitFunctionTemplateDecl(FunctionTemplateDecl *D) {
      BaseDeclVisitor::VisitFunctionTemplateDecl(D);
    
      if (D->getNameAsString().compare("Eval") == 0) {
         NamespaceDecl *ND = dyn_cast<NamespaceDecl>(D->getDeclContext());
         if (ND && ND->getNameAsString().compare("cling") == 0) {
            setEvalDecl(D->getTemplatedDecl());
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
   
   EvalInfo ASTTransformVisitor::VisitStmt(Stmt *Node) {
      for (Stmt::child_iterator
              I = Node->child_begin(), E = Node->child_end(); I != E; ++I)
         if (*I)
            *I = Visit(*I).getNewStmt();
      
      return EvalInfo(Node, 0);
   }
   
   EvalInfo ASTTransformVisitor::VisitCompoundStmt(CompoundStmt *S) {
      for (CompoundStmt::body_iterator
              I = S->body_begin(), E = S->body_end(); I != E; ++I) {
         *I = Visit(*I).getNewStmt();
      }
      return EvalInfo(S, 0);
   }
   
   EvalInfo ASTTransformVisitor::VisitImplicitCastExpr(ImplicitCastExpr *ICE) {
      return EvalInfo(ICE, 0);
   }
   
   EvalInfo ASTTransformVisitor::VisitDeclRefExpr(DeclRefExpr *DRE) {
      return EvalInfo(DRE, 0);
   }
   
   EvalInfo ASTTransformVisitor::VisitCallExpr(CallExpr *CE) {
      
      //      if (!CE->isTypeDependent()) {
      // setPrf(CE);
      //      }
      
      //      for (CallExpr::child_iterator
      //              I = CE->child_begin(), E = CE->child_end(); I != E; ++I) {
      //         *I = Visit(*I);
      //      } 
      
      return EvalInfo(CE, 0);
   }
   
   // Here is the test Eval function specialization. Here the CallExpr to the function
   // is created.
   EvalInfo ASTTransformVisitor::VisitBinaryOperator(BinaryOperator *binOp) {
      /*Stmt *rhs =*/ Visit(binOp->getRHS());
      Stmt *lhs = Visit(binOp->getLHS()).getNewStmt();
      if (CallExpr *CE = dyn_cast<CallExpr>(lhs)){
         if (CE->isValueDependent() || CE->isTypeDependent()) {
            if (FunctionDecl *FDecl = getEvalDecl()) {               
               if (/*Expr *lhsEx =*/ dyn_cast<Expr>(lhs)) {                  
                  // QualType lhsTy = lhsEx->getType();
                  // TemplateArgument Arg(lhsTy);

                  ASTContext *Ctx = &SemaPtr->getASTContext();

                  DeclContext *PrevContext = SemaPtr->CurContext;
                  SemaPtr->CurContext = FDecl->getDeclContext();

                  Sema::InstantiatingTemplate Inst(*SemaPtr, SourceLocation(), FDecl);
                  TemplateArgument Arg(Ctx->IntTy);
                  TemplateArgumentList TemplateArgs(TemplateArgumentList::OnStack, &Arg, 1U);
                  
                  Decl *D = SemaPtr->SubstDecl(FDecl, FDecl->getDeclContext(), MultiLevelTemplateArgumentList(TemplateArgs));

                  FunctionDecl *Fn = dyn_cast<FunctionDecl>(D);
                  SemaPtr->InstantiateFunctionDefinition(Fn->getLocation(), Fn, true, true);

                  SemaPtr->CurContext = PrevContext;                            

                  const FunctionProtoType *Proto = Fn->getType()->getAs<FunctionProtoType>();
                  QualType FuncT = SemaPtr->BuildFunctionType(Fn->getResultType()
                                                              , /* ParamsTypes */ 0
                                                              , /* NumParamTypes */ 0
                                                              , Proto->isVariadic()
                                                              , Proto->getTypeQuals()
                                                              , Fn->getLocation()
                                                              , Fn->getDeclName()
                                                              , Proto->getExtInfo());                  
                  DeclRefExpr *DRE = SemaPtr->BuildDeclRefExpr(Fn, FuncT, VK_RValue, SourceLocation()).takeAs<DeclRefExpr>();
                  CallExpr *EvalIntCall = SemaPtr->ActOnCallExpr(SemaPtr->getScopeForContext(SemaPtr->CurContext)
                                                                 , DRE
                                                                 , SourceLocation()
                                                                 , MultiExprArg()
                                                                 , SourceLocation()
                                                                 ).takeAs<CallExpr>();

                  return EvalInfo(EvalIntCall, 1);                  
               }               
            }
         }
      }
      
      return EvalInfo(binOp, 0);
   }

//endregion

}//end cling
