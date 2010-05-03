// RUN: echo '#define MYMACRO(v) if (v) { printf("string:%%s\\n", v);}' > cppmacros_1.C 
// RUN: echo 'void MYMACRO(void* i) {printf("MYMACRO param=%ld\\n", (long)i);}' > cppmacros_2.C
// RUN: cat %s | %cling -l | FileCheck %s

#include <cstdlib>
.L cppmacros_1.C
#undef MYMACRO
.L cppmacros_2.C
MYMACRO((void*)42); // CHECK: MYMACRO param=42
.q
