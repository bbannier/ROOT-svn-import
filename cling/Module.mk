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

### TODO: rename cling-based TCint to TCling, move into libRCling
CLINGLIB     := $(LPATH)/libRCling.$(SOEXT)
CLINGMAP     := $(CLINGLIB:.$(SOEXT)=.rootmap)

# used in the main Makefile
ALLHDRS      += $(patsubst $(MODDIRI)/%.h,include/%.h,$(CLINGH))
ALLLIBS      += $(CLINGLIB)
ALLMAPS      += $(CLINGMAP)

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

CLINGLIBEXTRA += -Llib -lReflex
CLINGLIBDEP   += $(REFLEXLIB)

.PHONY:         all-$(MODNAME) clean-$(MODNAME) distclean-$(MODNAME) check-cling-header

include/%.h:    $(CLINGDIRI)/%.h
		cp $< $@

$(CLINGLIB):    $(CLINGO) $(CLINGDO) $(CLINGLIBDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		   "$(SOFLAGS)" libRCling.$(SOEXT) $@ "$(CLINGO) $(CLINGDO)" \
		   "$(CLINGLIBEXTRA) -L$(LLVMDIR)/lib -lcling"

$(CLINGDS):     $(CLINGH) $(CLINGL) $(ROOTCINTTMPDEP)
		@echo "Generating dictionary $@..."
		$(ROOTCINTTMP) -f $@ -c $(CLINGH) $(CLINGL)

$(CLINGMAP):    $(RLIBMAP) $(MAKEFILEDEP) $(CLINGL)
		$(RLIBMAP) -o $@ -l $(CLINGLIB) -d $(CLINGLIBDEPM) -c $(CLINGL)

$(ROOTCLING):   $(ROOTCLINGO) $(BOOTLIBSDEP) $(CLINGLIBDEP)
		$(LD) $(LDFLAGS) -o $@ $(ROOTCLINGO) $(BOOTULIBS) \
		   $(CLINGLIBEXTRA) $(RPATH) $(BOOTLIBS) $(SYSLIBS) \
		   -L$(LLVMDIR)/lib -lcling 

all-$(MODNAME): $(CLINGLIB) $(CLINGMAP) $(ROOTCLING)

clean-$(MODNAME):
		@rm -f $(CLINGO) $(CLINGDO)

clean::         clean-$(MODNAME)

distclean-$(MODNAME): clean-$(MODNAME)
		@rm -f $(CLINGDEP) $(CLINGDS) $(CLINGDH) $(CLINGLIB) $(CLINGMAP)

distclean::     distclean-$(MODNAME)
		@(find . -name "*_dicthdr.h" -exec rm -f {} \; >/dev/null 2>&1;true)
		@rm -f lib/*.pch

##### extra rules ######
$(CLINGO) $(CLINGDO) $(ROOTCLINGO): CXXFLAGS += -D__STDC_LIMIT_MACROS \
   -D__STDC_CONSTANT_MACROS \
   -I$(LLVMDIR)/include -I. -Wno-unused-parameter -Wno-shadow

#CORELIBEXTRA += -L$(LLVMDIR)/lib -lclingInterpreter -lclingUserInterface \
# -lclingInterpreter -lclingMetaProcessor -lclingEditLine -lclangFrontend \
# -lclangSerialization -lclangSema -lclangLex -lclangParse -lclangCodeGen -lclangAnalysis \
# -lclangBasic -lclangDriver -lclangAST -Llib -lReflex -lLLVMMCDisassembler \
# -lLLVMLinker -lLLVMipo -lLLVMInterpreter -lLLVMInstrumentation -lLLVMJIT \
# -lLLVMExecutionEngine -lLLVMBitWriter -lLLVMX86Disassembler \
# -lLLVMX86AsmParser -lLLVMX86CodeGen -lLLVMSelectionDAG -lLLVMX86AsmPrinter \
# -lLLVMX86Info -lLLVMAsmPrinter -lLLVMMCParser -lLLVMCodeGen -lLLVMScalarOpts \
# -lLLVMInstCombine -lLLVMTransformUtils -lLLVMipa -lLLVMAsmParser \
# -lLLVMArchive -lLLVMBitReader -lLLVMAnalysis -lLLVMTarget -lLLVMMC \
# -lLLVMCore -lLLVMSupport -lLLVMSystem
CORELIBEXTRA += -Llib -lRCling
CORELIBDEP += $(CLINGLIB)
