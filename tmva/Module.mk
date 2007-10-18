# Module.mk for tmva module
# Copyright (c) 2006 Rene Brun and Fons Rademakers
#
# Author: Fons Rademakers, 20/6/2005


MODDIR       := tmva
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

TMVADIR      := $(MODDIR)
TMVADIRS     := $(TMVADIR)/src
TMVADIRI     := $(TMVADIR)/inc

##### libTMVA #####
TMVAL        := $(MODDIRI)/LinkDef.h
TMVADS       := $(MODDIRS)/G__TMVA.cxx
TMVADO       := $(TMVADS:.cxx=.o)
TMVADH       := $(TMVADS:.cxx=.h)

#LF
TMVATMPDS    := $(MODDIRS)/G__TMVATmp.cxx
TMVATMPDO    := $(TMVATMPDS:.cxx=.o)
TMVATMPDH    := $(TMVATMPDS:.cxx=.h)
TMVATMP2DS   := $(MODDIRS)/G__TMVATmp2.cxx
TMVATMP2DO   := $(TMVATMP2DS:.cxx=.o)
TMVATMP2DH   := $(TMVATMP2DS:.cxx=.h)

TMVAH        := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
TMVAH_CINT   := $(subst tmva/inc,include/TMVA,$(TMVAH))
TMVAS        := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
TMVAO        := $(TMVAS:.cxx=.o)

TMVADEP      := $(TMVAO:.o=.d) $(TMVADO:.o=.d)

#LF
TMVATMPDEP  := $(TMVATMPDO:.o=.d)

TMVALIB      := $(LPATH)/libTMVA.$(SOEXT)
TMVAMAP      := $(TMVALIB:.$(SOEXT)=.rootmap)

#LF
TMVANM       := $(TMVALIB:.$(SOEXT)=.nm)

# used in the main Makefile
ALLHDRS      += $(patsubst $(MODDIRI)/%.h,include/TMVA/%.h,$(TMVAH))
ALLLIBS      += $(TMVALIB)
ALLMAPS      += $(TMVAMAP)

# include all dependency files
INCLUDEFILES += $(TMVADEP)

##### local rules #####
include/TMVA/%.h: $(TMVADIRI)/%.h
		@(if [ ! -d "include/TMVA" ]; then     \
		   mkdir -p include/TMVA;              \
		fi)
		cp $< $@

#LF
$(TMVALIB):   $(TMVAO) $(TMVATMPDO) $(TMVATMP2DO) $(TMVADO) $(ORDER_) $(MAINLIBS) $(TMVALIBDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		   "$(SOFLAGS)" libTMVA.$(SOEXT) $@ "$(TMVAO) $(TMVATMPDO) $(TMVATMP2DO) $(TMVADO)" \
		   "$(TMVALIBEXTRA)"

#LF
$(TMVATMPDS):   $(TMVAH_CINT) $(TMVAL) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(TMVAH_CINT) $(TMVAL)

#LF
$(TMVATMP2DS):  $(TMVAH_CINT) $(TMVAL) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(TMVAH_CINT) $(TMVAL)

#LF
$(TMVADS):    $(TMVAH_CINT) $(TMVAL) $(ROOTCINTTMPEXE) $(TMVANM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(TMVANM) -. 3 -c $(TMVAH_CINT) $(TMVAL)

#LF
$(TMVANM):      $(TMVAO) $(TMVATMPDO) $(TMVATMP2DO) 
		@echo "Generating symbols file $@..."
		nm -p --defined-only $(TMVATMPDO) | awk '{printf("%s\n", $$3)'} > $(TMVANM)
		nm -p --defined-only $(TMVATMP2DO) | awk '{printf("%s\n", $$3)'} >> $(TMVANM)
		nm -p --defined-only $(TMVAO) | awk '{printf("%s\n", $$3)'} >> $(TMVANM)

$(TMVAMAP):     $(RLIBMAP) $(MAKEFILEDEP) $(TMVAL)
		$(RLIBMAP) -o $(TMVAMAP) -l $(TMVALIB) \
		   -d $(TMVALIBDEPM) -c $(TMVAL)

all-tmva:       $(TMVALIB) $(TMVAMAP)

clean-tmva:
		@rm -f $(TMVAO) $(TMVADO)

clean::         clean-tmva clean-pds-tmva

#LF
clean-pds-tmva:	
		rm -f $(TMVATMPDS) $(TMVATMPDO) $(TMVATMPDH) \
		$(TMVATMPDEP) $(TMVATMP2DS) $(TMVATMP2DO) $(TMVATMP2DH) $(TMVANM)

distclean-tmva: clean-tmva
		@rm -f $(TMVADEP) $(TMVADS) $(TMVADH) $(TMVALIB) $(TMVAMAP)
		@rm -rf include/TMVA

distclean::     distclean-tmva
