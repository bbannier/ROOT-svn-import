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
#include "llvm/Support/Casting.h"
#include "cling/Interpreter/InvocationOptions.h"

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
  class DynamicExprInfo;
  namespace runtime {
    namespace internal {
      template <typename T>
      T EvaluateT(DynamicExprInfo* ExprInfo, clang::DeclContext* DC);
      class LifetimeHandler;
    }
  }
  class ExecutionContext;
  class IncrementalParser;
  class InputValidator;
  class InterpreterCallbacks;
  class Value;

  class DynamicExprInfo {
  private:
    const char* m_Cache;

    /// \brief The unknown symbol surrounding environment, which has to be 
    /// included when replacing a node.
    ///
    /// For example:
    /// @code
    /// int a = 5;
    /// const char* b = dep->Symbol(a);
    /// @endcode
    /// This information is kept using the syntax: "dep->Symbol(*(int*)@)",
    /// where @ denotes that the runtime address the variable "a" is needed.
    const char*  m_Template;

    /// \brief Stores the addresses of the variables that m_Template describes.
    void** m_Addresses;
  public:
    DynamicExprInfo(const char* templ, void* addresses[]) : 
      m_Cache(0), m_Template(templ), m_Addresses(addresses){}
    const char* getExpr();
  };

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
      template<class T> T* getAs(){
        return llvm::dyn_cast<T>(getSingleDecl());
      }
      
      friend class Interpreter;
    };

    //---------------------------------------------------------------------
    //! Constructor
    //---------------------------------------------------------------------
    Interpreter(int argc, const char* const *argv,
                const char* startupPCH = 0, const char* llvmdir = 0);
    
    //---------------------------------------------------------------------
    //! Destructor
    //---------------------------------------------------------------------
    virtual ~Interpreter();

    const InvocationOptions& getOptions() const { return m_Opts; }
    InvocationOptions& getOptions() { return m_Opts; }

    const char* getVersion() const;
    std::string createUniqueName();
    void AddIncludePath(const char *incpath);
    void DumpIncludePath();
    int processLine(const std::string& input_line);
    
    int loadFile(const std::string& filename,
                 const std::string* trailcode = 0,
                 bool allowSharedLib = true);
    
    int executeFile(const std::string& fileWithArgs);
    clang::QualType getQualType(llvm::StringRef type);
    
    void enableDynamicLookup(bool value = true);
    bool isDynamicLookupEnabled();

    bool isPrintingAST() { return m_printAST; }
    void enablePrintAST(bool print = true);
    
    void dumpAST(bool showAST = true, int last = -1);
    
    clang::CompilerInstance* getCI() const;

    clang::PragmaNamespace& getPragmaHandler() const { return *m_PragmaHandler; }
    void installLazyFunctionCreator(void* (*fp)(const std::string&));
    
    llvm::raw_ostream& getValuePrinterStream() const { return *m_ValuePrintStream; }

    void RequestContinuation(const clang::SourceLocation&);

    void writeStartupPCH();
    
  private:
    InvocationOptions m_Opts; // Interpreter options
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
    void handleFrontendOptions();
    void processStartupPCH();
    void createWrappedSrc(const std::string& src, std::string& wrapped,
                          std::string& stmtFunc);  
    clang::CompilerInstance* compileFile(const std::string& filename,
                                         const std::string* trailcode = 0);
    friend class runtime::internal::LifetimeHandler;
    
  public:
    Value Evaluate(const char* expr, clang::DeclContext* DC);
    NamedDeclResult LookupDecl(llvm::StringRef Decl, clang::DeclContext* Within = 0);
    void setCallbacks(InterpreterCallbacks* C);
  };
  
} // namespace cling

#endif // CLING_INTERPRETER_H
