// RUN: cat %s | %cling -nologo
// RUN: cat %s | %cling -nologo | FileCheck %s
#include <cmath>

struct S{int i;} ss;
S s = {12 };

struct U{void f() const {};} uu;

struct V{V(): v(12) {}; int v; } vv;

int i = 12;
float f = sin(12);
int j = i;
extern "C" int printf(const char* fmt, ...);
printf("j=%d\n",j); // CHECK:j=12
.q
