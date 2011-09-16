# Module.mk for cling module
# Copyright (c) 2007 Rene Brun and Fons Rademakers
#
# Author: Axel Naumann, 2009-10-06

MODNAME      := cling
MODDIR       := $(ROOT_SRCDIR)/cint/$(MODNAME)
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

CLINGDIR     := $(MODDIR)
CLINGDIRS    := $(CLINGDIR)/src
CLINGDIRI    := $(CLINGDIR)/inc

##### libRCling #####
CLINGL       := $(MODDIRI)/LinkDef.h
CLINGDS      := $(call stripsrc,$(MODDIRS)/G__Cling.cxx)
CLINGDO      := $(CLINGDS:.cxx=.o)
CLINGDH      := $(CLINGDS:.cxx=.h)

CLINGH       := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
CLINGS       := $(filter-out $(ROOTCLINGS),$(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx)))
CLINGO       := $(call stripsrc,$(CLINGS:.cxx=.o))

CLINGDEP     := $(CLINGO:.o=.d) $(CLINGDO:.o=.d) $(ROOTCLINGO:.o=.d)

# used in the main Makefile
ALLHDRS      += $(patsubst $(MODDIRI)/%.h,include/%.h,$(CLINGH))

# include all dependency files
INCLUDEFILES += $(CLINGDEP)

##### rootcling #####
ROOTCLINGS   := $(MODDIRS)/rootcling.cxx
ROOTCLINGO   := $(call stripsrc,$(ROOTCLINGS:.cxx=.o))
ROOTCLING    := bin/rootcling$(EXEEXT)

# used in the main Makefile
ALLEXECS     += $(ROOTCLING)

##### local rules #####
ifeq ($(strip $(LLVMDIR)),)
PRINTME:=$(shell echo 'ERROR: you forgot to define LLVMDIR!' >&2)
EXITING-BECAUSE-OF-ERROR
endif

.PHONY:         all-$(MODNAME) clean-$(MODNAME) distclean-$(MODNAME) check-cling-header

include/%.h:    $(CLINGDIRI)/%.h
		cp $< $@

$(CLINGDS):     $(CLINGH) $(CLINGL) $(ROOTCINTTMPDEP)
		@echo "Generating dictionary $@..."
		$(ROOTCINTTMP) -f $@ -c $(CLINGH) $(CLINGL)

$(ROOTCLING):   $(ROOTCLINGO) $(BOOTLIBSDEP)
		$(LD) $(LDFLAGS) -o $@ $(ROOTCLINGO) $(BOOTULIBS) \
		   $(RPATH) $(BOOTLIBS) $(SYSLIBS) -lReflex \
		   -L$(LLVMDIR)/lib -lcling 

all-$(MODNAME): $(ROOTCLING)

clean-$(MODNAME):
		@rm -f $(CLINGO) $(CLINGDO)

clean::         clean-$(MODNAME)

distclean-$(MODNAME): clean-$(MODNAME)
		@rm -f $(CLINGDEP) $(CLINGDS) $(CLINGDH)

distclean::     distclean-$(MODNAME)
		@(find . -name "*_dicthdr.h" -exec rm -f {} \; >/dev/null 2>&1;true)
		@rm -f lib/*.pch

##### extra rules ######
$(CLINGO) $(CLINGDO) $(ROOTCLINGO): CXXFLAGS += -D__STDC_LIMIT_MACROS \
   -D__STDC_CONSTANT_MACROS \
   -I$(LLVMDIR)/include -I. -Wno-unused-parameter -Wno-shadow

CORELIBEXTRA += -L$(LLVMDIR)/lib -lclingInterpreter -lclingUserInterface \
 -lclingInterpreter -lclingMetaProcessor -lclingUITextInput \
 -lclangFrontend -lclangDriver \
 -lclangSerialization -lclangParse -lclangSema -lclangCodeGen -lclangAnalysis \
 -lclangAST -lclangLex -lclangBasic -Llib -lReflex \
 $(shell $(LLVMDIR)/bin/llvm-config --libs) -pthread
