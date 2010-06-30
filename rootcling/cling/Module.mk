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

CLINGL       := $(MODDIRI)/LinkDef.h
CLINGDS      := $(MODDIRS)/G__Cling.cxx
CLINGDO      := $(CLINGDS:.cxx=.o)
CLINGDH      := $(CLINGDS:.cxx=.h)

##### rootcling #####
ROOTCLINGS   := $(MODDIRS)/rootcling.cxx
ROOTCLINGO   := $(ROOTCLINGS:.cxx=.o)
ROOTCLING    := bin/rootcling$(EXEEXT)

##### libRCling #####
CLINGH       := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
CLINGS       := $(filter-out $(ROOTCLINGS),$(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx)))
CLINGO       := $(CLINGS:.cxx=.o)

CLINGDEP     := $(CLINGO:.o=.d) $(CLINGDO:.o=.d) $(ROOTCLINGO:.o=.d)

# used in the main Makefile
ALLHDRS      += $(patsubst $(MODDIRI)/%.h,include/%.h,$(CLINGH))
ALLEXECS     += $(ROOTCLING)

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

#$(CLINGLIB):    $(CLINGO) $(CLINGDO) $(ORDER_) $(MAINLIBS)
#		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
#		   "$(SOFLAGS)" libRCling.$(SOEXT) $@ "$(CLINGO) $(CLINGDO)" \
#		   "$(CLINGLIBEXTRA)" -L$(LLVMDIR)/lib -lCling

$(CLINGDS):     $(CLINGH) $(CLINGL) $(ROOTCINTTMPDEP)
		@echo "Generating dictionary $@..."
		$(ROOTCINTTMP) -f $@ -c $(CLINGH) $(CLINGL)

$(CLINGMAP):    $(RLIBMAP) $(MAKEFILEDEP) $(CLINGL)
		$(RLIBMAP) -o $(CLINGMAP) -l $(CLINGLIB) -d $(CLINGLIBDEPM) -c $(CLINGL)

$(ROOTCLING):   $(ROOTCLINGO) $(BOOTLIBSDEP)
		$(LD) $(LDFLAGS) -o $@ $(ROOTCLINGO) $(BOOTULIBS) \
		  $(RPATH) $(BOOTLIBS) $(CORELIBEXTRA) $(SYSLIBS)

all-$(MODNAME): $(CLINGLIB) $(CLINGMAP) $(ROOTCLING)

clean-$(MODNAME):
		@rm -f $(CLINGO) $(CLINGDO)

clean::         clean-$(MODNAME)

distclean-$(MODNAME): clean-$(MODNAME)
		@rm -f $(CLINGDEP) $(CLINGDS) $(CLINGDH) $(CLINGLIB) $(CLINGMAP)

distclean::     distclean-$(MODNAME)

##### extra rules ######
$(CLINGO) $(CLINGDO) $(ROOTCLINGO): CXXFLAGS += -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS \
                                  -I$(LLVMDIR)/include -I. -Wno-unused-parameter

CORELIBEXTRA += -L$(LLVMDIR)/lib -lclingInterpreter -lclingUserInterface \
 -lclingInterpreter -lclingMetaProcessor -lclingEditLine \
 -lclangFrontend \
 -lclangSema -lclangLex -lclangParse -lclangCodeGen -lclangAnalysis \
 -lclangBasic -lclangDriver -lclangAST -lclang -Llib -lReflex \
 -lLLVMLinker -lLLVMipo -lLLVMInterpreter -lLLVMInstrumentation -lLLVMJIT \
 -lLLVMExecutionEngine -lLLVMBitWriter -lLLVMX86AsmParser -lLLVMX86AsmPrinter \
 -lLLVMX86CodeGen -lLLVMSelectionDAG -lLLVMX86Info -lLLVMAsmPrinter \
 -lLLVMCodeGen -lLLVMScalarOpts -lLLVMTransformUtils -lLLVMipa -lLLVMAsmParser \
 -lLLVMArchive -lLLVMBitReader -lLLVMAnalysis -lLLVMTarget -lLLVMMCParser -lLLVMMC -lLLVMCore \
 -lLLVMSupport -lLLVMSystem
