#ifndef __CLINGDICT_H__
#define __CLINGDICT_H__

#include <vector>

#include "cling/Interpreter/Interpreter.h"

class ClingDict {
private:
   const char* fName;
   std::vector<const char*> fParseFiles;
public:
   ClingDict(const char* name, std::vector<const char*> parsefiles);
   void parse(cling::Interpreter* interpreter);
   virtual void select() {}
   virtual void generate();
};

#endif // __CLINGDICT_H__
