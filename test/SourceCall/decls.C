// RUN: %cling %s | FileCheck %s
// XFAIL: *
#include <cmath>

struct S{int i;};
S s = {12 };
typedef struct {int i;} T;
struct U{void f() const {};};
int i = 12;
float f = sin(12);
int j = i;

void decls() {
   printf("j=%d\n",j); // CHECK:j=12
}
