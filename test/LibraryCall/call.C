// RUN: cat %s | %cling -l | FileCheck %s

// TODO: find a way to test this in a self contained way.
// E.g. rely on an LLVM shared lib.
.L crypt
#include "crypt.h" 
setkey("HELLO");
printf("done\n"); // CHECK: done
.q
