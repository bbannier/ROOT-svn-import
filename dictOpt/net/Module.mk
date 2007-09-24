# Module.mk for net module
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: Fons Rademakers, 29/2/2000

MODDIR       := net
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

NETDIR       := $(MODDIR)
NETDIRS      := $(NETDIR)/src
NETDIRI      := $(NETDIR)/inc

##### libNet #####
NETL         := $(MODDIRI)/LinkDef.h
NETDS        := $(MODDIRS)/G__Net.cxx
NETDO        := $(NETDS:.cxx=.o)
NETDH        := $(NETDS:.cxx=.h)

#LF
NETTMPDS    := $(MODDIRS)/G__NetTmp.cxx
NETTMPDO    := $(NETTMPDS:.cxx=.o)
NETTMPDH    := $(NETTMPDS:.cxx=.h)
NETTMP2DS    := $(MODDIRS)/G__NetTmp2.cxx
NETTMP2DO    := $(NETTMP2DS:.cxx=.o)
NETTMP2DH    := $(NETTMP2DS:.cxx=.h)

NETH         := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
NETS         := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
NETO         := $(NETS:.cxx=.o)

NETDEP       := $(NETO:.o=.d) $(NETDO:.o=.d)

#LF
NETTMPDEP   := $(NETTMPDO:.o=.d)

NETLIB       := $(LPATH)/libNet.$(SOEXT)

#LF
NETMAP       := $(NETLIB:.$(SOEXT)=.rootmap)
NETDICTLIB  := $(LPATH)/libNetDict.$(SOEXT)
NETDICTMAP  := $(NETDICTLIB:.$(SOEXT)=.rootmap)

#LF
NETNM       := $(NETLIB:.$(SOEXT)=.nm)

EXTRANETFLAGS =

# used in the main Makefile
ALLHDRS      += $(patsubst $(MODDIRI)/%.h,include/%.h,$(NETH))
ALLLIBS      += $(NETLIB)
ALLMAPS      += $(NETMAP)

# include all dependency files
INCLUDEFILES += $(NETDEP)

##### local rules #####
include/%.h:    $(NETDIRI)/%.h
		cp $< $@

#LF
$(NETLIB):     $(NETO) $(NETTMPDO) $(NETTMP2DO) $(NETDO) $(ORDER_) $(MAINLIBS) $(NETLIBDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		"$(SOFLAGS)" libNet.$(SOEXT) $@ "$(NETO) $(NETTMPDO) $(NETTMP2DO) $(NETDO)" \
		"$(NETLIBEXTRA)"

#LF
#$(NETDICTLIB): $(NETDO) $(ORDER_) $(MAINLIBS) $(NETDICTLIBDEP)
#		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
#		"$(SOFLAGS)" libGedDict.$(SOEXT) $@ "$(NETDO) $(NETTMP2DO)"\
#		"$(NETDICTLIBEXTRA)"

#LF
$(NETTMPDS):   $(NETH) $(NETL) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(NETH) $(NETL)

#LF
$(NETTMP2DS):  $(NETH) $(NETL) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(NETH) $(NETL)

#LF
$(NETDS):      $(NETH) $(NETL) $(ROOTCINTTMPEXE) $(NETNM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(NETNM) -. 3 -c $(NETH) $(NETL)
#LF
$(NETMAP): $(RLIBMAP) $(MAKEFILEDEP) $(NETL)
		$(RLIBMAP) -o $(NETMAP) -l $(NETLIB) \
		-d $(NETLIBDEPM) -c $(NETL)
#LF
$(NETNM):      $(NETO) $(NETTMPDO) $(NETTMP2DO)
		@echo "Generating symbols file $@..."
		nm -g -p --defined-only $(NETTMPDO) | awk '{printf("%s\n", $$3)'} >> $(NETNM)
		nm -g -p --defined-only $(NETTMP2DO) | awk '{printf("%s\n", $$3)'} >> $(NETNM)
		nm -g -p --defined-only $(NETO) | awk '{printf("%s\n", $$3)'} >> $(NETNM)

all-net:        $(NETLIB) $(NETMAP)

clean-net:
		@rm -f $(NETO) $(NETDO)

clean::         clean-net clean-pds-net

#LF
clean-pds-net:	
		rm -f $(NETTMPDS) $(NETTMPDO) $(NETTMPDH) \
		$(NETTMPDEP) $(NETTMP2DS) $(NETTMP2DO) $(NETTMP2DH) $(NETNM)

distclean-net:  clean-net
		@rm -f $(NETDEP) $(NETDS) $(NETDH) $(NETLIB) $(NETMAP)

distclean::     distclean-net
