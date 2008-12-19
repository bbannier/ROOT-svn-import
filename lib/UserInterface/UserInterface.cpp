//------------------------------------------------------------------------------
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id: Interpreter.cpp 27029 2008-12-19 13:16:34Z axel $
// author:  Axel Naumann <axel@cern.ch>
//------------------------------------------------------------------------------

#include <cling/UserInterface/UserInterface.h>

#include <cling/Interpreter/Interpreter.h>

#include <llvm/System/DynamicLibrary.h>
#include <llvm/ADT/OwningPtr.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/Module.h>
#include <llvm/Function.h>
#include <llvm/Support/MemoryBuffer.h>
#
#include <Inuit/Widgets/UI.h>
#include <Inuit/Widgets/EditLine.h>
#include <Inuit/Drivers/TerminalDriver.h>
#include <Inuit/Drivers/InputDriver.h>

#include <iostream>

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
cling::UserInterface::UserInterface(Compiler& interp):
   m_Interp(&interp), m_EditLine(0), m_UI(0)
{
   TerminalDriver& td = TerminalDriver::Instance();
   InputDriver& ed = InputDriver::Instance(td);
   m_UI = new UI(Pos(td.GetSize().fX, 1), ed);
   m_UI->AddInputHandler(InputHandler, this);
   m_EditLine = new EditLine(m_UI, Pos(0, 0), td.GetSize().fX);
   m_UI->AddElement(m_EditLine);
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
// Interact with the user using as prompt
//---------------------------------------------------------------------------
void cling::UserInterface::runInteractively(const char* prompt /*= "[cling] $"*/)
{
   std::cerr << "Type a C code and press enter to run it." << std::endl;
   std::cerr << "Type .q, exit or ctrl+D to quit" << std::endl;
   m_UI->ProcessInputs();
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
      if (editLine && m_EditLine ==  editLine) {
         if (NextInteractiveLine(editLine->GetText()))
            editLine->SetText("");
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
   ExecuteModuleMain( module );
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
         llvm::sys::Path path(input.substr(3));
         if (path.isDynamicLibrary()) {
            std::string errMsg;
            if (llvm::sys::DynamicLibrary::LoadLibraryPermanently(input.substr(3).c_str(), &errMsg))
               std::cerr << "[i] Success!" << std::endl;
            else
               std::cerr << "[i] Failure: " << errMsg << std::endl;
         } else {
            if( m_Interp->addUnit( input.substr( 3 ) ) )
               std::cerr << "[i] Success!" << std::endl;
            else
               std::cerr << "[i] Failure" << std::endl;
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
      m_UI->RequestQuit();
      break;
   default:
      return false;
   }
   return true;
}


//---------------------------------------------------------------------------
// Call the Interpreter on a Module
//---------------------------------------------------------------------------
int cling::UserInterface::ExecuteModuleMain( llvm::Module *module )
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

