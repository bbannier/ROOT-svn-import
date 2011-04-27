//------------------------------------------------------------------------------
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Vassil Vassilev <vasil.georgiev.vasilev@cern.ch>
//------------------------------------------------------------------------------

#include "DynamicLookup.h"
#include "cling/Interpreter/Interpreter.h"

#include "clang/Lex/Preprocessor.h"
#include "clang/Sema/Scope.h"
#include "clang/Sema/Lookup.h"
#include "clang/Sema/Template.h"

namespace clang {
  class LookupResult;
  class Scope;
}

using namespace clang;

namespace cling {

  // Constructor
  DynamicIDHandler::DynamicIDHandler(Sema* Sema)
    : m_Sema(Sema), m_Context(Sema->getASTContext())
  {}

  // pin the vtable to this file
  DynamicIDHandler::~DynamicIDHandler(){}

  bool DynamicIDHandler::LookupUnqualified(LookupResult& R, Scope* S) {

    if (!IsDynamicLookup(R, S))
      return false;

    DeclarationName Name = R.getLookupName();
    IdentifierInfo *II = Name.getAsIdentifierInfo();
    SourceLocation Loc = R.getNameLoc();
    FunctionDecl *D 
      = dyn_cast<FunctionDecl>(R.getSema().ImplicitlyDefineFunction(Loc, *II, S));
    if (D) {            
      FunctionProtoType::ExtProtoInfo EPI;
      QualType QTy = m_Context.getFunctionType(m_Context.DependentTy,
                                               /*ArgArray*/0,
                                               /*NumArgs*/0,
                                               EPI);
      
      D->setType(QTy);
      R.addDecl(D);
      // Say that we can handle the situation. Clang should try to recover
      return true;
    }
    // We cannot handle the situation. Give up
    return false;              
  }

  bool DynamicIDHandler::IsDynamicLookup (LookupResult& R, Scope* S) {
    if (R.getLookupKind() != Sema::LookupOrdinaryName) return false;
    if (R.isForRedeclaration()) return false;
    // FIXME: Figure out better way to handle:
    // C++ [basic.lookup.classref]p1:
    //   In a class member access expression (5.2.5), if the . or -> token is
    //   immediately followed by an identifier followed by a <, the
    //   identifier must be looked up to determine whether the < is the
    //   beginning of a template argument list (14.2) or a less-than operator.
    //   The identifier is first looked up in the class of the object
    //   expression. If the identifier is not found, it is then looked up in
    //   the context of the entire postfix-expression and shall name a class
    //   or function template.
    // 
    // We want to ignore object(.|->)member<template>
    if (m_Sema->PP.LookAhead(0).getKind() == tok::less)
      // TODO: check for . or -> in the cached token stream
      return false;

    for (Scope* DepScope = S; DepScope; DepScope = DepScope->getParent()) {
      if (DeclContext* Ctx = static_cast<DeclContext*>(DepScope->getEntity())) {
        return !Ctx->isDependentContext();
      }
    }
    
    return true;
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
  }
  
  void DynamicExprTransformer::Initialize() {
    TemplateDecl* D = dyn_cast<TemplateDecl>(m_Interpreter->LookupDecl("cling").
                                             LookupDecl("runtime").
                                             LookupDecl("internal").
                                             LookupDecl("EvaluateProxyT").
                                             getSingleDecl());
    assert(D && "Cannot find EvaluateProxyT TemplateDecl!\n");
    
    m_EvalDecl = dyn_cast<FunctionDecl>(D->getTemplatedDecl());    

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
        m_CurDeclContext = D->getParent();
        // Here we expect clang::CompoundStmt
        ASTNodeInfo NewNode = Visit(D->getBody());
        
        D->setBody(NewNode.getAsSingleNode());
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

  ASTNodeInfo DynamicExprTransformer::VisitStmt(Stmt *Node) {
    for (Stmt::child_iterator
           I = Node->child_begin(), E = Node->child_end(); I != E; ++I) {
      if (*I) {
        ASTNodeInfo NewNode = Visit(*I);
        assert(NewNode.hasSingleNode() && 
               "Cannot have more than one stmt at that point");

        if (NewNode.isForReplacement()) {
          if (Expr *E = NewNode.getAs<Expr>())
            // Assume void if still not escaped
            *I = SubstituteUnknownSymbol(m_Context.VoidTy, E);
        } 
        else {
          *I = NewNode.getAsSingleNode();
        }
      }
    }
    
    return ASTNodeInfo(Node, 0);
  }
  
  ASTNodeInfo DynamicExprTransformer::VisitCompoundStmt(CompoundStmt *Node) {
    ASTNodes Children;
    if (GetChildren(Children, Node)) {
      ASTNodes::iterator it;
      for (it = Children.begin(); it != Children.end(); ++it) {
        ASTNodeInfo NewNode = Visit(*it);
        if (!NewNode.hasSingleNode()) {
          Children.insert(it, NewNode.getNodes().begin(), NewNode.getNodes().end());
          // Remove the last element, which is the one that is 
          // being replaced          
          Children.erase(it + NewNode.getNodes().size());
          Node->setStmts(m_Context, Children.data(), Children.size());
          // Resolve all 1:n replacements
          Visit(Node);
        }
        else {
          if (NewNode.isForReplacement()) {
            if (Expr *E = NewNode.getAs<Expr>())
              // Assume void if still not escaped
              *it = SubstituteUnknownSymbol(m_Context.VoidTy, E);
          }
          else {
            //assert(*it == NewNode.Stmt() && "Visitor shouldn't return something else!");
          }
        }
      }
    }

    Node->setStmts(m_Context, Children.data(), Children.size());

    return ASTNodeInfo(Node, 0);

  }

  ASTNodeInfo DynamicExprTransformer::VisitDeclStmt(DeclStmt *Node) {
    // Visit all the children, which are the contents of the DeclGroupRef
    for (Stmt::child_iterator
           I = Node->child_begin(), E = Node->child_end(); I != E; ++I) {
      // TODO: figure out how to filter const char * a = dep->symbol()
      // ASTNodeInfo NewNode = Visit(*I);
      // assert (!einfo.isMultiStmt() && "Not implemented yet!");
      // *I = einfo.Stmt();
      if (*I) {
        Expr* E = cast_or_null<Expr>(*I);
        if (!E || !IsArtificiallyDependent(E)) 
          continue;
        //FIXME: don't assume there is only one decl.
        assert(Node->isSingleDecl() && "There is more that one decl in stmt");
        VarDecl* CuredDecl = cast_or_null<VarDecl>(Node->getSingleDecl());
        assert(CuredDecl && "Not a variable declaration!");
        QualType CuredDeclTy = CuredDecl->getType();
        // check if the case is sometype * somevar = init;
        if (CuredDecl->hasInit() && CuredDeclTy->isAnyPointerType()) {
          *I = SubstituteUnknownSymbol(CuredDeclTy, CuredDecl->getInit());
          continue;
        }

        // 1. Check whether this is the case of MyClass A(dep->symbol())
        // 2. Insert the RuntimeUniverse's LifetimeHandler instance
        // 3. Change the A's initializer to *(MyClass*)instance.getMemory()
        // 4. Make A reference (&A)
        // 5. Set the new initializer of A
        if (CuredDeclTy->isLValueReferenceType())
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
          ASTNodeInfo NewNode;
          // 2.2 Get unique name for the LifetimeHandler instance and 
          // initialize it
          IdentifierInfo& II 
            = m_Context.Idents.get(m_Interpreter->createUniqueName());

          // Prepare the initialization Exprs.
          // We want to call LifetimeHandler(llvm::StringRef expr, 
          //                                 void* varaddr[],
          //                                 DeclContext DC,
          //                                 llvm::StringRef type)
          ASTOwningVector<Expr*> Inits(*m_Sema);
          // Add MyClass in LifetimeHandler unique("dep->Symbol(i)",
          //                                       {&i},
          //                                       DC,
          //                                       "MyClass")
          // Build Arg0 llvm::StringRef
          BuildEvalEnvironment(E);
          Inits.push_back(ConstructllvmStringRefExpr(m_EvalExpressionBuf.c_str()));
          // Build Arg1 void* varaddr[]
          Inits.push_back(BuildEvalArg1());
          // Build Arg2 clang::DeclContext* DC
          Inits.push_back(BuildEvalArg2());
          // Build Arg3 llvm::StringRef
          Inits.push_back(ConstructllvmStringRefExpr(CuredDeclTy.getAsString().
                                                     c_str()));

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
          NewNode.addNode(new (m_Context) DeclStmt(DeclGroupRef(HandlerInstance),
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
                                    SourceLocation(), Sema::LookupMemberName);
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
          Scope* S = m_Sema->getScopeForContext(m_Sema->CurContext);
          Expr* theCall = m_Sema->ActOnCallExpr(S,
                                                MemberExpr,
                                                SourceLocation(),
                                                MultiExprArg(),
                                                SourceLocation()).take();
          // Cast to the type LHS type
          Expr* Result 
            = m_Sema->BuildCStyleCastExpr(SourceLocation(),
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

          NewNode.addNode(Node);
          return NewNode;
        }
      }
    }
    return ASTNodeInfo(Node, 0);
  }

  ASTNodeInfo DynamicExprTransformer::VisitExpr(Expr* Node) {
    for (Stmt::child_iterator
           I = Node->child_begin(), E = Node->child_end(); I != E; ++I) {
      if (*I) {
        ASTNodeInfo NewNode = Visit(*I);
        assert(NewNode.hasSingleNode() && 
               "Cannot have more than one stmt at that point");
        if (NewNode.isForReplacement()) {
          if (Expr *E = NewNode.getAs<Expr>())
            // Assume void if still not escaped
            *I = SubstituteUnknownSymbol(m_Context.VoidTy, E);
        } 
        else {
          *I = NewNode.getAsSingleNode();
        }
      }
    }
    return ASTNodeInfo(Node, 0);
  }

  ASTNodeInfo DynamicExprTransformer::VisitBinaryOperator(BinaryOperator* Node) {
    ASTNodeInfo rhs = Visit(Node->getRHS());
    ASTNodeInfo lhs = Visit(Node->getLHS());
    assert((lhs.hasSingleNode() || rhs.hasSingleNode()) && 
           "1:N replacements are not implemented yet!");

    // Try find out the type of the left-hand-side of the operator
    // and give the hint to the right-hand-side in order to replace the 
    // dependent symbol
    if (Node->isAssignmentOp() && 
        rhs.isForReplacement() && 
        !lhs.isForReplacement()) {
      if (Expr* LHSExpr = lhs.getAs<Expr>())
        if (!IsArtificiallyDependent(LHSExpr)) {
          const QualType LHSTy = LHSExpr->getType();
          Node->setRHS(SubstituteUnknownSymbol(LHSTy, rhs.castTo<Expr>()));
          Node->setTypeDependent(false);
          Node->setValueDependent(false);
          return ASTNodeInfo(Node, /*needs eval*/false);
        }
    }
    
    return ASTNodeInfo(Node, IsArtificiallyDependent(Node));    
  }

  ASTNodeInfo DynamicExprTransformer::VisitCallExpr(CallExpr* E) {
    // FIXME: Maybe we need to handle the arguments
    // ASTNodeInfo NewNode = Visit(E->getCallee());
    return ASTNodeInfo (E, IsArtificiallyDependent(E));
  }
  
  ASTNodeInfo DynamicExprTransformer::VisitDeclRefExpr(DeclRefExpr* DRE) {
    return ASTNodeInfo(DRE, IsArtificiallyDependent(DRE));
  }
  
  ASTNodeInfo DynamicExprTransformer::VisitDependentScopeDeclRefExpr(DependentScopeDeclRefExpr* Node) {
    return ASTNodeInfo(Node, IsArtificiallyDependent(Node));
  }
  
  // end StmtVisitor
  
  // EvalBuilder
  
  Expr *DynamicExprTransformer::SubstituteUnknownSymbol(const QualType InstTy, Expr* SubTree) {
    assert(SubTree && "No subtree specified!");
    // Get the addresses
    BuildEvalEnvironment(SubTree);
    
    //Build the arguments for the call
    ASTOwningVector<Expr*> CallArgs(*m_Sema);
    BuildEvalArgs(CallArgs);
    
    // Build the call
    CallExpr* EvalCall = BuildEvalCallExpr(InstTy, SubTree, CallArgs);
    
    // Add substitution mapping
    getSubstSymbolMap()[EvalCall] = SubTree;
    
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
  // Arg list for 
  // T EvaluateProxyT(const char* expr, 
  //                  void* varaddr[],
  //                  clang::DeclContext* DC)
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
                                       /*Pascal=*/false,
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
    ParmVarDecl* p2 = getEvalDecl()->getParamDecl(2);
    TypeSourceInfo* TSI = m_Context.CreateTypeSourceInfo(p2->getOriginalType());
    const llvm::APInt DCAddr(8 * sizeof(void *), (uint64_t)m_CurDeclContext);
    
    Expr *Arg2 = IntegerLiteral::Create(m_Context,
                                        DCAddr, 
                                        m_Context.UnsignedLongTy, 
                                        SourceLocation());
    // TODO: Maybe is better to have a variable and nice cast
    // It will be better for debugging we will have DC from clang::DeclContext
    // type instead of (clang::DeclContext*) long int (the pointer)
    Arg2 = m_Sema->BuildCStyleCastExpr(SourceLocation(),
                                       TSI,
                                       SourceLocation(),
                                       Arg2).take();
    //m_Sema->ImpCastExprToType(Arg2, DeclContextTy, CK_IntegralToPointer);
    
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
                                         /*Pascal=*/false,
                                         ConstCharArray, 
                                         SourceLocation());
    const QualType CastTo = m_Context.getPointerType(m_Context.getConstType(m_Context.CharTy));
    // Cast const char lvalue to const char *
    m_Sema->ImpCastExprToType(Result, CastTo, CK_ArrayToPointerDecay);

    return Result;    
  }


  
  // Here is the test Eval function specialization. Here the CallExpr to the function
  // is created.
  CallExpr* 
  DynamicExprTransformer::BuildEvalCallExpr(const QualType InstTy,
                                            Expr *SubTree, 
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
    Decl *D = m_Sema->SubstDecl(FDecl, 
                                FDecl->getDeclContext(), 
                                MultiLevelTemplateArgumentList(TemplateArgs));
    
    FunctionDecl *Fn = dyn_cast<FunctionDecl>(D);
    // Creates new body of the substituted declaration
    m_Sema->InstantiateFunctionDefinition(Fn->getLocation(), Fn, true, true);
    
    m_Sema->CurContext = PrevContext;
    
    const FunctionProtoType *FPT = Fn->getType()->getAs<FunctionProtoType>();
    FunctionProtoType::ExtProtoInfo EPI = FPT->getExtProtoInfo();
    QualType FnTy = m_Context.getFunctionType(Fn->getResultType(),
                                               FPT->arg_type_begin(),
                                               FPT->getNumArgs(),
                                               EPI);
    DeclRefExpr *DRE = m_Sema->BuildDeclRefExpr(Fn,
                                                FnTy,
                                                VK_RValue,
                                                SourceLocation()
                                                ).takeAs<DeclRefExpr>();
    
    // TODO: Figure out a way to avoid passing in wrong source locations
    // of the symbol being replaced. This is important when we calculate the
    // size of the memory buffers and may lead to creation of wrong wrappers. 
    Scope* S = m_Sema->getScopeForContext(m_Sema->CurContext);
    CallExpr *EvalCall = m_Sema->ActOnCallExpr(S,
                                               DRE,
                                               SubTree->getLocStart(),
                                               move_arg(CallArgs),
                                               SubTree->getLocEnd()
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
    DeclContext* DC = m_CurDeclContext;
    while (DC) {
      if (DC->isDependentContext())
        return false;
      DC = DC->getParent();
    }
    return true;
  }

  bool DynamicExprTransformer::GetChildren(ASTNodes& Children, Stmt *Node) {
    if (std::distance(Node->child_begin(), Node->child_end()) < 1)
      return false;
    for (Stmt::child_iterator
           I = Node->child_begin(), E = Node->child_end(); I != E; ++I) {
      Children.push_back(*I);
    }
    return true;
  }

  void DynamicExprTransformer::AttachDynIDHandler() {
    if (!m_DynIDHandler.get()) {
      m_DynIDHandler.reset(new DynamicIDHandler(m_Sema));
    }

    m_Sema->ExternalSource = m_DynIDHandler.get();
  }

  void DynamicExprTransformer::DetachDynIDHandler() {
    m_Sema->ExternalSource = 0;
  }

  // end Helpers
  
} // end namespace cling

