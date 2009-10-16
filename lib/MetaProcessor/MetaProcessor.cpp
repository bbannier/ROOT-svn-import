//------------------------------------------------------------------------------
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id: Interpreter.cpp 27029 2008-12-19 13:16:34Z axel $
// author:  Axel Naumann <axel@cern.ch>
//------------------------------------------------------------------------------

#include <cling/MetaProcessor/MetaProcessor.h>

#include <cling/Interpreter/Interpreter.h>

#include <llvm/System/DynamicLibrary.h>
#include <llvm/System/Path.h>
#include <llvm/Support/MemoryBuffer.h>

#include <iostream>

#include <sys/stat.h>
#include <stdio.h>
#include <cling/EditLine/EditLine.h>

namespace llvm {
   class Module;
}

//---------------------------------------------------------------------------
// Construct an interface for an interpreter
//---------------------------------------------------------------------------
cling::MetaProcessor::MetaProcessor(Interpreter& interp):
   m_Interp(&interp),
   m_QuitRequested(false),
   m_contLevel(-1)
{
}


//---------------------------------------------------------------------------
// Destruct the interface
//---------------------------------------------------------------------------
cling::MetaProcessor::~MetaProcessor()
{
}

//---------------------------------------------------------------------------
// Compile and execute some code or process some meta command.
// Return 0 when successful; 1+indent level when a continuation is needed,
// and -1 in case an error was encountered.
//---------------------------------------------------------------------------
int cling::MetaProcessor::process(const char* code)
{
   if (ProcessMeta(code)) return 0;
   
   //----------------------------------------------------------------------
   // Check if the statement is complete.  Use the continuation prompt 
   // otherwise
   //----------------------------------------------------------------------

   std::string src = ""; // genSource("");     
   
   m_input.append( code );
   
   int indentLevel;
   std::vector<clang::FunctionDecl *> fnDecls;
   bool shouldBeTopLevel = false;
   switch (m_Interp->analyzeInput(src, m_input, indentLevel, &fnDecls)) {
      case Interpreter::Incomplete:
         return 1 + indentLevel;
      case Interpreter::TopLevel:
         shouldBeTopLevel = true;
      case Interpreter::Stmt:
         break;
   }
   
   
    //----------------------------------------------------------------------
   // Parse and run it
   //----------------------------------------------------------------------
   std::string errMsg;
   llvm::Module* module = m_Interp->linkSource( m_input, &errMsg );

   if(!module) {
      std::cerr << std::endl;
      std::cerr << "[!] Errors occured while parsing your code!" << std::endl;
      if (!errMsg.empty())
         std::cerr << "[!] " << errMsg << std::endl;
      std::cerr << std::endl;
      return -1;
   }
   m_Interp->executeModuleMain( module, "imain" );
   m_input.clear();
   return 0;
}


//---------------------------------------------------------------------------
// Process possible meta commands (.L,...)
//---------------------------------------------------------------------------
bool cling::MetaProcessor::ProcessMeta(const std::string& input)
{
   if (input[0] != '.') return false;
   switch (input[1]) {
   case 'L':
      {
         size_t first = 3;
         while (isspace(input[first])) ++first;
         size_t last = input.length();
         while (last && isspace(input[last - 1])) --last;
         if (!last) {
            std::cerr << "[i] Failure: no file name given!" << std::endl;
         } else {
            std::string filename = input.substr(first, last - first);
            llvm::sys::Path path(filename);
            if (path.isDynamicLibrary()) {
               std::string errMsg;
               if (!llvm::sys::DynamicLibrary::LoadLibraryPermanently(filename.c_str(), &errMsg))
                  std::cerr << "[i] Success!" << std::endl;
               else
                  std::cerr << "[i] Failure: " << errMsg << std::endl;
            } else {
               // TODO: Need to double check that it is a text file ...
               if( m_Interp->addUnit( filename ) )
                  std::cerr << "[i] Success!" << std::endl;
               else
                  std::cerr << "[i] Failure" << std::endl;
            }
         }
         break;
      }
   case 'x':
   case 'X':
      {
         size_t first = 3;
         while (isspace(input[first])) ++first;
         size_t last = input.length();
         while (last && isspace(input[last - 1])) --last;
         if (!last) {
            std::cerr << "[i] Failure: no file name given!" << std::endl;
         } else {
            m_Interp->executeFile(input.substr(first, last - first));
         }
         break;
      }
   case 'U':
      {
         llvm::sys::Path path(input.substr(3));
         if (path.isDynamicLibrary()) {
            std::cerr << "[i] Failure: cannot unload shared libraries yet!" << std::endl;
         }
         m_Interp->removeUnit( input.substr( 3 ) );
         break;
      }
   case 'q':
      m_QuitRequested = true;
      break;
   default:
      return false;
   }
   return true;
}

