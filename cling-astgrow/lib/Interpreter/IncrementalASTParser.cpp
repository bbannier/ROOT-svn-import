//------------------------------------------------------------------------------
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id: Interpreter.cpp 36507 2010-11-04 21:42:17Z axel $
// author:  Lukasz Janyst <ljanyst@cern.ch>
//------------------------------------------------------------------------------

#include "IncrementalASTParser.h"

#include "llvm/Support/MemoryBuffer.h"

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclGroup.h"
#include "clang/Parse/Parser.h"
#include "clang/Parse/ParseAST.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Sema/SemaConsumer.h"

#include "DependentNodesTransform.h"

#include <stdio.h>

namespace cling {
class MutableMemoryBuffer: public llvm::MemoryBuffer {
    std::string m_FileID;
    size_t m_Alloc;
  protected:
    void maybeRealloc(llvm::StringRef code, size_t oldlen) {
      size_t applen = code.size();
      char* B = 0;
      if (oldlen) {
        B = const_cast<char*>(getBufferStart());
        assert(!B[oldlen] && "old buffer is not 0 terminated!");
        // B + oldlen points to trailing '\0'
      }
      size_t newlen = oldlen + applen + 1;
      if (newlen > m_Alloc) {
        m_Alloc += 64*1024;
        B = (char*)realloc(B, m_Alloc);
      }
      memcpy(B + oldlen, code.data(), applen);
      B[newlen - 1] = 0;
      init(B, B + newlen - 1);
    }
    
  public:
    MutableMemoryBuffer(llvm::StringRef Code, llvm::StringRef Name)
    : MemoryBuffer(), m_FileID(Name), m_Alloc(0) {
      maybeRealloc(Code, 0);
    }
    
    virtual ~MutableMemoryBuffer() {
      free((void*)getBufferStart());
    }
    
    void append(llvm::StringRef code) {
      assert(getBufferSize() && "buffer is empty!");
      maybeRealloc(code, getBufferSize());
    }
    virtual const char *getBufferIdentifier() const {
      return m_FileID.c_str();
    }
  };

  class ChainedASTConsumer: public clang::ASTConsumer {
  public:
    ChainedASTConsumer() {}
    virtual ~ChainedASTConsumer() {}
    
#define CAC_DECL(WHAT, ARGS, PARAM) \
void WHAT ARGS { \
for (llvm::SmallVector<clang::ASTConsumer*,2>::iterator i = Consumers.begin(), \
e = Consumers.end(); i != e; ++i) (*i)->WHAT PARAM; \
}
    CAC_DECL(Initialize,(clang::ASTContext &Context),(Context));
    CAC_DECL(HandleTopLevelDecl,(clang::DeclGroupRef D),(D));
    CAC_DECL(HandleInterestingDecl,(clang::DeclGroupRef D),(D));
    CAC_DECL(HandleTranslationUnit,(clang::ASTContext &Ctx),(Ctx));
    CAC_DECL(HandleTagDeclDefinition,(clang::TagDecl *D),(D));
    CAC_DECL(CompleteTentativeDefinition,(clang::VarDecl *D),(D));
    CAC_DECL(HandleVTable,(clang::CXXRecordDecl *RD, bool DefinitionRequired), \
             (RD, DefinitionRequired));
    CAC_DECL(PrintStats,(),());
    
    clang::ASTMutationListener *GetASTMutationListener() {
      return Consumers.empty() ? 0 : Consumers[0]->GetASTMutationListener();
    }
    clang::ASTDeserializationListener *GetASTDeserializationListener() {
      return Consumers.empty() ? 0 : Consumers[0]->GetASTDeserializationListener();
    }

    llvm::SmallVector<clang::ASTConsumer*,2> Consumers;
  };
} // namespace cling

cling::IncrementalASTParser::IncrementalASTParser(clang::CompilerInstance* CI,
                                                  clang::ASTConsumer* Consumer):
m_Consumer(0), m_InterruptAtNextTopLevelDecl(false)
{
  assert(CI && "CompilerInstance is (null)!");
  m_CI.reset(CI);

  CI->createPreprocessor();
  clang::Preprocessor& PP = CI->getPreprocessor();
  clang::ASTContext *Ctx = new clang::ASTContext(CI->getLangOpts(),
                                                 PP.getSourceManager(), CI->getTarget(), PP.getIdentifierTable(),
                                                 PP.getSelectorTable(), PP.getBuiltinInfo(), 0);
  CI->setASTContext(Ctx);
  PP.getBuiltinInfo().InitializeBuiltins(PP.getIdentifierTable(),
                                         PP.getLangOptions().NoBuiltin);


  m_MemoryBuffer.reset(new MutableMemoryBuffer("//cling!\n", "CLING"));

  CI->getSourceManager().clearIDTables();
  CI->getSourceManager().createMainFileIDForMemBuffer(m_MemoryBuffer.get());
  if (CI->getSourceManager().getMainFileID().isInvalid()) {
    fprintf(stderr, "Interpreter::compileString: Failed to create main "
            "file id!\n");
    return;
  }

  m_Consumer = new ChainedASTConsumer();
  m_Consumer->Consumers.push_back(Consumer);
  m_Consumer->Initialize(*Ctx);
  m_CI->setASTConsumer(m_Consumer);
  
  bool CompleteTranslationUnit = false;
  clang::CodeCompleteConsumer *CompletionConsumer = 0;
  m_Sema.reset(new clang::Sema(PP, *Ctx, *m_Consumer, CompleteTranslationUnit,
                               CompletionConsumer));
  PP.EnterMainSourceFile();
  
  // Initialize the parser.
  m_Parser.reset(new clang::Parser(PP, *m_Sema.get()));
  m_Parser->Initialize();
  
  if (clang::SemaConsumer *SC = dyn_cast<clang::SemaConsumer>(m_Consumer))
    SC->InitializeSema(*m_Sema.get());
  
}

cling::IncrementalASTParser::~IncrementalASTParser()
{
  m_CI->takeLLVMContext(); // Don't take down the context with the CI.
}

clang::CompilerInstance*
cling::IncrementalASTParser::parse(llvm::StringRef src,
                                   int nTopLevelDecls /* = 1 */,
                                   clang::ASTConsumer* AddConsumer /* = 0 */) {
  // Add src to the memory buffer, parse it, and add it to
  // the AST. Returns the CompilerInstance (and thus the AST).
  // Diagnostics are reset for each call of parse: they are only covering
  // src.
  // If nTopLevelDecls > 0, return after parsing that many top level
  // declarations.

  clang::Preprocessor& PP = m_CI->getPreprocessor();
  m_CI->getDiagnosticClient().BeginSourceFile(m_CI->getLangOpts(), &PP);

  if (AddConsumer) {
    m_Consumer->Consumers.push_back(AddConsumer);
  }

  m_MemoryBuffer->append(src);
  
  clang::Lexer* L = static_cast<clang::Lexer*>(PP.getTopmostLexer());
  if (L) {
    L->updateBufferEnd(m_MemoryBuffer->getBufferEnd());
  }
  // BEGIN REPLACEMENT clang::ParseAST(PP, &CI->getASTConsumer(), CI->getASTContext());
  
  clang::ASTConsumer* Consumer = &m_CI->getASTConsumer();
  clang::Parser::DeclGroupPtrTy ADecl;
  while (!m_Parser->ParseTopLevelDecl(ADecl) && --nTopLevelDecls != 0) {
    // Not end of file.
    // If we got a null return and something *was* parsed, ignore it.  This
    // is due to a top-level semicolon, an action override, or a parse error
    // skipping something.
    if (ADecl) {
      clang::DeclGroupRef DGR = ADecl.getAsVal<clang::DeclGroupRef>();
      Consumer->HandleTopLevelDecl(DGR);
      if (m_InterruptAtNextTopLevelDecl) {
        break;
      }
    }
  };
  
  // Process any TopLevelDecls generated by #pragma weak.
  for (llvm::SmallVector<clang::Decl*,2>::iterator
       I = m_Sema->WeakTopLevelDecls().begin(),
       E = m_Sema->WeakTopLevelDecls().end(); I != E; ++I) {
    Consumer->HandleTopLevelDecl(clang::DeclGroupRef(*I));
  }

  {
    // Here we are substituting the dependent nodes with Cling invocations.
    DependentNodesTransform transformer;
    transformer.TransformNodes(m_Sema.get());
  }
  
  clang::ASTContext *Ctx = &m_CI->getASTContext();
  Consumer->HandleTranslationUnit(*Ctx);
  
  //if (SemaConsumer *SC = dyn_cast<SemaConsumer>(Consumer))
  //   SC->ForgetSema();
  
  // END REPLACEMENT clang::ParseAST(PP, &CI->getASTConsumer(), CI->getASTContext());
  
  
  if (AddConsumer) {
    m_Consumer->Consumers.pop_back();
  }
  //CI->setASTConsumer(0);
  //if (CI->hasPreprocessor()) {
  //   CI->getPreprocessor().EndSourceFile();
  //}
  //CI->clearOutputFiles(/*EraseFiles=*/CI->getDiagnostics().getNumErrors());
  m_CI->getDiagnosticClient().EndSourceFile();
  unsigned err_count = m_CI->getDiagnostics().getNumErrors();
  if (err_count) {
    fprintf(stderr, "Interpreter::compileString: Parse failed!\n");
    return 0;
  }
  return m_CI.get();  
}
