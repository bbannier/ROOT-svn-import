// RUN: cat %s | %cling -I%p
// RUN: cat %s | %cling -I%p | FileCheck %s

#include "SymbolResolverCallback.h"

gCling->setCallbacks(new cling::test::SymbolResolverCallback(gCling));
jksghdgsjdf->getVersion() // CHECK: {{.*Interpreter.*}}
hsdghfjagsp->Draw() // CHECK: (int const) 12
