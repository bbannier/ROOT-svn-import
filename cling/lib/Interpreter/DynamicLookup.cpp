//------------------------------------------------------------------------------
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Vassil Vassilev <vasil.georgiev.vasilev@cern.ch>
//------------------------------------------------------------------------------

#include "DynamicLookup.h"
#include "cling/Interpreter/Interpreter.h"

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

  // Constructor
  DynamicIDHandler::DynamicIDHandler(clang::Sema* Sema)
    : m_Sema(Sema), m_Context(Sema->getASTContext())
  {}


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
    Scope *S = m_Sema->getScopeForContext(m_Context.getTranslationUnitDecl());
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
  DynamicExprTransformer::DynamicExprTransformer(Interpreter* interp, Sema* Sema)
    : m_EvalDecl(0), 
      m_CurDeclContext(0),
      m_Interpreter(interp),
      m_Sema(Sema), 
      m_Context(Sema->getASTContext()) 
  {
    m_DynIDHandler.reset(new DynamicIDHandler(Sema));
    m_Sema->ExternalSource = m_DynIDHandler.get();
    
  }
  
  void DynamicExprTransformer::Initialize() {
    TemplateDecl* TD = dyn_cast<TemplateDecl>(m_Interpreter->LookupDecl("cling").LookupDecl("runtime").LookupDecl("internal").LookupDecl("EvaluateProxyT").getSingleDecl());
    assert(TD && "Cannot find EvaluateProxyT TemplateDecl!\n");
    
    m_EvalDecl = dyn_cast<FunctionDecl>(TD->getTemplatedDecl());    
    assert(m_EvalDecl && "Cannot find EvaluateProxyT!\n");

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
    // Handle the case: 
    // function-definition: 
    //   [decl-specifier-seq] declarator [ctor-initializer] function-body
    //TODO:[decl-specifier-seq] declarator [ctor-initializer] function-try-block
    //   function-body: compount-statement
    if (!D->isDependentContext() && D->isThisDeclarationADefinition()) {
      if (D->hasBody()) {
        // Here we expect clang::CompoundStmt
        EvalInfo EInfo = Visit(D->getBody());
        
        D->setBody(EInfo.Stmt());
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

  // Temp
  EvalInfo DynamicExprTransformer::VisitDeclStmt(DeclStmt *Node) {
    //Node is MyClass a;
    //Node MyClass b (a);
    // if (Node->isSingleDecl())
    //   if (VarDecl* VD = dyn_cast<VarDecl>(Node->getSingleDecl())) {
    //     if (VD->getNameAsString().compare("a") == 0)
    //       classA = VD;
    //     if (VD->getNameAsString().compare("b") == 0 && !VD->getType()->isLValueReferenceType()) {
    //       VD->setType(c.getLValueReferenceType(VD->getType()));
    //       Expr* Init = m_Sema->BuildDeclRefExpr(classA, classA->getType(), VK_LValue, SourceLocation()).take();
    //       VD->setInit(Init);

    //       //ASTOwningVector<Stmt*> Statements(*m_Sema);
          
    //       //IdentifierInfo& II = c.Idents.get("aaaaa");
          
    //       VarDecl* FakeVD = VarDecl::Create(c,
    //                                         //m_Sema->CurContext,
    //                                         VD->getDeclContext(),
    //                                         SourceLocation(),
    //                                         SourceLocation(),
    //                                         //&II,
    //                                         0,
    //                                         classA->getType(),
    //                                         /*TypeSourceInfo**/0,
    //                                         SC_None,
    //                                         SC_None);
    //       VD->getDeclContext()->addDecl(FakeVD);
    //       DeclStmt* DS = new (c) DeclStmt(DeclGroupRef(FakeVD), SourceLocation(), SourceLocation());

    //       EvalInfo EInfo(DS, 0);
    //       //EInfo.Stmts.push_back(Node);
    //       //Statements.push_back(DS);
    //       //Statements.push_back(Node);
    //       //Stmt* CS = m_Sema->ActOnCompoundStmt(SourceLocation(), SourceLocation(), move_arg(Statements), /*isStmtExpr*/ false).take();
    //       //EInfo.Stmts.push_back(CS);
    //       //return EvalInfo(DS, 0);
    //       return EInfo;
          
    //     }
    //   }
    // -------------------------------------------------------------------------
    // Visit all the children
    for (Stmt::child_iterator
           I = Node->child_begin(), E = Node->child_end(); I != E; ++I) {
      if (*I) {
        EvalInfo EInfo;
        //EvalInfo EInfo = Visit(*I);

        // In principle it is possible to have more than one declaration
        // e.g int a = 2, c = 4; can be int a = 2, b = 3, c = 4;
        // Allowing that needs many typechecking because we have to be sure that
        // the new added declarations have the same type
        // For now I don't forsee any reasonable use-case so I just disable it.
        //assert(!EInfo.isMultiStmt() && "Cannot have more than one stmt at that point");

        //if (EInfo.IsEvalNeeded) {
          // TODO: Make sure that we are in the case MyClass a(h->Draw());
          // 1-st: Add the LifetimeHandler
          // 2-nd: Transform the variable into reference.
        if (Expr* E = dyn_cast<Expr>(*I))
          if (IsArtificiallyDependent(E))
            if (VarDecl* VD = dyn_cast<VarDecl>(Node->getSingleDecl())) {
              if (!VD->getType()->isLValueReferenceType())
                // 1. Find the lifetime handler
                if (NamedDecl* LH = m_Interpreter->LookupDecl("cling").LookupDecl("runtime").LookupDecl("internal").LookupDecl("LifetimeHandler")) {
                  // 2. Create VarDecl pX
                  IdentifierInfo& II = m_Context.Idents.get("__aaaaa__");
                  VarDecl* FakeVD = VarDecl::Create(m_Context,
                                                    //m_Sema->CurContext,
                                                    VD->getDeclContext(),
                                                    SourceLocation(),
                                                    SourceLocation(),
                                                    &II,
                                                    //0,
                                                    VD->getType(),
                                                    /*TypeSourceInfo**/0,
                                                    SC_None,
                                                    SC_None);
                  VD->getDeclContext()->addDecl(FakeVD);
                  DeclStmt* DS = new (m_Context) DeclStmt(DeclGroupRef(FakeVD), SourceLocation(), SourceLocation());
                  EInfo.Stmts.push_back(DS);
                  LH->dump();
                }

              
              
              *I = SubstituteUnknownSymbol(VD->getType(), E);
              EInfo.Stmts.push_back(*I);
              return EInfo;
            }
        //} 
        //else {
        //  *I = EInfo.Stmt();
        //}
        //}
      }
    }
    return EvalInfo(Node, 0);
    // end Temp
  }
  
  EvalInfo DynamicExprTransformer::VisitStmt(Stmt *Node) {
    for (Stmt::child_iterator
           I = Node->child_begin(), E = Node->child_end(); I != E; ++I) {
      if (*I) {
        EvalInfo EInfo = Visit(*I);
        assert(!EInfo.isMultiStmt() && "Cannot have more than one stmt at that point");

        if (EInfo.IsEvalNeeded) {
          if (Expr *E = dyn_cast<Expr>(EInfo.Stmt()))
            // Assume void if still not escaped
            *I = SubstituteUnknownSymbol(m_Context.VoidTy, E);
        } 
        else {
          *I = EInfo.Stmt();
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
        assert(!EInfo.isMultiStmt() && "Cannot have more than one stmt at that point");
        if (EInfo.IsEvalNeeded) {
          if (Expr *E = dyn_cast<Expr>(EInfo.Stmt()))
            // Assume void if still not escaped
            *I = SubstituteUnknownSymbol(m_Context.VoidTy, E);
        } 
        else {
          *I = EInfo.Stmt();
        }
      }
    }
    return EvalInfo(Node, 0);
  }

  EvalInfo DynamicExprTransformer::VisitCompoundStmt(CompoundStmt *Node) {
    llvm::SmallVector<Stmt*, 32> Stmts;
    if (GetChildren(Stmts, Node)) {
      llvm::SmallVector<Stmt*, 32>::iterator it;
      for (it = Stmts.begin(); it != Stmts.end(); ++it) {
        EvalInfo EInfo = Visit(*it);
        if (EInfo.isMultiStmt()) {
          for (unsigned j = 0; j < EInfo.StmtCount(); ++j) {
            Stmts.insert(it + j, EInfo.Stmts[j]);
          }
          Node->setStmts(m_Context, Stmts.data(), Stmts.size());
          // Resolve all 1:n replacements
          Visit(Node);
        }
        else {
          if (EInfo.IsEvalNeeded) {
            if (Expr *E = dyn_cast<Expr>(EInfo.Stmt()))
              // Assume void if still not escaped
              *it = SubstituteUnknownSymbol(m_Context.VoidTy, E);
          }
          else {
            //assert(*it == EInfo.Stmt() && "Visitor shouldn't return something else!");
          }
        }
      }
    }

    Node->setStmts(m_Context, Stmts.data(), Stmts.size());

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
    const PrintingPolicy &Policy = m_Context.PrintingPolicy;
    
    StmtPrinterHelper *helper = new StmtPrinterHelper(Policy, m_Environment);      
    SubTree->printPretty(OS, helper, Policy);
    
    OS.flush();
  }
  
  // Prepare the actual arguments for the call
  // Arg list for static T Eval(size_t This, const char* expr, void* varaddr[], clang::DeclContext* DC )
  void DynamicExprTransformer::BuildEvalArgs(ASTOwningVector<Expr*> &Result) {
    // Arg 0:
    Expr *Arg0 = BuildEvalArg0();    
    Result.push_back(Arg0);
    
    // Arg 1:
    Expr *Arg1 = BuildEvalArg1();          
    Result.push_back(Arg1);
    
    // Arg 2:
    Expr *Arg2 = BuildEvalArg2();          
    Result.push_back(Arg2);
    
  }
    
  // Eval Arg0: const char* expr
  Expr *DynamicExprTransformer::BuildEvalArg0() {
    const QualType ConstChar = m_Context.getConstType(m_Context.CharTy);
    const QualType ConstCharArray = m_Context.getConstantArrayType(ConstChar, llvm::APInt(m_Context.getTypeSize(ConstChar), m_EvalExpressionBuf.length() + 1), ArrayType::Normal, /*IndexTypeQuals=*/ 0);
    Expr *Arg0 = StringLiteral::Create(m_Context, 
                                       &*m_EvalExpressionBuf.c_str(), 
                                       m_EvalExpressionBuf.length(), 
                                       /*Wide=*/ false,
                                       ConstCharArray, 
                                       SourceLocation());
    const QualType CastTo = m_Context.getPointerType(m_Context.getConstType(m_Context.CharTy));
    m_Sema->ImpCastExprToType(Arg0, CastTo, CK_ArrayToPointerDecay);
    
    return Arg0;
  }
  
  // Eval Arg1: void* varaddr[]
  Expr *DynamicExprTransformer::BuildEvalArg1() {
    QualType VarAddrTy = m_Sema->BuildArrayType(m_Context.VoidPtrTy, 
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
      m_Sema->ImpCastExprToType(UnOp, 
                                m_Context.getPointerType(m_Context.VoidPtrTy), 
                                CK_BitCast);
      Inits.push_back(UnOp);
    }
    
    // We need fake the SourceLocation just to avoid assert(InitList.isExplicit()....)
    SourceLocation SLoc = getEvalDecl()->getLocStart();
    SourceLocation ELoc = getEvalDecl()->getLocEnd();
    InitListExpr *ILE = m_Sema->ActOnInitList(SLoc, move_arg(Inits), ELoc).takeAs<InitListExpr>();
    Expr *Arg1 = m_Sema->BuildCompoundLiteralExpr(SourceLocation(), 
                                                  m_Context.CreateTypeSourceInfo(VarAddrTy), 
                                                  SourceLocation(),
                                                  ILE).take();
    m_Sema->ImpCastExprToType(Arg1,
                              m_Context.getPointerType(m_Context.VoidPtrTy),
                              CK_ArrayToPointerDecay);
    
    return Arg1;
  }
  
  // Eval Arg2: DeclContext* DC
  Expr *DynamicExprTransformer::BuildEvalArg2() {
    ParmVarDecl* param2 = getEvalDecl()->getParamDecl(2);
    QualType DeclContextTy = param2->getOriginalType();     
    
    const llvm::APInt DCAddr(8 * sizeof(void *), (uint64_t)m_CurDeclContext);
    
    Expr *Arg2 = IntegerLiteral::Create(m_Context,
                                        DCAddr, 
                                        m_Context.UnsignedLongTy, 
                                        SourceLocation());
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
    // TODO: Check all parent DeclContext whether they are dependent of not
    // Then we will be 100% sure that we are not visiting the wrong stmt.
    if (!Node->isValueDependent() || !Node->isTypeDependent())
      return false;     
    return true;
  }

  bool DynamicExprTransformer::GetChildren(llvm::SmallVector<Stmt*, 32> &Stmts, Stmt *Node) {
    if (std::distance(Node->child_begin(), Node->child_end()) < 1)
      return false;
    for (Stmt::child_iterator
           I = Node->child_begin(), E = Node->child_end(); I != E; ++I) {
      Stmts.push_back(*I);
    }
    return true;
  }

  // end Helpers
  
} // end namespace cling

