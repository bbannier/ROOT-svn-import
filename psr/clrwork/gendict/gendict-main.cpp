// gendict-main.cpp

// root headers
#include "TCling.h"

// cling headers
// FIXME: Need include of header file for cling::Interpreter here!!!

// our headers
#include "ClingDict.h"

#include <iostream>
#include <vector>

//______________________________________________________________________________
int main(int argc, const char** argv)
{
   const char* name = "";
   std::vector<const char*> parsefiles;
   bool f_opt = false;
   bool unknown_opt = false;
   for (int i = 1; i < argc; ++i) {
      const char* s = argv[i];
      if (f_opt) {
         name = s;
         f_opt = false;
      }
      else if (s[0] == '-') {
         if (s[1] == 'f') {
            f_opt = true;
         }
         else {
            unknown_opt = true;
         }
      }
      else {
         parsefiles.push_back(s);
         f_opt = false;
      }
   }
   if (!parsefiles.size() || unknown_opt || f_opt) {
      std::cout << "Usage: " << argv[0] << " [-f output_file] input_file..." << std::endl;
      return 1;
   }
   TCint* cint = new TCint("C/C++", "CINT C/C++ Interpreter");
   cint->AddIncludePath(".");
   cling::Interpreter* interpreter = cint->GetInterpreter();
   ClingDict dict(name, parsefiles);
   dict.parse(interpreter);
   dict.generate();
   return 0;
}

