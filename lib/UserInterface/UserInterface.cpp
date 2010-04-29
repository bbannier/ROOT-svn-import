//------------------------------------------------------------------------------
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id: Interpreter.cpp 27029 2008-12-19 13:16:34Z axel $
// author:  Axel Naumann <axel@cern.ch>
//------------------------------------------------------------------------------

#include <cling/UserInterface/UserInterface.h>

#include <cling/MetaProcessor/MetaProcessor.h>

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
   m_MetaProcessor(new MetaProcessor(interp))
{
}


//---------------------------------------------------------------------------
// Destruct the interface
//---------------------------------------------------------------------------
cling::UserInterface::~UserInterface()
{
   delete m_MetaProcessor;
}

//---------------------------------------------------------------------------
// Interact with the user using a prompt
//---------------------------------------------------------------------------
void cling::UserInterface::runInteractively(bool nologo /* = false */)
{
   if (!nologo) {
      std::cerr << std::endl;
      std::cerr << "**** Welcome to the cling prototype! ****" << std::endl;
      std::cerr << "* Type C code and press enter to run it *" << std::endl;
      std::cerr << "* Type .q, exit or ctrl+D to quit       *" << std::endl;
      std::cerr << "*****************************************" << std::endl;
   }
   struct stat buf;
   static const char* histfile = ".cling_history";
   using_history();
   max_input_history = 100;
   if (stat(histfile, &buf) == 0) {
      read_history(histfile);
   }
   const static std::string defaultPrompt("[cling]$ ");
   const static std::string defaultCont("... ");
   std::string prompt = defaultPrompt;
   while (!m_MetaProcessor->isQuitRequested()) {
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
      int indent = m_MetaProcessor->process(line);
      if (indent<=0) {
         prompt = defaultPrompt;
      } else {
         // Continuation requested.
         prompt = defaultCont + std::string(indent * 3, ' ');
      }
   }
}
