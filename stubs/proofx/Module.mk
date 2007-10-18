# Module.mk for the proofx module
# Copyright (c) 2005 Rene Brun and Fons Rademakers
#
# Author: Gerardo Ganis  12/12/2005

MODDIR       := proofx
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

PROOFXDIR    := $(MODDIR)
PROOFXDIRS   := $(PROOFXDIR)/src
PROOFXDIRI   := $(PROOFXDIR)/inc

##### libProofx #####
PROOFXL      := $(MODDIRI)/LinkDef.h
PROOFXDS     := $(MODDIRS)/G__Proofx.cxx
PROOFXDO     := $(PROOFXDS:.cxx=.o)
PROOFXDH     := $(PROOFXDS:.cxx=.h)

ifeq ($(PLATFORM),win32)
PROOFXH      := $(MODDIRI)/TXProofMgr.h $(MODDIRI)/TXSlave.h \
                $(MODDIRI)/TXSocket.h $(MODDIRI)/TXSocketHandler.h \
                $(MODDIRI)/TXHandler.h
PROOFXS      := $(MODDIRS)/TXProofMgr.cxx $(MODDIRS)/TXSlave.cxx \
                $(MODDIRS)/TXSocket.cxx $(MODDIRS)/TXSocketHandler.cxx \
                $(MODDIRS)/TXHandler.cxx
else
PROOFXH      := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
PROOFXS      := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
endif
PROOFXO      := $(PROOFXS:.cxx=.o)

#LF
PROOFXTMPDS    := $(MODDIRS)/G__ProofxTmp.cxx
PROOFXTMPDO    := $(PROOFXTMPDS:.cxx=.o)
PROOFXTMPDH    := $(PROOFXTMPDS:.cxx=.h)
PROOFXTMP2DS   := $(MODDIRS)/G__ProofxTmp2.cxx
PROOFXTMP2DO   := $(PROOFXTMP2DS:.cxx=.o)
PROOFXTMP2DH   := $(PROOFXTMP2DS:.cxx=.h)

PROOFXDEP    := $(PROOFXO:.o=.d) $(PROOFXDO:.o=.d)

#LF
PROOFXTMPDEP  := $(PROOFXTMPDO:.o=.d)

PROOFXLIB    := $(LPATH)/libProofx.$(SOEXT)
PROOFXMAP    := $(PROOFXLIB:.$(SOEXT)=.rootmap)

#LF
PROOFXNM       := $(PROOFXLIB:.$(SOEXT)=.nm)

# used in the main Makefile
ALLHDRS      += $(patsubst $(MODDIRI)/%.h,include/%.h,$(PROOFXH))
ALLLIBS      += $(PROOFXLIB)
ALLMAPS      += $(PROOFXMAP)

# include all dependency files
INCLUDEFILES += $(PROOFXDEP)

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
PROOFXINCEXTRA := $(PROOFXDIRI:%=-I%)
PROOFXINCEXTRA += $(XROOTDDIRI:%=-I%)
PROOFXINCEXTRA += $(PROOFDDIRI:%=-I%)

# Xrootd client libs
ifeq ($(PLATFORM),win32)
PROOFXLIBEXTRA += $(XROOTDDIRL)/libXrdClient.lib
else
PROOFXLIBEXTRA += -L$(XROOTDDIRL) -lXrdOuc -lXrdSys -lXrdNet \
                  -L$(XROOTDDIRP) -lXrdClient
endif

##### local rules #####
include/%.h:    $(PROOFXDIRI)/%.h $(XROOTDETAG)
		cp $< $@

#LF
$(PROOFXLIB):   $(PROOFXO) $(PROOFXTMPDO) $(PROOFXTMP2DO) $(PROOFXDO) $(ORDER_) $(MAINLIBS) \
		$(PROOFXLIBDEP) $(XRDPLUGINS)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		   "$(SOFLAGS)" libProofx.$(SOEXT) $@ "$(PROOFXO)  $(XPCONNO) \
			$(PROOFXTMPDO) $(PROOFXTMP2DO) $(PROOFXDO)" \
		   "$(PROOFXLIBEXTRA)"

#LF
$(PROOFXTMPDS):   $(PROOFXH) $(PROOFXL) $(ROOTCINTTMPEXE) $(XROOTDETAG)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(PROOFXINCEXTRA) $(PROOFXH) $(PROOFXL)

#LF
$(PROOFXTMP2DS):  $(PROOFXH) $(PROOFXL) $(ROOTCINTTMPEXE) $(XROOTDETAG)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(PROOFXINCEXTRA) $(PROOFXH) $(PROOFXL)

#LF
$(PROOFXDS):    $(PROOFXH) $(PROOFXL) $(ROOTCINTTMPEXE) $(XROOTDETAG) $(PROOFXNM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(PROOFXNM) -. 3 -c $(PROOFXINCEXTRA) $(PROOFXH) $(PROOFXL)

#LF
$(PROOFXNM):      $(PROOFXO) $(PROOFXTMPDO) $(PROOFXTMP2DO) 
		@echo "Generating symbols file $@..."
		nm -p --defined-only $(PROOFXTMPDO) | awk '{printf("%s\n", $$3)'} > $(PROOFXNM)
		nm -p --defined-only $(PROOFXTMP2DO) | awk '{printf("%s\n", $$3)'} >> $(PROOFXNM)
		nm -p --defined-only $(PROOFXO) | awk '{printf("%s\n", $$3)'} >> $(PROOFXNM)

$(PROOFXMAP):   $(RLIBMAP) $(MAKEFILEDEP) $(PROOFXL)
		$(RLIBMAP) -o $(PROOFXMAP) -l $(PROOFXLIB) \
		   -d $(PROOFXLIBDEPM) -c $(PROOFXL)

all-proofx:     $(PROOFXLIB) $(PROOFXMAP)

clean-proofx:
		@rm -f $(PROOFXO) $(PROOFXDO)

clean::         clean-proofx clean-pds-proofx

#LF
clean-pds-proofx:	
		rm -f $(PROOFXTMPDS) $(PROOFXTMPDO) $(PROOFXTMPDH) \
		$(PROOFXTMPDEP) $(PROOFXTMP2DS) $(PROOFXTMP2DO) $(PROOFXTMP2DH) $(PROOFXNM)

distclean-proofx: clean-proofx
		@rm -f $(PROOFXDEP) $(PROOFXDS) $(PROOFXDH) $(PROOFXLIB) $(PROOFXMAP)

distclean::     distclean-proofx

##### extra rules ######
$(PROOFXO) $(PROOFXTMPDO) $(PROOFXTMP2DO) $(PROOFXDO): $(XROOTDETAG)

ifeq ($(PLATFORM),win32)
$(PROOFXO) $(PROOFXTMPDO) $(PROOFXTMP2DO) $(PROOFXDO): CXXFLAGS += $(PROOFXINCEXTRA) $(EXTRA_XRDFLAGS)
else
ifneq ($(ICC_GE_9),)
# remove when xrootd has moved from strstream.h -> sstream.
$(PROOFXO) $(PROOFXTMPDO) $(PROOFXTMP2DO) $(PROOFXDO): CXXFLAGS += -Wno-deprecated $(PROOFXINCEXTRA) $(EXTRA_XRDFLAGS)
else
ifneq ($(GCC_MAJOR),2)
# remove when xrootd has moved from strstream.h -> sstream.
$(PROOFXO) $(PROOFXTMPDO) $(PROOFXTMP2DO) $(PROOFXDO): CXXFLAGS += -Wno-deprecated $(PROOFXINCEXTRA) $(EXTRA_XRDFLAGS)
else
$(PROOFXO) $(PROOFXTMPDO) $(PROOFXTMP2DO) $(PROOFXDO): CXXFLAGS += $(PROOFXINCEXTRA) $(EXTRA_XRDFLAGS)
endif
endif
endif
