// RUN: cat %s | %cling -l
// RUN: cat %s | %cling -l | FileCheck %s

#include <cstdlib>
int i;
struct S{int i};
i = 42;
printf("i=%d\n",i); // CHECK: i=42
if (i != 42) exit(1);
.q
