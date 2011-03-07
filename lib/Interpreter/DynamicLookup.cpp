//------------------------------------------------------------------------------
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Vassil Vassilev <vasil.georgiev.vasilev@cern.ch>
//------------------------------------------------------------------------------

#include "DynamicLookup.h"
#include "EvalInfo.h"
#include "clang/Sema/Scope.h"
#include "clang/Sema/Lookup.h"
#include "clang/Sema/Template.h"


namespace clang {
  class LookupResult;
  class Scope;
}

using namespace clang;

namespace cling {
  extern "C" int printf(const char* fmt, ...);
  // pin the vtable to this file
  DynamicIDHandler::~DynamicIDHandler(){}

  // If there is failed lookup we tell sema to create artificial declaration
  // which is of dependent type. So the lookup result is marked as dependent
  // and the diagnostics are suppressed. After that is our responsibility to
  // fix all these fake declarations and lookups. It is done by the
  // DynamicExpressionTransformer
  bool DynamicIDHandler::LookupUnqualified(LookupResult& R, Scope* S) {

    if (!IsDynamicLookup(R, S))
      return false;

    DeclarationName Name = R.getLookupName();
    IdentifierInfo *II = Name.getAsIdentifierInfo();
    SourceLocation NameLoc = R.getNameLoc();
    FunctionDecl *D = dyn_cast<FunctionDecl>(R.getSema().ImplicitlyDefineFunction(NameLoc, *II, S));
    if (D) { 
      BuiltinType *Ty = new BuiltinType(BuiltinType::Dependent);
      QualType QTy(Ty, 0);            
      D->setType(QTy);
      R.addDecl(D);
      // Mark this declaration for removal
      m_FakeDecls.push_back(D);
      
      // Say that we can handle the situation. Clang should try to recover
      return true;
    }
    // We cannot handle the situation. Give up
    return false;              
  }

  bool DynamicIDHandler::IsDynamicLookup (LookupResult& R, Scope* S) {
    if (R.getLookupKind() != Sema::LookupOrdinaryName) return false;
    if (R.isForRedeclaration()) return false;
    for (Scope* DepScope = S; DepScope; DepScope = DepScope->getParent()) {
      if (DeclContext* Ctx = static_cast<DeclContext*>(DepScope->getEntity())) {
        return !Ctx->isDependentContext();
      }
    }
    
    return true;
  }

  // Removes the implicitly created functions, which help to emulate the dynamic scopes
  void DynamicIDHandler::RemoveFakeDecls() {      
    Scope *S = m_Sema->getScopeForContext(m_Sema->getASTContext().getTranslationUnitDecl());
    for (unsigned int i = 0; i < m_FakeDecls.size(); ++i) {
      printf("\nI am about to remove:\n");
      m_FakeDecls[i]->dump();
      S->RemoveDecl(m_FakeDecls[i]);
    }
  }
} // end namespace cling

namespace {
  
  class StmtPrinterHelper : public PrinterHelper  {
  private:
    PrintingPolicy m_Policy;
    llvm::SmallVector<DeclRefExpr*, 64> &m_Environment;
  public:
    
    StmtPrinterHelper(const PrintingPolicy &Policy, llvm::SmallVector<DeclRefExpr*, 64> &Environment) : 
      m_Policy(Policy), m_Environment(Environment) {}
    
    virtual ~StmtPrinterHelper() {}
    
    
    // Handle only DeclRefExprs since they are local and the call wrapper
    // won't "see" them. Consequently we don't need to handle:
    // * DependentScopeDeclRefExpr
    // * CallExpr
    // * MemberExpr
    // * CXXDependentScopeMemberExpr
    virtual bool handledStmt(Stmt* S, llvm::raw_ostream& OS) {
      if (DeclRefExpr *Node = dyn_cast<DeclRefExpr>(S))
        // Exclude the artificially dependent DeclRefExprs, created by the Lookup
        if (!Node->isTypeDependent()) {
          if (NestedNameSpecifier *Qualifier = Node->getQualifier())
            Qualifier->print(OS, m_Policy);
          m_Environment.push_back(Node);
          OS << "*("; 
          // Copy-paste from the StmtPrinter
          QualType T = Node->getType();
          SplitQualType T_split = T.split();
          OS << QualType::getAsString(T_split);
          
          if (!T.isNull()) {
            // If the type is sugared, also dump a (shallow) desugared type.
            SplitQualType D_split = T.getSplitDesugaredType();
            if (T_split != D_split)
              OS << ":" << QualType::getAsString(D_split);
          }
          // end
          
          OS <<"*)@";
          
          if (Node->hasExplicitTemplateArgs())
            OS << TemplateSpecializationType::PrintTemplateArgumentList(
                                                                        Node->getTemplateArgs(),
                                                                        Node->getNumTemplateArgs(),
                                                                        m_Policy);  
          if (Node->hasExplicitTemplateArgs())
            assert((Node->getTemplateArgs() || Node->getNumTemplateArgs()) && "There shouldn't be template paramlist");
          
          return true;            
        }
      
      return false;
    }
   };
} // end anonymous namespace


namespace cling {
  // Constructors
  DynamicExprTransformer::DynamicExprTransformer(Sema* Sema)
      : m_EvalDecl(0), m_CurDeclContext(0), m_Sema(Sema) {
    m_DynIDHandler.reset(new DynamicIDHandler(Sema));
    m_Sema->ExternalSource = m_DynIDHandler.get();
    
  }
  
  DynamicExprTransformer::DynamicExprTransformer(): m_EvalDecl(0), m_CurDeclContext(0), m_Sema(0){
  }
  
  void DynamicExprTransformer::Initialize() {
    //m_DeclContextType = m_Interpreter->getQualType("clang::DeclContext");
  }
    
  // DeclVisitor
  
  void DynamicExprTransformer::Visit(Decl *D) {
    //Decl *PrevDecl = DynamicExprTransformer::CurrentDecl;
    //DynamicExprTransformer::CurrentDecl = D;
    BaseDeclVisitor::Visit(D);
    //DynamicExprTransformer::CurrentDecl = PrevDecl;     
  }
  
  void DynamicExprTransformer::VisitFunctionDecl(FunctionDecl *D) {
    if (!D->isDependentContext() && D->isThisDeclarationADefinition()) {
      Stmt *Old = D->getBody();
      Stmt *New = Visit(Old).getNewStmt();
      if (Old != New)
        D->setBody(New);
    }
  }
  
  void DynamicExprTransformer::VisitTemplateDecl(TemplateDecl *D) {     
    if (D->getNameAsString().compare("EvaluateProxyT") == 0) {
      NamespaceDecl* internal = dyn_cast<NamespaceDecl>(D->getDeclContext());
      if (internal && internal->getNameAsString().compare("internal") == 0) {
        NamespaceDecl* runtime = dyn_cast<NamespaceDecl>(internal->getParent());
        if (runtime && runtime->getNameAsString().compare("runtime") == 0) {
          NamespaceDecl* cling = dyn_cast<NamespaceDecl>(runtime->getParent());
          if (cling && cling->getNameAsString().compare("cling") == 0) {
            if (FunctionDecl *FDecl = dyn_cast<FunctionDecl>(D->getTemplatedDecl()))
              setEvalDecl(FDecl);
          }
        }
      }
    }
  }
  
  void DynamicExprTransformer::VisitDecl(Decl *D) {
    if (!ShouldVisit(D))
      return;
    
    if (DeclContext *DC = dyn_cast<DeclContext>(D))
      if (!(DC->isDependentContext()))
        static_cast<DynamicExprTransformer*>(this)->VisitDeclContext(DC);
  }
  
  void DynamicExprTransformer::VisitDeclContext(DeclContext *DC) {
    m_CurDeclContext = DC;
    for (DeclContext::decl_iterator
           I = DC->decls_begin(), E = DC->decls_end(); I != E; ++I)        
         if (ShouldVisit(*I))
           Visit(*I);
  }
  
  // end DeclVisitor
  
  // StmtVisitor   
  
  EvalInfo DynamicExprTransformer::VisitStmt(Stmt *Node) {
    for (Stmt::child_iterator
           I = Node->child_begin(), E = Node->child_end(); I != E; ++I) {
      if (*I) {
        EvalInfo EInfo = Visit(*I);
        if (EInfo.IsEvalNeeded) {
          if (Expr *E = dyn_cast<Expr>(EInfo.getNewStmt()))
            // Assume void if still not escaped
            *I = SubstituteUnknownSymbol(m_Sema->getASTContext().VoidTy, E);
        } 
        else {
          *I = EInfo.getNewStmt();
        }
      }
    }
    
    return EvalInfo(Node, 0);
  }
  
  EvalInfo DynamicExprTransformer::VisitExpr(Expr *Node) {
    for (Stmt::child_iterator
           I = Node->child_begin(), E = Node->child_end(); I != E; ++I) {
      if (*I) {
        EvalInfo EInfo = Visit(*I);
        if (EInfo.IsEvalNeeded) {
          if (Expr *E = dyn_cast<Expr>(EInfo.getNewStmt()))
            // Assume void if still not escaped
            *I = SubstituteUnknownSymbol(m_Sema->getASTContext().VoidTy, E);
        } 
        else {
          *I = EInfo.getNewStmt();
        }
      }
    }
    return EvalInfo(Node, 0);
  }
  
  EvalInfo DynamicExprTransformer::VisitCallExpr(CallExpr *E) {
    // FIXME: Maybe we need to handle the arguments
    // EvalInfo EInfo = Visit(E->getCallee());
    return EvalInfo (E, IsArtificiallyDependent(E));
  }
  
  EvalInfo DynamicExprTransformer::VisitDeclRefExpr(DeclRefExpr *DRE) {
    return EvalInfo(DRE, IsArtificiallyDependent(DRE));
  }
  
  EvalInfo DynamicExprTransformer::VisitDependentScopeDeclRefExpr(DependentScopeDeclRefExpr *Node) {
    return EvalInfo(Node, IsArtificiallyDependent(Node));
  }
  
  // end StmtVisitor
  
  // EvalBuilder
  
  Expr *DynamicExprTransformer::SubstituteUnknownSymbol(const QualType InstTy, Expr *SubTree) {
    // Get the addresses
    BuildEvalEnvironment(SubTree);
    
    //Build the arguments for the call
    ASTOwningVector<Expr*> CallArgs(*m_Sema);
    BuildEvalArgs(CallArgs);
    
    // Build the call
    CallExpr* EvalCall = BuildEvalCallExpr(InstTy, SubTree, CallArgs);
    
    // Add substitution mapping
    getSubstSymbolMap()[EvalCall] = SubTree;
    
    // Tell the DynamicIDHandler that the ID has been substituted and it can 
    // clean up
    m_DynIDHandler->RemoveFakeDecls();
    
    return EvalCall;
  }
  
  // Creates the string, which is going to be escaped.
  void DynamicExprTransformer::BuildEvalEnvironment(Expr *SubTree) {
    m_EvalExpressionBuf = "";
    llvm::raw_string_ostream OS(m_EvalExpressionBuf);
    const PrintingPolicy &Policy = m_Sema->getASTContext().PrintingPolicy;
    
    StmtPrinterHelper *helper = new StmtPrinterHelper(Policy, m_Environment);      
    SubTree->printPretty(OS, helper, Policy);
    
    OS.flush();
  }
  
  // Prepare the actual arguments for the call
  // Arg list for static T Eval(size_t This, const char* expr, void* varaddr[], clang::DeclContext* DC )
  void DynamicExprTransformer::BuildEvalArgs(ASTOwningVector<Expr*> &Result) {
    ASTContext &C = m_Sema->getASTContext();

    // Arg 0:
    Expr *Arg0 = BuildEvalArg0(C);    
    Result.push_back(Arg0);
    
    // Arg 1:
    Expr *Arg1 = BuildEvalArg1(C);          
    Result.push_back(Arg1);
    
    // Arg 2:
    Expr *Arg2 = BuildEvalArg2(C);          
    Result.push_back(Arg2);
    
  }
    
  // Eval Arg0: const char* expr
  Expr *DynamicExprTransformer::BuildEvalArg0(ASTContext &C) {
    const QualType ConstChar = C.getConstType(C.CharTy);
    const QualType ConstCharArray = C.getConstantArrayType(ConstChar, llvm::APInt(C.getTypeSize(ConstChar), m_EvalExpressionBuf.length() + 1), ArrayType::Normal, /*IndexTypeQuals=*/ 0);
    Expr *Arg0 = StringLiteral::Create(C, &*m_EvalExpressionBuf.c_str(), m_EvalExpressionBuf.length(), /*Wide=*/ false, ConstCharArray, SourceLocation());
    const QualType CastTo = C.getPointerType(C.getConstType(C.CharTy));
    m_Sema->ImpCastExprToType(Arg0, CastTo, CK_ArrayToPointerDecay);
    
    return Arg0;
  }
  
  // Eval Arg1: void* varaddr[]
  Expr *DynamicExprTransformer::BuildEvalArg1(ASTContext &C) {
    QualType VarAddrTy = m_Sema->BuildArrayType(C.VoidPtrTy, 
                                                 ArrayType::Normal,
                                                 /*ArraySize*/0,
                                                 Qualifiers(),
                                                 SourceRange(),
                                                 DeclarationName() );
    
    ASTOwningVector<Expr*> Inits(*m_Sema);
    for (unsigned int i = 0; i < m_Environment.size(); ++i) {
      Expr *UnOp = m_Sema->BuildUnaryOp(m_Sema->getScopeForContext(m_Sema->CurContext), 
                                         SourceLocation(), 
                                         UO_AddrOf,
                                         m_Environment[i]).takeAs<UnaryOperator>();
      m_Sema->ImpCastExprToType(UnOp, C.getPointerType(C.VoidPtrTy), CK_BitCast);
      Inits.push_back(UnOp);
    }
    
    // We need fake the SourceLocation just to avoid assert(InitList.isExplicit()....)
    SourceLocation SLoc = getEvalDecl()->getLocStart();
    SourceLocation ELoc = getEvalDecl()->getLocEnd();
    InitListExpr *ILE = m_Sema->ActOnInitList(SLoc, move_arg(Inits), ELoc).takeAs<InitListExpr>();
    Expr *Arg1 = m_Sema->BuildCompoundLiteralExpr(SourceLocation(), C.CreateTypeSourceInfo(VarAddrTy), SourceLocation(), ILE).takeAs<CompoundLiteralExpr>();
    m_Sema->ImpCastExprToType(Arg1, C.getPointerType(C.VoidPtrTy), CK_ArrayToPointerDecay);
    
    return Arg1;
  }
  
  // Eval Arg2: DeclContext* DC
  Expr *DynamicExprTransformer::BuildEvalArg2(ASTContext &C) {
    ParmVarDecl* param2 = getEvalDecl()->getParamDecl(2);
    QualType DeclContextTy = param2->getOriginalType();     
    
    const llvm::APInt DCAddr(8 * sizeof(void *), (uint64_t)m_CurDeclContext);
    
    Expr *Arg2 = IntegerLiteral::Create(C, DCAddr, C.UnsignedLongTy, SourceLocation());
    m_Sema->ImpCastExprToType(Arg2, DeclContextTy, CK_IntegralToPointer);
    
    return Arg2;
  }
  
  // Here is the test Eval function specialization. Here the CallExpr to the function
  // is created.
  CallExpr *DynamicExprTransformer::BuildEvalCallExpr(const QualType InstTy, Expr *SubTree, 
                                                   ASTOwningVector<Expr*> &CallArgs) {      
    // Set up new context for the new FunctionDecl
    DeclContext *PrevContext = m_Sema->CurContext;
    FunctionDecl *FDecl = getEvalDecl();
    
    assert(FDecl && "The Eval function not found!");
    
    m_Sema->CurContext = FDecl->getDeclContext();
    
    // Create template arguments
    Sema::InstantiatingTemplate Inst(*m_Sema, SourceLocation(), FDecl);
    TemplateArgument Arg(InstTy);
    TemplateArgumentList TemplateArgs(TemplateArgumentList::OnStack, &Arg, 1U);
    
    // Substitute the declaration of the templated function, with the 
    // specified template argument
    Decl *D = m_Sema->SubstDecl(FDecl, FDecl->getDeclContext(), MultiLevelTemplateArgumentList(TemplateArgs));
    
    FunctionDecl *Fn = dyn_cast<FunctionDecl>(D);
    // Creates new body of the substituted declaration
    m_Sema->InstantiateFunctionDefinition(Fn->getLocation(), Fn, true, true);
    
    m_Sema->CurContext = PrevContext;                            
    
    const FunctionProtoType *Proto = Fn->getType()->getAs<FunctionProtoType>();
    
    //Walk the params and prepare them for building a new function type
    llvm::SmallVectorImpl<QualType> ParamTypes(FDecl->getNumParams());
    for (FunctionDecl::param_iterator P = FDecl->param_begin(), PEnd = FDecl->param_end();
         P != PEnd;
         ++P) {
      ParamTypes.push_back((*P)->getType());
      
    }
    
    // Build function type, needed by BuildDeclRefExpr 
    QualType FuncT = m_Sema->BuildFunctionType(Fn->getResultType(),
                                               ParamTypes.data(),
                                               ParamTypes.size(),
                                               Proto->isVariadic(),
                                               Proto->getTypeQuals(),
                                               RQ_None,
                                               Fn->getLocation(),
                                               Fn->getDeclName(),
                                               Proto->getExtInfo());                  
    
    DeclRefExpr *DRE = m_Sema->BuildDeclRefExpr(Fn, FuncT, VK_RValue, SourceLocation()).takeAs<DeclRefExpr>();
    
    CallExpr *EvalCall = m_Sema->ActOnCallExpr(m_Sema->getScopeForContext(m_Sema->CurContext),
                                                DRE,
                                                SourceLocation(),
                                                move_arg(CallArgs),
                                                SourceLocation()
                                                ).takeAs<CallExpr>();
    assert (EvalCall && "Cannot create call to Eval");

    return EvalCall;                  
    
  } 
  
  // end EvalBuilder
  
  // Helpers
    
  bool DynamicExprTransformer::ShouldVisit(Decl *D) {
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
  
  bool DynamicExprTransformer::IsArtificiallyDependent(Expr *Node) {
    if (!Node->isValueDependent() || !Node->isTypeDependent())
      return false;     
    return true;
  }
  // end Helpers   
  
} // end namespace cling

