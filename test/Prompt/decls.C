// RUN: cat %s | %cling -l
#include <cmath>

struct S{int i;};
typedef struct {int i;} T;
struct U{void f() const {};}
int i = 12;
float f = sin(12);
int j = i;
.q
