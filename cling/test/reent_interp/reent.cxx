#include <iostream>
#include <string>

#include <cling/Interpreter/Interpreter.h>

#include <clang/Basic/LangOptions.h>
#include <clang/Basic/TargetInfo.h>

#include <llvm/ADT/OwningPtr.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/Module.h>
#include <llvm/Support/MemoryBuffer.h>


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


extern "C" {
int call_interp(const char* code) {
   clang::LangOptions langInfo;
   langInfo.C99         = 1;
   langInfo.HexFloats   = 1;
   langInfo.BCPLComment = 1; // Only for C99/C++.
   langInfo.Digraphs    = 1; // C94, C99, C++.

   llvm::OwningPtr<clang::TargetInfo> targetInfo;
   targetInfo.reset( clang::TargetInfo::CreateTargetInfo( HOST_TARGET ) );
   cling::Compiler interp( langInfo, targetInfo.get() );
   std::string wrapped_code("int main(int argc, char* argv[]) { ");
   wrapped_code += code;
   wrapped_code += "return 0;}";
   llvm::MemoryBuffer* buff;
   buff  = llvm::MemoryBuffer::getMemBufferCopy( &*wrapped_code.begin(),
                                                 &*wrapped_code.end(),
                                                 "reent.cxx" );
   //----------------------------------------------------------------------
   // Parse and run it
   //----------------------------------------------------------------------
   llvm::Module* module = interp.link( buff );

   if(!module) {
      std::cerr << std::endl;
      std::cerr << "[!] Errors occured while parsing your code!" << std::endl;
      std::cerr << std::endl;
      return -1;
   }
   return executeModuleMain( module );
};
}
