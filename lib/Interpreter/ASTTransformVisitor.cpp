//------------------------------------------------------------------------------
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id: ASTTransformVisitor.cpp 36608 2010-11-11 18:21:02Z vvassilev $
// author:  Vassil Vassilev <vasil.georgiev.vasilev@cern.ch>
//------------------------------------------------------------------------------

#include "ASTTransformVisitor.h"

#include "StmtAddressPrinter.h"

#include "llvm/ADT/SmallVector.h"
//#include "llvm/ADT.DenseMap.h"

//#include "clang/Sema/Lookup.h"

namespace llvm {
   class raw_string_ostream;
}

namespace {
 
   class StmtPrinterHelper : public PrinterHelper  {
   private:
      PrintingPolicy Policy;
   public:
      
      StmtPrinterHelper(const PrintingPolicy &Policy) : Policy(Policy) {}
      
      virtual ~StmtPrinterHelper() {}
      
      //TODO: Here goes the address printing      
      virtual bool handledStmt(Stmt* S, llvm::raw_ostream& OS) {

         // DeclRefExpr
         // DependentScopeDeclRefExpr
         // CallExpr
         // MemberExpr
         // CXXDependentScopeMemberExpr
         if (DeclRefExpr *Node = dyn_cast<DeclRefExpr>(S)) {
            if (NestedNameSpecifier *Qualifier = Node->getQualifier())
               Qualifier->print(OS, Policy);
            OS << Node->getNameInfo();
            if (Node->hasExplicitTemplateArgs())
               OS << TemplateSpecializationType::PrintTemplateArgumentList(
                                                                           Node->getTemplateArgs(),
                                                                           Node->getNumTemplateArgs(),
                                                                           Policy);  
            if (Node->hasExplicitTemplateArgs())
               assert((Node->getTemplateArgs() || Node->getNumTemplateArgs()) && "There shouldn't be template paramlist");

            return true;            
         }
         else if (DependentScopeDeclRefExpr *Node = dyn_cast<DependentScopeDeclRefExpr>(S)) {
            if (NestedNameSpecifier *Qualifier = Node->getQualifier())
               Qualifier->print(OS, Policy);
            OS << Node->getNameInfo();
            if (Node->hasExplicitTemplateArgs())
               OS << TemplateSpecializationType::PrintTemplateArgumentList(
                                                                           Node->getTemplateArgs(),
                                                                           Node->getNumTemplateArgs(),
                                                                           Policy);
            
            
            
            return true;
         }
         
         return false;
      }
   };
} // end anonymous namespace


namespace cling {

   //region DeclVisitor
   
   void ASTTransformVisitor::Visit(Decl *D) {
      //if (ShouldVisit(D)) {
         Decl *PrevDecl = ASTTransformVisitor::CurrentDecl;
         ASTTransformVisitor::CurrentDecl = D;
         BaseDeclVisitor::Visit(D);
         ASTTransformVisitor::CurrentDecl = PrevDecl;
       //}
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
 
   void ASTTransformVisitor::VisitTemplateDecl(TemplateDecl *D) {     
      if (D->getNameAsString().compare("Eval") == 0) {
         CXXRecordDecl *CXX = dyn_cast<CXXRecordDecl>(D->getDeclContext());
         if (CXX && CXX->getNameAsString().compare("Interpreter") == 0) {  
            NamespaceDecl *ND = dyn_cast<NamespaceDecl>(CXX->getDeclContext());
            if (ND && ND->getNameAsString().compare("cling") == 0) {
               if (FunctionDecl *FDecl = dyn_cast<FunctionDecl>(D->getTemplatedDecl()))
                  setEvalDecl(FDecl);
            }
         }
      }
   }
  
   void ASTTransformVisitor::VisitDecl(Decl *D) {
      if (!ShouldVisit(D))
         return;
      
      if (DeclContext *DC = dyn_cast<DeclContext>(D))
         static_cast<ASTTransformVisitor*>(this)->VisitDeclContext(DC);
   }
   
   void ASTTransformVisitor::VisitDeclContext(DeclContext *DC) {
      for (DeclContext::decl_iterator
              I = DC->decls_begin(), E = DC->decls_end(); I != E; ++I)        
         if (ShouldVisit(*I))
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
                  *I = SubstituteUnknownSymbol(SemaPtr->getASTContext().VoidTy, E);
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
                  *I = SubstituteUnknownSymbol(SemaPtr->getASTContext().VoidTy, E);
            } 
            else {
               *I = EInfo.getNewStmt();
            }
         }
      }
      return EvalInfo(Node, 0);
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
      if (IsArtificiallyDependent(E)) {
         // FIXME: Handle the arguments
         // EvalInfo EInfo = Visit(E->getCallee());
         
         return EvalInfo(E, 1);
         
      }
      return EvalInfo(E, 0);
   }
   
   // EvalInfo ASTTransformVisitor::VisitImplicitCastExpr(ImplicitCastExpr *ICE) {
   //    return EvalInfo(ICE, 0);
   // }
   
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
                  binOp->setRHS(SubstituteUnknownSymbol(returnTy, E));
               }
         }
      }
      
      return EvalInfo(binOp, IsArtificiallyDependent(binOp));
   }
   
   //endregion

   //region EvalBuilder


   Expr *ASTTransformVisitor::SubstituteUnknownSymbol(const QualType InstTy, Expr *SubTree) {
      CallExpr* EvalCall = BuildEvalCallExpr(InstTy, SubTree);
      getSubstSymbolMap()[EvalCall] = SubTree;
      
      return EvalCall;
   }

   // Here is the test Eval function specialization. Here the CallExpr to the function
   // is created.
   CallExpr *ASTTransformVisitor::BuildEvalCallExpr(const QualType InstTy, Expr *SubTree) {      
      // we need the ASTContext
      ASTContext &C = SemaPtr->getASTContext();
      // Set up new context for the new FunctionDecl
      DeclContext *PrevContext = SemaPtr->CurContext;
      FunctionDecl *FDecl = getEvalDecl();
      
      assert(FDecl && "The Eval function not found!");

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

      //ParmVarDecl *Param0 = FDecl->getParamDecl(0U);
      ParmVarDecl *Param1 = FDecl->getParamDecl(1U);

      // Pass the address of the Interpreter object in
      const llvm::APInt gClingAddr(8 * sizeof(void *), (uint64_t)gCling);
      IntegerLiteral *IntLiteral = IntegerLiteral::Create(C, gClingAddr, C.UnsignedLongTy, SourceLocation());

      CallArgs.push_back(IntLiteral);
      CallArgs.push_back(BuildEvalCharArg(Param1->getType(), SubTree));

      
      CallExpr *EvalCall = SemaPtr->ActOnCallExpr(SemaPtr->getScopeForContext(SemaPtr->CurContext)
                                                  , DRE
                                                  , SourceLocation()
                                                  //,MultiExprArg(CallArgs.take() , 1U)
                                                  , move_arg(CallArgs)
                                                  , SourceLocation()
                                                  ).takeAs<CallExpr>();
      return EvalCall;                  
      
   }

   // Helper function for converting the Stmt to string 
   const char *ASTTransformVisitor::ToString(Stmt *S) {
      ASTContext *c = &SemaPtr->getASTContext();
      std::string sbuf;
      llvm::raw_string_ostream OS(sbuf);
      const PrintingPolicy &Policy = c->PrintingPolicy;

      StmtPrinterHelper *helper = new StmtPrinterHelper(Policy);      
      S->printPretty(OS, helper, Policy);

      OS.flush();
      return sbuf.c_str();
   }
   
   // Creates the string, which is going to be escaped.
   Expr *ASTTransformVisitor::BuildEvalCharArg(QualType ToType, Expr *SubTree) {
      ASTContext *c = &SemaPtr->getASTContext();
      
      const char *str = ToString(SubTree);
      QualType constCharArray = c->getConstantArrayType(c->getConstType(c->CharTy), llvm::APInt(32, 16U), ArrayType::Normal, 0);
      Expr *SL = StringLiteral::Create(*c, &*str, strlen(str), false, constCharArray, SourceLocation());
      //FIXME: Figure out how handle the cast kinds in the different cases
      SemaPtr->ImpCastExprToType(SL, ToType, CK_ArrayToPointerDecay);

      return SL;
   }

   bool ASTTransformVisitor::ShouldVisit(Decl *D) {
      while (true) {
         if (isa<TemplateTemplateParmDecl>(D))
            return false;
         if (isa<ClassTemplateDecl>(D))
            return false;
         if (isa<FriendTemplateDecl>(D))
            return false;
         if (isa<ClassTemplatePartialSpecializationDecl>(D))
            return false;
         if (CXXRecordDecl *CXX = dyn_cast<CXXRecordDecl>(D)) {
            if (CXX->getDescribedClassTemplate())
               return false;
         }
         if (CXXMethodDecl *CXX = dyn_cast<CXXMethodDecl>(D)) {
            if (CXX->getDescribedFunctionTemplate())
               return false;
         }
         if (isa<TranslationUnitDecl>(D)) {
            break;
         }
         
         if (DeclContext* DC = D->getDeclContext())
            if (!(D = dyn_cast<Decl>(DC)))
                break;
      }
      
      return true;
   }

   bool ASTTransformVisitor::IsArtificiallyDependent(Expr *Node) {
      if (!Node->isValueDependent() || !Node->isTypeDependent())
          return false;     
      return true;
   }

   
//endregion

}//end cling
