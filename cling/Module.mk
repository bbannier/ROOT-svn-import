# Module.mk for cling module
# Copyright (c) 2007 Rene Brun and Fons Rademakers
#
# Author: Axel Naumann, 2009-10-06

MODNAME      := cling
MODDIR       := cling/$(MODNAME)
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

CLINGLIB     := $(LPATH)/libRCling.$(SOEXT)
CLINGMAP     := $(CLINGLIB:.$(SOEXT)=.rootmap)

# used in the main Makefile
ALLHDRS      += $(patsubst $(MODDIRI)/%.h,include/%.h,$(CLINGH))
ALLLIBS      += $(CLINGLIB)
ALLMAPS      += $(CLINGMAP)

# include all dependency files
INCLUDEFILES += $(CLINGDEP)

##### local rules #####
.PHONY:         all-$(MODNAME) clean-$(MODNAME) distclean-$(MODNAME)

include/%.h:    $(CLINGDIRI)/%.h
		cp $< $@

$(CLINGLIB):    $(CLINGO) $(CLINGDO) $(ORDER_) $(MAINLIBS)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		   "$(SOFLAGS)" libRCling.$(SOEXT) $@ "$(CLINGO) $(CLINGDO)" \
		   "$(CLINGLIBEXTRA)"

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

##### extra rules ######

