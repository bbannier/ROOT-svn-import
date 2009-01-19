//------------------------------------------------------------------------------
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id: Interpreter.cpp 27029 2008-12-19 13:16:34Z axel $
// author:  Axel Naumann <axel@cern.ch>
//------------------------------------------------------------------------------

// temporary - this has to go away after 2009-01-21!
#define CLING_USE_READLINE

#include <cling/UserInterface/UserInterface.h>

#include <cling/Interpreter/Interpreter.h>

#include <llvm/System/DynamicLibrary.h>
#include <llvm/Support/MemoryBuffer.h>
#
#include <Inuit/Widgets/UI.h>
#include <Inuit/Widgets/EditLine.h>
#include <Inuit/Drivers/TerminalDriver.h>
#include <Inuit/Drivers/InputDriver.h>

#include <iostream>

#ifdef CLING_USE_READLINE
#include <sys/stat.h>
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#endif

namespace llvm {
   class Module;
}

using namespace Inuit;

namespace {

   // Event handler for Inuit::UI
   static bool InputHandler(Inuit::UI& ui, const Inuit::Input& input, void* userParam)
   {
      cling::UserInterface* cui = (cling::UserInterface*)userParam;
      cui->HandleEvent(ui, input);
   }

   //------------------------------------------------------------------------------
   // String constants - MOVE SOMEWHERE REASONABLE!
   //------------------------------------------------------------------------------
   static std::string code_prefix = "#include <stdio.h>\nint main(int argc, char** argv) {\n";
   static std::string code_suffix = ";\nreturn 0; } ";

} // unnamed namespace


//---------------------------------------------------------------------------
// Construct an interface for an interpreter
//---------------------------------------------------------------------------
cling::UserInterface::UserInterface(Interpreter& interp, const char* prompt /*= "[cling] $"*/):
   m_Interp(&interp), m_EditLine(0), m_UI(0)
{
#ifndef CLING_USE_READLINE
   TerminalDriver& td = TerminalDriver::Instance();
   InputDriver& ed = InputDriver::Instance(td);
   m_UI = new UI(Pos(td.GetSize().fW, td.GetSize().fH), ed);
   m_UI->AddInputHandler(InputHandler, this);
   m_EditLine = new EditLine(m_UI, Pos(0, td.GetSize().fH - 1), td.GetSize().fW, prompt);
   m_UI->AddElement(m_EditLine);
#endif
}


//---------------------------------------------------------------------------
// Destruct the interface
//---------------------------------------------------------------------------
cling::UserInterface::~UserInterface()
{
   delete m_EditLine;
   delete m_UI;
}

//---------------------------------------------------------------------------
// Interact with the user using a prompt
//---------------------------------------------------------------------------
void cling::UserInterface::runInteractively()
{
   std::cerr << "**** Welcome to the cling prototype! ****" << std::endl;
   std::cerr << "* Type C code and press enter to run it *" << std::endl;
   std::cerr << "* Type .q, exit or ctrl+D to quit       *" << std::endl;
   std::cerr << "*****************************************" << std::endl;
   std::cerr << std::endl;
#ifdef CLING_USE_READLINE
   struct stat buf;
   static const char* histfile = ".cling_history";
   using_history();
   history_max_entries = 100;
   if (stat(histfile, &buf) == 0) {
      read_history(histfile);
   }
   m_QuitRequested = false;
   while (!m_QuitRequested) {
      char* line = readline("[cling]$ ");
      if (line) {
         NextInteractiveLine(line);
         add_history(line);
         free(line);
      } else break;
   }
   write_history(histfile);
#else
   m_UI->ProcessInputs();
#endif
}


//---------------------------------------------------------------------------
// Handle an event passed from EventHandler
//---------------------------------------------------------------------------
bool cling::UserInterface::HandleEvent(Inuit::UI& ui, const Inuit::Input& input)
{
   if (input.GetType() == Input::kTypePrintable
       && (input.GetPrintable() == 'q' || input.GetPrintable() == 'Q'))
      ui.RequestQuit();
   else if (input.GetType() == Input::kTypeNonPrintable
            && input.GetNonPrintable() == Inuit::Input::kNPEnter) {
      Inuit::EditLine* editLine = dynamic_cast<Inuit::EditLine*>(ui.GetFocusedWidget());
      if (editLine && m_EditLine == editLine) {
         ui.GetTerminalDriver().SetManagedMode(false);
         bool clearLine = NextInteractiveLine(editLine->GetText());
         ui.GetTerminalDriver().SetManagedMode(true);
         if (clearLine) {
            editLine->SetText("");
         }
      } else return false;
   } else return false;
   return true;
}

 
//---------------------------------------------------------------------------
// Process an interactive line, return whether processing was successful
//---------------------------------------------------------------------------
bool cling::UserInterface::NextInteractiveLine(const std::string& line)
{
   if (ProcessMeta(line)) return true;
   
   //----------------------------------------------------------------------
   // Wrap the code
   //----------------------------------------------------------------------
   std::string wrapped = code_prefix + line + code_suffix;
   llvm::MemoryBuffer* buff;
   buff  = llvm::MemoryBuffer::getMemBufferCopy( &*wrapped.begin(),
                                                 &*wrapped.end(),
                                                 "CLING" );

   //----------------------------------------------------------------------
   // Parse and run it
   //----------------------------------------------------------------------
   llvm::Module* module = m_Interp->link( buff );

   if(!module) {
      std::cerr << std::endl;
      std::cerr << "[!] Errors occured while parsing your code!" << std::endl;
      std::cerr << std::endl;
      return false;
   }
   m_Interp->executeModuleMain( module );
   return true;
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
            llvm::sys::Path path(input.substr(first, last - first));
            if (path.isDynamicLibrary()) {
               std::string errMsg;
               if (!llvm::sys::DynamicLibrary::LoadLibraryPermanently(input.substr(3).c_str(), &errMsg))
                  std::cerr << "[i] Success!" << std::endl;
               else
                  std::cerr << "[i] Failure: " << errMsg << std::endl;
            } else {
               if( m_Interp->addUnit( input.substr( 3 ) ) )
                  std::cerr << "[i] Success!" << std::endl;
               else
                  std::cerr << "[i] Failure" << std::endl;
            }
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
#ifdef CLING_USE_READLINE
      m_QuitRequested = true;
#else
      m_UI->RequestQuit();
#endif
      break;
   default:
      return false;
   }
   return true;
}

