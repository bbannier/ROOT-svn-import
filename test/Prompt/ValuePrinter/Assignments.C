// RUN: cat %s | %cling
// RUN: cat %s | %cling | FileCheck %s

int a = 12;
a // CHECK: (int) 12

const char* b = "b" // CHECK: (const char *) "b"

struct C {int d;} E = {22};
E // CHECK: {{\(C\) @0x[0-9A-Fa-f].*}}
E.d // CHECK: (int) 22

#include <string>
std::string s("abc") // CHECK: {{\(std::basic_string<char>\) @0x[0-9A-Fa-f].*}} 
s.c_str() // CHECK: (const char * const) "abc"

.q
