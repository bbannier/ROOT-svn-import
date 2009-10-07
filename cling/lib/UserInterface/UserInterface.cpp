//------------------------------------------------------------------------------
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id: Interpreter.cpp 27029 2008-12-19 13:16:34Z axel $
// author:  Axel Naumann <axel@cern.ch>
//------------------------------------------------------------------------------

#include <cling/UserInterface/UserInterface.h>

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
cling::UserInterface::UserInterface(Interpreter& interp, const char* prompt /*= "[cling] $"*/):
   m_Interp(&interp)
{
}


//---------------------------------------------------------------------------
// Destruct the interface
//---------------------------------------------------------------------------
cling::UserInterface::~UserInterface()
{
}

//---------------------------------------------------------------------------
// Interact with the user using a prompt
//---------------------------------------------------------------------------
void cling::UserInterface::runInteractively()
{
   std::cerr << std::endl;
   std::cerr << "**** Welcome to the cling prototype! ****" << std::endl;
   std::cerr << "* Type C code and press enter to run it *" << std::endl;
   std::cerr << "* Type .q, exit or ctrl+D to quit       *" << std::endl;
   std::cerr << "*****************************************" << std::endl;
   struct stat buf;
   static const char* histfile = ".cling_history";
   using_history();
   max_input_history = 100;
   if (stat(histfile, &buf) == 0) {
      read_history(histfile);
   }
   m_QuitRequested = false;
   const static std::string defaultPrompt("[cling]$ ");
   const static std::string defaultCont("... ");
   std::string prompt = defaultPrompt;
   while (!m_QuitRequested) {
      char* line = readline(prompt.c_str(), true);
      do {
         line = readline(prompt.c_str(), false);
         if (line) {
            for (const char* c = line; *c; ++c) {
               if (*c == '\a') {
                  line = 0;
                  break;
               }
            }
         }
      } while (!line);
      add_history(line);
      write_history(histfile);
      int indent = NextInteractiveLine(line);
      if (indent==0) {
         prompt = defaultPrompt;
      } else {
         prompt = defaultCont + std::string(indent * 3, ' ');
      }
   }
}


//---------------------------------------------------------------------------
// Process an interactive line, return whether processing was successful
//---------------------------------------------------------------------------
int cling::UserInterface::NextInteractiveLine(const std::string& line)
{
   if (ProcessMeta(line)) return true;
   
   //----------------------------------------------------------------------
   // Check if the statement is complete.  Use the continuation prompt 
   // otherwise
   //----------------------------------------------------------------------

   std::string src = ""; // genSource("");     
   
   m_input.append( line );
   
   int indentLevel;
   std::vector<clang::FunctionDecl *> fnDecls;
   bool shouldBeTopLevel = false;
   switch (m_Interp->analyzeInput(src, m_input, indentLevel, &fnDecls)) {
      case Interpreter::Incomplete:
         return indentLevel;
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
bool cling::UserInterface::ProcessMeta(const std::string& input)
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

