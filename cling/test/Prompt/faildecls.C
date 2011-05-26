// RUN: cat %s | %cling -nologo -l
// XFAIL: *

// "anonymous structs and classes must be class members"
typedef struct {int j;} T;
.q
