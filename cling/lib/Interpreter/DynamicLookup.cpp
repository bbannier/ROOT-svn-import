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
    // FIXME: This breaks for example 
    // MyClass a (dep->Symbol(), h->Draw());
    // the second argument is for redeclaration.
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
  
  EvalInfo DynamicExprTransformer::VisitCompoundStmt(CompoundStmt *Node) {
    llvm::SmallVector<Stmt*, 32> Stmts;
    if (GetChildren(Stmts, Node)) {
      llvm::SmallVector<Stmt*, 32>::iterator it;
      for (it = Stmts.begin(); it != Stmts.end(); ++it) {
        EvalInfo EInfo = Visit(*it);
        if (EInfo.isMultiStmt()) {
          Stmts.insert(it, EInfo.Stmts.begin(), EInfo.Stmts.end());
          // Remove the last element, which is the one that is 
          // being replaced          
          Stmts.erase(it + EInfo.StmtCount());
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

  EvalInfo DynamicExprTransformer::VisitDeclStmt(DeclStmt *Node) {
    // Visit all the children, which are the contents of the DeclGroupRef
    for (Stmt::child_iterator
           I = Node->child_begin(), E = Node->child_end(); I != E; ++I) {
      if (*I) {
        // 1. Check whether this is the case of MyClass A(dep->symbol())
        // 2. Insert the RuntimeUniverse's LifetimeHandler instance
        // 3. Change the A's initializer to *(MyClass*)instance.getMemory()
        // 4. Make A reference (&A)
        // 5. Set the new initializer of A
        Expr* E = cast_or_null<Expr>(*I);
        if (!E || !IsArtificiallyDependent(E)) 
          continue;
        //FIXME: don't assume there is only one decl.
        assert(Node->isSingleDecl() && "There is more that one decl in stmt");
        VarDecl* CuredDecl = cast_or_null<VarDecl>(Node->getSingleDecl());
        QualType CuredDeclTy = CuredDecl->getType();
        if (!CuredDecl && !CuredDeclTy->isLValueReferenceType())
          continue;
        
        // 2.1 Find the LifetimeHandler type
        CXXRecordDecl* Handler 
          = cast_or_null<CXXRecordDecl>(m_Interpreter->LookupDecl("cling").
                                        LookupDecl("runtime").
                                        LookupDecl("internal").
                                        LookupDecl("LifetimeHandler").
                                        getSingleDecl());
        assert(Handler && "LifetimeHandler type not found!");
        if (Handler) {
          EvalInfo EInfo;
          // 2.2 Get unique name for the LifetimeHandler instance and initialize it
          IdentifierInfo& II = m_Context.Idents.get(m_Interpreter->createUniqueName());

          // Prepare the initialization Exprs.
          // We want to call LifetimeHandler(llvm::StringRef expr, 
          //                                 llvm::StringRef type)
          ASTOwningVector<Expr*> Inits(*m_Sema);
          // Add MyClass in LifetimeHandler unique("MyClass", "dep->Symbol()")
          Inits.push_back(ConstructllvmStringRefExpr(CuredDeclTy->
                                                     getAsCXXRecordDecl()->
                                                     getQualifiedNameAsString().
                                                     c_str()));
          // Strip dep->Symbol(arg1, ..., argN) in the m_EvalExpressionBuf
          BuildEvalEnvironment(E);
          Inits.push_back(ConstructllvmStringRefExpr(m_EvalExpressionBuf.c_str()));

          // 2.3 Create a variable from LifetimeHandler.
          QualType HandlerTy 
            = m_Context.getTypeDeclType(Handler).getNonReferenceType();
          VarDecl* HandlerInstance = VarDecl::Create(m_Context,
                                                     CuredDecl->getDeclContext(),
                                                     SourceLocation(),
                                                     SourceLocation(),
                                                     &II,
                                                     HandlerTy,
                                                     /*TypeSourceInfo**/0,
                                                     SC_None,
                                                     SC_None);
          
          // 2.4 Call the best-match constructor. The method does overload 
          // resolution of the constructors and then initializes the new
          // variable with it
          m_Sema->AddCXXDirectInitializerToDecl(HandlerInstance,
                                                SourceLocation(),
                                                move_arg(Inits),
                                                SourceLocation(),
                                                /*TypeMayContainAuto*/ false);

          // 2.5 Register the instance in the enclosing context
          CuredDecl->getDeclContext()->addDecl(HandlerInstance);
          EInfo.Stmts.push_back(new (m_Context) 
                                DeclStmt(DeclGroupRef(HandlerInstance),
                                         SourceLocation(),
                                         SourceLocation())
                                );
          
          // 3.1 Find the declaration - LifetimeHandler::getMemory()
          CXXMethodDecl* getMemDecl 
            = m_Interpreter->LookupDecl("getMemory", Handler).getAs<CXXMethodDecl>();
          assert(getMemDecl && "LifetimeHandler::getMemory not found!");
          // 3.2 Build a DeclRefExpr, which holds the object
          DeclRefExpr* MemberExprBase 
            = m_Sema->BuildDeclRefExpr(HandlerInstance,
                                       HandlerTy,
                                       VK_LValue,
                                       SourceLocation()
                                       ).takeAs<DeclRefExpr>();
          // 3.3 Create a MemberExpr to getMemory from its declaration.
          CXXScopeSpec SS;
          LookupResult MemberLookup(*m_Sema, getMemDecl->getDeclName(), 
                                    SourceLocation(),Sema::LookupMemberName);
          // Add the declaration as if doesn't exist.
          // TODO: Check whether this is the most appropriate variant
          MemberLookup.addDecl(getMemDecl, AS_public);
          MemberLookup.resolveKind();          
          Expr* MemberExpr = 
            m_Sema->BuildMemberReferenceExpr(MemberExprBase,
                                             HandlerTy,
                                             SourceLocation(),
                                             /*IsArrow=*/false,
                                             SS,
                                             /*FirstQualifierInScope=*/0,
                                             MemberLookup,
                                             /*TemplateArgs=*/0
                                             ).take();
          // 3.4 Build the actual call
          Expr* theCall = m_Sema->ActOnCallExpr(0,
                                                MemberExpr,
                                                SourceLocation(),
                                                MultiExprArg(),
                                                SourceLocation()).take();
          // Cast to the type LHS type
          Expr* Result = 
            m_Sema->BuildCStyleCastExpr(SourceLocation(),
                                        m_Context.CreateTypeSourceInfo(m_Context.getPointerType(CuredDeclTy)),
                                        SourceLocation(),
                                        theCall).take();
          // Cast once more (dereference the cstyle cast)
          Result = m_Sema->BuildUnaryOp(/*Scope*/0,
                                        SourceLocation(),
                                        UO_Deref,
                                        Result).take();
          // 4.
          CuredDecl->setType(m_Context.getLValueReferenceType(CuredDeclTy));
          // 5.
          CuredDecl->setInit(Result);

          EInfo.Stmts.push_back(Node);
          return EInfo;
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

  // Construct initializer (llvm::StringRef(Str))
  Expr* DynamicExprTransformer::ConstructllvmStringRefExpr(const char* Str) {
    // Try to find llvm::StringRef
    CXXRecordDecl* CXXRD = dyn_cast<CXXRecordDecl>(m_Interpreter->
                                                   LookupDecl("llvm").
                                                   LookupDecl("StringRef").getSingleDecl());
    // Exit if not "llvm/ADT/StringRef.h" included
    if (!CXXRD)
      return 0;

    // Create the const char* Expr, which will be passed to the constructor
    const QualType ConstChar = m_Context.getConstType(m_Context.CharTy);
    const QualType ConstCharArray = m_Context.getConstantArrayType(ConstChar, llvm::APInt(m_Context.getTypeSize(ConstChar), strlen(Str) + 1), ArrayType::Normal, /*IndexTypeQuals=*/ 0);
    Expr *Result = StringLiteral::Create(m_Context, 
                                         &*Str, 
                                         strlen(Str), 
                                         /*Wide=*/ false,
                                         ConstCharArray, 
                                         SourceLocation());
    const QualType CastTo = m_Context.getPointerType(m_Context.getConstType(m_Context.CharTy));
    // Cast const char lvalue to const char *
    m_Sema->ImpCastExprToType(Result, CastTo, CK_ArrayToPointerDecay);

    return Result;    
  }


  
  // Here is the test Eval function specialization. Here the CallExpr to the function
  // is created.
  CallExpr* DynamicExprTransformer::BuildEvalCallExpr(const QualType InstTy, Expr *SubTree, 
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

