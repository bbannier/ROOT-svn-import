##===- cling/lib/EditLine/Makefile--------------------------*- Makefile -*-===##
# 
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
# 
##===----------------------------------------------------------------------===##
#
#  This implements the a C++ interpreter backend.
#
##===----------------------------------------------------------------------===##

LEVEL = ../../../..
LIBRARYNAME := clingEditLine
BUILD_ARCHIVE = 1
CXXFLAGS = -fno-rtti

CPPFLAGS += -I$(PROJ_SRC_DIR)/../../include -I$(PROJ_OBJ_DIR)/../../include \
'-DR__CURSESHDR="curses.h"'

include $(LEVEL)/Makefile.common

