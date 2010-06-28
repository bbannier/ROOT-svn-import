#include "ClingDict.h"

// reflex headers
#include "Reflex/DictionaryGenerator.h"

// clang headers
#include "clang/Frontend/CompilerInstance.h"

// our headers
#include "clr-scan.h"
#include "dict.h"

#include <string>
#include <vector>

//______________________________________________________________________________
ClingDict::ClingDict(const char* name, std::vector<const char*> parseFiles)
{
   fName = name;
   fParseFiles = parseFiles;
}

//______________________________________________________________________________
void ClingDict::parse(cling::Interpreter* clingInterpreter)
{
   TScanner scanner;
   for (int i = 0; i < fParseFiles.size(); ++i) {
      std::string fileName(fParseFiles[i]);
      clingInterpreter->compileFile(fileName);
      clang::CompilerInstance* CI = clingInterpreter->getCI();
      scanner.Scan(&CI->getASTContext());
   }
}

//______________________________________________________________________________
void ClingDict::generate()
{
   // print into file or standard output
   Reflex::DictionaryGenerator generator;
   GlobalScope_GenerateDict(generator);
   generator.Print(fName);
}

