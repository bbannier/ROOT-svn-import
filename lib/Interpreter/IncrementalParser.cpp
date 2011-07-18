//------------------------------------------------------------------------------
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Axel Naumann <axel@cern.ch>
//------------------------------------------------------------------------------

#include "IncrementalParser.h"

#include "llvm/LLVMContext.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/raw_os_ostream.h"

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclGroup.h"
#include "clang/AST/Mangle.h"
#include "clang/CodeGen/ModuleBuilder.h"
#include "clang/Parse/Parser.h"
#include "clang/Parse/ParseAST.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Lex/Pragma.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Sema/SemaConsumer.h"
#include "clang/Serialization/ASTWriter.h"

#include "cling/Interpreter/CIFactory.h"
#include "cling/Interpreter/Diagnostics.h"
#include "cling/Interpreter/Interpreter.h"
#include "ASTDumper.h"
#include "ChainedConsumer.h"
#include "DeclExtractor.h"
#include "DynamicLookup.h"
#include "ExecutionContext.h"
#include "ValuePrinterSynthesizer.h"

#include <stdio.h>
#include <sstream>
#include <iostream>

using namespace clang;

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
        if (m_Alloc) {
           fprintf(stderr, "MutableMemoryBuffer reallocation doesn't work (Preprocessor isn't told about the new location, should instead just add a new memory buffer)!\n");
        }
        m_Alloc += 64*1024;
        B = (char*)realloc(B, m_Alloc);
      }
      memcpy(B + oldlen, code.data(), applen);
      B[newlen - 1] = 0;
      init(B, B + newlen - 1, /*RequireNullTerminator=*/ true);
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

    virtual BufferKind getBufferKind () const {
      return MemoryBuffer_Malloc;
    }
  };

  
  IncrementalParser::IncrementalParser(Interpreter* interp, 
                                       PragmaNamespace* Pragma, 
                                       int argc, const char* const *argv,
                                       const char* llvmdir):
    m_Interpreter(interp),
    m_DynamicLookupEnabled(false),
    m_Consumer(0),
    m_LastTopLevelDecl(0),
    m_FirstTopLevelDecl(0),
    m_UsingStartupPCH(false)
  {
    //m_CIFactory.reset(new CIFactory(0, 0, llvmdir));
    m_MemoryBuffer.push_back(new MutableMemoryBuffer("//cling!\n", "CLING") );
    clang::CompilerInstance* CI = CIFactory::createCI(m_MemoryBuffer[0], Pragma,
                                                      argc, argv, llvmdir);
    assert(CI && "CompilerInstance is (null)!");
    m_CI.reset(CI);

    m_MBFileID = CI->getSourceManager().getMainFileID();
    //CI->getSourceManager().getBuffer(m_MBFileID, clang::SourceLocation()); // do we need it?


    if (CI->getSourceManager().getMainFileID().isInvalid()) {
      fprintf(stderr, "Interpreter::compileString: Failed to create main "
              "file id!\n");
      return;
    }
    
    m_Consumer = dyn_cast<ChainedConsumer>(&CI->getASTConsumer());
    assert(m_Consumer && "Expected ChainedConsumer!");
    // Add consumers to the ChainedConsumer, which owns them
    DeclExtractor* DE = new DeclExtractor();
    DE->Attach(m_Consumer);
    addConsumer(ChainedConsumer::kDeclExtractor, DE);
    addConsumer(ChainedConsumer::kValuePrinterSynthesizer,
                new ValuePrinterSynthesizer(interp));
    addConsumer(ChainedConsumer::kASTDumper, new ASTDumper());
    clang::CodeGenerator* CG = CreateLLVMCodeGen(CI->getDiagnostics(), 
                                                 "cling input",
                                                 CI->getCodeGenOpts(), 
                            /*Owned by codegen*/ * new llvm::LLVMContext()
                                                 );

    addConsumer(ChainedConsumer::kCodeGenerator, CG);
    m_Consumer->Initialize(CI->getASTContext());
    m_Consumer->InitializeSema(CI->getSema());
    // Initialize the parser.
    m_Parser.reset(new clang::Parser(CI->getPreprocessor(), CI->getSema()));
    CI->getPreprocessor().EnterMainSourceFile();
    m_Parser->Initialize();

    //if (clang::SemaConsumer *SC = dyn_cast<clang::SemaConsumer>(m_Consumer))
    //  SC->InitializeSema(CI->getSema()); // do we really need this? We know 
    // that we will have ChainedConsumer, which is initialized in createCI
    
    // Create the visitor that will transform all dependents that are left.
    m_Transformer.reset(new DynamicExprTransformer(interp, &CI->getSema()));
  }
  
  IncrementalParser::~IncrementalParser() {
     GetCodeGenerator()->ReleaseModule();
  }
  
  void IncrementalParser::Initialize(const char* startupPCH) {

    // Init the consumers    

    loadStartupPCH(startupPCH);
    if (!m_UsingStartupPCH) {
      CompileAsIs(""); // Consume initialization.
      // Set up common declarations which are going to be available
      // only at runtime
      // Make sure that the universe won't be included to compile time by using
      // -D __CLING__ as CompilerInstance's arguments
      CompileAsIs("#include \"cling/Interpreter/RuntimeUniverse.h\"");
    }

    // Attach the dynamic lookup
    if (isDynamicLookupEnabled())
      getTransformer()->Initialize();

    m_Consumer->Initialize(m_CI->getASTContext());
    m_Consumer->InitializeSema(m_CI->getSema());
  }

  void IncrementalParser::loadStartupPCH(const char* filename) {
    if (!filename || !filename[0]) return;
    bool Preamble = m_CI->getPreprocessorOpts().PrecompiledPreambleBytes.first !=0;
    llvm::OwningPtr<clang::ExternalASTSource> EAS(
      clang::CompilerInstance::
      createPCHExternalASTSource(filename,
                                 "", /* sysroot */
                                 true, /* disable PCH validation*/
                                 false, /* disable stat cache */
                                 m_CI->getPreprocessor(),
                                 m_CI->getASTContext(),
                                 0, /* deserialization listener */
                                 Preamble
                                 )
                                                  );
    if (EAS) {
       m_CI->getASTContext().setExternalSource(EAS);
       m_UsingStartupPCH = true;
    } else {
      // Valid file name but no (valid) PCH - recreate.
      bool Chaining = m_CI->getInvocation().getFrontendOpts().ChainedPCH &&
        !m_CI->getPreprocessorOpts().ImplicitPCHInclude.empty();
      // We use createOutputFile here because this is exposed via libclang, and we
      // must disable the RemoveFileOnSignal behavior.
      llvm::raw_ostream *OS = m_CI->createOutputFile(filename, /*Binary=*/true,
                                                     /*RemoveFileOnSignal=*/false,
                                                     filename);
      m_StartupPCHGenerator.reset(new clang::PCHGenerator(m_CI->getPreprocessor(),
                                                          filename,
                                                          Chaining,
                                                          "", /*isysroot*/
                                                          OS
                                                          )
                                  );
      m_StartupPCHGenerator->InitializeSema(m_CI->getSema());
      addConsumer(ChainedConsumer::kPCHGenerator, m_StartupPCHGenerator.get());
    }
  }

  void IncrementalParser::writeStartupPCH() {
    if (!m_StartupPCHGenerator) return;
    m_StartupPCHGenerator->HandleTranslationUnit(m_CI->getASTContext());
    m_StartupPCHGenerator.reset(); // deletes StartupPCHGenerator
  }

  clang::CompilerInstance*
  IncrementalParser::CompileLineFromPrompt(llvm::StringRef input) {
    assert(input.str()[0] != '#' 
           && "Preprocessed line! Call CompilePreprocessed instead");
    
    bool p, q;
    p = m_Consumer->EnableConsumer(ChainedConsumer::kDeclExtractor);
    q = m_Consumer->EnableConsumer(ChainedConsumer::kValuePrinterSynthesizer);
    clang::CompilerInstance* Result = Compile(input);
    m_Consumer->RestorePreviousState(ChainedConsumer::kDeclExtractor, p);
    m_Consumer->RestorePreviousState(ChainedConsumer::kValuePrinterSynthesizer, q);

    return Result;    

  }
  
  clang::CompilerInstance*
  IncrementalParser::CompileAsIs(llvm::StringRef input) {
    bool p, q;
    p = m_Consumer->DisableConsumer(ChainedConsumer::kDeclExtractor);
    q = m_Consumer->DisableConsumer(ChainedConsumer::kValuePrinterSynthesizer);
    clang::CompilerInstance* Result = Compile(input);
    m_Consumer->RestorePreviousState(ChainedConsumer::kDeclExtractor, p);
    m_Consumer->RestorePreviousState(ChainedConsumer::kValuePrinterSynthesizer, q);

    return Result;
  }

  clang::CompilerInstance*
  IncrementalParser::Compile(llvm::StringRef input)
  {
    // Add src to the memory buffer, parse it, and add it to
    // the AST. Returns the CompilerInstance (and thus the AST).
    // Diagnostics are reset for each call of parse: they are only covering
    // src.
    clang::Preprocessor& PP = m_CI->getPreprocessor();
    DiagnosticClient& DClient = m_CI->getDiagnosticClient();
    DClient.BeginSourceFile(m_CI->getLangOpts(), &PP);

    if (input.size()) {
      std::ostringstream source_name;
      source_name << "input_line_" << (m_MemoryBuffer.size()+1);
      m_MemoryBuffer.push_back( new MutableMemoryBuffer("//cling!\n", source_name.str()) );
      MutableMemoryBuffer *currentBuffer = m_MemoryBuffer.back();
      currentBuffer->append(input);
      clang::FileID FID = m_CI->getSourceManager().createFileIDForMemBuffer(currentBuffer);
      
      PP.EnterSourceFile(FID, 0, clang::SourceLocation());     
      
      clang::Token &tok = const_cast<clang::Token&>(m_Parser->getCurToken());
      tok.setKind(clang::tok::semi);
    }

    clang::Parser::DeclGroupPtrTy ADecl;
    
    bool atEOF = false;
    if (m_Parser->getCurToken().is(clang::tok::eof)) {
      atEOF = true;
    }
    else {
      atEOF = m_Parser->ParseTopLevelDecl(ADecl);
    }
    // Reset the module builder to clean up global initializers, c'tors, d'tors:
    GetCodeGenerator()->Initialize(getCI()->getASTContext());

    while (!atEOF) {
      // Not end of file.
      // If we got a null return and something *was* parsed, ignore it.  This
      // is due to a top-level semicolon, an action override, or a parse error
      // skipping something.
      if (ADecl) {
        clang::DeclGroupRef DGR = ADecl.getAsVal<clang::DeclGroupRef>();
        for (clang::DeclGroupRef::iterator i=DGR.begin(); i< DGR.end(); ++i) {
          if (!m_FirstTopLevelDecl)
            m_FirstTopLevelDecl = *i;
          
          m_LastTopLevelDecl = *i;
          if (isDynamicLookupEnabled())
            getTransformer()->Visit(m_LastTopLevelDecl);
        } 
        m_Consumer->HandleTopLevelDecl(DGR);
      } // ADecl
      if (m_Parser->getCurToken().is(clang::tok::eof)) {
        atEOF = true;
      }
      else {
        atEOF = m_Parser->ParseTopLevelDecl(ADecl);
      }
    };
    
    getCI()->getSema().PerformPendingInstantiations();
    
    // Process any TopLevelDecls generated by #pragma weak.
    for (llvm::SmallVector<clang::Decl*,2>::iterator
           I = getCI()->getSema().WeakTopLevelDecls().begin(),
           E = getCI()->getSema().WeakTopLevelDecls().end(); I != E; ++I) {
      m_Consumer->HandleTopLevelDecl(clang::DeclGroupRef(*I));
    }

    m_Consumer->HandleTranslationUnit(getCI()->getASTContext());
    DClient.EndSourceFile();

    m_Interpreter->runStaticInitializersOnce();

    m_CI->getDiagnostics().Reset();

    return m_CI.get();
  }

  void IncrementalParser::enableDynamicLookup(bool value) {
    m_DynamicLookupEnabled = value;
    if (isDynamicLookupEnabled())
      getTransformer()->AttachDynIDHandler();
    else
      getTransformer()->DetachDynIDHandler();
  }

  DynamicExprTransformer* 
  IncrementalParser::getOrCreateTransformer(Interpreter* interp) {
    if (!m_Transformer) {
      m_Transformer.reset(new DynamicExprTransformer(interp, 
                                                     &getCI()->getSema()));
      m_Transformer->Initialize();
    }
    return m_Transformer.get();
  }

  void IncrementalParser::addConsumer(ChainedConsumer::EConsumerIndex I, clang::ASTConsumer* consumer) {
    if (m_Consumer->Exists(I))
      return;

    m_Consumer->Add(I, consumer);
    if (I == ChainedConsumer::kCodeGenerator)
      m_Consumer->EnableConsumer(I);

    consumer->Initialize(getCI()->getSema().getASTContext());
    if (getCI()->hasSema()) {
      clang::SemaConsumer* SC = dyn_cast<clang::SemaConsumer>(consumer);
      if (SC) {
        SC->InitializeSema(getCI()->getSema());
      }
    }
  }

  CodeGenerator* IncrementalParser::GetCodeGenerator() { 
    return 
      (CodeGenerator*)m_Consumer->getConsumer(ChainedConsumer::kCodeGenerator); 
  }

} // namespace cling
