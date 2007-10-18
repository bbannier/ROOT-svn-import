# Module.mk for netx module
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: G. Ganis, 8/7/2004

MODDIR       := netx
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

NETXDIR      := $(MODDIR)
NETXDIRS     := $(NETXDIR)/src
NETXDIRI     := $(NETXDIR)/inc

##### libNetx #####
NETXL        := $(MODDIRI)/LinkDef.h
NETXDS       := $(MODDIRS)/G__Netx.cxx
NETXDO       := $(NETXDS:.cxx=.o)
NETXDH       := $(NETXDS:.cxx=.h)

NETXH        := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
NETXS        := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
NETXO        := $(NETXS:.cxx=.o)

#LF
NETXTMPDS    := $(MODDIRS)/G__NetxTmp.cxx
NETXTMPDO    := $(NETXTMPDS:.cxx=.o)
NETXTMPDH    := $(NETXTMPDS:.cxx=.h)
NETXTMP2DS   := $(MODDIRS)/G__NetxTmp2.cxx
NETXTMP2DO   := $(NETXTMP2DS:.cxx=.o)
NETXTMP2DH   := $(NETXTMP2DS:.cxx=.h)

NETXDEP      := $(NETXO:.o=.d) $(NETXDO:.o=.d)

#LF
NETXTMPDEP  := $(NETXTMPDO:.o=.d)

NETXLIB      := $(LPATH)/libNetx.$(SOEXT)
NETXMAP      := $(NETXLIB:.$(SOEXT)=.rootmap)

#LF
NETXNM       := $(NETXLIB:.$(SOEXT)=.nm)

# used in the main Makefile
ALLHDRS      += $(patsubst $(MODDIRI)/%.h,include/%.h,$(NETXH))
ALLLIBS      += $(NETXLIB)
ALLMAPS      += $(NETXMAP)

# include all dependency files
INCLUDEFILES += $(NETXDEP)

# These are undefined if using an external XROOTD distribution
# The new XROOTD build system based on autotools installs the headers
# under <dir>/include/xrootd, while the old system under <dir>/src
ifneq ($(XROOTDDIR),)
ifeq ($(XROOTDDIRI),)
XROOTDDIRI   := $(XROOTDDIR)/include/xrootd
ifeq ($(wildcard $(XROOTDDIRI)/*.hh),)
XROOTDDIRI   := $(XROOTDDIR)/src
endif
XROOTDDIRL   := $(XROOTDDIR)/lib
XROOTDDIRP   := $(XROOTDDIRL)
endif
endif

# Xrootd includes
NETXINCEXTRA := $(XROOTDDIRI:%=-I%)
ifneq ($(EXTRA_XRDFLAGS),)
NETXINCEXTRA += -Iproofd/inc
endif

# Xrootd client libs
ifeq ($(PLATFORM),win32)
NETXLIBEXTRA += $(XROOTDDIRL)/libXrdClient.lib
else
NETXLIBEXTRA += -L$(XROOTDDIRL) -lXrdOuc -lXrdSys \
                -L$(XROOTDDIRP) -lXrdClient
endif

##### local rules #####
include/%.h:    $(NETXDIRI)/%.h $(XROOTDETAG)
		cp $< $@

#LF
$(NETXLIB):   $(NETXO) $(NETXTMPDO) $(NETXTMP2DO) $(NETXDO) $(ORDER_) $(MAINLIBS) $(NETXLIBDEP) \
		$(XRDPLUGINS)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		   "$(SOFLAGS)" libNetx.$(SOEXT) $@ "$(NETXO) $(NETXTMPDO) $(NETXTMP2DO) $(NETXDO)" \
		   "$(NETXLIBEXTRA)"
#LF
$(NETXTMPDS):   $(NETXH) $(NETXL) $(ROOTCINTTMPEXE) $(XROOTDETAG)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(NETXINCEXTRA) $(NETXH) $(NETXL)

#LF
$(NETXTMP2DS):  $(NETXH1) $(NETXL) $(ROOTCINTTMPEXE) $(XROOTDETAG)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(NETXINCEXTRA) $(NETXH) $(NETXL)

#LF
$(NETXDS):    $(NETXH1) $(NETXL) $(ROOTCINTTMPEXE) $(XROOTDETAG) $(NETXNM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(NETXNM) -. 3 -c $(NETXINCEXTRA) $(NETXH) $(NETXL)

#LF
$(NETXNM):      $(NETXO) $(NETXTMPDO) $(NETXTMP2DO) 
		@echo "Generating symbols file $@..."
		nm -p --defined-only $(NETXTMPDO) | awk '{printf("%s\n", $$3)'} > $(NETXNM)
		nm -p --defined-only $(NETXTMP2DO) | awk '{printf("%s\n", $$3)'} >> $(NETXNM)
		nm -p --defined-only $(NETXO) | awk '{printf("%s\n", $$3)'} >> $(NETXNM)

$(NETXMAP):     $(RLIBMAP) $(MAKEFILEDEP) $(NETXL)
		$(RLIBMAP) -o $(NETXMAP) -l $(NETXLIB) -d $(NETXLIBDEPM) -c $(NETXL)

all-netx:       $(NETXLIB) $(NETXMAP)

clean-netx:
		@rm -f $(NETXO) $(NETXDO)

clean::         clean-netx clean-pds-netx

#LF
clean-pds-netx:	
		rm -f $(NETXTMPDS) $(NETXTMPDO) $(NETXTMPDH) \
		$(NETXTMPDEP) $(NETXTMP2DS) $(NETXTMP2DO) $(NETXTMP2DH) $(NETXNM)

distclean-netx: clean-netx
		@rm -f $(NETXDEP) $(NETXDS) $(NETXDH) $(NETXLIB) $(NETXMAP)

distclean::     distclean-netx

##### extra rules ######
$(NETXO) $(NETXTMPDO) $(NETXTMP2DO) $(NETXDO): $(XROOTDETAG)
$(NETXO) $(NETXTMPDO) $(NETXTMP2DO) $(NETXDO): CXXFLAGS += $(NETXINCEXTRA) $(EXTRA_XRDFLAGS)
ifeq ($(PLATFORM),win32)
$(NETXO) $(NETXTMPDO) $(NETXTMP2DO) $(NETXDO): CXXFLAGS += -DNOGDI $(EXTRA_XRDFLAGS)
endif

