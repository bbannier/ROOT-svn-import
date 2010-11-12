// RUN: cat %s | %cling -l | FileCheck %s

.L cling-test-library
extern "C" int cling_testlibrary_function();
int i = cling_testlibrary_function();
printf("got i=%d\n", i); // CHECK: got i=66
.q
