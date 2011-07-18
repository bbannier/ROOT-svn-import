//--------------------------------------------------------------------*- C++ -*-
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Lukasz Janyst <ljanyst@cern.ch>
//------------------------------------------------------------------------------

#ifndef CLING_INTERPRETER_H
#define CLING_INTERPRETER_H

#include "llvm/ADT/OwningPtr.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Casting.h"
#include "cling/Interpreter/InvocationOptions.h"

#include <string>

namespace llvm {
  class raw_ostream;
  struct GenericValue;
}

namespace clang {
  class ASTConsumer;
  class ASTContext;
  class CompilerInstance;
  class CompoundStmt;
  class Decl;
  class DeclContext;
  class Expr;
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

  ///\brief Helper structure used to provide specific context of the evaluated
  /// expression, when needed.
  ///
  /// Consider:
  /// @code
  /// int a = 5;
  /// const char* b = dep->Symbol(a);
  /// @endcode
  /// In the particular case we need to pass a context to the evaluator of the
  /// unknown symbol. The addresses of the items in the context are not known at
  /// compile time, so they cannot be embedded directly. Instead of that we
  /// need to create an array of addresses of those context items (mainly
  /// variables) and insert them into the evaluated expression at runtime
  /// This information is kept using the syntax: "dep->Symbol(*(int*)@)",
  /// where @ denotes that the runtime address the variable "a" is needed.
  ///
  class DynamicExprInfo {
  private:
    const char* m_Cache;

    /// \brief The expression template.
    const char*  m_Template;

    /// \brief The variable list.
    void** m_Addresses;
  public:
    DynamicExprInfo(const char* templ, void* addresses[]) : 
      m_Cache(0), m_Template(templ), m_Addresses(addresses){}

    ///\brief Performs the insertions of the context in the expression just
    /// before evaluation. To be used only at runtime.
    ///
    const char* getExpr();
  };

  //---------------------------------------------------------------------------
  //! Class for managing many translation units supporting automatic
  //! forward declarations and linking
  //---------------------------------------------------------------------------
  class Interpreter {
  public:

    ///\brief Implements named parameter idiom - allows the idiom 
    /// LookupDecl().LookupDecl()...
    /// 
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
    bool processLine(const std::string& input_line);
    
    bool loadFile(const std::string& filename,
                  const std::string* trailcode = 0,
                  bool allowSharedLib = true);
    
    bool executeFile(const std::string& fileWithArgs);
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

    void runStaticInitializersOnce() const;
    
  private:
    InvocationOptions m_Opts; // Interpreter options
    llvm::OwningPtr<ExecutionContext> m_ExecutionContext;
    llvm::OwningPtr<IncrementalParser> m_IncrParser; // incremental AST and its parser
    llvm::OwningPtr<InputValidator> m_InputValidator; // balanced paren etc
    clang::PragmaNamespace* m_PragmaHandler; // pragma cling ..., owned by Preprocessor
    unsigned long long m_UniqueCounter; // number of generated call wrappers
    bool m_printAST; // whether to print the AST to be processed
    bool m_ValuePrinterEnabled; // whether the value printer is loaded
    llvm::OwningPtr<llvm::raw_ostream> m_ValuePrintStream; // stream to dump values into
    clang::Decl *m_LastDump; // last dump point

  private:
    void handleFrontendOptions();
    void processStartupPCH();
    bool handleLine(llvm::StringRef Input, llvm::StringRef FunctionName);
    void WrapInput(std::string& input, std::string& fname);
    bool RunFunction(llvm::StringRef fname, llvm::GenericValue* res = 0);
    friend class runtime::internal::LifetimeHandler;
    
  public:
    ///\brief Evaluates given expression within given declaration context.
    ///
    /// @param[in] expr The expression.
    /// @param[in] DC The declaration context in which the expression is going
    /// to be evaluated.
    Value Evaluate(const char* expr, clang::DeclContext* DC);

    ///\brief Looks up declaration within given declaration context. Does top
    /// down lookup.
    ///
    ///@param[in] Decl Declaration name.
    ///@param[in] Within Starting declaration context.
    ///
    NamedDeclResult LookupDecl(llvm::StringRef Decl, clang::DeclContext* Within = 0);

    ///\brief Sets callbacks needed for the dynamic lookup.
    void setCallbacks(InterpreterCallbacks* C);
  };
  
} // namespace cling

#endif // CLING_INTERPRETER_H
