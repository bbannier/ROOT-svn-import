#ifndef __TRY_DICT_H__
#define __TRY_DICT_H__

#include <vector>

#include "cling/Interpreter/Interpreter.h"

class ClingDict {
private:
   const char* fName;
   std::vector<const char*> fParseFiles;
   bool parseFile(clang::CompilerInstance* CI, const char* fileName);
public:
   ClingDict(const char* name, std::vector<const char*> parsefiles);
   bool parse(clang::CompilerInstance* CI);
   virtual void select() {}
   virtual void generate();
};

#endif // __TRY_DICT_H__
