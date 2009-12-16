# Module.mk for the bench module
# Copyright (c) 2005 Rene Brun and Fons Rademakers
#
# Author: 

MODNAME      := proofbench
MODDIR       := proof/$(MODNAME)
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

BENCHDIR    := $(MODDIR)
BENCHDIRS   := $(BENCHDIR)/src
BENCHDIRI   := $(BENCHDIR)/inc

##### libProofBench #####
BENCHL      := $(MODDIRI)/LinkDef.h
BENCHDS     := $(MODDIRS)/G__Bench.cxx
BENCHDO     := $(BENCHDS:.cxx=.o)
BENCHDH     := $(BENCHDS:.cxx=.h)

BENCHH      := $(MODDIRI)/TProofBench.h
ifeq ($(PLATFORM),win32)
BENCHS      := $(MODDIRS)/TProofBench.cxx
else
BENCHH      := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
BENCHS      := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
endif
# BENCHH      += test/Event.h
BENCHS      += test/Event.cxx
BENCHO      := $(BENCHS:.cxx=.o)

BENCHDEP    := $(BENCHO:.o=.d) $(BENCHDO:.o=.d)

BENCHLIB    := $(LPATH)/libProofBench.$(SOEXT)
BENCHMAP    := $(BENCHLIB:.$(SOEXT)=.rootmap)

# used in the main Makefile
ALLHDRS      += $(patsubst $(MODDIRI)/%.h,include/%.h,$(BENCHH))
ALLLIBS      += $(BENCHLIB)
ALLMAPS      += $(BENCHMAP)

# include all dependency files
INCLUDEFILES += $(BENCHDEP)

##### local rules #####
.PHONY:         all-$(MODNAME) clean-$(MODNAME) distclean-$(MODNAME)

include/%.h:    $(BENCHDIRI)/%.h
		cp $< $@

$(BENCHLIB):   $(BENCHO) $(BENCHDO) $(ORDER_) $(MAINLIBS) \
                $(BENCHLIBDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		   "$(SOFLAGS)" libProofBench.$(SOEXT) $@ \
		   "$(BENCHO) $(BENCHDO)"

$(BENCHDS):    $(BENCHH) $(BENCHL) $(ROOTCINTTMPDEP)
		@echo "Generating dictionary $@..."
		$(ROOTCINTTMP) -f $@ -c $(BENCHH) $(BENCHL)

$(BENCHMAP):   $(RLIBMAP) $(MAKEFILEDEP) $(BENCHL)
		$(RLIBMAP) -o $(BENCHMAP) -l $(BENCHLIB) \
		   -d $(PROOFBENCHLIBDEPM) -c $(BENCHL)

all-$(MODNAME): $(BENCHLIB) $(BENCHMAP)

clean-$(MODNAME):
		@rm -f $(BENCHO) $(BENCHDO)

clean::         clean-$(MODNAME)

distclean-$(MODNAME): clean-$(MODNAME)
		@rm -f $(BENCHDEP) $(BENCHDS) $(BENCHDH) $(BENCHLIB) $(BENCHMAP)

distclean::     distclean-$(MODNAME)

##### extra rules ######
$(BENCHO) $(BENCHDO): CXXFLAGS += -I.
