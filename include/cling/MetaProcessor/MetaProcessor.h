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
   //! Class for the user interaction with the interpreter
   //---------------------------------------------------------------------------
   class MetaProcessor
   {
   public:
      MetaProcessor(Interpreter& interp);
      ~MetaProcessor();

      bool process(const char* code);
      bool isQuitRequested() const { return m_QuitRequested; }
      void requestQuit(bool req) { m_QuitRequested = req; }

   private:
      bool ProcessMeta(const std::string& input);

      Interpreter* m_Interp;
      bool m_QuitRequested; // Whether the event loop has been requested top end
      int m_contLevel;     // How many continuation level (i.e. open nested blocks)
      std::string m_input; // Accumulation of the incomplete lines.
   };
}

#endif // CLING_METAPROCESSOR_H


