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
      if (Decl* FD = dyn_cast<FunctionDecl>(*I))
        if (CompoundStmt* CS = dyn_cast<CompoundStmt>(FD->getBody())) {
          for (CompoundStmt::body_iterator J = CS->body_begin();
               J != CS->body_end(); ++J) {
            if ((CS->body_end() - CS->body_begin() == 1) || 
                (((J+1) != CS->body_end()) && 
                 !isa<NullStmt>(*(J + 1)))) {
              if (Expr* To = dyn_cast<Expr>(*J)) {
                  LoadValuePrinter();
                  ChainedConsumer* C
                    = dyn_cast<ChainedConsumer>(&m_Sema->Consumer);
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
    // 1. Get the flags
    QualType QT = E->getType();
    if (!QT.isNull() && QT->isVoidType()) {
      return 0;
    } else {
      int Flags = 0;
      enum DumperFlags {
        kIsPtr = 1,
        kIsConst = 2,
        kIsPolymorphic = 4
      };
      
      if (E->isRValue()) Flags |= kIsConst;
      if (QT.isConstant(*m_Context) || QT.isLocalConstQualified()) {
        Flags |= kIsConst;
      }
      if (QT->isPointerType()) {
        // treat arrary-to-pointer decay as array:
        QualType PQT = QT->getPointeeType();
        const Type* PTT = PQT.getTypePtr();
        if (!PTT || !PTT->isArrayType()) {
          Flags |= kIsPtr;
          const RecordType* RT = dyn_cast<RecordType>(QT.getTypePtr());
          if (RT) {
            RecordDecl* RD = RT->getDecl();
            if (RD) {
              CXXRecordDecl* CRD = dyn_cast<CXXRecordDecl>(RD);
              if (CRD && CRD->isPolymorphic()) {
                Flags |= kIsPolymorphic;
              }
            }
          }
        }
      }
      // 2. Call gCling->getValuePrinterStream()
      // 2.1. Find gCling
      SourceLocation NoSLoc = SourceLocation();
      VarDecl* VD = dyn_cast<VarDecl>(m_Interpreter->
                                      LookupDecl("cling").
                                      LookupDecl("runtime").
                                      LookupDecl("gCling").getSingleDecl());
      assert(VD && "gCling not found!");
      CXXRecordDecl* RD = dyn_cast<CXXRecordDecl>(m_Interpreter->
                                                  LookupDecl("cling").
                                                  LookupDecl("Interpreter").
                                                  getSingleDecl()
                                                  );
      QualType RDTy = m_Context->getPointerType(m_Context->getTypeDeclType(RD));
      // 2.2 Find getValuePrinterStream()
      CXXMethodDecl* getValPrinterDecl = m_Interpreter->
        LookupDecl("getValuePrinterStream", RD).getAs<CXXMethodDecl>();
      assert(getValPrinterDecl && "Decl not found!");
      
      // 2.3 Build a DeclRefExpr, which holds the object
      DeclRefExpr* MemberExprBase = m_Sema->BuildDeclRefExpr(VD, RDTy, 
                                                             VK_LValue, NoSLoc
                                                        ).takeAs<DeclRefExpr>();
      // 2.4 Create a MemberExpr to getMemory from its declaration.
      CXXScopeSpec SS;
      LookupResult MemberLookup(*m_Sema, getValPrinterDecl->getDeclName(), 
                                NoSLoc, Sema::LookupMemberName);
      // Add the declaration as if doesn't exist. Skips the Lookup, because
      // we have the declaration already so just add it in
      MemberLookup.addDecl(getValPrinterDecl, AS_public);
      MemberLookup.resolveKind();
      Expr* MemberExpr = m_Sema->BuildMemberReferenceExpr(MemberExprBase,
                                                          RDTy,
                                                          NoSLoc,
                                                          /*IsArrow=*/true,
                                                          SS,
                                                    /*FirstQualifierInScope=*/0,
                                                          MemberLookup,
                                                          /*TemplateArgs=*/0
                                                          ).take();
      // 2.5 Build the gCling->getValuePrinterStream()
      Scope* S = m_Sema->getScopeForContext(m_Sema->CurContext);
      Expr* TheInnerCall = m_Sema->ActOnCallExpr(S, MemberExpr, NoSLoc,
                                                 MultiExprArg(), NoSLoc).take();

      // 3. Build the final Find cling::valuePrinterInternal::PrintValue call
      // 3.1. Find cling::valuePrinterInternal::PrintValue
      TemplateDecl* TD = dyn_cast<TemplateDecl>(m_Interpreter->
                                                LookupDecl("cling").
                                             LookupDecl("valuePrinterInternal").
                                                LookupDecl("PrintValue").
                                                getSingleDecl());
      // 3.2. Instantiate the TemplateDecl
      FunctionDecl* TDecl = dyn_cast<FunctionDecl>(TD->getTemplatedDecl());
      
      assert(TDecl && "The PrintValue function not found!");

      // Set up new context for the new FunctionDecl
      DeclContext* PrevContext = m_Sema->CurContext;      
      m_Sema->CurContext = TDecl->getDeclContext();
      
      // Create template arguments
      Sema::InstantiatingTemplate Inst(*m_Sema, NoSLoc, TDecl);
      // Only the last argument is templated
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

      // 3.3. Build DeclRefExpr from the found decl
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
      
      // 3.4. Prepare the params

      // 3.4.1. Create IntegerLiteral, holding the flags
      const llvm::APInt Val(m_Context->getTypeSize(m_Context->IntTy), Flags);
      
      Expr* FlagsIL = IntegerLiteral::Create(*m_Context, Val, m_Context->IntTy,
                                             NoSLoc);
      
      ASTOwningVector<Expr*> CallArgs(*m_Sema);
      CallArgs.push_back(TheInnerCall);
      CallArgs.push_back(FlagsIL);
      CallArgs.push_back(E);

      S = m_Sema->getScopeForContext(m_Sema->CurContext);
      Expr* Result = m_Sema->ActOnCallExpr(S, DRE, NoSLoc, 
                                           move_arg(CallArgs), NoSLoc).take();
      assert(Result && "Cannot create value printer!");
      return Result;
    }

  }
  void ValuePrinterSynthesizer::LoadValuePrinter() {
    if (!IsValuePrinterLoaded) {
      m_Interpreter->processLine("#include \"cling/Interpreter/Interpreter.h\"");
      m_Interpreter->processLine("#include \"cling/Interpreter/ValuePrinter.h\"");
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
