//------------------------------------------------------------------------------
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id: ASTTransformVisitor.cpp 36608 2010-11-11 18:21:02Z vvassilev $
// author:  Vassil Vassilev <vasil.georgiev.vasilev@cern.ch>
//------------------------------------------------------------------------------

#include "StmtAddressPrinter.h"

//===----------------------------------------------------------------------===//
// StmtPrinter Visitor
//===----------------------------------------------------------------------===//

namespace cling {
       
   //===----------------------------------------------------------------------===//
   //  Expr printing methods.
   //===----------------------------------------------------------------------===//
   
   void StmtAddressPrinter::VisitDeclRefExpr(DeclRefExpr *Node) {
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
   }
   
   void StmtAddressPrinter::VisitDependentScopeDeclRefExpr(
                                                           DependentScopeDeclRefExpr *Node) {
      if (NestedNameSpecifier *Qualifier = Node->getQualifier())
         Qualifier->print(OS, Policy);
      OS << Node->getNameInfo();
      if (Node->hasExplicitTemplateArgs())
         OS << TemplateSpecializationType::PrintTemplateArgumentList(
                                                                     Node->getTemplateArgs(),
                                                                     Node->getNumTemplateArgs(),
                                                                     Policy);
   }
   
   void StmtAddressPrinter::VisitCallExpr(CallExpr *Call) {
      PrintExpr(Call->getCallee());
      OS << "(";
      for (unsigned i = 0, e = Call->getNumArgs(); i != e; ++i) {
         if (isa<CXXDefaultArgExpr>(Call->getArg(i))) {
            // Don't print any defaulted arguments
            break;
         }
         
         if (i) OS << ", ";
         PrintExpr(Call->getArg(i));
      }
      OS << ")";
   }
   void StmtAddressPrinter::VisitMemberExpr(MemberExpr *Node) {
      // FIXME: Suppress printing implicit bases (like "this")
      PrintExpr(Node->getBase());
      if (FieldDecl *FD = dyn_cast<FieldDecl>(Node->getMemberDecl()))
         if (FD->isAnonymousStructOrUnion())
            return;
      OS << (Node->isArrow() ? "->" : ".");
      if (NestedNameSpecifier *Qualifier = Node->getQualifier())
         Qualifier->print(OS, Policy);
      
      OS << Node->getMemberNameInfo();
      
      if (Node->hasExplicitTemplateArgs())
         OS << TemplateSpecializationType::PrintTemplateArgumentList(
                                                                     Node->getTemplateArgs(),
                                                                     Node->getNumTemplateArgs(),
                                                                     Policy);
   }   
   
   void StmtAddressPrinter::VisitCXXDependentScopeMemberExpr(
                                                             CXXDependentScopeMemberExpr *Node) {
      if (!Node->isImplicitAccess()) {
         PrintExpr(Node->getBase());
         OS << (Node->isArrow() ? "->" : ".");
      }
      if (NestedNameSpecifier *Qualifier = Node->getQualifier())
         Qualifier->print(OS, Policy);
      else if (Node->hasExplicitTemplateArgs())
         // FIXME: Track use of "template" keyword explicitly?
         OS << "template ";
      
      OS << Node->getMemberNameInfo();
      
      if (Node->hasExplicitTemplateArgs()) {
         OS << TemplateSpecializationType::PrintTemplateArgumentList(
                                                                     Node->getTemplateArgs(),
                                                                     Node->getNumTemplateArgs(),
                                                                     Policy);
      }
   }
}
