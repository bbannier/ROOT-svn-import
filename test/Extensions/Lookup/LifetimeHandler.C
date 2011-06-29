// RUN: cat %s | %cling -I%p
// RUN: cat %s | %cling -I%p | FileCheck %s

#include "SymbolResolverCallback.h"

gCling->setCallbacks(new cling::test::SymbolResolverCallback(gCling));

class MyClass { private:  const char* Name; public:  MyClass(const char* n):Name(n){} const char* getName(){return Name;} };

extern "C" int printf(const char* fmt, ...);

MyClass my(sadasds->getVersion());
printf("%s\n", my.getName()); // CHECK: {{.*Interpreter.*}}

.q
