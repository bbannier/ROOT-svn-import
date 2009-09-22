//--------------------------------------------------------------------*- C++ -*-
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Lukasz Janyst <ljanyst@cern.ch>
//------------------------------------------------------------------------------

#ifndef CLING_USERINTERFACE_H
#define CLING_USERINTERFACE_H

#include <string>

namespace llvm {
   class Module;
}

namespace cling {
   class Interpreter;

   //---------------------------------------------------------------------------
   //! Class for the user interaction with the interpreter
   //---------------------------------------------------------------------------
   class UserInterface
   {
   public:
      UserInterface(Interpreter& interp, const char* prompt = "[cling] $");
      ~UserInterface();

      void runInteractively();
      void executeSingleCodeLine(const char* line);
      void loadFile(const char* file);

   private:
      bool NextInteractiveLine(const std::string& line);
      bool ProcessMeta(const std::string& input);

      Interpreter* m_Interp;
      bool m_QuitRequested;
   };
}

#endif // CLING_USERINTERFACE_H


