//------------------------------------------------------------------------------
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id: ASTTransformVisitor.cpp 36608 2010-11-11 18:21:02Z vvassilev $
// author:  Vassil Vassilev <vasil.georgiev.vasilev@cern.ch>
//------------------------------------------------------------------------------

#include "clang/AST/StmtVisitor.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclObjC.h"
#include "clang/AST/PrettyPrinter.h"
#include "llvm/Support/Format.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ExprCXX.h"

//FIXME: Forward declare what you can
using namespace clang;
namespace cling {
   // The purpose of this visitor is to traverse given subtree of the AST
   // and print the types and addresses of the DeclRefExpr-s: For example
   // int a = 5; b = 5; a + b we should get: (*(int*)) 0x128) + (*(int*)) 0x132)
   // If it encounters dependent symbols like h->Draw() or any other function
   // call it should escape it as it is.
   class StmtAddressPrinter : public StmtVisitor<StmtAddressPrinter> {
      llvm::raw_ostream &OS;
      ASTContext &Context;
      PrintingPolicy Policy;
      
   public:
      StmtAddressPrinter(llvm::raw_ostream &os, ASTContext &C,
                         const PrintingPolicy &Policy
                         )
         : OS(os), Context(C), Policy(Policy) {}
      
      void PrintExpr(Expr *E) {
         if (E)
            Visit(E);
         else
            OS << "<null expr>";
      }      
      
      void Visit(Stmt* S) {
         StmtVisitor<StmtAddressPrinter>::Visit(S);
      }

      void VisitDeclRefExpr(DeclRefExpr *Node);
      void VisitDependentScopeDeclRefExpr(DependentScopeDeclRefExpr *Node);
      void VisitCallExpr(CallExpr *Call);
      void VisitMemberExpr(MemberExpr *Node);
      void VisitCXXDependentScopeMemberExpr(CXXDependentScopeMemberExpr *Node);
  };
} // end cling
