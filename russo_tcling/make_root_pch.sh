#!/bin/sh
clang++ -cc1 -fexceptions -DG__REGEXP -DG__UNIX -DG__SHAREDLIB -DG__OSFDLL -DG__ROOT -DG__REDIRECTIO -DG__STD_EXCEPTION -DG__64BIT -DG__HAVE_CONFIG -DG__NOMAKEINFO -DG__CINTBODY -I/local2/russo/root_tcl/include -I/local2/russo/root_tcl/cint/cint/src -I/local2/russo/root_tcl/graf2d/freetype/src/freetype-2.3.12/include -emit-pch -x c++-header -relocatable-pch root.h -o root.pch
