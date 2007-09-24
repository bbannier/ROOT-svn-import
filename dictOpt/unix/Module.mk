# Module.mk for unix module
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: Fons Rademakers, 29/2/2000

MODDIR       := unix
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

UNIXDIR      := $(MODDIR)
UNIXDIRS     := $(UNIXDIR)/src
UNIXDIRI     := $(UNIXDIR)/inc

##### libUnix (part of libCore) #####
UNIXL        := $(MODDIRI)/LinkDef.h
UNIXDS       := $(MODDIRS)/G__Unix.cxx
UNIXDO       := $(UNIXDS:.cxx=.o)
UNIXDH       := $(UNIXDS:.cxx=.h)

#LF
UNIXTMPDS    := $(MODDIRS)/G__HistTmp.cxx
UNIXTMPDO    := $(UNIXTMPDS:.cxx=.o)
UNIXTMPDH    := $(UNIXTMPDS:.cxx=.h)
UNIXTMP2DS    := $(MODDIRS)/G__HistTmp2.cxx
UNIXTMP2DO    := $(UNIXTMP2DS:.cxx=.o)
UNIXTMP2DH    := $(UNIXTMP2DS:.cxx=.h)

UNIXH        := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
UNIXS        := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
UNIXO        := $(UNIXS:.cxx=.o)

UNIXDEP      := $(UNIXO:.o=.d) $(UNIXDO:.o=.d)

#LF
UNIXTMPDEP   := $(UNIXTMPDO:.o=.d)

#LF
UNIXTMPDEP   := $(UNIXTMPDO:.o=.d)

#LF
UNIXLIB      := $(MODDIRS)/libUnix.$(SOEXT)
UNIXNM       := $(UNIXLIB:.$(SOEXT)=.nm)

# used in the main Makefile
ALLHDRS     += $(patsubst $(MODDIRI)/%.h,include/%.h,$(UNIXH))

# include all dependency files
INCLUDEFILES += $(UNIXDEP)

##### local rules #####
include/%.h:    $(UNIXDIRI)/%.h
		cp $< $@

#LF
$(UNIXTMPDS):   $(UNIXH) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(UNIXH) $(UNIXL)
#LF
$(UNIXTMP2DS):  $(UNIXH) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(UNIXH) $(UNIXL)
#LF
$(UNIXDS):     $(UNIXH) $(UNIXL) $(ROOTCINTTMPEXE) $(UNIXNM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(UNIXNM) -. 3 -c $(UNIXH) $(UNIXL)

#LF
$(UNIXNM):      $(UNIXO) $(UNIXTMP2DO)
		@echo "Generating symbols file $@..."
		nm -g -p --defined-only $(UNIXTMP2DO) | awk '{printf("%s\n", $$3)'} > $(UNIXNM)
		nm -g -p --defined-only $(UNIXO) | awk '{printf("%s\n", $$3)'} >> $(UNIXNM)

all-unix:       $(UNIXO) $(UNIXDO)

clean-unix:
		@rm -f $(UNIXO) $(UNIXDO)

clean::         clean-unix

distclean-unix: clean-unix
		@rm -f $(UNIXDEP) $(UNIXDS) $(UNIXDH)

distclean::     distclean-unix
