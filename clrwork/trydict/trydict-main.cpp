// clang headers
#include "clang/Frontend/CompilerInstance.h"

// root headers
#include "TSystem.h"

// our headers
#include "ClingDict.h"

#include <vector>
#include <iostream>

//______________________________________________________________________________
void AddIncludePath(clang::CompilerInstance* CI, const char* path)
{
   // Add the given path to the list of directories in which the interpreter
   // looks for include files. Only one path item can be specified at a
   // time, i.e. "path1:path2" is not supported.
   // R__LOCKGUARD(gCINTMutex);
   char* incpath = gSystem->ExpandPathName(path);
   // G__add_ipath(incpath);
   clang::HeaderSearchOptions& headerOpts = CI->getHeaderSearchOpts();
   const bool IsUserSupplied = false;
   const bool IsFramework = false;
   headerOpts.AddPath(incpath, clang::frontend::Angled, IsUserSupplied, IsFramework);
   delete[] incpath;
}

//______________________________________________________________________________
void SetupLangOptions(clang::CompilerInstance* CI)
{
   clang::LangOptions& langInfo = CI->getLangOpts();
   //langInfo.C99 = 1;
   //langInfo.HexFloats = 1;
   langInfo.BCPLComment = 1; // Only for C99/C++.
   langInfo.Digraphs = 1; // C94, C99, C++.
   langInfo.CPlusPlus = 1;
   //langInfo.CPlusPlus0x = 1;
   langInfo.CXXOperatorNames = 1;
   langInfo.Bool = 1;
   langInfo.NeXTRuntime = 1;
   langInfo.NoInline = 1;
   langInfo.Exceptions = 1;
   langInfo.GNUMode = 1;
   langInfo.NoInline = 1;
   langInfo.GNUInline = 1;
   langInfo.DollarIdents = 1;
   langInfo.POSIXThreads = 1;
}

//______________________________________________________________________________
clang::CompilerInstance* GetCI()
{
   cling::Interpreter* interpreter = new cling::Interpreter();
   clang::CompilerInstance* CI = interpreter->getCI();
   SetupLangOptions(CI);
   AddIncludePath(CI, ".");
   TString include = gSystem->Getenv("ROOTSYS");
   include.Append("/include");
   AddIncludePath(CI, include);
   return CI;
}

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
   clang::CompilerInstance* CI = GetCI();
   ClingDict dict(name, parsefiles);
   dict.parse(CI);
   dict.generate();
   return 0;
}

