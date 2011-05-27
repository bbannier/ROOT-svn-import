// RUN: cat %s | %cling -I%p
// RUN: cat %s | %cling -I%p | FileCheck %s

#include "ResolveAlwaysAsGCling.h"

gCling->setCallbacks(new cling::test::ResolveAlwaysAsGClingCallback(gCling));
jksghdgsjdf->getVersion() // CHECK: {{.*Interpreter.*}}
hsdghfjagsp->Draw() // CHECK: (int const) 12
