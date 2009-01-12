//--------------------------------------------------------------------*- C++ -*-
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Lukasz Janyst <ljanyst@cern.ch>
//------------------------------------------------------------------------------

#ifndef CLING_USERINTERFACE_H
#define CLING_USERINTERFACE_H

#include <string>

namespace Inuit {
   class Input;
   class UI;
   class EditLine;
}

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

      bool HandleEvent(Inuit::UI& ui, const Inuit::Input& input);

   private:
      static bool EventHandler(Inuit::UI& ui, const Inuit::Input& input, void* userParam);
      bool NextInteractiveLine(const std::string& line);
      bool ProcessMeta(const std::string& input);

      Interpreter* m_Interp;
      Inuit::EditLine* m_EditLine;
      Inuit::UI* m_UI;
   };
}

#endif // CLING_USERINTERFACE_H


