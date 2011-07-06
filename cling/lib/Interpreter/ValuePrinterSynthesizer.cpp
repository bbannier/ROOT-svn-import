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
    : m_Context(0), m_Sema(0), m_Interpreter(Interp)
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
  // cling::valuePrinterInternal::PrintValue((void*) raw_ostream, 
  //                                         (ASTContext)Ctx, (Expr*)E, &i);
  Expr* ValuePrinterSynthesizer::SynthesizeVP(Expr* E) {
    QualType QT = E->getType();
    if (!QT.isNull() && QT->isVoidType())
      return 0;

    // 1. Call gCling->getValuePrinterStream()
    // 1.1. Find gCling
    SourceLocation NoSLoc = SourceLocation();
    DeclarationName Name = &m_Context->Idents.get("PrintValue");
    LookupResult R(*m_Sema, Name, NoSLoc, Sema::LookupOrdinaryName,
                   Sema::ForRedeclaration);
    DeclContext* DC 
      = cast_or_null<DeclContext>(m_Interpreter->
                                  LookupDecl("cling").
                                  LookupDecl("valuePrinterInternal").
                                  getSingleDecl()
                              );
    CXXScopeSpec CSS;
    bool isMemberOfUnknownSpecialization;
    Scope* scope = m_Sema->getScopeForContext(DC);
    m_Sema->LookupTemplateName(R, scope, CSS, QualType(), 
                               /*EnteringContext*/true, 
                               isMemberOfUnknownSpecialization);

    m_Sema->LookupQualifiedName(R, DC);
    Expr* UnresolvedLookup 
      = m_Sema->BuildDeclarationNameExpr(CSS, R, /*adl*/ false).take();
    
    // 2.4. Prepare the params
    
    // 2.4.1 Lookup the llvm::raw_ostream
    CXXRecordDecl* RawOStreamRD
      = dyn_cast<CXXRecordDecl>(m_Interpreter->LookupDecl("llvm").
                          LookupDecl("raw_ostream").getSingleDecl());
    assert(RawOStreamRD && "Declaration of the expr not found!");
    QualType RawOStreamRDTy = m_Context->getTypeDeclType(RawOStreamRD);
    // 2.4.2 Lookup the expr type
    CXXRecordDecl* ExprRD
      = dyn_cast<CXXRecordDecl>(m_Interpreter->LookupDecl("clang").
                          LookupDecl("Expr").getSingleDecl());
    assert(ExprRD && "Declaration of the expr not found!");
    QualType ExprRDTy = m_Context->getTypeDeclType(ExprRD);
    // 2.4.3 Lookup ASTContext type
    CXXRecordDecl* ASTContextRD
      = dyn_cast<CXXRecordDecl>(m_Interpreter->LookupDecl("clang").
                                LookupDecl("ASTContext").getSingleDecl());
    assert(ASTContextRD && "Declaration of the expr not found!");
    QualType ASTContextRDTy = m_Context->getTypeDeclType(ASTContextRD);

    Expr* RawOStreamTy
      = Synthesize::CStyleCastPtrExpr(m_Sema, RawOStreamRDTy,
                               (uint64_t)&m_Interpreter->getValuePrinterStream()
                                      );

    Expr* ExprTy = Synthesize::CStyleCastPtrExpr(m_Sema, ExprRDTy, (uint64_t)E);
    Expr* ASTContextTy = Synthesize::CStyleCastPtrExpr(m_Sema,
                                                       ASTContextRDTy,
                                                       (uint64_t)m_Context);

    ASTOwningVector<Expr*> CallArgs(*m_Sema);
    CallArgs.push_back(RawOStreamTy);
    CallArgs.push_back(ExprTy);
    CallArgs.push_back(ASTContextTy);
    CallArgs.push_back(E);
    
    Scope* S = m_Sema->getScopeForContext(m_Sema->CurContext);
    Expr* Result = m_Sema->ActOnCallExpr(S, UnresolvedLookup, NoSLoc, 
                                         move_arg(CallArgs), NoSLoc).take();
    assert(Result && "Cannot create value printer!");
    return Result;
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
