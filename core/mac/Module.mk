# Module.mk for mac module
# Copyright (c) 2011 Rene Brun and Fons Rademakers
#
# Author: Timur Pocheptsov, 5/12/2011

MODNAME      := mac
MODDIR       := $(ROOT_SRCDIR)/core/$(MODNAME)
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

MACDIR       := $(MODDIR)
MACDIRS      := $(MACDIR)/src
MACDIRI      := $(MACDIR)/inc

##### libMac  (part of libCore) #####
MACL	       := $(MODDIRI)/LinkDef.h
MACDS			 := $(MODDIRS)/G__Mac.cxx
MACDO        := $(MODDIRS)/G__Mac.o
MACDH        := $(MACDS:.mm=.h)

MACH         := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
MACS         := $(wildcard $(MODDIRS)/*.mm)
MACO         := $(call stripsrc,$(MACS:.mm=.o))

MACDEP		:= $(MACO:.o=.d) $(MACDO:.o=.d)

# used in the main Makefile
ALLHDRS     += $(patsubst $(MODDIRI)/%.h,include/%.h,$(MACH))

# include all dependency files
INCLUDEFILES += $(MACDEP)

##### local rules #####
.PHONY:         all-$(MODNAME) clean-$(MODNAME) distclean-$(MODNAME)

include/%.h:    $(MACDIRI)/%.h
		cp $< $@

$(MACDS):      $(MACH) $(MACL) $(ROOTCINTTMPDEP)
		$(MAKEDIR)
		@echo "Generating dictionary $@..."
		$(ROOTCINTTMP) -f $@ -c $(MACH) $(MACL)

all-$(MODNAME): $(MACO) $(MACDO)

clean-$(MODNAME):
		@rm -f $(MACO) $(MACDO)

clean::         clean-$(MODNAME)

#distclean-$(MODNAME): clean-$(MODNAME)
#		@rm -f $(MACDEP) $(MACDS) $(MACDH)

distclean::     distclean-$(MODNAME)
