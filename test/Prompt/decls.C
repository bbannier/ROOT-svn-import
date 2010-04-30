// RUN: cat %s | %cling -l
// RUN: cat %s | %cling -l | FileCheck %s
#include <cmath>

struct S{int i;};
struct U{void f() const {};};
int i = 12;
float f = sin(12);
int j = i;
printf("j=%d\n",j); // CHECK:j=12
.q
