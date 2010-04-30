// RUN: %cling %s | FileCheck %s
#define MYMACRO(v) \
   if (v) { \
      printf("string:%s\n", v);\
   }

void cppmacros() {
   MYMACRO("PARAM"); // CHECK: string:PARAM
}
