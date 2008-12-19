//------------------------------------------------------------------------------
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Lukasz Janyst <ljanyst@cern.ch>
//------------------------------------------------------------------------------

#include <iostream>
#include <vector>
#include <string>

#include <clang/Basic/LangOptions.h>
#include <clang/Basic/TargetInfo.h>

#include <llvm/ADT/OwningPtr.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/Module.h>
#include <llvm/Function.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/System/DynamicLibrary.h>

#include <cling/Interpreter/Interpreter.h>
#include <cling/UserInterface/UserInterface.h>

//------------------------------------------------------------------------------
// String constants
//------------------------------------------------------------------------------
std::string code_prefix = "#include <stdio.h>\nint main(int argc, char** argv) {\n";
std::string code_suffix = ";\nreturn 0; } ";

/*
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
*/

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
   // Set up the compiler
   //---------------------------------------------------------------------------
   clang::LangOptions langInfo;
   langInfo.C99         = 1;
   langInfo.HexFloats   = 1;
   langInfo.BCPLComment = 1; // Only for C99/C++.
   langInfo.Digraphs    = 1; // C94, C99, C++.

   llvm::OwningPtr<clang::TargetInfo> targetInfo;
   targetInfo.reset( clang::TargetInfo::CreateTargetInfo( HOST_TARGET ) );

   cling::Compiler compiler( langInfo, targetInfo.get() );
   cling::UserInterface ui(compiler);

   //---------------------------------------------------------------------------
   // We're supposed to parse a file
   //---------------------------------------------------------------------------
   if( !interactive ) {
      llvm::Module* module = compiler.link( argv[1] );
      if(!module) {
         std::cerr << "[!] Errors occured while parsing your code!" << std::endl;
         return 1;
      }
      return ui.ExecuteModuleMain( module );
   }
   //----------------------------------------------------------------------------
   // We're interactive
   //----------------------------------------------------------------------------
   else {
      ui.runInteractively();
      /*
      std::cerr << "Type a C code and press enter to run it." << std::endl;
      std::cerr << "Type .q, exit or ctrl+D to quit" << std::endl;
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

         bool quitRequested = false;

         //----------------------------------------------------------------------
         // Check if we are a preprocessor command
         //----------------------------------------------------------------------
         if( input.size() >= 2 && input[0] == '.' ) {
            switch( input[1] ) {
               case 'L':
                  {
                     llvm::sys::Path path(input.substr(3));
                     if (path.isDynamicLibrary()) {
                        std::string errMsg;
                        if (llvm::sys::DynamicLibrary::LoadLibraryPermanently(input.substr(3).c_str(), &errMsg))
                           std::cerr << "[i] Success!" << std::endl;
                        else
                           std::cerr << "[i] Failure: " << errMsg << std::endl;
                     } else {
                        if( compiler.addUnit( input.substr( 3 ) ) )
                           std::cerr << "[i] Success!" << std::endl;
                        else
                           std::cerr << "[i] Failure" << std::endl;
                     }
                     break;
                  }
               case 'U':
                  {
                     llvm::sys::Path path(input.substr(3));
                     if (path.isDynamicLibrary()) {
                        std::cerr << "[i] Failure: cannot unload shared libraries yet!" << std::endl;
                     }
                     compiler.removeUnit( input.substr( 3 ) );
                     break;
                  }
               case 'q':
                  quitRequested = true;
            }
            continue;
         }


         if (quitRequested)
            break;

         //----------------------------------------------------------------------
         // Wrap the code
         //----------------------------------------------------------------------
         std::string wrapped = code_prefix + input + code_suffix;
         llvm::MemoryBuffer* buff;
         buff  = llvm::MemoryBuffer::getMemBufferCopy( &*wrapped.begin(),
                                                       &*wrapped.end(),
                                                       "CLING" );

         //----------------------------------------------------------------------
         // Parse and run it
         //----------------------------------------------------------------------
         llvm::Module* module = compiler.link( buff );

         if(!module) {
            std::cerr << std::endl;
            std::cerr << "[!] Errors occured while parsing your code!" << std::endl;
            std::cerr << std::endl;
            continue;
         }
         executeModuleMain( module );
      }
      */
   }
   return 0;
}
