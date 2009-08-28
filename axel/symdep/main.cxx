#include "TSymbolDependencyParser.h"
#include <iostream>
#include <stdexcept>


int main(int argc, char* argv[]) {
   if (argc < 2) {
      std::cerr << "ERROR: missing arguments (file names)!" << std::endl;
      throw std::runtime_error("missing arguments (file names)!");
   }


   std::list<const char*> alllibs;
   for (int i = 1; i < argc; ++i) {
      alllibs.push_back(argv[i]);
   }

   TSymbolDependencyParser depParser(alllibs);

   std::list<const char*> neededlibs;
   depParser.Parse(neededlibs);

   for (std::list<const char*>::const_iterator iLib = neededlibs.begin(),
           iLibE = neededlibs.end(); iLib != iLibE; ++iLib)
      std::cout << *iLib << " ";
   std::cout << std::endl;

   return 0;
}
