//--------------------------------------------------------------------*- C++ -*-
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Axel Naumann <axel@cern.ch>
//------------------------------------------------------------------------------

#ifndef CLING_INVOCATIONOPTIONS_H
#define CLING_INVOCATIONOPTIONS_H

#include <string>
#include <vector>

namespace cling {
   class InvocationOptions {
   public:
      InvocationOptions(): NoLogo(false) {}
      bool NoLogo;
      bool ShowVersion;
      bool Verbose;
      bool Help;

      std::vector<std::string> LibsToLoad;
      std::vector<std::string> LibSearchPath;

      static InvocationOptions CreateFromArgs(int argc, const char* const argv[],
                                              std::vector<unsigned>& leftoverArgs
                                              /* , Diagnostic &Diags */);

      void PrintHelp();
   };
}

#endif // INVOCATIONOPTIONS
