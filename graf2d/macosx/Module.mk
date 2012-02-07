# Module.mk for macosx module
# Copyright (c) 2011 Rene Brun and Fons Rademakers
#
# Author: Timur Pocheptsov, 22/11/2011

MODNAME      := macosx
MODDIR       := $(ROOT_SRCDIR)/graf2d/$(MODNAME)
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

MACOSXDIR       := $(MODDIR)
MACOSXDIRS      := $(MACOSXDIR)/src
MACOSXDIRI      := $(MACOSXDIR)/inc

##### libGMacOSX #####
MACOSXL         := $(MODDIRI)/LinkDef.h
MACOSXDS        := $(call stripsrc,$(MODDIRS)/G__MACOSX.cxx)
MACOSXDO        := $(MACOSXDS:.cxx=.o)
MACOSXDH        := $(MACOSXDS:.cxx=.h)

MACOSXH1        := $(wildcard $(MODDIRI)/T*.h)
MACOSXH         := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
MACOSXS         := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
MACOSXSOBJCPP   := $(wildcard $(MODDIRS)/*.mm)
MACOSXOBJCPPO   := $(MACOSXSOBJCPP:.mm=.o)
MACOSXO         := $(MACOSXS:.cxx=.o)


MACOSXDEP       := $(MACOSXO:.o=.d) $(MACOSXDO:.o=.d) $(MACOSXOBJCPPO:.o=.d)

MACOSXLIB       := $(LPATH)/libGMACOSX.$(SOEXT)
MACOSXMAP       := $(MACOSXLIB:.$(SOEXT)=.rootmap)

# used in the main Makefile
ALLHDRS     += $(patsubst $(MODDIRI)/%.h,include/%.h,$(MACOSXH))
ALLLIBS     += $(MACOSXLIB)
ALLMAPS     += $(MACOSXMAP)

# include all dependency files
INCLUDEFILES += $(MACOSXDEP)

##### local rules #####
.PHONY:         all-$(MODNAME) clean-$(MODNAME) distclean-$(MODNAME)

include/%.h:    $(MACOSXDIRI)/%.h
		cp $< $@

$(MACOSXLIB):      $(MACOSXO) $(MACOSXOBJCPPO) $(MACOSXDO) $(ORDER_) $(MAINLIBS) $(MACOSXLIBDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)"  \
         "$(SOFLAGS)" libGMACOSX.$(SOEXT) $@  "$(MACOSXO) $(MACOSXOBJCPPO) -framework Cocoa $(MACOSXDO) $(MACOSXLIBEXTRA)"

$(MACOSXDS):       $(MACOSXH1) $(MACOSXL) $(ROOTCINTTMPDEP)
		$(MAKEDIR)
		@echo "Generating dictionary $@..."
		$(ROOTCINTTMP) -f $@ -c $(MACOSXH1) $(MACOSXL)

$(MACOSXMAP):      $(RLIBMAP) $(MAKEFILEDEP) $(MACOSXL)
		$(RLIBMAP) -o $@ -l $(MACOSXLIB) \
		   -d $(MACOSXLIBDEPM) -c $(MACOSXL)

all-$(MODNAME): $(MACOSXLIB) $(MACOSXMAP)

clean-$(MODNAME):
		@rm -f $(MACOSXO) $(MACOSXDO) $(MACOSXOBJCPPO)

clean::         clean-$(MODNAME)

distclean-$(MODNAME): clean-$(MODNAME)
		@rm -f $(MACOSXDEP) $(MACOSXDS) $(MACOSXDH) $(MACOSXLIB) $(MACOSXMAP)

distclean::     distclean-$(MODNAME)
