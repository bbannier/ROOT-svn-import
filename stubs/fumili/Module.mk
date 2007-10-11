# Module.mk for fumili module
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: Rene Brun, 07/05/2003

MODDIR       := fumili
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

FUMILIDIR    := $(MODDIR)
FUMILIDIRS   := $(FUMILIDIR)/src
FUMILIDIRI   := $(FUMILIDIR)/inc

##### libFumili #####
FUMILIL      := $(MODDIRI)/LinkDef.h
FUMILIDS     := $(MODDIRS)/G__Fumili.cxx
FUMILIDO     := $(FUMILIDS:.cxx=.o)
FUMILIDH     := $(FUMILIDS:.cxx=.h)

#LF
FUMILITMPDS    := $(MODDIRS)/G__FumiliTmp.cxx
FUMILITMPDO    := $(FUMILITMPDS:.cxx=.o)
FUMILITMPDH    := $(FUMILITMPDS:.cxx=.h)
FUMILITMP2DS   := $(MODDIRS)/G__FumiliTmp2.cxx
FUMILITMP2DO   := $(FUMILITMP2DS:.cxx=.o)
FUMILITMP2DH   := $(FUMILITMP2DS:.cxx=.h)

FUMILIH      := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
FUMILIS      := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
FUMILIO      := $(FUMILIS:.cxx=.o)

FUMILIDEP    := $(FUMILIO:.o=.d) $(FUMILIDO:.o=.d)

#LF
FUMILITMPDEP  := $(FUMILITMPDO:.o=.d)

FUMILILIB    := $(LPATH)/libFumili.$(SOEXT)
FUMILIMAP    := $(FUMILILIB:.$(SOEXT)=.rootmap)

#LF
FUMILINM       := $(FUMILILIB:.$(SOEXT)=.nm)

# used in the main Makefile
ALLHDRS     += $(patsubst $(MODDIRI)/%.h,include/%.h,$(FUMILIH))
ALLLIBS     += $(FUMILILIB)
ALLMAPS     += $(FUMILIMAP)

# include all dependency files
INCLUDEFILES += $(FUMILIDEP)

##### local rules #####
include/%.h:    $(FUMILIDIRI)/%.h
		cp $< $@

#LF
$(FUMILILIB):   $(FUMILIO) $(FUMILITMPDO) $(FUMILITMP2DO) $(FUMILIDO) $(ORDER_) $(MAINLIBS) $(FUMILILIBDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		   "$(SOFLAGS)" libFumili.$(SOEXT) $@ "$(FUMILIO) $(FUMILITMPDO) $(FUMILITMP2DO) $(FUMILIDO)" \
		   "$(FUMILILIBEXTRA)"

#LF
$(FUMILITMPDS):   $(FUMILIH) $(FUMILIL) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(FUMILIH) $(FUMILIL)

#LF
$(FUMILITMP2DS):  $(FUMILIH) $(FUMILIL) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(FUMILIH) $(FUMILIL)

#LF
$(FUMILIDS):    $(FUMILIH) $(FUMILIL) $(ROOTCINTTMPEXE) $(FUMILINM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(FUMILINM) -. 3 -c $(FUMILIH) $(FUMILIL)

#LF
$(FUMILIDICTMAP): $(RLIBMAP) $(MAKEFILEDEP) $(FUMILIL)
		$(RLIBMAP) -o $(FUMILIDICTMAP) -l $(FUMILIDICTLIB) \
		-d $(FUMILILIB) $(FUMILILIBDEPM) -c $(FUMILIL)
#LF
$(FUMILINM):      $(FUMILIO) $(FUMILITMPDO) $(FUMILITMP2DO) 
		@echo "Generating symbols file $@..."
		nm -g -p --defined-only $(FUMILITMPDO) | awk '{printf("%s\n", $$3)'} > $(FUMILINM)
		nm -g -p --defined-only $(FUMILITMP2DO) | awk '{printf("%s\n", $$3)'} >> $(FUMILINM)
		nm -g -p --defined-only $(FUMILIO) | awk '{printf("%s\n", $$3)'} >> $(FUMILINM)

$(FUMILIMAP):   $(RLIBMAP) $(MAKEFILEDEP) $(FUMILIL)
		$(RLIBMAP) -o $(FUMILIMAP) -l $(FUMILILIB) \
		   -d $(FUMILILIBDEPM) -c $(FUMILIL)

all-fumili:     $(FUMILILIB) $(FUMILIMAP)

clean-fumili:
		@rm -f $(FUMILIO) $(FUMILIDO)

clean::         clean-fumili clean-pds-fumili

#LF
clean-pds-fumili:	
		rm -f $(FUMILITMPDS) $(FUMILITMPDO) $(FUMILITMPDH) \
		$(FUMILITMPDEP) $(FUMILITMP2DS) $(FUMILITMP2DO) $(FUMILITMP2DH) $(FUMILINM)

distclean-fumili: clean-fumili
		@rm -f $(FUMILIDEP) $(FUMILIDS) $(FUMILIDH) $(FUMILILIB) $(FUMILIMAP)

distclean::     distclean-fumili
