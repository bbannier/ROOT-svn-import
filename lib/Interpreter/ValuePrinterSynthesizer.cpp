//------------------------------------------------------------------------------
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Vassil Vassilev <vasil.georgiev.vasilev@cern.ch>
//------------------------------------------------------------------------------

#include "ValuePrinterSynthesizer.h"

#include "clang/AST/DeclGroup.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"
#include "clang/Sema/Lookup.h"
#include "clang/Sema/Scope.h"
#include "clang/Sema/Sema.h"
#include "clang/Sema/Template.h"

#include "cling/Interpreter/Interpreter.h"
#include "ASTUtils.h"
#include "ChainedConsumer.h"

using namespace clang;
namespace cling {


  ValuePrinterSynthesizer::ValuePrinterSynthesizer(Interpreter* Interp) 
    : m_Context(0), m_Sema(0), m_Interpreter(Interp), IsValuePrinterLoaded(false)
  {

  }

  ValuePrinterSynthesizer::~ValuePrinterSynthesizer() {

  }

  void ValuePrinterSynthesizer::Initialize(ASTContext& Ctx) {
    m_Context = &Ctx;
  }

  void ValuePrinterSynthesizer::InitializeSema(Sema& S) {
    m_Sema = &S;
  }

  void ValuePrinterSynthesizer::HandleTopLevelDecl(DeclGroupRef DGR) {
    for (DeclGroupRef::iterator I = DGR.begin(), E = DGR.end(); I != E; ++I)
      if (FunctionDecl* FD = dyn_cast<FunctionDecl>(*I)) {
        if (FD->getNameAsString().find("__cling_Un1Qu3"))
          return;

        if (CompoundStmt* CS = dyn_cast<CompoundStmt>(FD->getBody())) {
          for (CompoundStmt::body_iterator 
                 J = CS->body_begin(), E = CS->body_end(); J != E; ++J) {
            if (J+1 == E || !isa<NullStmt>(*(J+1))) {
              if (Expr* To = dyn_cast<Expr>(*J)) {
                LoadValuePrinter();
                ChainedConsumer* C = dyn_cast<ChainedConsumer>(&m_Sema->Consumer);
                bool p, q;
                p = C->DisableConsumer(ChainedConsumer::kDeclExtractor);
                q = C->DisableConsumer(ChainedConsumer::kValuePrinterSynthesizer);
                Expr* Result = SynthesizeVP(To);
                // Check if it is non-void
                if (Result)
                  *J = Result;
                C->RestorePreviousState(ChainedConsumer::kDeclExtractor, p);
                C->RestorePreviousState(ChainedConsumer::kValuePrinterSynthesizer, q);
              }
            }
          }
          // Clear the artificial NullStmt-s
          if (!ClearNullStmts(CS)) {
            // if no body remove the wrapper
            DeclContext* DC = FD->getDeclContext();
            Scope* S = m_Sema->getScopeForContext(DC);
            S->RemoveDecl(FD);
            DC->removeDecl(FD); 
          }
        }
      }
  }

  void ValuePrinterSynthesizer::ForgetSema() {
    m_Sema = 0;
  }

  // We need to artificially create:
  // cling::valuePrinterInternal::PrintValue(gCling->getValuePrinterStream(), 0, i);
  // So we need the following AST:
  // (CallExpr 0x2fdc4b8 'void'
  //   (ImplicitCastExpr 0x2fdc4a0 'void (*)(llvm::raw_ostream &, int, const int &)' <FunctionToPointerDecay>
  //     (DeclRefExpr 0x2fdc460 'void (llvm::raw_ostream &, int, const int &)' lvalue Function 0x2fd1b50 'PrintValue' 'void (llvm::raw_ostream &, int, const int &)' (FunctionTemplate 0x23b51c0 'PrintValue')))
  //   (CXXMemberCallExpr 0x2fdc388 'llvm::raw_ostream':'class llvm::raw_ostream' lvalue
  //     (MemberExpr 0x2fdc350 '<bound member function type>' ->getValuePrinterStream 0x235ae10
  //       (ImplicitCastExpr 0x2fdc3b0 'const class cling::Interpreter *' <NoOp>
  //         (ImplicitCastExpr 0x2fdc338 'class cling::Interpreter *' <LValueToRValue>
  //           (DeclRefExpr 0x2fdc310 'class cling::Interpreter *' lvalue Var 0x1aa89c0 'gCling' 'class cling::Interpreter *')))))
  //   (IntegerLiteral 0x2fdc3c8 'int' 0)
  //   (ImplicitCastExpr 0x2fdc4f8 'const int':'const int' lvalue <NoOp>
  //     (DeclRefExpr 0x2fdc3f0 'int' lvalue Var 0x2fd1420 'i' 'int')))

  Expr* ValuePrinterSynthesizer::SynthesizeVP(Expr* E) {
    QualType QT = E->getType();
    if (!QT.isNull() && QT->isVoidType())
      return 0;

    // 1. Call gCling->getValuePrinterStream()
    // 1.1. Find gCling
    SourceLocation NoSLoc = SourceLocation();
    Expr* TheInnerCall = Synthesize::CStyleCastPtrExpr(m_Sema, m_Context->VoidPtrTy, (uint64_t)&m_Interpreter->getValuePrinterStream());
    
    // 2. Build the final Find cling::valuePrinterInternal::PrintValue call
    // 2.1. Find cling::valuePrinterInternal::PrintValue
    TemplateDecl* TD = dyn_cast<TemplateDecl>(m_Interpreter->
                                              LookupDecl("cling").
                                              LookupDecl("valuePrinterInternal").
                                              LookupDecl("PrintValue").
                                              getSingleDecl());
    // 2.2. Instantiate the TemplateDecl
    FunctionDecl* TDecl = dyn_cast<FunctionDecl>(TD->getTemplatedDecl());
    
    assert(TDecl && "The PrintValue function not found!");
    
    // Set up new context for the new FunctionDecl
    DeclContext* PrevContext = m_Sema->CurContext;      
    m_Sema->CurContext = TDecl->getDeclContext();
    
    // Create template arguments
    Sema::InstantiatingTemplate Inst(*m_Sema, NoSLoc, TDecl);
    // Only the last argument is templated
    Scope* S = m_Sema->getScopeForContext(m_Sema->CurContext);

    // The VP expects non const pointer type. Make sure it gets it
    QualType ArgTy = E->getType().getCanonicalType();
    ArgTy.removeLocalConst();
    if (!ArgTy->isPointerType()) {
      E = m_Sema->BuildUnaryOp(S, NoSLoc, UO_AddrOf, E).take();
    }
    TemplateArgument Arg(E->getType().getCanonicalType());
    TemplateArgumentList TemplateArgs(TemplateArgumentList::OnStack, &Arg, 1U);
    
    // Substitute the declaration of the templated function, with the 
    // specified template argument
    Decl* D = m_Sema->SubstDecl(TDecl, 
                                TDecl->getDeclContext(), 
                                MultiLevelTemplateArgumentList(TemplateArgs));
    
    FunctionDecl* FD = dyn_cast<FunctionDecl>(D);
    // Creates new body of the substituted declaration
    m_Sema->InstantiateFunctionDefinition(FD->getLocation(), FD, true, true);
    
    m_Sema->CurContext = PrevContext;
    
    // 2.3. Build DeclRefExpr from the found decl
    const FunctionProtoType* FPT = FD->getType()->getAs<FunctionProtoType>();
    FunctionProtoType::ExtProtoInfo EPI = FPT->getExtProtoInfo();
    QualType FnTy = m_Context->getFunctionType(FD->getResultType(),
                                               FPT->arg_type_begin(),
                                               FPT->getNumArgs(),
                                               EPI);
    DeclRefExpr* DRE = m_Sema->BuildDeclRefExpr(FD,
                                                FnTy,
                                                VK_RValue,
                                                NoSLoc
                                                ).takeAs<DeclRefExpr>();
    
    // 2.4. Prepare the params
    
    // 2.4.1 Build the ValuePrinterInfo(Expr*, ASTContext*)
    // 2.4.1.1 Lookup the expr type
    CXXRecordDecl* ExprRD
      = dyn_cast<CXXRecordDecl>(m_Interpreter->LookupDecl("clang").
                          LookupDecl("Expr").getSingleDecl());
    assert(ExprRD && "Declaration of the expr not found!");
    QualType ExprRDTy = m_Context->getTypeDeclType(ExprRD);
    // 2.4.1.2 Lookup ASTContext type
    CXXRecordDecl* ASTContextRD
      = dyn_cast<CXXRecordDecl>(m_Interpreter->LookupDecl("clang").
                                LookupDecl("ASTContext").getSingleDecl());
    assert(ASTContextRD && "Declaration of the expr not found!");
    QualType ASTContextRDTy = m_Context->getTypeDeclType(ASTContextRD);

    Expr* ExprTy = Synthesize::CStyleCastPtrExpr(m_Sema, ExprRDTy, (uint64_t)E);
    Expr* ASTContextTy = Synthesize::CStyleCastPtrExpr(m_Sema,
                                                       ASTContextRDTy,
                                                       (uint64_t)m_Context);

    // 2.4.1.3 Lookup ValuePrinterInfo
    CXXRecordDecl* VPIRD 
      = dyn_cast<CXXRecordDecl>(m_Interpreter->LookupDecl("cling").
                                LookupDecl("ValuePrinterInfo").getSingleDecl());
    assert(VPIRD && "llvm::StringRef not found. Are you missing StringRef.h?");

    QualType VPIRDTy = m_Context->getTypeDeclType(VPIRD);
    TypeSourceInfo* VPITSI = m_Context->CreateTypeSourceInfo(VPIRDTy);
    ParsedType VPIPT = m_Sema->CreateParsedType(VPIRDTy, VPITSI);
    
    // Prepare VPI's arguments
    ASTOwningVector<Expr*> VPIArgs(*m_Sema);
    VPIArgs.push_back(ExprTy);
    VPIArgs.push_back(ASTContextTy);
    
    // create the temporary in the expr
    Expr* VPI = m_Sema->ActOnCXXTypeConstructExpr(VPIPT,
                                                  SourceLocation(),
                                                  move_arg(VPIArgs),
                                                  SourceLocation()
                                                  ).take();

    // const llvm::APInt Val(m_Context->getTypeSize(m_Context->IntTy), Flags);
    
    // Expr* FlagsIL = IntegerLiteral::Create(*m_Context, Val, m_Context->IntTy,
    //                                        NoSLoc);
    
    ASTOwningVector<Expr*> CallArgs(*m_Sema);
    CallArgs.push_back(TheInnerCall);
    //CallArgs.push_back(VPI);
    CallArgs.push_back(ExprTy);
    CallArgs.push_back(ASTContextTy);
    CallArgs.push_back(E);
    
    S = m_Sema->getScopeForContext(m_Sema->CurContext);
    Expr* Result = m_Sema->ActOnCallExpr(S, DRE, NoSLoc, 
                                         move_arg(CallArgs), NoSLoc).take();
    // make sure if there are temporaries to be cleared
    Result = m_Sema->MaybeCreateExprWithCleanups(Result);
    assert(Result && "Cannot create value printer!");
    return Result;    
  }

  void ValuePrinterSynthesizer::LoadValuePrinter() {
    if (!IsValuePrinterLoaded) {
      IsValuePrinterLoaded = true;
    }
  }

  unsigned ValuePrinterSynthesizer::ClearNullStmts(CompoundStmt* CS) {
    llvm::SmallVector<clang::Stmt*, 8> FBody;
    for (StmtRange range = CS->children(); range; ++range)
      if (!isa<NullStmt>(*range))
        FBody.push_back(*range);

    CS->setStmts(*m_Context, FBody.data(), FBody.size());
    return FBody.size();
  }

} // namespace cling
