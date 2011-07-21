// RUN: cat %s | %cling -I%p
// RUN: cat %s | %cling -I%p | FileCheck %s

#include "SymbolResolverCallback.h"

cling::test::SymbolResolverCallback* SRC;
SRC = new cling::test::SymbolResolverCallback(gCling, /*Enabled*/ true); // TODO: remove when global inits are fixed
SRC->Initialize(); // cannot call interpreter->processline in the ctor.
gCling->setCallbacks(SRC);

class MyClass { private:  const char* Name; public:  MyClass(const char* n):Name(n){} const char* getName(){return Name;} };

extern "C" int printf(const char* fmt, ...);

/*SRC->setEnabled();*/ MyClass my(sadasds->getVersion()); /*SRC->setEnabled(false);*/ 
printf("%s\n", my.getName()); // CHECK: {{.*Interpreter.*}}

.q
