// RUN: cat %s | %cling -l
// RUN: cat %s | %cling -l | FileCheck %s
#include <cmath>

struct S{int i;};
S s = {12 };

struct U{void f() const {};};

struct V{V(): v(12) {}; int v; };

int i = 12;
float f = sin(12);
int j = i;
printf("j=%d\n",j); // CHECK:j=12
.q
