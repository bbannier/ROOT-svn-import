# Module.mk for cling module
# Copyright (c) 2007 Rene Brun and Fons Rademakers
#
# Author: Axel Naumann, 2009-10-06

MODNAME      := cling
MODDIR       := cint/$(MODNAME)
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

CLINGDIR     := $(MODDIR)
CLINGDIRS    := $(CLINGDIR)/src
CLINGDIRI    := $(CLINGDIR)/inc

##### libRCling #####
CLINGL       := $(MODDIRI)/LinkDef.h
CLINGDS      := $(MODDIRS)/G__Cling.cxx
CLINGDO      := $(CLINGDS:.cxx=.o)
CLINGDH      := $(CLINGDS:.cxx=.h)

CLINGH       := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
CLINGS       := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
CLINGO       := $(CLINGS:.cxx=.o)

CLINGDEP     := $(CLINGO:.o=.d) $(CLINGDO:.o=.d)

# used in the main Makefile
ALLHDRS      += $(patsubst $(MODDIRI)/%.h,include/%.h,$(CLINGH))

### TODO: rename cling-based TCint to TCling, move into libRCling
#CLINGLIB     := $(LPATH)/libRCling.$(SOEXT)
#CLINGMAP     := $(CLINGLIB:.$(SOEXT)=.rootmap)

#ALLLIBS      += $(CLINGLIB)
#ALLMAPS      += $(CLINGMAP)

# include all dependency files
INCLUDEFILES += $(CLINGDEP)

##### local rules #####
.PHONY:         all-$(MODNAME) clean-$(MODNAME) distclean-$(MODNAME) check-cling-header

include/%.h:    $(CLINGDIRI)/%.h
		cp $< $@

$(CLINGLIB):    $(CLINGO) $(CLINGDO) $(ORDER_) $(MAINLIBS)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		   "$(SOFLAGS)" libRCling.$(SOEXT) $@ "$(CLINGO) $(CLINGDO)" \
		   "$(CLINGLIBEXTRA)" -L$(LLVMDIR)/lib -lCling

$(CLINGDS):     $(CLINGH) $(CLINGL) $(ROOTCINTTMPDEP)
		@echo "Generating dictionary $@..."
		$(ROOTCINTTMP) -f $@ -c $(CLINGH) $(CLINGL)

$(CLINGMAP):    $(RLIBMAP) $(MAKEFILEDEP) $(CLINGL)
		$(RLIBMAP) -o $(CLINGMAP) -l $(CLINGLIB) -d $(CLINGLIBDEPM) -c $(CLINGL)

all-$(MODNAME): $(CLINGLIB) $(CLINGMAP)

clean-$(MODNAME):
		@rm -f $(CLINGO) $(CLINGDO)

clean::         clean-$(MODNAME)

distclean-$(MODNAME): clean-$(MODNAME)
		@rm -f $(CLINGDEP) $(CLINGDS) $(CLINGDH) $(CLINGLIB) $(CLINGMAP)

distclean::     distclean-$(MODNAME)

$(CLINGO): check-cling-header
check-cling-header:
	diff $(CLINGDIRI)/TCint.h include/TCint.h > /dev/null \
	  || cp $(CLINGDIRI)/TCint.h include/TCint.h

##### extra rules ######
$(CLINGO) $(clingdo): CXXFLAGS += -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS \
                                  -I$(LLVMDIR)/include
# remove TCint and its dictionary:
METAO := $(subst $(METATCINTDO),$(CLINGDO),$(subst $(METATCINTO),$(CLINGO),$(METAO)))
dummy := $(shell echo "METAO=$(METAO)" 2>&1)
CORELIBEXTRA += -L$(LLVMDIR)/lib -lclingInterpreter -lclingUserInterface \
 -lclingInterpreter -lclingUserInterface -lclingEditLine \
 -lclangFrontend \
 -lclangSema -lclangLex -lclangParse -lclangCodeGen -lclangAnalysis \
 -lclangAST -lclangBasic \
 -lLLVMLinker -lLLVMipo -lLLVMInterpreter -lLLVMInstrumentation -lLLVMJIT \
 -lLLVMExecutionEngine -lLLVMBitWriter -lLLVMX86AsmParser -lLLVMX86AsmPrinter \
 -lLLVMX86CodeGen -lLLVMSelectionDAG -lLLVMX86Info -lLLVMAsmPrinter \
 -lLLVMCodeGen -lLLVMScalarOpts -lLLVMTransformUtils -lLLVMipa -lLLVMAsmParser \
 -lLLVMArchive -lLLVMBitReader -lLLVMAnalysis -lLLVMTarget -lLLVMMC -lLLVMCore \
 -lLLVMSupport -lLLVMSystem

# This hack does two things:
# 1. remove core/meta/inc/TCint.h from the list of files to be copied to include/
# 2. copy right now cint/cling/inc/TCint.h to include if include/TCint.h differs from cling's
ALLHEADERS := $(subst include/TCint.h,,$(ALLHEADRS)) $(shell diff $(CLINGDIRI)/TCint.h include/TCint.h > /dev/null || cp $(CLINGDIRI)/TCint.h include/TCint.h)
