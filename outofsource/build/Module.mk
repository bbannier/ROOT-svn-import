# Module.mk for build module
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: Fons Rademakers, 29/2/2000

MODDIR       := $(SRCDIR)/build

RMKDEPDIR    := $(MODDIR)/rmkdepend
BINDEXPDIR   := $(MODDIR)/win/bindexplib

##### rmkdepend #####
RMKDEPH      := $(wildcard $(RMKDEPDIR)/*.h)
RMKDEPS1     := $(wildcard $(RMKDEPDIR)/*.c)
RMKDEPS2     := $(wildcard $(RMKDEPDIR)/*.cxx)
RMKDEPO1     := $(subst $(SRCDIR)/,,$(RMKDEPS1:.c=.o))
RMKDEPO2     := $(subst $(SRCDIR)/,,$(RMKDEPS2:.cxx=.o))
RMKDEPO      := $(RMKDEPO1) $(RMKDEPO2)
RMKDEP       := bin/rmkdepend$(EXEEXT)
ifeq ($(PLATFORM),win32)
#RMKDEPCFLAGS := -DINCLUDEDIR=\"/usr/include\" -DOBJSUFFIX=\".obj\"
RMKDEPCFLAGS := -DINCLUDEDIR=\"/usr/include\" -DOBJSUFFIX=\".o\"
else
RMKDEPCFLAGS := -DINCLUDEDIR=\"/usr/include\" -DOBJSUFFIX=\".o\"
endif

##### bindexplib #####
ifeq ($(PLATFORM),win32)
BINDEXPS     := $(wildcard $(BINDEXPDIR)/*.cxx)
BINDEXPO     := $(subst $(SRCDIR)/,,$(BINDEXPS:.cxx=.o))
BINDEXP      := bin/bindexplib$(EXEEXT)

W32PRAGMA    := build/win/w32pragma.h
ALLHDRS      += include/w32pragma.h
endif

##### local rules #####
$(RMKDEP):      $(RMKDEPO)
		$(LD) $(LDFLAGS) -o $@ $(RMKDEPO)

ifeq ($(PLATFORM),win32)
include/%.h:    build/win/%.h
		cp $< $@

$(BINDEXP):     $(BINDEXPO)
		$(LD) $(LDFLAGS) -o $@ $(BINDEXPO)

all-build:      $(RMKDEP) $(BINDEXP)
else
all-build:      $(RMKDEP)
endif

clean-build:
		@rm -f $(RMKDEPO) $(BINDEXPO)

clean::         clean-build

distclean-build: clean-build
		@rm -f $(RMKDEP) $(BINDEXP)

distclean::     distclean-build


##### dependencies #####
$(subst $(SRCDIR)/,,$(RMKDEPDIR)/cppsetup.o): $(RMKDEPDIR)/def.h $(RMKDEPDIR)/ifparser.h
$(subst $(SRCDIR)/,,$(RMKDEPDIR)/ifparser.o): $(RMKDEPDIR)/ifparser.h
$(subst $(SRCDIR)/,,$(RMKDEPDIR)/include.o):  $(RMKDEPDIR)/def.h
$(subst $(SRCDIR)/,,$(RMKDEPDIR)/main.o):     $(RMKDEPDIR)/def.h $(RMKDEPDIR)/imakemdep.h
$(subst $(SRCDIR)/,,$(RMKDEPDIR)/parse.o):    $(RMKDEPDIR)/def.h
$(subst $(SRCDIR)/,,$(RMKDEPDIR)/pr.o):       $(RMKDEPDIR)/def.h
$(subst $(SRCDIR)/,,$(RMKDEPDIR)/mainroot.o): $(RMKDEPDIR)/def.h

##### local rules #####
$(RMKDEPO1): CFLAGS += $(RMKDEPCFLAGS)
$(RMKDEPO2): CXXFLAGS += $(RMKDEPCFLAGS)
$(RMKDEPO) : RMKDEP:=
$(RMKDEPO) : PCHCFLAGS:=
$(RMKDEPO) : PCHCXXFLAGS:=
