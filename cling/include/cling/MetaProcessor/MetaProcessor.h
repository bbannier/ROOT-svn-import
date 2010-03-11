//--------------------------------------------------------------------*- C++ -*-
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Lukasz Janyst <ljanyst@cern.ch>
//------------------------------------------------------------------------------

#ifndef CLING_METAPROCESSOR_H
#define CLING_METAPROCESSOR_H

#include <string>

namespace llvm {
   class Module;
}

namespace cling {

   class Interpreter;

   //---------------------------------------------------------------------------
   // Class for the user interaction with the interpreter
   //---------------------------------------------------------------------------
   class MetaProcessor
   {
   private:
      Interpreter& m_Interp; // the interpreter
      int m_contLevel;     // continuation indentation
      std::string m_input; // pending statement
      bool m_QuitRequested; // quitting?
   private:
      bool ProcessMeta(const std::string& input_line);
   public:
      MetaProcessor(Interpreter& interp);
      ~MetaProcessor();
      int process(const char* input_line);
      bool isQuitRequested() const { return m_QuitRequested; }
      void requestQuit(bool req) { m_QuitRequested = req; }
   };
}

#endif // CLING_METAPROCESSOR_H


