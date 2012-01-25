#!/bin/sh
clang++ -cc1 -fexceptions -D_GNU_SOURCE -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS -I${LLVMDIR}/include -emit-pch -x c++-header test1.h -o test1.pch
