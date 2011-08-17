// RUN: cat %s | %cling -I%p
// RUN: cat %s | %cling -I%p | FileCheck %s
// XFAIL: *
// We should revise the destruction of the LifetimeHandlers, because
// its destructor uses gCling and the CompilerInstance, which are 
// already gone

#include "SymbolResolverCallback.h"

.dynamicExtensions 

cling::test::SymbolResolverCallback* SRC = new cling::test::SymbolResolverCallback(gCling, /*Enabled=*/ false);
gCling->setCallbacks(SRC);

.x LifetimeHandler.h
// CHECK: Alpha's single arg ctor called {{.*Interpreter.*}}
// CHECK: After Alpha is Beta {{.*Interpreter.*}}
// CHECK: Alpha dtor called {{.*Interpreter.*}}

Alpha a(sadasds->getVersion()); 
printf("%s\n", a.getVar()); // CHECK: {{.*Interpreter.*}}

.q
