// RUN: cat %s | %cling -l | FileCheck %s

// RUN: .I %llvmlibsdir/
.L libcling-test-library.so
extern "C" int cling_testlibrary_function();
int i = cling_testlibrary_function();
extern "C" int printf(const char* fmt, ...);
printf("got i=%d\n", i); // CHECK: got i=66
.q
