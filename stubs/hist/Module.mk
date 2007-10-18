# Module.mk for hist module
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: Fons Rademakers, 29/2/2000

MODDIR       := hist
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

HISTDIR      := $(MODDIR)
HISTDIRS     := $(HISTDIR)/src
HISTDIRI     := $(HISTDIR)/inc

##### libHist #####
HISTL        := $(MODDIRI)/LinkDef.h
HISTDS       := $(MODDIRS)/G__Hist.cxx
HISTDO       := $(HISTDS:.cxx=.o)
HISTDH       := $(HISTDS:.cxx=.h)

#LF
HISTTMPDS    := $(MODDIRS)/G__HistTmp.cxx
HISTTMPDO    := $(HISTTMPDS:.cxx=.o)
HISTTMPDH    := $(HISTTMPDS:.cxx=.h)
HISTTMP2DS    := $(MODDIRS)/G__HistTmp2.cxx
HISTTMP2DO    := $(HISTTMP2DS:.cxx=.o)
HISTTMP2DH    := $(HISTTMP2DS:.cxx=.h)

HISTH        := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
HISTS        := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
HISTO        := $(HISTS:.cxx=.o)

#LF
#HISTO        :=$(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.o))

HISTDEP      := $(HISTO:.o=.d) $(HISTDO:.o=.d)

#LF
HISTTMPDEP   := $(HISTTMPDO:.o=.d)

HISTLIB      := $(LPATH)/libHist.$(SOEXT)

#LF
HISTMAP      := $(HISTLIB:.$(SOEXT)=.rootmap)
HISTDICTLIB  := $(LPATH)/libHistDict.$(SOEXT)
HISTDICTMAP  := $(HISTDICTLIB:.$(SOEXT)=.rootmap)

#LF
HISTNM       := $(HISTLIB:.$(SOEXT)=.nm)

# used in the main Makefile
ALLHDRS     += $(patsubst $(MODDIRI)/%.h,include/%.h,$(HISTH))
ALLLIBS     += $(HISTLIB) $(HISTDICTLIB)
ALLMAPS     += $(HISTMAP)

# include all dependency files
INCLUDEFILES += $(HISTDEP)

##### local rules #####
include/%.h:    $(HISTDIRI)/%.h
		cp $< $@

#LF
$(HISTLIB):     $(HISTO) $(HISTTMPDO) $(HISTTMP2DO) $(HISTDO) $(ORDER_) $(MAINLIBS) $(HISTLIBDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		"$(SOFLAGS)" libHist.$(SOEXT) $@ "$(HISTO) $(HISTTMPDO) $(HISTTMP2DO) $(HISTDO)"\
		"$(HISTLIBEXTRA)"

#LF
$(HISTDICTLIB): $(HISTDO) $(ORDER_) $(MAINLIBS) $(HISTDICTLIBDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		"$(SOFLAGS)" libHistDict.$(SOEXT) $@ "$(HISTDO)" "$(HISTTMP2DO)"\
		"$(HISTDICTLIBEXTRA)"

#LF
$(HISTTMPDS):   $(HISTH) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(HISTH) $(HISTL)

#LF
$(HISTTMP2DS):  $(HISTH) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(HISTH) $(HISTL)

#LF
$(HISTDS):      $(HISTH) $(HISTL) $(ROOTCINTTMPEXE) $(HISTNM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(HISTNM) -. 3 -c $(HISTH) $(HISTL)

#LF
$(HISTMAP): $(RLIBMAP) $(MAKEFILEDEP) $(HISTL)
		$(RLIBMAP) -o $(HISTMAP) -l $(HISTLIB) \
		-d $(HISTLIBDEPM) -c $(HISTL)
#LF
$(HISTNM):      $(HISTTMPDO) $(HISTTMP2DO) $(HISTO)
		@echo "Generating symbols file $@..."
		nm -p --defined-only $(HISTTMPDO) | awk '{printf("%s\n", $$3)'} > $(HISTNM)
		nm -p --defined-only $(HISTTMP2DO) | awk '{printf("%s\n", $$3)'} >> $(HISTNM)
		nm -p --defined-only $(HISTO) | awk '{printf("%s\n", $$3)'} >> $(HISTNM)

#LF
all-hist:       $(HISTLIB) $(HISTMAP)

clean-hist:
		@rm -f $(HISTO) $(HISTDO) $(HISTNM)

clean::         clean-hist clean-pds-hist

#LF
clean-pds-hist:	
		rm -f $(HISTTMPDS) $(HISTTMPDO) $(HISTTMPDH) \
		$(HISTTMPDEP) $(HISTTMP2DS) $(HISTTMP2DO) $(HISTTMP2DH) $(HISTNM)

#LF
distclean-hist: clean-hist
		@rm -f $(HISTDEP) $(HISTDS) $(HISTDH) $(HISTLIB) $(HISTDICTMAP)

distclean::     distclean-hist
