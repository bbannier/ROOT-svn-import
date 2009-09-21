//------------------------------------------------------------------------------
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Lukasz Janyst <ljanyst@cern.ch>
//------------------------------------------------------------------------------

#include <cling/Interpreter/Interpreter.h>

#include <llvm/Support/raw_ostream.h>
#include <llvm/ADT/OwningPtr.h>
#include <llvm/Linker.h>
#include <llvm/Module.h>
#include <llvm/Function.h>
#include <llvm/Bitcode/BitstreamWriter.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/Config/config.h>
#include <llvm/Transforms/Utils/Cloning.h>

#include <clang/Basic/FileManager.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Index/TranslationUnit.h>
#include <clang/Lex/HeaderSearch.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/Frontend/InitHeaderSearch.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <clang/CodeGen/ModuleBuilder.h>
#include <clang/AST/ASTContext.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/AST/Decl.h>
#include <clang/AST/DeclarationName.h>
#include <clang/AST/Stmt.h>
#include <clang/Parse/Parser.h>
#include <clang/Basic/TargetInfo.h>

// Private CLANG headers
#include <Sema/Sema.h>

#include <iostream>
#include <stdexcept>

namespace cling
{
   //---------------------------------------------------------------------------
   // Constructor
   //---------------------------------------------------------------------------
   Interpreter::Interpreter( clang::LangOptions language, clang::TargetInfo* target /*= 0*/):
      m_lang( language ), m_target( target ), m_module( 0 )
   {
      m_fileMgr    = new clang::FileManager();
      m_diagClient = new clang::TextDiagnosticPrinter( llvm::errs() );
      if (!m_target) {
         m_ownedTarget.reset( clang::TargetInfo::CreateTargetInfo( LLVM_HOSTTRIPLE ) );
         m_target = m_ownedTarget.get();
      }
   }

   //---------------------------------------------------------------------------
   // Destructor
   //---------------------------------------------------------------------------
   Interpreter::~Interpreter()
   {
      delete m_fileMgr;
      delete m_diagClient;
   }

   //---------------------------------------------------------------------------
   // Add a translation unit
   //---------------------------------------------------------------------------
   bool Interpreter::addUnit( const std::string& fileName )
   {
      //------------------------------------------------------------------------
      // Check if the unit file name was specified and if we alread have
      // an unit named like that
      //------------------------------------------------------------------------
      if( fileName.empty() )
         return false;

      std::map<UnitID_t, UnitInfo_t>::iterator it;
      it = m_units.find( fileName );
      if( it != m_units.end() )
         return false;

      //------------------------------------------------------------------------
      // Process the unit
      //------------------------------------------------------------------------
      clang::idx::TranslationUnit* tu = parse( fileName );
      return addUnit( fileName, tu );
   }

   //---------------------------------------------------------------------------
   // Add a translation unit
   //---------------------------------------------------------------------------
   bool Interpreter::addUnit( const llvm::MemoryBuffer* buffer, UnitID_t& id )
   {
      return false;
   }

   //---------------------------------------------------------------------------
   // Remove the translation unit
   //---------------------------------------------------------------------------
   bool Interpreter::removeUnit( const UnitID_t& id )
   {
      return false;
   }

   //---------------------------------------------------------------------------
   // Get a compiled  module linking together all translation units
   //---------------------------------------------------------------------------
   llvm::Module* Interpreter::getModule()
   {
      return m_module;
   }

   //---------------------------------------------------------------------------
   // Compile the filename and link it to all the modules known to the
   // compiler but do not add it to the list
   //---------------------------------------------------------------------------
   llvm::Module* Interpreter::link( const std::string& fileName,
                                    std::string* errMsg )
   {
      clang::idx::TranslationUnit* tu     = parse( fileName );
      llvm::Module*           module = compile( tu );
      llvm::Module*           result = link( module, errMsg );
      delete tu;
      delete module;
      return result;
   }

   //---------------------------------------------------------------------------
   // Compile the buffer and link it to all the modules known to the
   // compiler but do not add it to the list
   //---------------------------------------------------------------------------
   llvm::Module* Interpreter::link( const llvm::MemoryBuffer* buff,
                                    std::string* errMsg )
   {
      clang::idx::TranslationUnit* tu     = parse( buff );
      llvm::Module*           module = compile( tu );
      llvm::Module*           result = link( module, errMsg );
      delete tu;
      delete module;
      return result;
   }

   //---------------------------------------------------------------------------
   // Link the module to all the modules known to the compiler but do
   // not add it to the list
   //---------------------------------------------------------------------------
   llvm::Module* Interpreter::link( llvm::Module *module, std::string* errMsg )
   {
      if( !module ) {
         if (errMsg) *errMsg = "Module is NULL";
         return 0;
      }

      //------------------------------------------------------------------------
      // We have some module so we should link the current one to it
      //------------------------------------------------------------------------
      llvm::Linker linker( "executable", llvm::CloneModule(module) );

      if( m_module )
         if (linker.LinkInModule( llvm::CloneModule( m_module ), errMsg ))
            return 0;

      return linker.releaseModule();
   }

   //---------------------------------------------------------------------------
   // Parse memory buffer
   //---------------------------------------------------------------------------
   clang::idx::TranslationUnit* Interpreter::parse( const llvm::MemoryBuffer* buff )
   {
      //------------------------------------------------------------------------
      // Create a file manager
      //------------------------------------------------------------------------
      llvm::OwningPtr<clang::SourceManager> srcMgr( new clang::SourceManager() );

      //------------------------------------------------------------------------
      // Register with the source manager
      //------------------------------------------------------------------------
      if( buff )
         srcMgr->createMainFileIDForMemBuffer( buff );

      if( srcMgr->getMainFileID().isInvalid() )
         return 0;

      return parse( srcMgr.get() );
   }

   //---------------------------------------------------------------------------
   // Parse file
   //---------------------------------------------------------------------------
   clang::idx::TranslationUnit* Interpreter::parse( const std::string& fileName )
   {
      //------------------------------------------------------------------------
      // Create a file manager
      //------------------------------------------------------------------------
      llvm::OwningPtr<clang::SourceManager> srcMgr( new clang::SourceManager() );

      //------------------------------------------------------------------------
      // Feed in the file
      //------------------------------------------------------------------------
      const clang::FileEntry *file = m_fileMgr->getFile( fileName );
      if( file )
         srcMgr->createMainFileID( file, clang::SourceLocation() );

      if( srcMgr->getMainFileID().isInvalid() )
         return 0;

      return parse( srcMgr.get() );
   }

   //---------------------------------------------------------------------------
   // Parse
   //---------------------------------------------------------------------------
   clang::idx::TranslationUnit* Interpreter::parse( clang::SourceManager* srcMgr )
   {
      //------------------------------------------------------------------------
      // Return immediately if no target was specified
      //------------------------------------------------------------------------
      if( !m_target )
         return 0;

      //------------------------------------------------------------------------
      // Create the header database
      //------------------------------------------------------------------------
      clang::HeaderSearch     headerInfo( *m_fileMgr );
      clang::InitHeaderSearch hiInit( headerInfo );

      hiInit.AddDefaultEnvVarPaths( m_lang );
      hiInit.AddDefaultSystemIncludePaths( m_lang );
      hiInit.AddPath( CLANG_SYS_HEADERS, clang::InitHeaderSearch::System,
                      false, false, false );
      hiInit.Realize();

      //-------------------------------------------------------------------------
      // Create diagnostics
      //-------------------------------------------------------------------------
      clang::Diagnostic diag( m_diagClient );
      diag.setSuppressSystemWarnings( true );

      //-------------------------------------------------------------------------
      // Create the preprocessor - a memory leak yuck!!
      //-------------------------------------------------------------------------
      //llvm::OwningPtr<clang::Preprocessor>  pp;
      //pp.reset( new clang::Preprocessor( diag, m_lang, *m_target, *srcMgr,
      //                                   headerInfo ) );

      clang::Preprocessor*  pp =
         new clang::Preprocessor( diag, m_lang, *m_target, *srcMgr,
                                  headerInfo);


      //-------------------------------------------------------------------------
      // Parse the AST and generate the code
      //-------------------------------------------------------------------------
      clang::ASTContext* context;
      clang::ASTConsumer dummyConsumer;
      context = new clang::ASTContext( m_lang, *srcMgr, *m_target,
                                       pp->getIdentifierTable(),
                                       pp->getSelectorTable(),
                                       m_target->getTargetBuiltins());

      clang::idx::TranslationUnit *tu = new clang::idx::TranslationUnit(*context);
      clang::Sema   sema(*pp, *context, dummyConsumer);
      clang::Parser p(*pp, sema);

      pp->EnterMainSourceFile();
      p.Initialize();

      insertDeclarations( tu, &sema );

      clang::Parser::DeclTy *adecl;
  
      while( !p.ParseTopLevelDecl(adecl) ) {}

      //      dumpTU( tu );

      return tu;
   }

   //----------------------------------------------------------------------------
   // Compile the translation unit
   //----------------------------------------------------------------------------
   llvm::Module* Interpreter::compile( clang::idx::TranslationUnit* tu )
   {
      if( !tu )
         return 0;

      //-------------------------------------------------------------------------
      // Create diagnostics
      //-------------------------------------------------------------------------
      clang::Diagnostic diag( m_diagClient );
      diag.setSuppressSystemWarnings( true );

      //-------------------------------------------------------------------------
      // Create the code generator
      //-------------------------------------------------------------------------
      llvm::OwningPtr<clang::CodeGenerator> codeGen;
      codeGen.reset( CreateLLVMCodeGen( diag, m_lang, "-", false ) );

      //-------------------------------------------------------------------------
      // Loop over the AST
      //-------------------------------------------------------------------------
      clang::idx::TranslationUnit::iterator it;
      codeGen->InitializeTU( *tu );

      for( it = tu->begin(); it != tu->end(); ++it )
         codeGen->HandleTopLevelDecl( *it );

      codeGen->HandleTranslationUnit(*tu);

      //-------------------------------------------------------------------------
      // Return the module
      //-------------------------------------------------------------------------
      llvm::Module* module = codeGen->ReleaseModule();
      return module;
   }

   //----------------------------------------------------------------------------
   // Add the translation unit
   //----------------------------------------------------------------------------
   bool Interpreter::addUnit( const UnitID_t& id, clang::idx::TranslationUnit* tu )
   {
      //-------------------------------------------------------------------------
      // Check if we've got a valid translation unit
      //-------------------------------------------------------------------------
      if( !tu )
         return false;

      //-------------------------------------------------------------------------
      // Process the unit
      //-------------------------------------------------------------------------
      UnitInfo_t uinfo;
      uinfo.ast    = tu;
      uinfo.decls  = extractDeclarations( tu );
      uinfo.module = compile( tu );

      if( !uinfo.module ) {
         delete tu;
         return false;
      }

      //-------------------------------------------------------------------------
      // Linke in the module
      //-------------------------------------------------------------------------
      llvm::Module* module = link( uinfo.module );
      if( !module ) {
         delete tu;
         delete module;
         return false;
      }

      //-------------------------------------------------------------------------
      // Everything went ok - register
      //-------------------------------------------------------------------------
      m_units[id] = uinfo;
      m_module = module;
      
      return true;
   }

   //----------------------------------------------------------------------------
   // Extract the function declarations
   //----------------------------------------------------------------------------
   std::vector<clang::Decl*>
   Interpreter::extractDeclarations( clang::idx::TranslationUnit* tu )
   {
      std::vector<clang::Decl*> vect;
      if( !tu )
         return vect;

      std::set<clang::Decl*> decls_before;
      std::vector<std::pair<clang::Decl*, const clang::ASTContext*> >::iterator
         dit, ditend = m_decls.end();
      for (dit = m_decls.begin(); dit != ditend; ++dit)
         decls_before.insert(dit->first);

      //-------------------------------------------------------------------------
      // Loop over the declarations
      //-------------------------------------------------------------------------
      clang::ASTContext& context = tu->getContext();
      const clang::SourceManager& srcMgr = context.getSourceManager();
      clang::idx::TranslationUnitDecl* tud = context.getTranslationUnitDecl();
      clang::idx::TranslationUnit::iterator it, itend = tud->decls_end();
      for( it = tud->decls_begin(); it != itend; ++it ) {
         clang::FunctionDecl* decl = dynamic_cast<clang::FunctionDecl*>( *it );
         if( decl && decls_before.find(decl) == decls_before.end()) {
            vect.push_back( decl );
            m_decls.push_back( std::make_pair(*it, &context ) );
         }         
      }
   }

   //----------------------------------------------------------------------------
   // Insert the implicit declarations to the translation unit
   //----------------------------------------------------------------------------
   void Interpreter::insertDeclarations( clang::idx::TranslationUnit* tu, clang::Sema* sema )
   {
      clang::ASTContext& context = tu->getContext();
      clang::IdentifierTable&      table    = context.Idents;
      clang::DeclarationNameTable& declTab  = context.DeclarationNames;
      clang::idx::TranslationUnitDecl* tud = context.getTranslationUnitDecl();
      std::vector<std::pair<clang::Decl*, const clang::ASTContext*> >::iterator it;
      for( it = m_decls.begin(); it != m_decls.end(); ++it ) {
         clang::FunctionDecl* func = dynamic_cast<clang::FunctionDecl*>( it->first );
         if( func ) {
            clang::IdentifierInfo&       id       = table.get(std::string(func->getNameAsString()));
            clang::DeclarationName       dName    = declTab.getIdentifier( &id );
            
            clang::FunctionDecl* decl = clang::FunctionDecl::Create( context,
                                                                     tud,
                                                                     func->getLocation(),
                                                                     dName,
                                                                     typeCopy( func->getType(), *it->second, context ) );
            tud->addDecl( decl );
            sema->IdResolver.AddDecl( decl );
            if (sema->TUScope)
               sema->TUScope->AddDecl( decl );
         }
         
      }
   }

   //----------------------------------------------------------------------------
   // Dump the translation unit
   //----------------------------------------------------------------------------
   void Interpreter::dumpTU( clang::idx::TranslationUnit* tu )
   {
      clang::idx::TranslationUnit::iterator it;
      for( it = tu->begin(); it != tu->end(); ++it ) {
         clang::Stmt* body = (*it) ? (*it)->getBody() : 0;
         if( body ) {
            std::cerr << "--- AST ---" << std::endl;
            body->dumpAll();
            std::cerr<<std::endl;
         }
      }
   }

   //-----------------------------------------------------------------------------
   // Copy given type to the target AST context using serializers - grr ugly :)
   //-----------------------------------------------------------------------------
   clang::QualType Interpreter::typeCopy( clang::QualType source,
                                          const clang::ASTContext& sourceContext,
                                          clang::ASTContext& targetContext )
   {
      const clang::BuiltinType*       bt;
      const clang::PointerType*       pt;
      const clang::FunctionType*      ft1;
      const clang::FunctionTypeProto* ft2;

      //--------------------------------------------------------------------------
      // Deal with a builtin type
      //--------------------------------------------------------------------------
      if( (bt = source.getTypePtr()->getAsBuiltinType()) ) {
         return clang::QualType( targetContext.getBuiltinType( bt->getKind() ).getTypePtr(),
                                 source.getCVRQualifiers() );
      }

      //--------------------------------------------------------------------------
      // Deal with a pointer type
      //--------------------------------------------------------------------------
      else if( source.getTypePtr()->isPointerType() ) {
         pt = source.getTypePtr()->getAsPointerType();
         clang::QualType pointee = typeCopy( pt->getPointeeType(),
                                             sourceContext,
                                             targetContext );
         clang::QualType result = targetContext.getPointerType( pointee );
         result.setCVRQualifiers( source.getCVRQualifiers() );
         return result;
      }

      //--------------------------------------------------------------------------
      // Deal with a function type
      //--------------------------------------------------------------------------
      else if( source.getTypePtr()->isFunctionType() ) {
         ft1  = dynamic_cast<clang::FunctionType*>(source.getTypePtr());
         ft2  = dynamic_cast<clang::FunctionTypeProto*>(source.getTypePtr());

         //-----------------------------------------------------------------------
         // No parameters
         //-----------------------------------------------------------------------
         if( !ft2 )
            return targetContext.getFunctionTypeNoProto( typeCopy( ft1->getResultType(),
                                                                   sourceContext,
                                                                   targetContext ) );

         //-----------------------------------------------------------------------
         // We have some parameters
         //-----------------------------------------------------------------------
         std::vector<clang::QualType> args;
         clang::FunctionTypeProto::arg_type_iterator it;
         for( it = ft2->arg_type_begin(); it != ft2->arg_type_end(); ++it )
            args.push_back( typeCopy( *it, sourceContext, targetContext ) );

         return targetContext.getFunctionType( typeCopy( ft2->getResultType(),
                                                         sourceContext,
                                                         targetContext ),
                                               &args.front(), args.size(),
                                               ft2->isVariadic(),
                                               ft2->getTypeQuals() );
      }

      throw std::runtime_error( std::string("Unable to convert type: ") + source.getAsString() );
      return source;
   }

   /*
     Trying to use llvm::CloneModule() instead...

   //-----------------------------------------------------------------------------
   // Create a copy of an existing module - the linker destroys the source!!
   //-----------------------------------------------------------------------------
   llvm::Module* Interpreter::copyModule( const llvm::Module* src )
   {
      //--------------------------------------------------------------------------
      // Create the buffer and the writer
      //--------------------------------------------------------------------------
      std::vector<unsigned char> buffer;
      llvm::BitstreamWriter      stream(buffer);
      buffer.reserve(256*1024);

      //--------------------------------------------------------------------------
      // Write the module
      //--------------------------------------------------------------------------
      llvm::WriteBitcodeToStream( src, stream );

      //--------------------------------------------------------------------------
      // Create a memory buffer and read the module back
      //--------------------------------------------------------------------------
      buffer.push_back( 0 ); // required by the llvm::MemoryBuffer
      llvm::MemoryBuffer* buff = llvm::MemoryBuffer::getMemBuffer( (const char*)&buffer.front(),
                                                                   (const char*)&buffer.back(),
                                                                   src->getModuleIdentifier().c_str() );
      llvm::Module* result = ParseBitcodeFile( buff );

      //--------------------------------------------------------------------------
      // Cleanup and return
      //--------------------------------------------------------------------------
      delete buff;
      return result;
   }
   */

   //---------------------------------------------------------------------------
   // Call the Interpreter on a Module
   //---------------------------------------------------------------------------
   int Interpreter::executeModuleMain( llvm::Module *module,
                                       const std::string& name)
   {
      //---------------------------------------------------------------------------
      // Create the execution engine
      //---------------------------------------------------------------------------
      llvm::OwningPtr<llvm::ExecutionEngine> engine( llvm::ExecutionEngine::create( module ) );

      if( !engine ) {
         std::cout << "[!] Unable to create the execution engine!" << std::endl;
         return 1;
      }

      //---------------------------------------------------------------------------
      // Look for the imain function
      //---------------------------------------------------------------------------
      llvm::Function* func( module->getFunction( name ) );
      if( !func ) {
         std::cerr << "[!] Cannot find the entry function" << name << "!" << std::endl;
         return 1;
      }

      //---------------------------------------------------------------------------
      // Create argv
      //---------------------------------------------------------------------------
      std::vector<std::string> params;
      params.push_back( "executable" );

      return engine->runFunctionAsMain( func,  params, 0 );   
   }


   //---------------------------------------------------------------------------
   // Call the Interpreter on a File
   //---------------------------------------------------------------------------
   int Interpreter::executeFile( const std::string& filename,
                                 const std::string& funcname)
   {
      llvm::Module* module = link( filename );
      if(!module) {
         std::cerr << "[!] Errors occured while parsing file " << filename << "!" << std::endl;
         return 1;
      }
      std::string myfuncname(funcname);
      if (funcname == "()") {
         size_t posSlash = filename.find_last_of('/');
         ++posSlash; // npos to 0, good!
         size_t posDot = filename.find('.'); // npos is OK, too.
         myfuncname = filename.substr(posSlash, posDot);
      }

      return executeModuleMain( module, myfuncname );
   }
}
