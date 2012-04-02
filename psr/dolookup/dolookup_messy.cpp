// vim: ts=2 sw=2 foldmethod=indent
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclGroup.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclBase.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclFriend.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/ExprCXX.h"
#include "clang/AST/Expr.h"
#include "clang/AST/PrettyPrinter.h"
#include "clang/AST/Stmt.h"
#include "clang/AST/Type.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/IdentifierTable.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/Specifiers.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/TokenKinds.h"
#include "clang/Basic/Version.h"
#include "clang/CodeGen/ModuleBuilder.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/TextDiagnosticBuffer.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Lex/HeaderSearch.h"
#include "clang/Lex/MacroInfo.h"
#include "clang/Lex/PPCallbacks.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Parse/ParseAST.h"
#define private public
#include "clang/Parse/Parser.h"
#undef public
#include "clang/Sema/DeclSpec.h"
#include "clang/Sema/Sema.h"
#include "clang/Sema/Lookup.h"
#include "clang/Sema/Ownership.h"
#include "clang/Sema/Overload.h"
#include "clang/Sema/Scope.h"
#include "llvm/ADT/OwningPtr.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/Assembly/PrintModulePass.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/GlobalVariable.h"
#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
#include "llvm/PassManager.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/DynamicLibrary.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetSelect.h"

#include <cstdio>
#include <iostream>
#include <sstream>
#include <stack>
#include <string>
#include <utility>
#include <vector>

namespace clang {
clang::ImplicitConversionSequence
TryCopyInitialization(clang::Sema &S, clang::Expr *From, clang::QualType ToType,
                      bool SuppressUserConversions,
                      bool InOverloadResolution,
                      bool AllowObjCWritebackConversion,
                      bool AllowExplicit = false);
}

// Function Table
void init(const char* llvmdir);
void cleanup();
clang::CompilerInstance* createCI(const char* llvmdir);
clang::CompilerInstance* getCI();

// Local Variables
static const char* llvm_install_dir = "/local2/russo/llvm";
//static const char* fake_argv[] = { "clang", "-x", "c++", "-fexceptions", "-D__CLING__", "-isysroot", "/local2/russo/root_debug/include", 0 };
static const char* fake_argv[] = { "clang", "-x", "c++", "-fexceptions", "-D__CLING__", 0 };
static const int fake_argc = (sizeof(fake_argv) / sizeof(const char*)) - 1;

static llvm::LLVMContext* m_llvm_context = 0; // We own, our types.
static clang::CompilerInstance* m_CI = 0; // We own, our compiler instance.
//static llvm::Module* m_prev_module = 0; // We do *not* own, m_engine owns it.

//
// Functions
//

void init(const char* llvmdir)
{
  //
  //  Initialize the llvm library.
  //
  llvm::InitializeAllTargets();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmPrinters();
  llvm::InitializeAllAsmParsers();
  //
  //  Create a llvm context to use.
  //
  //m_llvm_context = &llvm::getGlobalContext();
  m_llvm_context = new llvm::LLVMContext;
  m_CI = createCI(llvmdir);
  //m_prev_module = new llvm::Module("_Clang_first", *m_llvm_context);
}

void cleanup()
{
  //delete m_prev_module;
  //m_prev_module = 0; // Don't do this, the engine does it.
  //m_CI->takeLLVMContext(); // Don't take down the context with the CI.
  delete m_CI;
  m_CI = 0;
  delete m_llvm_context;
  m_llvm_context = 0;
  // Shutdown the llvm library.
  llvm::llvm_shutdown();
}

clang::CompilerInstance*
createCI(const char* llvmdir)
{
  //
  //  Create and setup a compiler instance.
  //
  clang::CompilerInstance* CI = new clang::CompilerInstance();
  llvm::IntrusiveRefCntPtr<clang::DiagnosticIDs> DiagID(
    new clang::DiagnosticIDs());
  //bool first_time = true;
  //CI->setLLVMContext(m_llvm_context);
  {
    //
    //  Buffer the error messages while we process
    //  the compiler options.
    //
    clang::TextDiagnosticBuffer* DiagsBuffer = new clang::TextDiagnosticBuffer();
    // Diags takes ownership of DiagsBuffer
    clang::DiagnosticsEngine Diags(DiagID, DiagsBuffer);
    clang::CompilerInvocation::CreateFromArgs(CI->getInvocation(),
        fake_argv + 1, fake_argv + fake_argc, Diags);
    if (
      CI->getHeaderSearchOpts().UseBuiltinIncludes &&
      CI->getHeaderSearchOpts().ResourceDir.empty()
    ) {
      if (llvmdir) {
        llvm::sys::Path P(llvmdir);
        P.appendComponent("lib");
        P.appendComponent("clang");
        P.appendComponent(CLANG_VERSION_STRING);
        CI->getHeaderSearchOpts().ResourceDir = P.str();
      }
    }
    CI->createDiagnostics(fake_argc - 1, const_cast<char**>(fake_argv + 1));
    if (!CI->hasDiagnostics()) {
      //CI->takeLLVMContext();
      delete CI;
      CI = 0;
      return 0;
    }
    // Output the buffered error messages now.
    DiagsBuffer->FlushDiagnostics(CI->getDiagnostics());
    if (CI->getDiagnostics().getClient()->getNumErrors()) {
      //CI->takeLLVMContext();
      delete CI;
      CI = 0;
      return 0;
    }
  }
  CI->setTarget(clang::TargetInfo::CreateTargetInfo(CI->getDiagnostics(),
                CI->getTargetOpts()));
  if (!CI->hasTarget()) {
    //CI->takeLLVMContext();
    delete CI;
    CI = 0;
    return 0;
  }
  CI->getTarget().setForcedLangOptions(CI->getLangOpts());
  if (!CI->hasFileManager()) {
    CI->createFileManager();
  }
  if (!CI->hasSourceManager()) {
    CI->createSourceManager(CI->getFileManager());
  }
  //else {
  //  CI->getSourceManager().clearIDTables();
  //}
  //CI->createPreprocessor(); // Note: This single line takes almost all the time!
  return CI;
}

clang::CompilerInstance*
getCI()
{
  if (!m_CI) {
    return 0;
  }
  //m_CI->getSourceManager().clearIDTables();
  return m_CI;
}

int main(int argc, char** argv)
{
  init(llvm_install_dir);
  clang::CompilerInstance* CI = 0;
  CI = getCI();
  if (!CI) {
    return 0;
  }
  clang::Parser* P = 0;
  // Begin per-file stuff.
  if (!CI->hasPreprocessor()) {
    CI->createPreprocessor();
  }
  //clang::Preprocessor& PP = CI->getPreprocessor();
  CI->getDiagnosticClient().BeginSourceFile(
    CI->getLangOpts(), &CI->getPreprocessor());
  CI->createASTContext();
  //clang::ASTContext& Context = CI->getASTContext();
  //CI->getASTContext().setASTMutationListern(Consumer.GetASTMutationListener();
  CI->setASTConsumer(new clang::ASTConsumer());
  //clang::ASTConsumer& Consumer = CI->getASTConsumer();
  //CI->getPreprocessor().getBuiltinInfo().InitializeBuiltins(
  //  CI->getPreprocessor().getIdentifierTable(),
  //  CI->getPreprocessor().getLangOptions());
  CI->createPCHExternalASTSource("it3.h.pch", true, false, false, 0);
  {
    llvm::MemoryBuffer* SB = llvm::MemoryBuffer::getMemBufferCopy(argv[2],
      "lookup.arg.types");
    const clang::FileEntry* File = CI->getFileManager().getVirtualFile(
      SB->getBufferIdentifier(), SB->getBufferSize(), 0);
    CI->getSourceManager().createMainFileID(File);
    CI->getSourceManager().overrideFileContents(File, SB);
  }
  if (!CI->hasSema()) {
    CI->createSema(clang::TU_Complete, 0);
  }
  //clang::Sema& Sema = CI->getSema();
  P = new clang::Parser(CI->getPreprocessor(), CI->getSema());
  CI->getPreprocessor().EnterMainSourceFile();
  P->Initialize();
  CI->getSema().Initialize();
  clang::Scope* TUScope = CI->getSema().getCurScope();
  if (clang::ExternalASTSource*
    External = CI->getSema().getASTContext().getExternalSource()) {
    External->StartTranslationUnit(&CI->getASTConsumer());
  }
  clang::TranslationUnitDecl* TU =
    CI->getASTContext().getTranslationUnitDecl();
  if (!TU) {
    fprintf(
      stderr
      , "astread: No translation unit decl in passed "
      "PCH file!\n"
    );
    return 0;
  }
  //TU->dump();
  //clang::ExprResult Result(P->ParseExpression());
  std::vector<clang::QualType> GivenArgTypes;
  std::vector<clang::Expr*> GivenArgs;
  clang::PrintingPolicy Policy(CI->getASTContext().getPrintingPolicy());
  Policy.SuppressTagKeyword = true;
  //Policy.SuppressTag = true;
  Policy.SuppressUnwrittenScope = true;
  Policy.SuppressInitializers = true;
  Policy.AnonymousTagLocations = false;
  std::string proto;
  {
     bool first_time = true;
     while (P->getCurToken().isNot(clang::tok::eof)) {
        clang::TypeResult Res(P->ParseTypeName());
        if (Res.isUsable()) {
           clang::QualType QT(Res.get().get());
           GivenArgTypes.push_back(QT.getCanonicalType());
           {
             clang::Expr* val = new (CI->getSema().getASTContext())
               clang::OpaqueValueExpr(clang::SourceLocation(),
               QT.getCanonicalType(),
               clang::Expr::getValueKindForType(QT));
             GivenArgs.push_back(val);
           }
           if (first_time) {
              first_time = false;
           }
           else {
              proto += ',';
           }
           proto += QT.getCanonicalType().getAsString(Policy);
           fprintf(stderr, "%s\n", proto.c_str());
        }
        if (!P->getCurToken().is(clang::tok::comma)) {
           break;
        }
        P->ConsumeToken();
     }
  }

#if 0
static bool TryCopyInitialization(const CanQualType FromQTy,
                                  const CanQualType ToQTy,
                                  Sema &S,
                                  SourceLocation Loc,
                                  ExprValueKind FromVK) {
  OpaqueValueExpr TmpExpr(Loc, FromQTy, FromVK);
  ImplicitConversionSequence ICS =
    TryCopyInitialization(S, &TmpExpr, ToQTy, true, true, false);

  return !ICS.isBad();
}
#endif // 0

  // Shutdown parser.
  {
    //CI->getDiagnosticClient().EndSourceFile();
    //delete P;
    //P = 0;
    //CI->setSema(0);
    //CI->setASTContext(0);
    //CI->setASTConsumer(0);
    //if (CI->hasPreprocessor()) {
      //CI->getPreprocessor().EndSourceFile();
      //delete &CI->getPreprocessor();
      //CI->setPreprocessor(0);
    //}
    //CI->clearOutputFiles(true/*EraseFiles*/);
    //CI->getSourceManager().clearIDTables();
    //CI->setSourceManager(0);
    //CI->setFileManager(0);
  }
  // Begin per-file stuff.
  //if (!CI->hasFileManager()) {
  //  CI->createFileManager();
  //}
  //if (!CI->hasSourceManager()) {
  //  CI->createSourceManager(CI->getFileManager());
  //}
  //if (!CI->hasPreprocessor()) {
  //  CI->createPreprocessor();
  //}
  //clang::Preprocessor& PP = CI->getPreprocessor();
  //CI->getDiagnosticClient().BeginSourceFile(
  //  CI->getLangOpts(), &CI->getPreprocessor());
  //CI->createASTContext();
  //clang::ASTContext& Context = CI->getASTContext();
  //CI->getASTContext().setASTMutationListern(Consumer.GetASTMutationListener();
  //CI->setASTConsumer(new clang::ASTConsumer());
  //clang::ASTConsumer& Consumer = CI->getASTConsumer();
  //CI->getPreprocessor().getBuiltinInfo().InitializeBuiltins(
  //  CI->getPreprocessor().getIdentifierTable(),
  //  CI->getPreprocessor().getLangOptions());
  //CI->createPCHExternalASTSource("it3.h.pch", true, false, false, 0);
  {
    llvm::MemoryBuffer* SB = llvm::MemoryBuffer::getMemBufferCopy(argv[1],
    "lookup.name.file");
    //const clang::FileEntry* File = CI->getFileManager().getVirtualFile(
    //  SB->getBufferIdentifier(), SB->getBufferSize(), 0);
    //CI->getSourceManager().createMainFileID(File);
    //CI->getSourceManager().overrideFileContents(File, SB);
    clang::FileID FID = CI->getSourceManager().createFileIDForMemBuffer(SB);
    CI->getPreprocessor().EnterSourceFile(FID, 0, clang::SourceLocation());
    CI->getPreprocessor().Lex(const_cast<clang::Token&>(P->getCurToken()));
  }
  //if (!CI->hasSema()) {
  //  CI->createSema(clang::TU_Complete, 0);
  //}
  //clang::Sema& Sema = CI->getSema();
  //P = new clang::Parser(CI->getPreprocessor(), CI->getSema());
  //CI->getPreprocessor().EnterMainSourceFile();
  //P->Initialize();
  //CI->getSema().Initialize();
  //clang::Scope* TUScope = CI->getSema().getCurScope();
  //if (clang::ExternalASTSource* External =
  //  CI->getSema().getASTContext().getExternalSource()
  //) {
  //  External->StartTranslationUnit(&CI->getASTConsumer());
  //}
  //clang::TranslationUnitDecl* TU =
  //  CI->getASTContext().getTranslationUnitDecl();
  //if (!TU) {
  //  fprintf(
  //    stderr
  //    , "astread: No translation unit decl in passed "
  //    "PCH file!\n"
  //  );
  //  return 0;
  //}
  //TU->dump();
  {
  clang::ExprResult Res;
  clang::tok::TokenKind SavedKind = P->getCurToken().getKind();
  bool NotCastExpr = false;
  clang::Parser::TypeCastState isTypeCast = clang::Parser::NotTypeCast;
  //case clang::tok::identifier:
#if 0
  const clang::Token& Next = P->NextToken();
  if (Next.is(clang::tok::coloncolon) ||
      (!P->ColonIsSacred && Next.is(clang::tok::colon)) ||
      Next.is(clang::tok::less) ||
      Next.is(clang::tok::l_paren) ||
      Next.is(clang::tok::l_brace)) {
    // If TryAnnotateTypeOrScopeToken annotates the token, tail recurse.
    if (P->TryAnnotateTypeOrScopeToken()) {
      return clang::ExprError();
    }
    if (!P->getCurToken().is(clang::tok::identifier)) {
      return /*call ourself*/;//ParseCastExpression(isUnaryExpression, isAddressOfOperand);
    }
  }
#endif // 0
  // Consume the identifier so that we can see if it is followed by a '(' or
  // '.'.
  clang::IdentifierInfo& II = *P->getCurToken().getIdentifierInfo();
  clang::SourceLocation ILoc = P->ConsumeToken();
  // Function designators are allowed to be undeclared (C99 6.5.1p2), so we
  // need to know whether or not this identifier is a function designator or
  // not.
  clang::UnqualifiedId Name;
  Name.setIdentifier(&II, ILoc);
  clang::CXXScopeSpec ScopeSpec;
  clang::SourceLocation TemplateKWLoc;
  //clang::CastExpressionIdValidator Validator(
  //  isTypeCast != clang::Parser::NotTypeCast,
  //  isTypeCast != clang::Parser::IsTypeCast);
  //Res = P->Actions.ActOnIdExpression(P->getCurScope(), ScopeSpec,
  //  TemplateKWLoc, Name, P->getCurToken().is(clang::tok::l_paren),
  //  isAddressOfOperand, &Validator);
  Res = P->getActions().ActOnIdExpression(P->getCurScope(), ScopeSpec,
    TemplateKWLoc, Name, P->getCurToken().is(clang::tok::l_paren),
    false /*isAddressOfOperand*/, 0);
#if 0
  clang::SourceLocation Loc;
  //switch (P->getCurToken().getKind())
  //case clang::tok::l_paren:
  //
  //  Process the argument list now.
  //
  clang::tok::TokenKind OpKind = P->getCurToken().getKind();
  clang::Expr* ExecConfig = 0;
  clang::BalancedDelimiterTracker PT(*P, clang::tok::l_paren);
  PT.consumeOpen();
  Loc = PT.getOpenLocation();
  clang::ExprVector ArgExprs(P->Actions);
  clang::CommaLocsTy CommaLocs;
  if (OpKind == clang::tok::l_paren || !Res.isInvalid()) {
    if (P->getCurToken().isNot(clang::tok::r_paren)) {
      if (P->ParseExpressionList(ArgExprs, CommaLocs, &clang::Sema::CodeCompleteCall,
                              Res.get())) {
        Res = ExprError(); 
      }
    }                      
  }
  // Match the ')'.
  if (Res.isInvalid()) {
    SkipUntil(tok::r_paren);
  } else if (getCurToken().isNot(tok::r_paren)) {
    PT.consumeClose();
    Res = ExprError();
  } else {
#endif // 0
    //Res = P->Actions.ActOnCallExpr(getCurScope(), Res.take(), Loc,
    //  move_arg(ArgExprs), P->getCurToken().getLocation(), ExecConfig);
    //ExprResult
    //Sema::ActOnCallExpr(Scope *S, Expr *Fn, SourceLocation LParenLoc,
    //                    MultiExprArg ArgExprs, SourceLocation RParenLoc,
    //                    Expr *ExecConfig, bool IsExecConfig)
    //unsigned NumArgs = ArgExprs.size();
    unsigned NumArgs = GivenArgs.size();
#if 0
    // Since this might be a postfix expression, get rid of ParenListExprs.
    clang::ExprResult Result = Sema->
      MaybeConvertParenListExprToParenExpr(S, Fn);
    if (Result.isInvalid()) return clang::ExprError();
    Fn = Result.take();
#endif // 0
    //clang::Expr** Args = ArgExprs.release();
    clang::Expr** Args = GivenArgs.data();
#if 0
    if (llvn::isa<clang::CXXPseudoDestructorExpr>(Fn)) {
    }
#endif // 0
#if 0
    // Determine whether this is a dependent call inside a C++ template,
    // in which case we won't do any semantic analysis now.
    // FIXME: Will need to cache the results of name lookup (including ADL) in
    // Fn.

    bool Dependent = false;
    if (Fn->isTypeDependent())
      Dependent = true;
    else if (Expr::hasAnyTypeDependentArguments(
      llvm::makeArrayRef(Args, NumArgs)))
      Dependent = true;
    if (Dependent) {
      if (ExecConfig) {
        return Owned(new (Context) CUDAKernelCallExpr(
            Context, Fn, cast<CallExpr>(ExecConfig), Args, NumArgs,
            Context.DependentTy, VK_RValue, RParenLoc));
      } else {
        return Owned(new (Context) CallExpr(Context, Fn, Args, NumArgs,
                                            Context.DependentTy, VK_RValue,
                                            RParenLoc));
      }
    }
#endif // 0
    clang::Expr* Fn = Res.take();
    // Determine whether this is a call to an object (C++ [over.call.object]).
    clang::SourceLocation LParenLoc;
    clang::SourceLocation RParenLoc;
    if (Fn->getType()->isRecordType()) {}
      //return CI->getSema().Owned(CI->getSema().BuildCallToObjectOfClassType(
      //  P->getCurScope(), Fn, LParenLoc, Args, NumArgs, RParenLoc));
    if (Fn->getType() == CI->getSema().getASTContext().UnknownAnyTy) {}
#if 0
    {
      ExprResult result = rebuildUnknownAnyFunction(*this, Fn);
      if (result.isInvalid()) return ExprError();
      Fn = result.take();
    }
#endif // 0
    if (Fn->getType() == CI->getSema().getASTContext().BoundMemberTy) {}
#if 0
    {
      return BuildCallToMemberFunction(S, Fn, LParenLoc, Args, NumArgs,
                                       RParenLoc);
    }
#endif // 0
    clang::UnresolvedLookupExpr* ULE = 0;
    if (Fn->getType() == CI->getSema().getASTContext().OverloadTy) {
      clang::OverloadExpr::FindResult find = clang::OverloadExpr::find(Fn);
      if (!find.HasFormOfMemberPointer) {
        clang::OverloadExpr* ovl = find.Expression;
        if (llvm::isa<clang::UnresolvedLookupExpr>(ovl)) {
          ULE = llvm::cast<clang::UnresolvedLookupExpr>(ovl);
          //return BuildOverloadedCallExpr(S, Fn, ULE, LParenLoc, Args, NumArgs,
          //                               RParenLoc, ExecConfig);
        } //else {
          //return BuildCallToMemberFunction(S, Fn, LParenLoc, Args, NumArgs,
          //                                 RParenLoc);
        //}
      }
    }
#if 0
    // Check for overloaded calls.  This can happen even in C due to extensions.
    if (Fn->getType() == CI->getSema().getASTContext().OverloadTy) {
      clang::OverloadExpr::FindResult find = clang::OverloadExpr::find(Fn);
      // We aren't supposed to apply this logic for if there's an '&' involved.
      if (!find.HasFormOfMemberPointer) {
        clang::OverloadExpr* ovl = find.Expression;
        if (llvm::isa<clang::UnresolvedLookupExpr>(ovl)) {
          clang::UnresolvedLookupExpr* ULE =
            llvm::cast<clang::UnresolvedLookupExpr>(ovl);
          return BuildOverloadedCallExpr(S, Fn, ULE, LParenLoc, Args, NumArgs,
                                         RParenLoc, ExecConfig);
        } else {
          return BuildCallToMemberFunction(S, Fn, LParenLoc, Args, NumArgs,
                                           RParenLoc);
        }
      }
    }
    // If we're directly calling a function, get the appropriate declaration.
    if (Fn->getType() == Context.UnknownAnyTy) {
      ExprResult result = rebuildUnknownAnyFunction(*this, Fn);
      if (result.isInvalid()) return ExprError();
      Fn = result.take();
    }
    Expr* NakedFn = Fn->IgnoreParens();
    NamedDecl* NDecl = 0;
    if (UnaryOperator* UnOp = dyn_cast<UnaryOperator>(NakedFn))
      if (UnOp->getOpcode() == UO_AddrOf)
        NakedFn = UnOp->getSubExpr()->IgnoreParens();
    if (isa<DeclRefExpr>(NakedFn))
      NDecl = cast<DeclRefExpr>(NakedFn)->getDecl();
    else if (isa<MemberExpr>(NakedFn))
      NDecl = cast<MemberExpr>(NakedFn)->getMemberDecl();
    //return BuildResolvedCallExpr(Fn, NDecl, LParenLoc, Args, NumArgs, RParenLoc,
    //                             ExecConfig, IsExecConfig);
    //
#endif // 0
    //// return BuildOverloadedCallExpr(S, Fn, ULE, LParenLoc, Args, NumArgs,
    ////   RParenLoc, ExecConfig);
    //ExprResult
    //Sema::BuildOverloadedCallExpr(Scope *S, Expr *Fn,
    //                              UnresolvedLookupExpr *ULE,
    //                              SourceLocation LParenLoc,
    //                              Expr **Args, unsigned NumArgs,
    //                              SourceLocation RParenLoc,
    //                              Expr *ExecConfig,
    //                              bool AllowTypoCorrection)
    //
    ///UnbridgedCastsSet UnbridgedCasts;
    ///if (checkArgPlaceholdersForOverload(*this, Args, NumArgs, UnbridgedCasts))
    ///  return ExprError();
    clang::OverloadCandidateSet CandidateSet(Fn->getExprLoc());
    // Add the functions denoted by the callee to the set of candidate
    // functions, including those from argument-dependent lookup.
    //AddOverloadedCallCandidates(ULE, llvm::makeArrayRef(Args, NumArgs),
    //                            CandidateSet);
    //void Sema::AddOverloadedCallCandidates(UnresolvedLookupExpr *ULE,
    //                                       llvm::ArrayRef<Expr *> Args,
    //                                       OverloadCandidateSet &CandidateSet,
    //                                       bool PartialOverloading)
    clang::TemplateArgumentListInfo TABuffer;
    clang::TemplateArgumentListInfo* ExplicitTemplateArgs = 0;
    if (ULE->hasExplicitTemplateArgs()) {
      ULE->copyTemplateArgumentsInto(TABuffer);
      ExplicitTemplateArgs = &TABuffer;
    }
    for (clang::UnresolvedLookupExpr::decls_iterator I = ULE->decls_begin(),
           E = ULE->decls_end(); I != E; ++I) {
      //AddOverloadedCallCandidate(*this, I.getPair(),
      //  ExplicitTemplateArgs, Args,
      // CandidateSet, PartialOverloading, /*KnownValid*/ true);
      //static void AddOverloadedCallCandidate(Sema &S,
      //  DeclAccessPair FoundDecl,
      //  TemplateArgumentListInfo *ExplicitTemplateArgs,
      //  llvm::ArrayRef<Expr *> Args,
      //  OverloadCandidateSet &CandidateSet,
      //  bool PartialOverloading,
      //  bool KnownValid)
      clang::DeclAccessPair FoundDecl = I.getPair();
      bool KnownValid = true;
      bool PartialOverloading = false;
      clang::NamedDecl* Callee = FoundDecl.getDecl();
      if (llvm::isa<clang::UsingShadowDecl>(Callee))
        Callee = llvm::cast<clang::UsingShadowDecl>(Callee)->getTargetDecl();
      if (clang::FunctionDecl* Func =
        llvm::dyn_cast<clang::FunctionDecl>(Callee)) {
        //if (ExplicitTemplateArgs) {
        //  assert(!KnownValid && "Explicit template arguments?");
        //  return;
        //}
        //S.AddOverloadCandidate(Func, FoundDecl, Args, CandidateSet, false,
        //                       PartialOverloading);
        //void
        //Sema::AddOverloadCandidate(FunctionDecl *Function,
        //                           DeclAccessPair FoundDecl,
        //                           llvm::ArrayRef<Expr *> Args,
        //                           OverloadCandidateSet& CandidateSet,
        //                           bool SuppressUserConversions,
        //                           bool PartialOverloading,
        //                           bool AllowExplicit) {
        clang::FunctionDecl* Function = Func;
        bool SuppressUserConversions = false;
        bool AllowExplicit = false;
        const clang::FunctionProtoType* Proto =
          llvm::dyn_cast<clang::FunctionProtoType>(
            Function->getType()->getAs<clang::FunctionType>());
          //assert(Proto && "Functions without a prototype cannot be overloaded");
          //assert(!Function->getDescribedFunctionTemplate() &&
          //       "Use AddTemplateOverloadCandidate for function templates");

#if 0
          if (clang::CXXMethodDecl* Method =
            llvm::dyn_cast<clang::CXXMethodDecl>(Function)) {
            if (!llvm::isa<clang::CXXConstructorDecl>(Method)) {
              // If we get here, it's because we're calling a member function
              // that is named without a member access expression (e.g.,
              // "this->f") that was either written explicitly or created
              // implicitly. This can happen with a qualified call to a member
              // function, e.g., X::f(). We use an empty type for the implied
              // object argument (C++ [over.call.func]p3), and the acting context
              // is irrelevant.
              AddMethodCandidate(Method, FoundDecl, Method->getParent(),
                                 QualType(), Expr::Classification::makeSimpleLValue(),
                                 Args, CandidateSet, SuppressUserConversions);
              return;
            }
            // We treat a constructor like a non-member function, since its object
            // argument doesn't participate in overload resolution.
          }
#endif // 0

#if 0
          if (!CandidateSet.isNewCandidate(Function))
            return;
#endif // 0

          // Overload resolution is always an unevaluated context.
          //EnterExpressionEvaluationContext Unevaluated(*this, Sema::Unevaluated);
#if 0
          if (clang::CXXConstructorDecl* Constructor =
            llvm::dyn_cast<clang::CXXConstructorDecl>(Function)) {
            // C++ [class.copy]p3:
            //   A member function template is never instantiated to perform the copy
            //   of a class object to an object of its class type.
            clang::QualType ClassType = CI->getSema().getASTContext().
              getTypeDeclType(Constructor->getParent());
            if (Args.size() == 1 &&
                Constructor->isSpecializationCopyingObject() &&
                (Context.hasSameUnqualifiedType(ClassType, Args[0]->getType()) ||
                 IsDerivedFrom(Args[0]->getType(), ClassType)))
              return;
          }
#endif // 0
          // Add this candidate
          clang::OverloadCandidate& Candidate =
            CandidateSet.addCandidate(NumArgs);
          Candidate.FoundDecl = FoundDecl;
          Candidate.Function = Function;
          Candidate.Viable = true;
          Candidate.IsSurrogate = false;
          Candidate.IgnoreObjectArgument = false;
          Candidate.ExplicitCallArguments = NumArgs;
          unsigned NumArgsInProto = Proto->getNumArgs();
          // (C++ 13.3.2p2): A candidate function having fewer than m
          // parameters is viable only if it has an ellipsis in its parameter
          // list (8.3.5).
          if ((NumArgs + (PartialOverloading && NumArgs)) > NumArgsInProto &&
              !Proto->isVariadic()) {
            Candidate.Viable = false;
            Candidate.FailureKind = clang::ovl_fail_too_many_arguments;
            //return;
          }
          // (C++ 13.3.2p2): A candidate function having more than m parameters
          // is viable only if the (m+1)st parameter has a default argument
          // (8.3.6). For the purposes of overload resolution, the
          // parameter list is truncated on the right, so that there are
          // exactly m parameters.
          unsigned MinRequiredArgs = Function->getMinRequiredArguments();
          if (NumArgs < MinRequiredArgs && !PartialOverloading) {
            // Not enough arguments.
            Candidate.Viable = false;
            Candidate.FailureKind = clang::ovl_fail_too_few_arguments;
            //return;
          }
          // Determine the implicit conversion sequences for each of the
          // arguments.
          for (unsigned ArgIdx = 0; ArgIdx < NumArgs; ++ArgIdx) {
            if (ArgIdx < NumArgsInProto) {
              // (C++ 13.3.2p3): for F to be a viable function, there shall
              // exist for each argument an implicit conversion sequence
              // (13.3.3.1) that converts that argument to the corresponding
              // parameter of F.
              clang::QualType ParamType = Proto->getArgType(ArgIdx);
              Candidate.Conversions[ArgIdx]
                = clang::TryCopyInitialization(CI->getSema(),
                    Args[ArgIdx], ParamType,
                    /*SuppressUserConversions=*/false,
                    /*InOverloadResolution=*/true,
                    /*AllowObjCWritebackConversion=*/
                    /*getLangOptions().ObjCAutoRefCount*/false,
                    /*AllowExplicit=*/false);
              if (Candidate.Conversions[ArgIdx].isBad()) {
                Candidate.Viable = false;
                Candidate.FailureKind = clang::ovl_fail_bad_conversion;
                break;
              }
            } else {
              // (C++ 13.3.2p2): For the purposes of overload resolution, any
              // argument for which there is no corresponding parameter is
              // considered to ""match the ellipsis" (C+ 13.3.3.1.3).
              Candidate.Conversions[ArgIdx].setEllipsis();
            }
          }
        //}
        //////
        ////return;
      }
#if 0
      if (clang::FunctionTemplateDecl* FuncTemplate =
        llvm::dyn_cast<clang::FunctionTemplateDecl>(Callee)) {
        S.AddTemplateOverloadCandidate(FuncTemplate, FoundDecl,
                                       ExplicitTemplateArgs, Args, CandidateSet);
        return;
      }
      assert(!KnownValid && "unhandled case in overloaded call candidate");
#endif // 0
    }
    int BreakHere = 0;
    clang::OverloadCandidateSet::iterator Best;
    clang::OverloadingResult OR = CandidateSet.BestViableFunction(CI->getSema(), Fn->getLocStart(), Best);
    switch (OR) {
      case clang::OR_Success:
        break;
      case clang::OR_No_Viable_Function:
        break;
      case clang::OR_Ambiguous:
        break;
      case clang::OR_Deleted:
        break;
      default:
        break;
    }
#if 0
    ////////
    if (ULE->requiresADL())
      // FIXME
      AddArgumentDependentLookupCandidates(ULE->getName(), /*Operator*/ false,
                                           ULE->getExprLoc(),
                                           Args, ExplicitTemplateArgs,
                                           CandidateSet, PartialOverloading,
                                           ULE->isStdAssociatedNamespace());
    ////////
    // If we found nothing, try to recover.
    // BuildRecoveryCallExpr diagnoses the error itself, so we just bail
    // out if it fails.
    if (CandidateSet.empty()) {
      // In Microsoft mode, if we are inside a template class member function then
      // create a type dependent CallExpr. The goal is to postpone name lookup
      // to instantiation time to be able to search into type dependent base
      // classes.
      if (getLangOptions().MicrosoftMode && CurContext->isDependentContext() && 
          (isa<FunctionDecl>(CurContext) || isa<CXXRecordDecl>(CurContext))) {
        CallExpr *CE = new (Context) CallExpr(Context, Fn, Args, NumArgs,
                                            Context.DependentTy, VK_RValue,
                                            RParenLoc);
        CE->setTypeDependent(true);
        return Owned(CE);
      }
      return BuildRecoveryCallExpr(*this, S, Fn, ULE, LParenLoc,
                                   llvm::MutableArrayRef<Expr *>(Args, NumArgs),
                                   RParenLoc, /*EmptyLookup=*/true,
                                   AllowTypoCorrection);
    }
    UnbridgedCasts.restore();
    OverloadCandidateSet::iterator Best;
    switch (CandidateSet.BestViableFunction(*this, Fn->getLocStart(), Best)) {
    case OR_Success: {
      FunctionDecl *FDecl = Best->Function;
      MarkFunctionReferenced(Fn->getExprLoc(), FDecl);
      CheckUnresolvedLookupAccess(ULE, Best->FoundDecl);
      DiagnoseUseOfDecl(FDecl, ULE->getNameLoc());
      Fn = FixOverloadedFunctionReference(Fn, Best->FoundDecl, FDecl);
      return BuildResolvedCallExpr(Fn, FDecl, LParenLoc, Args, NumArgs, RParenLoc,
                                   ExecConfig);
    }
    case OR_No_Viable_Function: {
      // Try to recover by looking for viable functions which the user might
      // have meant to call.
      ExprResult Recovery = BuildRecoveryCallExpr(*this, S, Fn, ULE, LParenLoc,
                                    llvm::MutableArrayRef<Expr *>(Args, NumArgs),
                                                  RParenLoc,
                                                  /*EmptyLookup=*/false,
                                                  AllowTypoCorrection);
      if (!Recovery.isInvalid())
        return Recovery;
      Diag(Fn->getSourceRange().getBegin(),
           diag::err_ovl_no_viable_function_in_call)
        << ULE->getName() << Fn->getSourceRange();
      CandidateSet.NoteCandidates(*this, OCD_AllCandidates,
                                  llvm::makeArrayRef(Args, NumArgs));
      break;
    }
    case OR_Ambiguous:
      Diag(Fn->getSourceRange().getBegin(), diag::err_ovl_ambiguous_call)
        << ULE->getName() << Fn->getSourceRange();
      CandidateSet.NoteCandidates(*this, OCD_ViableCandidates,
                                  llvm::makeArrayRef(Args, NumArgs));
      break;
    case OR_Deleted:
      {
        Diag(Fn->getSourceRange().getBegin(), diag::err_ovl_deleted_call)
          << Best->Function->isDeleted()
          << ULE->getName()
          << getDeletedOrUnavailableSuffix(Best->Function)
          << Fn->getSourceRange();
        CandidateSet.NoteCandidates(*this, OCD_AllCandidates,
                                    llvm::makeArrayRef(Args, NumArgs));

        // We emitted an error for the unvailable/deleted function call but keep
        // the call in the AST.
        FunctionDecl *FDecl = Best->Function;
        Fn = FixOverloadedFunctionReference(Fn, Best->FoundDecl, FDecl);
        return BuildResolvedCallExpr(Fn, FDecl, LParenLoc, Args, NumArgs,
                                     RParenLoc, ExecConfig);
      }
    }
    // Overload resolution failed.
    return ExprError();
    //
    PT.consumeClose();
  //}
#endif // 0
  }
  //
  //  All done.
  //
  cleanup();
  return 0;
}

