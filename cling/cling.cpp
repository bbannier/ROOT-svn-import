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

int main( int argc, char **argv )
{
  //----------------------------------------------------------------------------
  // Check the commandline parameters
  //----------------------------------------------------------------------------
  if( argc != 2 )
  {
    std::cerr << "[!] You have to specify the input file!" << std::endl;
    return 1;
  }

  //----------------------------------------------------------------------------
  // Set up the language options for gnu c99
  //----------------------------------------------------------------------------
  clang::LangOptions langInfo;
  langInfo.C99         = 1;
  langInfo.HexFloats   = 1;
  langInfo.BCPLComment = 1; // Only for C99/C++.
  langInfo.Digraphs    = 1; // C94, C99, C++.

  //----------------------------------------------------------------------------
  // Create the header searching lists
  //----------------------------------------------------------------------------
  clang::FileManager      fileMgr;
  clang::HeaderSearch     headerInfo( fileMgr );
  clang::InitHeaderSearch hiInit( headerInfo );

  hiInit.AddDefaultEnvVarPaths( langInfo );
  hiInit.AddDefaultSystemIncludePaths( langInfo );
  hiInit.AddPath( "/build/ljanyst/llvm/svn00/install/Headers",
                  clang::InitHeaderSearch::System, false, false, false );
  hiInit.Realize();

  //----------------------------------------------------------------------------
  // Create the stuff for the preprocessor
  //----------------------------------------------------------------------------
  llvm::OwningPtr<clang::DiagnosticClient> diagClient( new clang::TextDiagnosticPrinter() );
  llvm::OwningPtr<clang::TargetInfo>       targetInfo( clang::TargetInfo::CreateTargetInfo( "x86_64-unknown-linux-gnu" ) );
  llvm::OwningPtr<clang::SourceManager>    sourceManager( new clang::SourceManager() );
  clang::Diagnostic                        diag( diagClient.get() );
  diag.setSuppressSystemWarnings( true );

  //----------------------------------------------------------------------------
  // Create the preprocessor
  //----------------------------------------------------------------------------
  llvm::OwningPtr<clang::Preprocessor> pp( new clang::Preprocessor( diag, langInfo, *targetInfo.get(),
                                                                    *sourceManager.get(), headerInfo ) );

  //----------------------------------------------------------------------------
  // Feed in the file
  //----------------------------------------------------------------------------
  const clang::FileEntry *file = fileMgr.getFile( argv[1] );
  if( file )
    sourceManager->createMainFileID( file, clang::SourceLocation() );

  if( sourceManager->getMainFileID() == 0 )
  {
    std::cerr << "[!] The input file you have specified does not exist!" << std::endl;
    return 1;
  }

  //----------------------------------------------------------------------------
  // Create an AST consumer
  //----------------------------------------------------------------------------
  llvm::OwningPtr<clang::CodeGenerator> consumer( CreateLLVMCodeGen( diag, langInfo, argv[1], false ) );

  //----------------------------------------------------------------------------
  // Parse
  //----------------------------------------------------------------------------
  clang::ParseAST( *pp.get(), consumer.get() );

  //----------------------------------------------------------------------------
  // Get the parsed module
  //----------------------------------------------------------------------------
  llvm::Module* module = consumer->ReleaseModule();
  if(!module)
    return 0;

  //-----------------------------------------------------------------------------
  // Create the execution engine
  //-----------------------------------------------------------------------------
  llvm::OwningPtr<llvm::ExecutionEngine> engine( llvm::ExecutionEngine::create( module ) );

  if( !engine )
  {
    std::cout << "[!] Unable to create the execution engine!" << std::endl;
    return 1;
  }

  //-----------------------------------------------------------------------------
  // Look for the main function
  //-----------------------------------------------------------------------------
  llvm::Function* func( module->getFunction( "main" ) );
  if( !func )
  {
    std::cerr << "[!] Cannot find the entry function!" << std::endl;
    return 1;
  }

  std::vector<std::string> params;
  params.push_back( "executable" );

  return engine->runFunctionAsMain( func,  params, 0 );
}
