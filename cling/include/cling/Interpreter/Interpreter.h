//--------------------------------------------------------------------*- C++ -*-
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Lukasz Janyst <ljanyst@cern.ch>
//------------------------------------------------------------------------------

#ifndef CLING_INTERPRETER_H
#define CLING_INTERPRETER_H

#include "llvm/ADT/OwningPtr.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ExecutionEngine/GenericValue.h"

#include <string>

namespace llvm {
  class raw_ostream;
}

namespace clang {
  class ASTConsumer;
  class ASTContext;
  class CompilerInstance;
  class Decl;
  class DeclContext;
  class NamedDecl;
  class PragmaNamespace;
  class SourceLocation;
  class QualType;
}

namespace cling {
  namespace runtime {
    namespace internal {
      template <typename T>
      T EvaluateProxyT(const char* expr, void* varaddr[], clang::DeclContext* DC); 
    }
  }
  class ExecutionContext;
  class IncrementalParser;
  class InputValidator;
  class Value;
  //---------------------------------------------------------------------------
  //! Class for managing many translation units supporting automatic
  //! forward declarations and linking
  //---------------------------------------------------------------------------
  class Interpreter {
  public:
    class NamedDeclResult {
    private:
      Interpreter* m_Interpreter;
      clang::ASTContext& m_Context;
      clang::DeclContext* m_CurDeclContext;
      clang::NamedDecl* m_Result;
      NamedDeclResult(llvm::StringRef Decl, Interpreter* interp, clang::DeclContext* Within = 0);
    public:
      NamedDeclResult& LookupDecl(llvm::StringRef);
      operator clang::NamedDecl* () const { return getSingleDecl(); }
      clang::NamedDecl* getSingleDecl() const;
      template<typename T> T* getAs(){
        return static_cast<T*>(getSingleDecl());
      }
      
      friend class Interpreter;
    };

    
    //---------------------------------------------------------------------
    //! Constructor
    //---------------------------------------------------------------------
    Interpreter(const char* llvmdir = 0);
    
    //---------------------------------------------------------------------
    //! Destructor
    //---------------------------------------------------------------------
    virtual ~Interpreter();

    const char* getVersion() const;
    std::string createUniqueName();
    void AddIncludePath(const char *incpath);
    int processLine(const std::string& input_line);
    
    int loadFile(const std::string& filename,
                 const std::string* trailcode = 0,
                 bool allowSharedLib = true);
    
    int executeFile(const std::string& filename);
    clang::QualType getQualType(llvm::StringRef type);
    
    bool setDynamicLookup(bool value = true);
    bool setPrintAST(bool print = true);
    
    void dumpAST(bool showAST = true, int last = -1);
    
    clang::CompilerInstance* getCI() const;

    clang::PragmaNamespace& getPragmaHandler() const { return *m_PragmaHandler; }
    void installLazyFunctionCreator(void* (*fp)(const std::string&));
    
    llvm::raw_ostream& getValuePrinterStream() const { return *m_ValuePrintStream; }

    void RequestContinuation(const clang::SourceLocation&);
    
  private:
    llvm::OwningPtr<ExecutionContext> m_ExecutionContext; // compiler instance.
    llvm::OwningPtr<IncrementalParser> m_IncrParser; // incremental AST and its parser
    llvm::OwningPtr<InputValidator> m_InputValidator; // balanced paren etc
    clang::PragmaNamespace* m_PragmaHandler; // pragma cling ..., owned by Preprocessor
    unsigned long long m_UniqueCounter; // number of generated call wrappers
    bool m_printAST; // whether to print the AST to be processed
    llvm::OwningPtr<llvm::raw_ostream> m_ValuePrintStream; // stream to dump values into
    clang::Decl *m_LastDump; // last dump point
    clang::ASTConsumer* m_ASTDumper;
    
  private:
    
    void createWrappedSrc(const std::string& src, std::string& wrapped,
                          std::string& stmtFunc);  
    clang::CompilerInstance* compileFile(const std::string& filename,
                                         const std::string* trailcode = 0);
    Value EvaluateWithContext(const char* expr, 
                              void* varaddr[], 
                              clang::DeclContext* DC);
    // Define EvaluateProxyT as friend because it will use EvalCore
    template<typename T> 
    friend T runtime::internal::EvaluateProxyT(const char* expr,
                                               void* varaddr[],
                                               clang::DeclContext* DC); 
    
  public:
    Value Evaluate(const char* expr, clang::DeclContext* DC);
    NamedDeclResult LookupDecl(llvm::StringRef Decl, clang::DeclContext* Within = 0);
  };
  
} // namespace cling

#endif // CLING_INTERPRETER_H
