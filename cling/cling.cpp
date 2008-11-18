//------------------------------------------------------------------------------
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Lukasz Janyst <ljanyst@cern.ch>
//------------------------------------------------------------------------------

#include <iostream>
#include <vector>
#include <string>

#include <clang/Driver/TextDiagnosticPrinter.h>
#include <clang/Driver/CompileOptions.h>
#include <clang/Basic/LangOptions.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/Basic/FileManager.h>
#include <clang/Lex/HeaderSearch.h>
#include <clang/Driver/InitHeaderSearch.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/CodeGen/ModuleBuilder.h>
#include <clang/Sema/ParseAST.h>

#include <llvm/ADT/OwningPtr.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/Module.h>
#include <llvm/Function.h>
#include <llvm/Support/MemoryBuffer.h>

//------------------------------------------------------------------------------
// String constants
//------------------------------------------------------------------------------
std::string code_prefix = "#include <stdio.h>\nint main(int argc, char** argv) {\n";
std::string code_suffix = "\nreturn 0; } ";

//------------------------------------------------------------------------------
// Execute the module
//------------------------------------------------------------------------------
int executeModuleMain( llvm::Module *module )
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
   // Look for the main function
   //---------------------------------------------------------------------------
   llvm::Function* func( module->getFunction( "main" ) );
   if( !func ) {
      std::cerr << "[!] Cannot find the entry function!" << std::endl;
      return 1;
   }

   //---------------------------------------------------------------------------
   // Create argv
   //---------------------------------------------------------------------------
   std::vector<std::string> params;
   params.push_back( "executable" );

   return engine->runFunctionAsMain( func,  params, 0 );
}

//------------------------------------------------------------------------------
// Parse
//------------------------------------------------------------------------------
llvm::Module* parse( clang::LangOptions*      langInfo,
                     clang::TargetInfo*       targetInfo,
                     clang::DiagnosticClient* diagClient,
                     clang::SourceManager*    srcMgr,
                     clang::FileManager*      fileMgr )
{
   //---------------------------------------------------------------------------
   // Create the header database
   //---------------------------------------------------------------------------
   clang::HeaderSearch     headerInfo( *fileMgr );
   clang::InitHeaderSearch hiInit( headerInfo );

   hiInit.AddDefaultEnvVarPaths( *langInfo );
   hiInit.AddDefaultSystemIncludePaths( *langInfo );
   hiInit.AddPath( CLANG_SYS_HEADERS, clang::InitHeaderSearch::System,
                   false, false, false );
   hiInit.Realize();

   //----------------------------------------------------------------------------
   // Create diagnostics
   //----------------------------------------------------------------------------
   clang::Diagnostic diag( diagClient );
   diag.setSuppressSystemWarnings( true );

   //----------------------------------------------------------------------------
   // Create the preprocessor and code generator
   //----------------------------------------------------------------------------
   llvm::OwningPtr<clang::Preprocessor> pp( new clang::Preprocessor( diag, *langInfo, *targetInfo,
                                                                    *srcMgr, headerInfo ) );
   llvm::OwningPtr<clang::CodeGenerator> consumer( CreateLLVMCodeGen( diag, *langInfo, "-", false ) );

   //----------------------------------------------------------------------------
   // Pars and return the module
   //----------------------------------------------------------------------------
   clang::ParseAST( *pp.get(), consumer.get() );

   llvm::Module* module = consumer->ReleaseModule();
   return module;
}

//------------------------------------------------------------------------------
// Parse file
//------------------------------------------------------------------------------
llvm::Module* parseFile( std::string filePath,
                         clang::LangOptions*      langInfo,
                         clang::TargetInfo*       targetInfo,
                         clang::DiagnosticClient* diag,
                         clang::SourceManager*    srcMgr,
                         clang::FileManager*      fileMgr )
{
   //---------------------------------------------------------------------------
   // Feed in the file
   //---------------------------------------------------------------------------
   const clang::FileEntry *file = fileMgr->getFile( filePath );
   if( file )
      srcMgr->createMainFileID( file, clang::SourceLocation() );

   if( srcMgr->getMainFileID() == 0 ) {
      std::cerr << "[!] The input file you have specified does not exist!" << std::endl;
      return 0;
   }
   return parse( langInfo, targetInfo, diag, srcMgr, fileMgr );
}


//------------------------------------------------------------------------------
// Parse a string
//------------------------------------------------------------------------------
llvm::Module* parseString( std::string              code,
                           clang::LangOptions*      langInfo,
                           clang::TargetInfo*       targetInfo,
                           clang::DiagnosticClient* diag,
                           clang::SourceManager*    srcMgr,
                           clang::FileManager*      fileMgr )
{
   //---------------------------------------------------------------------------
   // Wrap the code and create a memory buffer
   //---------------------------------------------------------------------------
   std::string wrapped = code_prefix + code + code_suffix;
   llvm::MemoryBuffer* buff =llvm::MemoryBuffer::getMemBufferCopy( &*wrapped.begin(), &*wrapped.rbegin(), "MemoryBuffer" );

   //---------------------------------------------------------------------------
   // Register with the source manager
   //---------------------------------------------------------------------------
   if( buff )
     srcMgr->createMainFileIDForMemBuffer( buff );

   if( srcMgr->getMainFileID() == 0 ) {
      std::cerr << "[!] Cannot create a memory buffer from your input!" << std::endl;
      return 0;
   }

   return parse( langInfo, targetInfo, diag, srcMgr, fileMgr );
}

//------------------------------------------------------------------------------
// Let the show begin
//------------------------------------------------------------------------------
int main( int argc, char **argv )
{
   //---------------------------------------------------------------------------
   // Check the commandline parameters
   //---------------------------------------------------------------------------
   if( argc != 2 ) {
      std::cerr << "[!] You have to specify the input file!" << std::endl;
      return 1;
   }

   //---------------------------------------------------------------------------
   // Check if we should run in the "interactive" mode
   //---------------------------------------------------------------------------
   bool interactive = false;
   if( std::string( argv[1] ) == "-i" )
      interactive = true;

   //---------------------------------------------------------------------------
   // Set up the language options for gnu c99
   //---------------------------------------------------------------------------
   clang::LangOptions langInfo;
   langInfo.C99         = 1;
   langInfo.HexFloats   = 1;
   langInfo.BCPLComment = 1; // Only for C99/C++.
   langInfo.Digraphs    = 1; // C94, C99, C++.

   //---------------------------------------------------------------------------
   // Create the stuff for the preprocessor
   //---------------------------------------------------------------------------
   llvm::OwningPtr<clang::DiagnosticClient> diagClient( new clang::TextDiagnosticPrinter() );
   llvm::OwningPtr<clang::TargetInfo>       targetInfo( clang::TargetInfo::CreateTargetInfo( HOST_TARGET ) );
   llvm::OwningPtr<clang::SourceManager>    srcMgr( new clang::SourceManager() );
   clang::FileManager                       fileMgr;

   //---------------------------------------------------------------------------
   // We're supposed to parse a file
   //---------------------------------------------------------------------------
   if( !interactive ) {
      llvm::Module* module = parseFile( argv[1], &langInfo, targetInfo.get(),
                                        diagClient.get(), srcMgr.get(), &fileMgr );
      if(!module) {
         std::cerr << "[!] Errors occured while parsing your code!" << std::endl;
         return 1;
      }
      return executeModuleMain( module );
   }
   //----------------------------------------------------------------------------
   // We're interactive
   //----------------------------------------------------------------------------
   else {
      std::cerr << "Type a C code and press enter to run it." << std::endl;
      std::cerr << "Type exit or ctrl+D to quit" << std::endl;
      std::string input;

      //------------------------------------------------------------------------
      // Loop
      //------------------------------------------------------------------------
      while( 1 ) {
         //---------------------------------------------------------------------
         // Get the user input
         //---------------------------------------------------------------------
         std::cout << "[cling] $ ";
         std::getline( std::cin, input );
         if( !std::cin.good() || input == "exit" ) {
            std::cerr << std::endl;
            break;
         }

         //----------------------------------------------------------------------
         // Parse and run it
         //----------------------------------------------------------------------
         srcMgr->clearIDTables();
         llvm::Module* module = parseString( input, &langInfo, targetInfo.get(),
                                             diagClient.get(), srcMgr.get(), &fileMgr );

         if(!module) {
            std::cerr << std::endl;
            std::cerr << "[!] Errors occured while parsing your code!" << std::endl;
            std::cerr << std::endl;
            continue;
         }
         executeModuleMain( module );
      }
   }
   return 0;
}
