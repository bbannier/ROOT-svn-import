//------------------------------------------------------------------------------
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id: ASTTransformVisitor.cpp 36608 2010-11-11 18:21:02Z vvassilev $
// author:  Vassil Vassilev <vasil.georgiev.vasilev@cern.ch>
//------------------------------------------------------------------------------

#include "ASTTransformVisitor.h"

#include "StmtAddressPrinter.h"

#include "llvm/ADT/SmallVector.h"

namespace llvm {
   class raw_string_ostream;
}

namespace cling {

   //region Constructors

   ASTTransformVisitor::ASTTransformVisitor()
     : EvalDecl(0), SemaPtr(0), CurrentDecl(0)
   {
   }

   ASTTransformVisitor::ASTTransformVisitor(Sema *SemaPtr)
     : EvalDecl(0), SemaPtr(SemaPtr), CurrentDecl(0)
   {
   }

   //endregion


   //region Destructor

   //ASTTransformVisitor::~ASTTransformVisitor()
   //{
   //   delete EvalDecl;
   //   EvalDecl = 0;
   //   delete SemaPtr;
   //   SemaPtr = 0;
   //   delete CurrentDecl;
   //   CurrentDecl = 0;
   //}
   
   //endregion


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
              I = Node->child_begin(), E = Node->child_end(); I != E; ++I) {
         if (*I) {
            EvalInfo EInfo = Visit(*I);
            if (EInfo.IsEvalNeeded) {
               if (Expr *E = dyn_cast<Expr>(EInfo.getNewStmt()))
                  // Assume void if still not escaped
                  *I = BuildEvalCallExpr(SemaPtr->getASTContext().VoidTy, E);
            } 
            else {
               *I = EInfo.getNewStmt();
            }
         }
      }
      
      return EvalInfo(Node, 0);
   }
   
   EvalInfo ASTTransformVisitor::VisitExpr(Expr *Node) {
      for (Stmt::child_iterator
              I = Node->child_begin(), E = Node->child_end(); I != E; ++I) {
         if (*I) {
            EvalInfo EInfo = Visit(*I);
            if (EInfo.IsEvalNeeded) {
               if (Expr *E = dyn_cast<Expr>(EInfo.getNewStmt()))
                  // Assume void if still not escaped
                  *I = BuildEvalCallExpr(SemaPtr->getASTContext().VoidTy, E);
            } 
            else {
               *I = EInfo.getNewStmt();
            }
         }
      }
      return EvalInfo(Node, Node->isTypeDependent() || Node->isValueDependent());
   }

   // EvalInfo ASTTransformVisitor::VisitCompoundStmt(CompoundStmt *S) {
   //    for (CompoundStmt::body_iterator
   //            I = S->body_begin(), E = S->body_end(); I != E; ++I) {
   //       EvalInfo EInfo = Visit(*I);
   //       if (EInfo.IsEvalNeeded) {
   //          if (Expr *Exp = dyn_cast<Expr>(EInfo.getNewStmt())) {
   //             QualType T = Exp->getType();
   //             // Assume if still dependent void
   //             if (Exp->isTypeDependent() || Exp->isValueDependent())
   //                T = SemaPtr->getASTContext().VoidTy;

   //             *I = BuildEvalCallExpr(T);
   //          }
   //       } 
   //       else {
   //          *I = EInfo.getNewStmt();
   //       }
   //    }
   //    return EvalInfo(S, 0);
   // }

   EvalInfo ASTTransformVisitor::VisitCallExpr(CallExpr *E) {
      if (E->isTypeDependent() || E->isValueDependent()) {
         // FIXME: Handle the arguments
         EvalInfo EInfo = Visit(E->getCallee());

         return EvalInfo(E, 1);
      
      }
      return EvalInfo(E, 0);
   }
   
   EvalInfo ASTTransformVisitor::VisitImplicitCastExpr(ImplicitCastExpr *ICE) {
      return EvalInfo(ICE, 0);
   }
   
   EvalInfo ASTTransformVisitor::VisitDeclRefExpr(DeclRefExpr *DRE) {
      return EvalInfo(DRE, 0);
   }
      
   EvalInfo ASTTransformVisitor::VisitDependentScopeDeclRefExpr(DependentScopeDeclRefExpr *Node) {
      return EvalInfo(Node, 1);
   }

   EvalInfo ASTTransformVisitor::VisitBinaryOperator(BinaryOperator *binOp) {
      EvalInfo rhs = Visit(binOp->getRHS());
      EvalInfo lhs = Visit(binOp->getLHS());

      if (binOp->isAssignmentOp()) {
         if (rhs.IsEvalNeeded && !lhs.IsEvalNeeded) {
            if (Expr *E = dyn_cast<Expr>(lhs.getNewStmt()))
               if (!E->isTypeDependent() || !E->isValueDependent()) {
                  const QualType returnTy = E->getType();
                  binOp->setRHS(BuildEvalCallExpr(returnTy, E));
               }
         }
      }
      
      return EvalInfo(binOp, 0);
   }
   
   //endregion

   //region EvalBuilder


   // Here is the test Eval function specialization. Here the CallExpr to the function
   // is created.
   CallExpr *ASTTransformVisitor::BuildEvalCallExpr(const QualType InstTy, Expr *SubTree) {
      // Set up new context for the new FunctionDecl
      DeclContext *PrevContext = SemaPtr->CurContext;
      FunctionDecl *FDecl = getEvalDecl();
      SemaPtr->CurContext = FDecl->getDeclContext();
      
      // Create template arguments
      Sema::InstantiatingTemplate Inst(*SemaPtr, SourceLocation(), FDecl);
      TemplateArgument Arg(InstTy);
      TemplateArgumentList TemplateArgs(TemplateArgumentList::OnStack, &Arg, 1U);
      
      // Substitute the declaration of the templated function, with the 
      // specified template argument
      Decl *D = SemaPtr->SubstDecl(FDecl, FDecl->getDeclContext(), MultiLevelTemplateArgumentList(TemplateArgs));
      
      FunctionDecl *Fn = dyn_cast<FunctionDecl>(D);
      // Creates new body of the substituted declaration
      SemaPtr->InstantiateFunctionDefinition(Fn->getLocation(), Fn, true, true);
      
      SemaPtr->CurContext = PrevContext;                            
      
      const FunctionProtoType *Proto = Fn->getType()->getAs<FunctionProtoType>();

      //Walk the params and prepare them for building a new function type
      llvm::SmallVectorImpl<QualType> ParamTypes(FDecl->getNumParams());
      for (FunctionDecl::param_iterator P = FDecl->param_begin(), PEnd = FDecl->param_end();
           P != PEnd;
           ++P) {
         ParamTypes.push_back((*P)->getType());
         
      }
      
      // Build function type, needed by BuildDeclRefExpr 
      QualType FuncT = SemaPtr->BuildFunctionType(Fn->getResultType()
                                                  , ParamTypes.data()
                                                  , ParamTypes.size()
                                                  , Proto->isVariadic()
                                                  , Proto->getTypeQuals()
                                                  , Fn->getLocation()
                                                  , Fn->getDeclName()
                                                  , Proto->getExtInfo());                  
      DeclRefExpr *DRE = SemaPtr->BuildDeclRefExpr(Fn, FuncT, VK_RValue, SourceLocation()).takeAs<DeclRefExpr>();
      
      // Prepare the actual arguments for the call
      ASTOwningVector<Expr*> CallArgs(*SemaPtr);
      CallArgs.push_back(BuildEvalCharArg(FDecl->getParamDecl(0U)->getType(), SubTree));

      
      CallExpr *EvalCall = SemaPtr->ActOnCallExpr(SemaPtr->getScopeForContext(SemaPtr->CurContext)
                                                  , DRE
                                                  , SourceLocation()
                                                  //,MultiExprArg(CallArgs.take() , 1U)
                                                  , move_arg(CallArgs)
                                                  , SourceLocation()
                                                  ).takeAs<CallExpr>();
      // FIXME: Take in mind the string format specifiers in printf("%..")
      return EvalCall;                  
      
   }
   
   // Creates the string, which is going to be escaped.
   Expr *ASTTransformVisitor::BuildEvalCharArg(QualType ToType, Expr *SubTree) {
      ASTContext *c = &SemaPtr->getASTContext();
      //TODO: Here goes the address printing
      std::string sbuf;
      llvm::raw_string_ostream OS(sbuf);

      StmtAddressPrinter printer(OS, *c, PrintingPolicy(SemaPtr->getLangOptions()));
      //      printer.Visit(SubTree);
      printer.PrintExpr(SubTree);
      OS.flush();
      const char *str = sbuf.c_str();
      QualType constCharArray = c->getConstantArrayType(c->getConstType(c->CharTy), llvm::APInt(32, 16U), ArrayType::Normal, 0);
      Expr *SL = StringLiteral::Create(*c, &*str, strlen(str), false, constCharArray, SourceLocation());
      //FIXME: Figure out how handle the cast kinds in the different cases
      SemaPtr->ImpCastExprToType(SL, ToType, CK_ArrayToPointerDecay);

      return SL;
   }
   
//endregion

}//end cling
