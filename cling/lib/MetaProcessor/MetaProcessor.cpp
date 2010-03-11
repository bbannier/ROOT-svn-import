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
      m_Interp(interp),
      m_contLevel(-1),
      m_QuitRequested(false)
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
//
// Return:
//
//            0: success
// indent level: prompt for more input
//
//---------------------------------------------------------------------------
int
cling::MetaProcessor::process(const char* input_text)
{
   if (!input_text) { // null pointer, nothing to do.
      return 0;
   }
   if (!input_text[0]) { // empty string, nothing to do.
      return 0;
   }
   std::string input_line(input_text);
   if (input_line == "\n") { // just a blank line, nothing to do.
      return 0;
   }
   //
   //  Check for and handle any '.' commands.
   //
   bool was_meta = false;
   if ((input_line[0] == '.') && (input_line.size() > 1)) {
      was_meta = ProcessMeta(input_line);
   }
   if (was_meta) {
      return 0;
   }
   //
   //  Accumulate the input lines.
   //
   m_input.append(input_line);
   //
   // Check if the current statement is now complete.
   // If not, return to prompt for more.
   //
   std::string src = "";
   int indentLevel = 0;
   std::vector<clang::FunctionDecl*> fnDecls;
   Interpreter::InputType kind_of_input =
      m_Interp.analyzeInput(src, m_input, indentLevel, &fnDecls);
   if (kind_of_input == Interpreter::Incomplete) {
      return indentLevel + 1;
   }
   //
   //  We have a complete statement, compile and execute it.
   //
   m_Interp.processLine(m_input);
   //
   //  All done.
   //
   m_input.clear(); // reset pending statment
   return 0;
}


//---------------------------------------------------------------------------
// Process possible meta commands (.L,...)
//---------------------------------------------------------------------------
bool
cling::MetaProcessor::ProcessMeta(const std::string& input_line)
{
   // The command is the char right after the initial '.' char.
   const char cmd_char = input_line[1];
   //
   //  Handle all one character commands.
   //
   //--
   //
   //  .q
   //
   //  Quit.
   //
   if (cmd_char == 'q') {
      m_QuitRequested = true;
      return true;
   }
   //
   //  Handle all commands with parameters.
   //
   //--
   //  Make sure we have a parameter.
   if (input_line.size() < 4) { // must have at least ".x <something>"
      return false;
   }
   // Must have at least one space after the command char.
   if (input_line[2] != ' ') {
      return false;
   }
   //
   //  Trim blanks from beginning and ending of parameter.
   //
   std::string::size_type first = input_line.find_first_not_of(" ", 3);
   std::string::size_type last = input_line.find_last_not_of(" ", 3);
   if (first == std::string::npos) { // all blanks after command char
      return false;
   }
   std::string::size_type len = 0;
   if (last == std::string::npos) {
      len = input_line.size() - first;
   }
   else {
      len = (last + 1) - first;
   }
   // Construct our parameter.
   std::string param(input_line, first, len);
#if 0
   //
   //  .L <filename>
   //
   //  Load code fragment.
   //
   if (cmd_char == 'L') {
      llvm::sys::Path path(param);
      if (path.isDynamicLibrary()) {
         std::string errMsg;
         bool err =
            llvm::sys::DynamicLibrary::LoadLibraryPermanently(
               param.c_str(), &errMsg);
         if (err) {
            std::cerr << "[i] Failure: " << errMsg << std::endl;
         }
         else {
            std::cerr << "[i] Success!" << std::endl;
         }
         return true;
      }
      // TODO: Need to double check that it is a text file ...
      bool ok = m_Interp.addUnit(param);
      if (ok) {
         std::cerr << "[i] Success!" << std::endl;
      }
      else {
         std::cerr << "[i] Failure" << std::endl;
      }
      return true;
   }
   //
   //  .x <filename>
   //  .X <filename>
   //
   //  Execute function from file, function name is filename
   //  without extension.
   //
   if ((cmd_char == 'x') || (cmd_char == 'X')) {
      m_Interp.executeFile(param);
      return true;
   }
   //
   //  .U <filename>
   //
   //  Unload code fragment.
   //
   if (cmd_char == 'U') {
      llvm::sys::Path path(param);
      if (path.isDynamicLibrary()) {
         std::cerr << "[i] Failure: cannot unload shared libraries yet!"
                   << std::endl;
      }
      m_Interp.removeUnit(param);
      return true;
   }
#endif // 0
   //
   //  Unrecognized command.
   //
   return false;
}

