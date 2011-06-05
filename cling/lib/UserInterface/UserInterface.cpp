//------------------------------------------------------------------------------
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Axel Naumann <axel@cern.ch>
//------------------------------------------------------------------------------

#include <cling/UserInterface/UserInterface.h>

#include <cling/MetaProcessor/MetaProcessor.h>
#include "textinput/TextInput.h"
#include "textinput/StreamReader.h"
#include "textinput/TerminalDisplay.h"

#include <iostream>
#include <sys/stat.h>

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
  static const char* histfile = ".cling_history";
  const static std::string defaultPrompt("[cling]$ ");
  const static std::string defaultCont("... ");

  using namespace textinput;
  StreamReader* R = StreamReader::Create();
  TerminalDisplay* D = TerminalDisplay::Create();
  TextInput TI(*R, *D, histfile);
  TI.SetPrompt(defaultPrompt.c_str());
  std::string line;
  
  int promptIndent = 0;
  while (!m_MetaProcessor->isQuitRequested()) {
    TextInput::EReadResult RR = TI.ReadInput();
    TI.TakeInput(line);
    if (RR == TextInput::kRREOF) {
      m_MetaProcessor->requestQuit(true);
      continue;
    }
    int indent = m_MetaProcessor->process(line.c_str());
    if (indent<=0) {
      if (promptIndent) {
        TI.SetPrompt(defaultPrompt.c_str());
      }
    } else {
      if (promptIndent != indent) {
        // Continuation requested.
        TI.SetPrompt((defaultCont + std::string(indent * 3, ' ')).c_str());
      }
    }
  }
}
