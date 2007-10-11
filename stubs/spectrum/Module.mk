# Module.mk for spectrum module
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: Rene Brun, 28/09/2006

MODDIR       := spectrum
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

SPECTRUMDIR  := $(MODDIR)
SPECTRUMDIRS := $(SPECTRUMDIR)/src
SPECTRUMDIRI := $(SPECTRUMDIR)/inc

##### libSpectrum #####
SPECTRUML    := $(MODDIRI)/LinkDef.h
SPECTRUMDS   := $(MODDIRS)/G__Spectrum.cxx
SPECTRUMDO   := $(SPECTRUMDS:.cxx=.o)
SPECTRUMDH   := $(SPECTRUMDS:.cxx=.h)

#LF
SPECTRUMTMPDS    := $(MODDIRS)/G__SpectrumTmp.cxx
SPECTRUMTMPDO    := $(SPECTRUMTMPDS:.cxx=.o)
SPECTRUMTMPDH    := $(SPECTRUMTMPDS:.cxx=.h)
SPECTRUMTMP2DS   := $(MODDIRS)/G__SpectrumTmp2.cxx
SPECTRUMTMP2DO   := $(SPECTRUMTMP2DS:.cxx=.o)
SPECTRUMTMP2DH   := $(SPECTRUMTMP2DS:.cxx=.h)

SPECTRUMH    := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
SPECTRUMS    := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
SPECTRUMO    := $(SPECTRUMS:.cxx=.o)

SPECTRUMDEP  := $(SPECTRUMO:.o=.d) $(SPECTRUMDO:.o=.d)

#LF
SPECTRUMTMPDEP  := $(SPECTRUMTMPDO:.o=.d)

SPECTRUMLIB  := $(LPATH)/libSpectrum.$(SOEXT)
SPECTRUMMAP  := $(SPECTRUMLIB:.$(SOEXT)=.rootmap)

#LF
SPECTRUMNM       := $(SPECTRUMLIB:.$(SOEXT)=.nm)

# used in the main Makefile
ALLHDRS      += $(patsubst $(MODDIRI)/%.h,include/%.h,$(SPECTRUMH))
ALLLIBS      += $(SPECTRUMLIB)
ALLMAPS      += $(SPECTRUMMAP)

# include all dependency files
INCLUDEFILES += $(SPECTRUMDEP)

##### local rules #####
include/%.h:    $(SPECTRUMDIRI)/%.h
		cp $< $@

#LF
$(SPECTRUMLIB):   $(SPECTRUMO) $(SPECTRUMTMPDO) $(SPECTRUMTMP2DO) $(SPECTRUMDO) $(ORDER_) $(MAINLIBS) $(SPECTRUMLIBDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		   "$(SOFLAGS)" libSpectrum.$(SOEXT) $@ "$(SPECTRUMO) $(SPECTRUMTMPDO) $(SPECTRUMTMP2DO) $(SPECTRUMDO)" \
		   "$(SPECTRUMLIBEXTRA)"

#LF
$(SPECTRUMTMPDS):   $(SPECTRUMH) $(SPECTRUML) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(SPECTRUMH) $(SPECTRUML)

#LF
$(SPECTRUMTMP2DS):  $(SPECTRUMH) $(SPECTRUML) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(SPECTRUMH) $(SPECTRUML)

#LF
$(SPECTRUMDS):    $(SPECTRUMH) $(SPECTRUML) $(ROOTCINTTMPEXE) $(SPECTRUMNM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(SPECTRUMNM) -. 3 -c $(SPECTRUMH) $(SPECTRUML)

#LF
$(SPECTRUMDICTMAP): $(RLIBMAP) $(MAKEFILEDEP) $(SPECTRUML)
		$(RLIBMAP) -o $(SPECTRUMDICTMAP) -l $(SPECTRUMDICTLIB) \
		-d $(SPECTRUMLIB) $(SPECTRUMLIBDEPM) -c $(SPECTRUML)
#LF
$(SPECTRUMNM):      $(SPECTRUMO) $(SPECTRUMTMPDO) $(SPECTRUMTMP2DO) 
		@echo "Generating symbols file $@..."
		nm -g -p --defined-only $(SPECTRUMTMPDO) | awk '{printf("%s\n", $$3)'} > $(SPECTRUMNM)
		nm -g -p --defined-only $(SPECTRUMTMP2DO) | awk '{printf("%s\n", $$3)'} >> $(SPECTRUMNM)
		nm -g -p --defined-only $(SPECTRUMO) | awk '{printf("%s\n", $$3)'} >> $(SPECTRUMNM)

$(SPECTRUMMAP): $(RLIBMAP) $(MAKEFILEDEP) $(SPECTRUML)
		$(RLIBMAP) -o $(SPECTRUMMAP) -l $(SPECTRUMLIB) \
		   -d $(SPECTRUMLIBDEPM) -c $(SPECTRUML)

all-spectrum:   $(SPECTRUMLIB) $(SPECTRUMMAP)

clean-spectrum:
		@rm -f $(SPECTRUMO) $(SPECTRUMDO)

clean::         clean-spectrum clean-pds-spectrum

#LF
clean-pds-spectrum:	
		rm -f $(SPECTRUMTMPDS) $(SPECTRUMTMPDO) $(SPECTRUMTMPDH) \
		$(SPECTRUMTMPDEP) $(SPECTRUMTMP2DS) $(SPECTRUMTMP2DO) $(SPECTRUMTMP2DH) $(SPECTRUMNM)

distclean-spectrum: clean-spectrum
		@rm -f $(SPECTRUMDEP) $(SPECTRUMDS) $(SPECTRUMDH) $(SPECTRUMLIB) $(SPECTRUMMAP)

distclean::     distclean-spectrum
