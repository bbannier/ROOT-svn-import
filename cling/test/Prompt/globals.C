// RUN: cat %s | %cling -nologo
// RUN: cat %s | %cling -nologo | FileCheck %s

#include <cstdlib>
#include <stdio.h>

int i;
struct S{int i;} s;
i = 42;
printf("i=%d\n",i); // CHECK: i=42
if (i != 42) exit(1);
.q
