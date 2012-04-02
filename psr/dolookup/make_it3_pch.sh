#!/bin/sh
clang++ -cc1 -fexceptions -D_GNU_SOURCE -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS -I/local2/russo/llvm/include -emit-pch -x c++-header it3.h -o it3.h.pch
