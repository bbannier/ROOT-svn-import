# Module.mk for splot module
# Copyright (c) 2005 Rene Brun and Fons Rademakers
#
# Author: Rene Brun, 27/8/2003

MODDIR      := splot
MODDIRS     := $(MODDIR)/src
MODDIRI     := $(MODDIR)/inc

SPLOTDIR    := $(MODDIR)
SPLOTDIRS   := $(SPLOTDIR)/src
SPLOTDIRI   := $(SPLOTDIR)/inc

##### libSPlot #####
SPLOTL      := $(MODDIRI)/LinkDef.h
SPLOTDS     := $(MODDIRS)/G__SPlot.cxx
SPLOTDO     := $(SPLOTDS:.cxx=.o)
SPLOTDH     := $(SPLOTDS:.cxx=.h)

#LF
SPLOTTMPDS    := $(MODDIRS)/G__SPlotTmp.cxx
SPLOTTMPDO    := $(SPLOTTMPDS:.cxx=.o)
SPLOTTMPDH    := $(SPLOTTMPDS:.cxx=.h)
SPLOTTMP2DS   := $(MODDIRS)/G__SPlotTmp2.cxx
SPLOTTMP2DO   := $(SPLOTTMP2DS:.cxx=.o)
SPLOTTMP2DH   := $(SPLOTTMP2DS:.cxx=.h)

SPLOTH      := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
SPLOTS      := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
SPLOTO      := $(SPLOTS:.cxx=.o)

SPLOTDEP    := $(SPLOTO:.o=.d) $(SPLOTDO:.o=.d)

#LF
SPLOTTMPDEP  := $(SPLOTTMPDO:.o=.d)

SPLOTLIB    := $(LPATH)/libSPlot.$(SOEXT)
SPLOTMAP    := $(SPLOTLIB:.$(SOEXT)=.rootmap)

#LF
SPLOTNM       := $(SPLOTLIB:.$(SOEXT)=.nm)

# used in the main Makefile
ALLHDRS     += $(patsubst $(MODDIRI)/%.h,include/%.h,$(SPLOTH))
ALLLIBS     += $(SPLOTLIB)
ALLMAPS     += $(SPLOTMAP)

# include all dependency files
INCLUDEFILES += $(SPLOTDEP)

##### local rules #####
include/%.h:    $(SPLOTDIRI)/%.h
		cp $< $@

#LF
$(SPLOTLIB):   $(SPLOTO) $(SPLOTTMPDO) $(SPLOTTMP2DO) $(SPLOTDO) $(ORDER_) $(MAINLIBS) $(SPLOTLIBDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		   "$(SOFLAGS)" libSPlot.$(SOEXT) $@ "$(SPLOTO) $(SPLOTTMPDO) $(SPLOTTMP2DO) $(SPLOTDO)" \
		   "$(SPLOTLIBEXTRA)"

#LF
$(SPLOTTMPDS):   $(SPLOTH) $(SPLOTL) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(SPLOTH) $(SPLOTL)

#LF
$(SPLOTTMP2DS):  $(SPLOTH) $(SPLOTL) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(SPLOTH) $(SPLOTL)

#LF
$(SPLOTDS):    $(SPLOTH) $(SPLOTL) $(ROOTCINTTMPEXE) $(SPLOTNM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(SPLOTNM) -. 3 -c $(SPLOTH) $(SPLOTL)

#LF
$(SPLOTDICTMAP): $(RLIBMAP) $(MAKEFILEDEP) $(SPLOTL)
		$(RLIBMAP) -o $(SPLOTDICTMAP) -l $(SPLOTDICTLIB) \
		-d $(SPLOTLIB) $(SPLOTLIBDEPM) -c $(SPLOTL)
#LF
$(SPLOTNM):      $(SPLOTO) $(SPLOTTMPDO) $(SPLOTTMP2DO) 
		@echo "Generating symbols file $@..."
		nm -g -p --defined-only $(SPLOTTMPDO) | awk '{printf("%s\n", $$3)'} > $(SPLOTNM)
		nm -g -p --defined-only $(SPLOTTMP2DO) | awk '{printf("%s\n", $$3)'} >> $(SPLOTNM)
		nm -g -p --defined-only $(SPLOTO) | awk '{printf("%s\n", $$3)'} >> $(SPLOTNM)

$(SPLOTMAP):    $(RLIBMAP) $(MAKEFILEDEP) $(SPLOTL)
		$(RLIBMAP) -o $(SPLOTMAP) -l $(SPLOTLIB) \
		   -d $(SPLOTLIBDEPM) -c $(SPLOTL)

all-splot:     $(SPLOTLIB) $(SPLOTMAP)

clean-splot:
		@rm -f $(SPLOTO) $(SPLOTDO)

clean::         clean-splot clean-pds-splot

#LF
clean-pds-splot:	
		rm -f $(SPLOTTMPDS) $(SPLOTTMPDO) $(SPLOTTMPDH) \
		$(SPLOTTMPDEP) $(SPLOTTMP2DS) $(SPLOTTMP2DO) $(SPLOTTMP2DH) $(SPLOTNM)

distclean-splot: clean-splot
		@rm -f $(SPLOTDEP) $(SPLOTDS) $(SPLOTDH) $(SPLOTLIB) $(SPLOTMAP)

distclean::     distclean-splot
