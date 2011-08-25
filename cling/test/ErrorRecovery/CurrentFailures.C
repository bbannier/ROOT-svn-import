// RUN: cat %s | %cling -I%p | FileCheck %s
// XFAIL: *

.x CurrentFailures.h

 // First one is okay and the redefinition errors are not handled properly
.L CurrentFailures.h
.L CurrentFailures.h

dep->getVersion();
a

extern "C" int printf // expected-error {{expected ';' after top level declarator}}
extern "C" int printf(const char* fmt, ...); // expected-error {{redefinition of 'printf' as different kind of symbol}} expected-note {{previous definition is here}}
