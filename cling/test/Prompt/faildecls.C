// RUN: cat %s | %cling -l
// XFAIL

// "anonymous structs and classes must be class members"
typedef struct {int j;} T;
.q
