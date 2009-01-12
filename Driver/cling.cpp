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

//------------------------------------------------------------------------------
// Let the show begin
//------------------------------------------------------------------------------
int main( int argc, char **argv )
{

   //---------------------------------------------------------------------------
   // Check if we should run in the "interactive" mode
   //---------------------------------------------------------------------------
   bool interactive = (argc == 1);
   if( !interactive && std::string( argv[1] ) == "-i" )
      interactive = true;

   //---------------------------------------------------------------------------
   // Set up the interpreter
   //---------------------------------------------------------------------------
   clang::LangOptions langInfo;
   langInfo.C99         = 1;
   langInfo.HexFloats   = 1;
   langInfo.BCPLComment = 1; // Only for C99/C++.
   langInfo.Digraphs    = 1; // C94, C99, C++.

   llvm::OwningPtr<clang::TargetInfo> targetInfo;
   targetInfo.reset( clang::TargetInfo::CreateTargetInfo( HOST_TARGET ) );

   cling::Interpreter interpreter( langInfo, targetInfo.get() );

   //---------------------------------------------------------------------------
   // We're supposed to parse a file
   //---------------------------------------------------------------------------
   if( !interactive ) {
      llvm::Module* module = interpreter.link( argv[1] );
      if(!module) {
         std::cerr << "[!] Errors occured while parsing your code!" << std::endl;
         return 1;
      }
      return interpreter.executeModuleMain( module );
   }
   //----------------------------------------------------------------------------
   // We're interactive
   //----------------------------------------------------------------------------
   else {
      cling::UserInterface ui(interpreter);
      ui.runInteractively();
   }
   return 0;
}
