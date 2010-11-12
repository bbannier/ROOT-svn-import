// RUN: %cling %s | FileCheck %s
#ifdef MYMACRO
# undef MYMACRO
#endif

void MYMACRO(void* i) {
   printf("MYMACRO param=%ld\n", (long)i); // CHECK: MYMACRO param=12
}

void cppundef() {
   MYMACRO((void*)12);
}
