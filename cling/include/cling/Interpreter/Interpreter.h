//--------------------------------------------------------------------*- C++ -*-
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Lukasz Janyst <ljanyst@cern.ch>
//------------------------------------------------------------------------------

#ifndef CLING_COMPILER_H
#define CLING_COMPILER_H

#include <clang/Basic/LangOptions.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/AST/Type.h>
#include <llvm/ADT/OwningPtr.h>

#include <string>
#include <map>
#include <vector>
#include <utility>

//------------------------------------------------------------------------------
// Forward declarations from LLVM
//------------------------------------------------------------------------------
namespace llvm
{
   class MemoryBuffer;
   class Module;
   class LLVMContext;
}

//------------------------------------------------------------------------------
// Forward declarations from CLANG
//------------------------------------------------------------------------------
namespace clang
{
   class TranslationUnitDecl;
   class DeclContext;
   class Decl;
   class SourceManager;
   class FileManager;
   class TargetInfo;
   class Sema;
   class ASTContext;
   class FunctionDecl;
   class Preprocessor;
   class Token;
   class HeaderSearch;
}

namespace cling
{
   class ParseEnvironment;
   
   //---------------------------------------------------------------------------
   //! Class for managing many translation units supporting automatic
   //! forward declarations and linking
   //---------------------------------------------------------------------------
   class Interpreter
   {
   public:
      
      //---------------------------------------------------------------------
      //! Mark the type of statement found in the command line
      //---------------------------------------------------------------------
      enum InputType { Incomplete, TopLevel, Stmt }; 
      
      //---------------------------------------------------------------------
      //! Unit identifier - must be constructable from a std::string
      //---------------------------------------------------------------------
      typedef std::string UnitID_t;

      //---------------------------------------------------------------------
      //! Constructor
      //!
      //! param language the language definition
      //---------------------------------------------------------------------
      Interpreter(clang::LangOptions language);

      //---------------------------------------------------------------------
      // Destructor
      //---------------------------------------------------------------------
      virtual ~Interpreter();

      //---------------------------------------------------------------------
      //! Add a translation unit
      //!
      //! @param  fileName path of the file to be parsed
      //! @return true if the file was successfuly processed, false otherwise
      //---------------------------------------------------------------------
      virtual bool addUnit( const std::string& fileName );

      //---------------------------------------------------------------------
      //! Add a translation unit
      //!
      //! @param  buffer a buffer to be processed
      //! @param  id     the ID that was assigned to the translation unit
      //! @return true if the buffer was successfuly processed, false
      //!         otherwise
      //---------------------------------------------------------------------
      virtual bool addUnit( const llvm::MemoryBuffer* buffer,
                            UnitID_t& id );

      //---------------------------------------------------------------------
      //! Remove the translation unit
      //!
      //! @param id id of the translation unit to be removed
      //! @return true if the unit was removed, false otherwise
      //---------------------------------------------------------------------
      virtual bool removeUnit( const UnitID_t& id );

      //---------------------------------------------------------------------
      //! Check if the input is complete or not.
      //!
      //! @param context code to prefix to the input
      //! @param line code to analyze
      //! @param indentLevel keep track of the block nesting
      //! @param return function declaration found in the code
      //! @return true if the unit was removed, false otherwise
      //---------------------------------------------------------------------
      InputType analyzeInput(const std::string& contextSource,
                             const std::string& line,
                             int& indentLevel,
                             std::vector<clang::FunctionDecl*> *fds);
      
      //---------------------------------------------------------------------
      //! Get a compiled  module linking together all translation units
      //!
      //! @return the module or 0, consult the diagnostics if a 0 pointer
      //!         is returned
      //---------------------------------------------------------------------
      virtual llvm::Module* getModule();

      //---------------------------------------------------------------------
      //! Compile the filename and link it to all the modules known to the
      //! compiler but do not add it to the list
      //!
      //! @return the resulting module or 0, consult the diagnostics if a 0
      //!         pointer is returned
      //---------------------------------------------------------------------
      virtual llvm::Module* linkFile( const std::string& fileName,
                                            std::string* errMsg = 0);

      //---------------------------------------------------------------------
      //! Compile the buffer and link it to all the modules known to the
      //! compiler but do not add it to the list
      //!
      //! @param  buff the input buffer, the compiler takes the ownership
      //!         over it
      //! @return the resulting module or 0, consult the diagnostics if a 0
      //!         pointer is returned
      //---------------------------------------------------------------------
      virtual llvm::Module* linkSource( const std::string& src,
                                              std::string* errMsg = 0);

      //---------------------------------------------------------------------
      //! Link the module to all the modules known to the compiler but do
      //! not add it to the list
      //!
      //! @return the resulting module or 0, consult the diagnostics if a 0
      //!         pointer is returned
      //---------------------------------------------------------------------
      virtual llvm::Module* linkModule( llvm::Module *module,
                                        std::string* errMsg = 0);

      //---------------------------------------------------------------------
      //! Execute a module containing a function funcname.
      //!
      //! @return funcname()'s return value
      int executeModuleMain( llvm::Module *module,
                             const std::string& funcname = "main" );

      //---------------------------------------------------------------------
      //! Execute a file containing a function funcname.
      //! If funcname == "()", funcname is the stem (no extension) of
      //! filename.
      //!
      //! @return funcname()'s return value
      int executeFile( const std::string& filename,
                       const std::string& funcname = "()" );

      //---------------------------------------------------------------------
      //! Set the diagnostic client (deletes 
      //!
      //! @param clinet      the new client
      //! @param freeCurrent if true the current client will be deleted
      //---------------------------------------------------------------------
      virtual void setDiagnosticClient( clang::DiagnosticClient* client,
                                        bool freeCurrent )
      {
         if( freeCurrent )
            delete m_diagClient;
         m_diagClient = client;
      }

      //---------------------------------------------------------------------
      //! Get the diagnostic client
      //---------------------------------------------------------------------
      virtual const clang::DiagnosticClient* getDiagnosticClient() const
      {
         return m_diagClient;
      }

      //---------------------------------------------------------------------
      //! Get the diagnostic client
      //---------------------------------------------------------------------
      virtual clang::DiagnosticClient* getDiagnosticClient()
      {
         return m_diagClient;
      }

   protected:
      struct UnitInfo_t
      {
         llvm::Module*                 module; //!< compiled module
         clang::TranslationUnitDecl*   ast;    //!< full ast
         std::vector<clang::Decl*>     decls;  //!< function declarations
      };

      std::map<UnitID_t, UnitInfo_t> m_units;
      clang::LangOptions             m_lang;
      clang::TargetInfo*             m_target;
      clang::FileManager*            m_fileMgr;
      clang::DiagnosticClient*       m_diagClient;
      std::vector<std::pair<clang::Decl*, const clang::ASTContext*> >      m_decls;
      llvm::Module*                  m_module;
      llvm::LLVMContext*             m_llvmContext;


   private:
      ParseEnvironment* parseSource( const std::string& source );
      ParseEnvironment* parseFile( const std::string& fileName );
      ParseEnvironment* parse( clang::SourceManager* srcMgr );
      llvm::Module*     compile( clang::TranslationUnitDecl* tu );
      bool              addUnit( const UnitID_t& id,
                                 clang::TranslationUnitDecl* tu );

      std::vector<clang::Decl*>
      extractDeclarations( clang::TranslationUnitDecl* tu );
      void insertDeclarations( clang::TranslationUnitDecl* tu, clang::Sema* sema );
      void dumpTU( clang::DeclContext* dc );
      clang::QualType typeCopy( clang::QualType source,
                                const clang::ASTContext& sourceContext,
                                clang::ASTContext& targetContext );
      llvm::Module* copyModule( const llvm::Module* src );
      
      int analyzeTokens(clang::Preprocessor& PP,
                        clang::Token& lastTok,
                        int& indentLevel,
                        bool& tokWasDo);
      
      bool splitInput(const std::string& input, std::vector<std::string>& statements);
            
   };
}

#endif // CLING_COMPILER_H
