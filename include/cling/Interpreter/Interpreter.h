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
}

//------------------------------------------------------------------------------
// Forward declarations from CLANG
//------------------------------------------------------------------------------
namespace clang
{
   class TranslationUnit;
   class Decl;
   class SourceManager;
   class FileManager;
   class TargetInfo;
   class Sema;
   class ASTContext;
}

namespace cling
{
   //---------------------------------------------------------------------------
   //! Class for managing many translation units supporting automatic
   //! forward declarations and linking
   //---------------------------------------------------------------------------
   class Interpreter
   {
   public:
      //---------------------------------------------------------------------
      //! Unit identifier - must be constructable from a std::string
      //---------------------------------------------------------------------
      typedef std::string UnitID_t;

      //---------------------------------------------------------------------
      //! Constructor
      //!
      //! param language the language definition
      //! param target   the target definition, the user remains the owner
      //!                of the object!
      //---------------------------------------------------------------------
      Interpreter( clang::LangOptions language, clang::TargetInfo* target );

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
      virtual llvm::Module* link( const std::string& fileName,
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
      virtual llvm::Module* link( const llvm::MemoryBuffer* buff,
                                  std::string* errMsg = 0);

      //---------------------------------------------------------------------
      //! Link the module to all the modules known to the compiler but do
      //! not add it to the list
      //!
      //! @return the resulting module or 0, consult the diagnostics if a 0
      //!         pointer is returned
      //---------------------------------------------------------------------
      virtual llvm::Module* link( llvm::Module *module,
                                  std::string* errMsg = 0);

      //---------------------------------------------------------------------
      //! Execute a module containing a function main(int, char*[]).
      //!
      //! @return main()'s return value
      int executeModuleMain( llvm::Module *module,
                             const std::string& name = "main" );

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
         llvm::Module*             module; //!< compiled module
         clang::TranslationUnit*   ast;    //!< full ast
         std::vector<clang::Decl*> decls;  //!< function declarations
      };

      std::map<UnitID_t, UnitInfo_t> m_units;
      clang::LangOptions             m_lang;
      clang::TargetInfo*             m_target;
      clang::FileManager*            m_fileMgr;
      clang::DiagnosticClient*       m_diagClient;
      std::vector<std::pair<clang::Decl*, clang::ASTContext*> >      m_decls;
      llvm::Module*                  m_module;

   private:
      clang::TranslationUnit* parse( const llvm::MemoryBuffer* buff );
      clang::TranslationUnit* parse( const std::string& fileName );
      clang::TranslationUnit* parse( clang::SourceManager* srcMgr );
      llvm::Module*           compile( clang::TranslationUnit* tu );
      bool                    addUnit( const UnitID_t& id,
                                       clang::TranslationUnit* tu );

      std::vector<clang::Decl*>
      extractDeclarations( clang::TranslationUnit* tu );
      void insertDeclarations( clang::TranslationUnit* tu, clang::Sema* sema );
      void dumpTU( clang::TranslationUnit* tu );
      clang::QualType typeCopy( clang::QualType source,
                                clang::ASTContext& sourceContext,
                                clang::ASTContext& targetContext );
      llvm::Module* copyModule( const llvm::Module* src );
   };
}

#endif // CLING_COMPILER_H
