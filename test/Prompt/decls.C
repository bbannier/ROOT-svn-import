// RUN: cat %s | %cling -nologo
// RUN: cat %s | %cling -nologo | FileCheck %s
#include <cmath>
#include <stdio.h>

struct S{int i;} ss;
S s = {12 };

struct U{void f() const {};};

struct V{V(): v(12) {}; int v; };

int i = 12;
float f = sin(12);
int j = i;
printf("j=%d\n",j); // CHECK:j=12
.q
