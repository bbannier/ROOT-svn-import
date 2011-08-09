// RUN: cat %s | %cling -I%p
// RUN: cat %s | %cling -I%p | FileCheck %s

#include "SymbolResolverCallback.h"

.dynamicExtensions

gCling->setCallbacks(new cling::test::SymbolResolverCallback(gCling, /*Enabled=*/true));
jksghdgsjdf->getVersion() // CHECK: {{.*Interpreter.*}}
hsdghfjagsp->Draw() // CHECK: (int const) 12

h->Add10(h->Add10(h->Add10(0))) // CHECK: (int const) 30
h->PrintString(std::string("test")); // CHECK: test
int a[5] = {1,2,3,4,5};
h->PrintArray(a, 5); // CHECK: 12345
.q
