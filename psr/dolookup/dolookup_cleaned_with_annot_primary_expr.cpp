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
#include "clang/Sema/ParsedTemplate.h"
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
TryCopyInitialization(clang::Sema& S, clang::Expr* From, clang::QualType ToType,
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
  //
  //  Setup to parse the argument list.
  //
  if (!CI->hasPreprocessor()) {
    CI->createPreprocessor();
  }
  CI->getDiagnosticClient().BeginSourceFile(
    CI->getLangOpts(), &CI->getPreprocessor());
  CI->createASTContext();
  CI->setASTConsumer(new clang::ASTConsumer());
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
  //
  //  Parse the arguments now.
  //
  std::vector<clang::QualType> GivenArgTypes;
  std::vector<clang::Expr*> GivenArgs;
  {
    clang::PrintingPolicy Policy(CI->getASTContext().getPrintingPolicy());
    Policy.SuppressTagKeyword = true;
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
            clang::Expr* val = new(CI->getSema().getASTContext())
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
  }
  //
  //  Setup to parse the function name.
  //
  {
    llvm::MemoryBuffer* SB = llvm::MemoryBuffer::getMemBufferCopy(argv[1],
                             "lookup.name.file");
    clang::FileID FID = CI->getSourceManager().createFileIDForMemBuffer(SB);
    CI->getPreprocessor().EnterSourceFile(FID, 0, clang::SourceLocation());
    CI->getPreprocessor().Lex(const_cast<clang::Token&>(P->getCurToken()));
  }
  //
  //  Parse the function name and do overload resolution.
  //
  {
    // Parse until we get to the left parenthesis.
    // This is ParseCastExpression.
    clang::ExprResult Res;
    {
      bool failed = false;
      bool done = false;
      while (!done) {
        clang::tok::TokenKind SavedKind = P->getCurToken().getKind();
        switch (SavedKind) {
          // FIXME: need clang::tok::kw_typename and clang::tok::kw_decltype
          case clang::tok::annot_primary_expr: { // "id." or "id->"
              Res = P->getExprAnnotation(const_cast<clang::Token&>(P->getCurToken()));
              P->ConsumeToken();
              done = true;
            }
            break;
          case clang::tok::identifier: { // "id("
              const clang::Token& Next = P->NextToken();
              if (Next.is(clang::tok::coloncolon) ||
                  (/*!P->ColonIsSacred &&*/ Next.is(clang::tok::colon)) ||
                  Next.is(clang::tok::less) ||
                  Next.is(clang::tok::l_paren) ||
                  Next.is(clang::tok::l_brace)) {
                if (P->TryAnnotateTypeOrScopeToken()) {
                  failed = true;
                  done = true;
                  break;
                }
                if (!P->getCurToken().is(clang::tok::identifier)) {
                  // Token was annotated, reparse.
                  continue;
                }
              }
              // We have an unqualified-id as function name.
              clang::Parser::TypeCastState isTypeCast = clang::Parser::NotTypeCast;
              clang::IdentifierInfo& II = *P->getCurToken().getIdentifierInfo();
              clang::SourceLocation ILoc = P->ConsumeToken();
              clang::UnqualifiedId Name;
              Name.setIdentifier(&II, ILoc);
              clang::CXXScopeSpec ScopeSpec;
              clang::SourceLocation TemplateKWLoc;
              Res = P->getActions().ActOnIdExpression(P->getCurScope(), ScopeSpec, TemplateKWLoc, Name, /*HasTrailingLParen*/P->getCurToken().is(clang::tok::l_paren), /*IsAddressOfOperand*/false, /*CorrectionCandidateCallback*/0);
              done = true;
            }
            break;
          case clang::tok::annot_cxxscope: { // "id::id::f" or "id::f"
              if (P->TryAnnotateTypeOrScopeToken()) {
                failed = true;
                done = true;
                break;
              }
              if (!P->getCurToken().is(clang::tok::annot_cxxscope)) {
                // Token was re-annotated, reparse.
                continue;
              }
              // We have a scope followed something that is not
              // a scope or type name.
              clang::Token Next = P->NextToken();
              if (Next.is(clang::tok::annot_template_id)) {
                // We have a qualified template id.
                clang::TemplateIdAnnotation* TemplateId = P->takeTemplateIdAnnotation(Next);
                if (TemplateId->Kind == clang::TNK_Type_template) {
                  // The template_id is a type, add it to the annotation
                  // and reparse.
                  clang::CXXScopeSpec SS;
                  P->ParseOptionalCXXScopeSpecifier(SS, clang::ParsedType(), /*EnteringContext=*/false);
                  P->AnnotateTemplateIdTokenAsType();
                  continue;
                }
              }
              // We have a non-type name with a scope qualifier.
              clang::CXXScopeSpec SS;
              P->ParseOptionalCXXScopeSpecifier(SS, clang::ParsedType(), /*EnteringContext=*/false);
              clang::SourceLocation TemplateKWLoc;
              clang::UnqualifiedId Name;
              if (P->ParseUnqualifiedId(SS,
                                        /*EnteringContext=*/false,
                                        /*AllowDestructorName=*/false,
                                        /*AllowConstructorName=*/false,
                                        /*ObjectType=*/ clang::ParsedType(),
                                        TemplateKWLoc,
                                        Name)) {
                failed = true;
                done = true;
                break;
              }
              Res = P->getActions().ActOnIdExpression(P->getCurScope(), SS, TemplateKWLoc, Name, /*HasTrailingLParen*/P->getCurToken().is(clang::tok::l_paren), /*isAddressOfOperand*/false, /*CorrectionCandidateCallback*/0);
              done = true;
            }
            break;
          case clang::tok::annot_template_id: { // "id<A>(" or "id<A>::id"
              clang::TemplateIdAnnotation* TemplateId = P->takeTemplateIdAnnotation(P->getCurToken());
              if (TemplateId->Kind == clang::TNK_Type_template) {
                // Convert the template_id to an annotation
                // token and reparse.
                P->AnnotateTemplateIdTokenAsType();
                continue;
              }
              // We have a template_id that is not a type, so
              // it must be a function.
              clang::CXXScopeSpec SS;
              P->ParseOptionalCXXScopeSpecifier(SS, clang::ParsedType(), /*EnteringContext=*/false);
              clang::SourceLocation TemplateKWLoc;
              clang::UnqualifiedId Name;
              if (P->ParseUnqualifiedId(SS,
                                        /*EnteringContext=*/false,
                                        /*AllowDestructorName=*/false,
                                        /*AllowConstructorName=*/false,
                                        /*ObjectType=*/ clang::ParsedType(),
                                        TemplateKWLoc,
                                        Name)) {
                failed = true;
                done = true;
                break;
              }
              Res = P->getActions().ActOnIdExpression(P->getCurScope(), SS, TemplateKWLoc, Name, /*HasTrailingLParen*/P->getCurToken().is(clang::tok::l_paren), /*isAddressOfOperand*/false, /*CorrectionCandidateCallback*/0);
              done = true;
            }
            break;
          case clang::tok::kw_operator: { // "operator<<("
              clang::CXXScopeSpec SS;
              P->ParseOptionalCXXScopeSpecifier(SS, clang::ParsedType(), /*EnteringContext=*/false);
              clang::SourceLocation TemplateKWLoc;
              clang::UnqualifiedId Name;
              if (P->ParseUnqualifiedId(SS,
                                        /*EnteringContext=*/false,
                                        /*AllowDestructorName=*/false,
                                        /*AllowConstructorName=*/false,
                                        /*ObjectType=*/ clang::ParsedType(),
                                        TemplateKWLoc,
                                        Name)) {
                failed = true;
                done = true;
                break;
              }
              Res = P->getActions().ActOnIdExpression(P->getCurScope(), SS, TemplateKWLoc, Name, /*HasTrailingLParen*/P->getCurToken().is(clang::tok::l_paren), /*isAddressOfOperand*/false, /*CorrectionCandidateCallback*/0);
              done = true;
            }
            break;
          case clang::tok::coloncolon: { // "::id(" or "::id::id" or "::new" or "::delete"
              // We have a global scope specifier, collect any
              // following qualifiers in an annotation token.
              if (P->TryAnnotateTypeOrScopeToken()) {
                failed = true;
                done = true;
                break;
              }
              if (!P->getCurToken().is(clang::tok::coloncolon)) {
                // Token was annotated, reparse.
                continue;
              }
              // We have a bare global scope specifier, check
              // for global new and delete, they are special.
              clang::SourceLocation CCLoc = P->ConsumeToken();
              if (P->getCurToken().is(clang::tok::kw_new)) {
                //return ParseCXXNewExpression(true, CCLoc);
                failed = true;
                done = true;
                break;
              }
              if (P->getCurToken().is(clang::tok::kw_delete)) {
                //return ParseCXXDeleteExpression(true, CCLoc);
                failed = true;
                done = true;
                break;
              }
              failed = true;
              done = true;
              break;
            }
            break;
          case clang::tok::kw_new: { // "new("
              failed = true;
              done = true;
              break;
            }
            break;
          case clang::tok::kw_delete: { // "delete("
              failed = true;
              done = true;
              break;
            }
            break;
          default: {
              failed = true;
              done = true;
              break;
            }
        }
      }
      if (failed) {
        cleanup();
        return 0;
      }
    }
    // At this point we may have performed
    // a lookup on the function name.
    // This is ParsePostfixExpressionSuffix.
    {
      bool done = false;
      while (!done) {
        switch (P->getCurToken().getKind()) {
          case clang::tok::l_paren: {
            }
            break;
          case clang::tok::arrow:
          case clang::tok::period: {
            }
            break;
        }
      }
    }
    //
    //   Now do overload resolution.
    //
    {
      unsigned NumArgs = GivenArgs.size();
      clang::Expr** Args = GivenArgs.data();
      clang::Expr* Fn = Res.take();
      clang::SourceLocation LParenLoc;
      clang::SourceLocation RParenLoc;
      if (llvm::isa<clang::CXXPseudoDestructorExpr>(Fn)) {
        //return Owned(new (Context) CallExpr(Context, Fn, 0, 0, Context.VoidTy, VK_RValue, RParenLoc));
      }
      bool Dependent = false;
      if (Fn->isTypeDependent()) {
        Dependent = true;
      }
      else if (clang::Expr::hasAnyTypeDependentArguments(llvm::makeArrayRef(Args, NumArgs))) {
        Dependent = true;
      }
      if (Dependent) {
        //return Owned(new (Context) CallExpr(Context, Fn, Args, NumArgs, Context.DependentTy, VK_RValue, RParenLoc));
      }
      if (Fn->getType()->isRecordType()) {
        // Found call of either MyClass::operator() or MyClass::operator int().
        //return CI->getSema().Owned(CI->getSema().BuildCallToObjectOfClassType(P->getCurScope(), Fn, LParenLoc, Args, NumArgs, RParenLoc));
      }
      if (Fn->getType() == CI->getSema().getASTContext().UnknownAnyTy) {
        //ExprResult result = rebuildUnknownAnyFunction(*this, Fn);
        //if (result.isInvalid()) return ExprError();
        //Fn = result.take();
      }
      if (Fn->getType() == CI->getSema().getASTContext().BoundMemberTy) {
        // Found a non-static or overloaded member function.
        //return BuildCallToMemberFunction(S, Fn, LParenLoc, Args, NumArgs, RParenLoc);
      }
      clang::UnresolvedLookupExpr* ULE = 0;
      if (Fn->getType() == CI->getSema().getASTContext().OverloadTy) {
        clang::OverloadExpr::FindResult find = clang::OverloadExpr::find(Fn);
        if (!find.HasFormOfMemberPointer) {
          clang::OverloadExpr* ovl = find.Expression;
          if (llvm::isa<clang::UnresolvedLookupExpr>(ovl)) {
            ULE = llvm::cast<clang::UnresolvedLookupExpr>(ovl);
          } //else {
          //return BuildCallToMemberFunction(S, Fn, LParenLoc, Args, NumArgs, RParenLoc);
          //}
        }
      }
      if (!ULE) {
        // Not overloaded.
        if (Fn->getType() == CI->getSema().getASTContext().UnknownAnyTy) {
          //ExprResult result = rebuildUnknownAnyFunction(*this, Fn);
          //if (result.isInvalid()) return ExprError();
          //Fn = result.take();
        }
        clang::Expr* NakedFn = Fn->IgnoreParens();
        clang::NamedDecl* NDecl = 0;
        if (clang::UnaryOperator* UnOp = llvm::dyn_cast<clang::UnaryOperator>(NakedFn)) {
          if (UnOp->getOpcode() == clang::UO_AddrOf) {
            NakedFn = UnOp->getSubExpr()->IgnoreParens();
          }
        }
        if (llvm::isa<clang::DeclRefExpr>(NakedFn)) {
          NDecl = llvm::cast<clang::DeclRefExpr>(NakedFn)->getDecl();
        }
        else if (llvm::isa<clang::MemberExpr>(NakedFn)) {
          NDecl = llvm::cast<clang::MemberExpr>(NakedFn)->getMemberDecl();
        }
        //return BuildResolvedCallExpr(Fn, NDecl, LParenLoc, Args, NumArgs, RParenLoc, ExecConfig, IsExecConfig);
      }
      // Overloaded function.
      clang::OverloadCandidateSet CandidateSet(Fn->getExprLoc());
      clang::TemplateArgumentListInfo TABuffer;
      clang::TemplateArgumentListInfo* ExplicitTemplateArgs = 0;
      if (ULE->hasExplicitTemplateArgs()) {
        ULE->copyTemplateArgumentsInto(TABuffer);
        ExplicitTemplateArgs = &TABuffer;
      }
      for (clang::UnresolvedLookupExpr::decls_iterator I = ULE->decls_begin(), E = ULE->decls_end(); I != E; ++I) {
        clang::DeclAccessPair FoundDecl = I.getPair();
        bool KnownValid = true;
        bool PartialOverloading = false;
        clang::NamedDecl* Callee = FoundDecl.getDecl();
        if (llvm::isa<clang::UsingShadowDecl>(Callee)) {
          Callee = llvm::cast<clang::UsingShadowDecl>(Callee)->getTargetDecl();
        }
        if (clang::FunctionDecl* Func = llvm::dyn_cast<clang::FunctionDecl>(Callee)) {
          clang::FunctionDecl* Function = Func;
          bool SuppressUserConversions = false;
          bool AllowExplicit = false;
          const clang::FunctionProtoType* Proto = llvm::dyn_cast<clang::FunctionProtoType>(Function->getType()->getAs<clang::FunctionType>());
          clang::OverloadCandidate& Candidate = CandidateSet.addCandidate(NumArgs);
          Candidate.FoundDecl = FoundDecl;
          Candidate.Function = Function;
          Candidate.Viable = true;
          Candidate.IsSurrogate = false;
          Candidate.IgnoreObjectArgument = false;
          Candidate.ExplicitCallArguments = NumArgs;
          unsigned NumArgsInProto = Proto->getNumArgs();
          if ((NumArgs + (PartialOverloading && NumArgs)) > NumArgsInProto && !Proto->isVariadic()) {
            Candidate.Viable = false;
            Candidate.FailureKind = clang::ovl_fail_too_many_arguments;
            //return;
          }
          unsigned MinRequiredArgs = Function->getMinRequiredArguments();
          if (NumArgs < MinRequiredArgs && !PartialOverloading) {
            Candidate.Viable = false;
            Candidate.FailureKind = clang::ovl_fail_too_few_arguments;
            //return;
          }
          // Determine the implicit conversion sequences for each of the
          // arguments.
          for (unsigned ArgIdx = 0; ArgIdx < NumArgs; ++ArgIdx) {
            if (ArgIdx < NumArgsInProto) {
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
            }
            else {
              Candidate.Conversions[ArgIdx].setEllipsis();
            }
          }
        }
      }
  #if 0
      if (ULE->requiresADL()) {
        CI->getSema().AddArgumentDependentLookupCandidates(ULE->getName(), /*Operator*/ false, ULE->getExprLoc(), Args, ExplicitTemplateArgs, CandidateSet, PartialOverloading, ULE->isStdAssociatedNamespace());
      }
  #endif // 0
      if (CandidateSet.empty()) {
        cleanup();
        return 0;
      }
      int BreakHere = 0;
      clang::OverloadCandidateSet::iterator Best;
      clang::OverloadingResult OR = CandidateSet.BestViableFunction(CI->getSema(), Fn->getLocStart(), Best);
      switch (OR) {
        case clang::OR_Success: {
            clang::FunctionDecl* FDecl = Best->Function;
            //CI->getSema().MarkFunctionReferenced(Fn->getExprLoc(), FDecl);
            //Fn = FixOverloadedFunctionReference(Fn, Best->FoundDecl, FDecl);
            //return BuildResolvedCallExpr(Fn, FDecl, LParenLoc, Args, NumArgs, RParenLoc, ExecConfig);
          }
          break;
        case clang::OR_No_Viable_Function: {
          }
          break;
        case clang::OR_Ambiguous: {
          }
          break;
        case clang::OR_Deleted: {
            clang::FunctionDecl* FDecl = Best->Function;
            //CI->getSema().MarkFunctionReferenced(Fn->getExprLoc(), FDecl);
            //Fn = FixOverloadedFunctionReference(Fn, Best->FoundDecl, FDecl);
            //return BuildResolvedCallExpr(Fn, FDecl, LParenLoc, Args, NumArgs, RParenLoc, ExecConfig);
          }
          break;
        default: {
            // Impossible!
            cleanup();
            return 0;
          }
          break;
      }
    }
  }
  //
  //  All done.
  //
  cleanup();
  return 0;
}

