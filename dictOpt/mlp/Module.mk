# Module.mk for mlp module
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: Rene Brun, 27/8/2003

MODDIR       := mlp
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

MLPDIR       := $(MODDIR)
MLPDIRS      := $(MLPDIR)/src
MLPDIRI      := $(MLPDIR)/inc

##### libMLP #####
MLPL         := $(MODDIRI)/LinkDef.h
MLPDS        := $(MODDIRS)/G__MLP.cxx
MLPDO        := $(MLPDS:.cxx=.o)
MLPDH        := $(MLPDS:.cxx=.h)

#LF
MLPTMPDS    := $(MODDIRS)/G__MLPTmp.cxx
MLPTMPDO    := $(MLPTMPDS:.cxx=.o)
MLPTMPDH    := $(MLPTMPDS:.cxx=.h)
MLPTMP2DS   := $(MODDIRS)/G__MLPTmp2.cxx
MLPTMP2DO   := $(MLPTMP2DS:.cxx=.o)
MLPTMP2DH   := $(MLPTMP2DS:.cxx=.h)

MLPH         := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
MLPS         := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
MLPO         := $(MLPS:.cxx=.o)

MLPDEP       := $(MLPO:.o=.d) $(MLPDO:.o=.d)

#LF
MLPTMPDEP  := $(MLPTMPDO:.o=.d)

MLPLIB       := $(LPATH)/libMLP.$(SOEXT)
MLPMAP       := $(MLPLIB:.$(SOEXT)=.rootmap)

#LF
MLPNM       := $(MLPLIB:.$(SOEXT)=.nm)

# used in the main Makefile
ALLHDRS     += $(patsubst $(MODDIRI)/%.h,include/%.h,$(MLPH))
ALLLIBS     += $(MLPLIB)
ALLMAPS     += $(MLPMAP)

# include all dependency files
INCLUDEFILES += $(MLPDEP)

##### local rules #####
include/%.h:    $(MLPDIRI)/%.h
		cp $< $@

#LF
$(MLPLIB):   $(MLPO) $(MLPTMPDO) $(MLPTMP2DO) $(MLPDO) $(ORDER_) $(MAINLIBS) $(MLPLIBDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		   "$(SOFLAGS)" libMLP.$(SOEXT) $@ "$(MLPO) $(MLPTMPDO) $(MLPTMP2DO) $(MLPDO)" \
		   "$(MLPLIBEXTRA)"

#LF
$(MLPTMPDS):   $(MLPH) $(MLPL) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(MLPH) $(MLPL)

#LF
$(MLPTMP2DS):  $(MLPH) $(MLPL) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(MLPH) $(MLPL)

#LF
$(MLPDS):    $(MLPH) $(MLPL) $(ROOTCINTTMPEXE) $(MLPNM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(MLPNM) -. 3 -c $(MLPH) $(MLPL)

#LF
$(MLPNM):      $(MLPO) $(MLPTMPDO) $(MLPTMP2DO) 
		@echo "Generating symbols file $@..."
		nm -g -p --defined-only $(MLPTMPDO) | awk '{printf("%s\n", $$3)'} > $(MLPNM)
		nm -g -p --defined-only $(MLPTMP2DO) | awk '{printf("%s\n", $$3)'} >> $(MLPNM)
		nm -g -p --defined-only $(MLPO) | awk '{printf("%s\n", $$3)'} >> $(MLPNM)

$(MLPMAP):      $(RLIBMAP) $(MAKEFILEDEP) $(MLPL)
		$(RLIBMAP) -o $(MLPMAP) -l $(MLPLIB) -d $(MLPLIBDEPM) -c $(MLPL)

all-mlp:        $(MLPLIB) $(MLPMAP)

clean-mlp:
		@rm -f $(MLPO) $(MLPDO)

clean::         clean-mlp clean-pds-mlp

#LF
clean-pds-mlp:	
		rm -f $(MLPTMPDS) $(MLPTMPDO) $(MLPTMPDH) \
		$(MLPTMPDEP) $(MLPTMP2DS) $(MLPTMP2DO) $(MLPTMP2DH) $(MLPNM)

distclean-mlp:  clean-mlp
		@rm -f $(MLPDEP) $(MLPDS) $(MLPDH) $(MLPLIB) $(MLPMAP)

distclean::     distclean-mlp
