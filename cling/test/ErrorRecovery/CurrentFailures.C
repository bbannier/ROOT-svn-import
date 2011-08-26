// RUN: cat %s | %cling -I%p | FileCheck %s
// XFAIL: *

dep->getVersion();
a

extern "C" int printf // expected-error {{expected ';' after top level declarator}}
extern "C" int printf(const char* fmt, ...); // expected-error {{redefinition of 'printf' as different kind of symbol}} expected-note {{previous definition is here}}
