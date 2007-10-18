# Module.mk for fitpanel module
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: Ilka Antcheva, 02/10/2006

MODDIR       := fitpanel
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

FITPANELDIR  := $(MODDIR)
FITPANELDIRS := $(FITPANELDIR)/src
FITPANELDIRI := $(FITPANELDIR)/inc

##### libFitPanel #####
FITPANELL    := $(MODDIRI)/LinkDef.h
FITPANELDS   := $(MODDIRS)/G__FitPanel.cxx
FITPANELDO   := $(FITPANELDS:.cxx=.o)
FITPANELDH   := $(FITPANELDS:.cxx=.h)

#LF
FITPANELTMPDS    := $(MODDIRS)/G__FitPanelTmp.cxx
FITPANELTMPDO    := $(FITPANELTMPDS:.cxx=.o)
FITPANELTMPDH    := $(FITPANELTMPDS:.cxx=.h)
FITPANELTMP2DS   := $(MODDIRS)/G__FitPanelTmp2.cxx
FITPANELTMP2DO   := $(FITPANELTMP2DS:.cxx=.o)
FITPANELTMP2DH   := $(FITPANELTMP2DS:.cxx=.h)

FITPANELH    := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
FITPANELS    := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
FITPANELO    := $(FITPANELS:.cxx=.o)

FITPANELDEP  := $(FITPANELO:.o=.d) $(FITPANELDO:.o=.d)

#LF
FITPANELTMPDEP  := $(FITPANELTMPDO:.o=.d)

FITPANELLIB  := $(LPATH)/libFitPanel.$(SOEXT)
FITPANELMAP  := $(FITPANELLIB:.$(SOEXT)=.rootmap)

#LF
FITPANELNM       := $(FITPANELLIB:.$(SOEXT)=.nm)

# used in the main Makefile
ALLHDRS      += $(patsubst $(MODDIRI)/%.h,include/%.h,$(FITPANELH))
ALLLIBS      += $(FITPANELLIB)
ALLMAPS      += $(FITPANELMAP)

# include all dependency files
INCLUDEFILES += $(FITPANELDEP)

##### local rules #####
include/%.h:    $(FITPANELDIRI)/%.h
		cp $< $@

#LF
$(FITPANELLIB):   $(FITPANELO) $(FITPANELTMPDO) $(FITPANELTMP2DO) $(FITPANELDO) $(ORDER_) $(MAINLIBS) $(FITPANELLIBDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		   "$(SOFLAGS)" libFitPanel.$(SOEXT) $@ "$(FITPANELO) $(FITPANELTMPDO) $(FITPANELTMP2DO) $(FITPANELDO)" \
		   "$(FITPANELLIBEXTRA)"

#LF
$(FITPANELTMPDS):   $(FITPANELH) $(FITPANELL) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(FITPANELH) $(FITPANELL)

#LF
$(FITPANELTMP2DS):  $(FITPANELH) $(FITPANELL) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(FITPANELH) $(FITPANELL)

#LF
$(FITPANELDS):    $(FITPANELH) $(FITPANELL) $(ROOTCINTTMPEXE) $(FITPANELNM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(FITPANELNM) -. 3 -c $(FITPANELH) $(FITPANELL)

#LF
$(FITPANELDICTMAP): $(RLIBMAP) $(MAKEFILEDEP) $(FITPANELL)
		$(RLIBMAP) -o $(FITPANELDICTMAP) -l $(FITPANELDICTLIB) \
		-d $(FITPANELLIB) $(FITPANELLIBDEPM) -c $(FITPANELL)
#LF
$(FITPANELNM):      $(FITPANELO) $(FITPANELTMPDO) $(FITPANELTMP2DO) 
		@echo "Generating symbols file $@..."
		nm -p --defined-only $(FITPANELTMPDO) | awk '{printf("%s\n", $$3)'} > $(FITPANELNM)
		nm -p --defined-only $(FITPANELTMP2DO) | awk '{printf("%s\n", $$3)'} >> $(FITPANELNM)
		nm -p --defined-only $(FITPANELO) | awk '{printf("%s\n", $$3)'} >> $(FITPANELNM)

$(FITPANELMAP): $(RLIBMAP) $(MAKEFILEDEP) $(FITPANELL)
		$(RLIBMAP) -o $(FITPANELMAP) -l $(FITPANELLIB) \
		   -d $(FITPANELLIBDEPM) -c $(FITPANELL)

all-fitpanel:   $(FITPANELLIB) $(FITPANELMAP)

clean-fitpanel:
		@rm -f $(FITPANELO) $(FITPANELDO)

clean::         clean-fitpanel clean-pds-fitpanel

#LF
clean-pds-fitpanel:	
		rm -f $(FITPANELTMPDS) $(FITPANELTMPDO) $(FITPANELTMPDH) \
		$(FITPANELTMPDEP) $(FITPANELTMP2DS) $(FITPANELTMP2DO) $(FITPANELTMP2DH) $(FITPANELNM)

distclean-fitpanel: clean-fitpanel
		@rm -f $(FITPANELDEP) $(FITPANELDS) $(FITPANELDH) $(FITPANELLIB) $(FITPANELMAP)

distclean::     distclean-fitpanel
