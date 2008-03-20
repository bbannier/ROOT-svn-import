# Module.mk for spectrum module
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: Rene Brun, 28/09/2006

MODDIR       := hist/spectrum
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

SPECTRUMH    := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
SPECTRUMS    := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
SPECTRUMO    := $(SPECTRUMS:.cxx=.o)

SPECTRUMDEP  := $(SPECTRUMO:.o=.d) $(SPECTRUMDO:.o=.d)

SPECTRUMLIB  := $(LPATH)/libSpectrum.$(SOEXT)
SPECTRUMMAP  := $(SPECTRUMLIB:.$(SOEXT)=.rootmap)

# used in the main Makefile
ALLHDRS      += $(patsubst $(MODDIRI)/%.h,include/%.h,$(SPECTRUMH))
ALLLIBS      += $(SPECTRUMLIB)
ALLMAPS      += $(SPECTRUMMAP)

# include all dependency files
INCLUDEFILES += $(SPECTRUMDEP)

##### local rules #####
.PHONY:         all-spectrum map-spectrum clean-spectrum distclean-spectrum

include/%.h:    $(SPECTRUMDIRI)/%.h
		cp $< $@

$(SPECTRUMLIB): $(SPECTRUMO) $(SPECTRUMDO) $(ORDER_) $(MAINLIBS) $(SPECTRUMLIBDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		   "$(SOFLAGS)" libSpectrum.$(SOEXT) $@ "$(SPECTRUMO) $(SPECTRUMDO)" \
		   "$(SPECTRUMLIBEXTRA)"

$(SPECTRUMDS):  $(SPECTRUMH) $(SPECTRUML) $(ROOTCINTTMPDEP)
		@echo "Generating dictionary $@..."
		$(ROOTCINTTMP) -f $@ -c $(SPECTRUMH) $(SPECTRUML)

$(SPECTRUMMAP): $(RLIBMAP) $(MAKEFILEDEP) $(SPECTRUML)
		$(RLIBMAP) -o $(SPECTRUMMAP) -l $(SPECTRUMLIB) \
		   -d $(SPECTRUMLIBDEPM) -c $(SPECTRUML)

all-spectrum:   $(SPECTRUMLIB) $(SPECTRUMMAP)

clean-spectrum:
		@rm -f $(SPECTRUMO) $(SPECTRUMDO)

clean::         clean-spectrum

distclean-spectrum: clean-spectrum
		@rm -f $(SPECTRUMDEP) $(SPECTRUMDS) $(SPECTRUMDH) $(SPECTRUMLIB) $(SPECTRUMMAP)

distclean::     distclean-spectrum
