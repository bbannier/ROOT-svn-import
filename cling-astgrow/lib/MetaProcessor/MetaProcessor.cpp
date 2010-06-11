//------------------------------------------------------------------------------
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id: Interpreter.cpp 27029 2008-12-19 13:16:34Z axel $
// author:  Axel Naumann <axel@cern.ch>
//------------------------------------------------------------------------------

#include "cling/MetaProcessor/MetaProcessor.h"

#include "cling/Interpreter/Interpreter.h"

#include <cstdio>

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
#if 0
   std::string src = "";
   int indentLevel = 0;
   std::vector<clang::FunctionDecl*> fnDecls;
   Interpreter::InputType kind_of_input =
      m_Interp.analyzeInput(src, m_input, indentLevel, &fnDecls);
   if (kind_of_input == Interpreter::Incomplete) {
      return indentLevel + 1;
   }
#endif // 0
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

   //  .q
   //
   //  Quit.
   //
   if (cmd_char == 'q') {
      m_QuitRequested = true;
      return true;
   }

   //
   //  Extract command and parameter:
   //    .command parameter
   //
   std::string cmd = input_line.substr(1, std::string::npos);
   std::string param;
   std::string::size_type endcmd = input_line.find_first_of(" \t\n", 2);
   if (endcmd != std::string::npos) { // have a blank after command
      cmd = input_line.substr(1, endcmd - 1);

      std::string::size_type firstparam = input_line.find_first_not_of(" \t\n", endcmd);
      std::string::size_type lastparam = input_line.find_last_not_of(" \t\n");

      if (firstparam != std::string::npos) { // have a parameter
         //
         //  Trim blanks from beginning and ending of parameter.
         //
         std::string::size_type len = (lastparam + 1) - firstparam;
         // Construct our parameter.
         param = input_line.substr(firstparam, len);
      }
   }

   //
   //  .L <filename>
   //
   //  Load code fragment.
   //
   if (cmd_char == 'L') {
      //fprintf(stderr, "Begin load file '%s'.\n", param.c_str());
      int err = m_Interp.loadFile(param);
      //fprintf(stderr, "End load file '%s'.\n", param.c_str());
      if (err) {
         //fprintf(stderr, "Load file failed.\n");
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
      int err = m_Interp.executeFile(param);
      if (err) {
         //fprintf(stderr, "Execute file failed.\n");
      }
      return true;
   }
   //
   //  .printAST [0|1]
   //
   //  Toggle the printing pf the AST or if 1 or 0 is given
   //  enable or disable it.
   //
   if (cmd == "printAST") {
      if (param.empty()) {
         // toggle:
         bool print = !m_Interp.setPrintAST(true);
         m_Interp.setPrintAST(print);
         printf("%srinting AST\n", print?"P":"Not p");
      } else if (param == "1") {
         m_Interp.setPrintAST(true);
      } else if (param == "0") {
         m_Interp.setPrintAST(false);
      } else {
         fprintf(stderr, ".printAST: parameter must be '0' or '1' or nothing, not %s.\n", param.c_str());
      }
      return true;
   }

   //
   //  .U <filename>
   //
   //  Unload code fragment.
   //
   //if (cmd_char == 'U') {
   //   llvm::sys::Path path(param);
   //   if (path.isDynamicLibrary()) {
   //      std::cerr << "[i] Failure: cannot unload shared libraries yet!"
   //                << std::endl;
   //   }
   //   int err = m_Interp.unloadFile(param);
   //   if (err) {
   //      //fprintf(stderr, "Unload file failed.\n");
   //   }
   //   return true;
   //}
   //
   //  Unrecognized command.
   //
   //fprintf(stderr, "Unrecognized command.\n");
   return false;
}

