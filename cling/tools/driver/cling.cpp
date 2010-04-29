//------------------------------------------------------------------------------
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Lukasz Janyst <ljanyst@cern.ch>
//------------------------------------------------------------------------------

#include <iostream>
#include <vector>
#include <string>

#include <clang/Basic/LangOptions.h>

#include <llvm/ADT/OwningPtr.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/Module.h>
#include <llvm/Function.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/System/DynamicLibrary.h>
#include <llvm/System/Signals.h>
#include <llvm/Support/PrettyStackTrace.h>

#include <cling/Interpreter/Interpreter.h>
#include <cling/UserInterface/UserInterface.h>

//------------------------------------------------------------------------------
// Let the show begin
//------------------------------------------------------------------------------
int main( int argc, char **argv )
{

   llvm::sys::PrintStackTraceOnErrorSignal();
   llvm::PrettyStackTraceProgram X(argc, argv);

   // TODO: factor out, use llvm's / clang option parsing tools

   //---------------------------------------------------------------------------
   // Determine which file to interpret
   //---------------------------------------------------------------------------
   int fileArgN = 1;
   while (fileArgN < argc && argv[fileArgN][0] == '-')
      ++fileArgN;

   //---------------------------------------------------------------------------
   // Check if we should run in the "interactive" mode
   //---------------------------------------------------------------------------
   // no file? interactive!
   bool interactive = (fileArgN == argc);

   interactive |= argc > 1 && std::string(argv[1]) == "-i";

   // for now, -l and -i are mutually exclusive
   bool nologo = argc < 2 || std::string(argv[1]) == "-l";

   //---------------------------------------------------------------------------
   // Set up the interpreter
   //---------------------------------------------------------------------------
   clang::LangOptions langInfo;
   //langInfo.C99         = 1;
   //langInfo.HexFloats   = 1;
   langInfo.BCPLComment = 1; // Only for C99/C++.
   langInfo.Digraphs    = 1; // C94, C99, C++.
   langInfo.CPlusPlus   = 1;
   //langInfo.CPlusPlus0x = 01;
   langInfo.CXXOperatorNames = 1;
   langInfo.Bool = 1;

   langInfo.NeXTRuntime = 1;
   langInfo.NoInline = 1;
   
   langInfo.Exceptions = 1;
   langInfo.GNUMode = 1;
   langInfo.NoInline = 1;
   langInfo.GNUInline = 1;
   langInfo.DollarIdents = 1;

   langInfo.POSIXThreads = 1;

   cling::Interpreter interpreter;
   clang::CompilerInstance* CI = interpreter.getCI();
   clang::HeaderSearchOptions& headerOpts = CI->getHeaderSearchOpts();

   const bool IsUserSupplied = false;
   const bool IsFramework = false;
   headerOpts.AddPath (".", clang::frontend::Angled, IsUserSupplied, IsFramework);   

   //---------------------------------------------------------------------------
   // We're supposed to parse a file
   //---------------------------------------------------------------------------
   if( !interactive ) {
      return interpreter.executeFile(argv[fileArgN]);
   }
   //----------------------------------------------------------------------------
   // We're interactive
   //----------------------------------------------------------------------------
   //else {
      cling::UserInterface ui(interpreter);
      ui.runInteractively(nologo);
   //}
   return 0;
}
