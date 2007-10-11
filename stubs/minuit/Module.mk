# Module.mk for minuit module
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: Fons Rademakers, 29/2/2000

MODDIR       := minuit
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

MINUITDIR    := $(MODDIR)
MINUITDIRS   := $(MINUITDIR)/src
MINUITDIRI   := $(MINUITDIR)/inc

##### libMinuit #####
MINUITL      := $(MODDIRI)/LinkDef.h
MINUITDS     := $(MODDIRS)/G__Minuit.cxx
MINUITDO     := $(MINUITDS:.cxx=.o)
MINUITDH     := $(MINUITDS:.cxx=.h)

#LF
MINUITTMPDS    := $(MODDIRS)/G__MinuitTmp.cxx
MINUITTMPDO    := $(MINUITTMPDS:.cxx=.o)
MINUITTMPDH    := $(MINUITTMPDS:.cxx=.h)
MINUITTMP2DS    := $(MODDIRS)/G__MinuitTmp2.cxx
MINUITTMP2DO    := $(MINUITTMP2DS:.cxx=.o)
MINUITTMP2DH    := $(MINUITTMP2DS:.cxx=.h)

MINUITH      := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
MINUITS      := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
MINUITO      := $(MINUITS:.cxx=.o)

MINUITDEP    := $(MINUITO:.o=.d) $(MINUITDO:.o=.d)

#LF
MINUITTMPDEP   := $(MINUITTMPDO:.o=.d)

MINUITLIB    := $(LPATH)/libMinuit.$(SOEXT)

#LF
MINUITMAP    := $(MINUITLIB:.$(SOEXT)=.rootmap)

MINUITDICTLIB  := $(LPATH)/libMinuitDict.$(SOEXT)
MINUITDICTMAP  := $(MINUITDICTLIB:.$(SOEXT)=.rootmap)

#LF
MINUITNM       := $(MINUITLIB:.$(SOEXT)=.nm)

# used in the main Makefile
ALLHDRS     += $(patsubst $(MODDIRI)/%.h,include/%.h,$(MINUITH))
ALLLIBS     += $(MINUITLIB)
ALLMAPS     += $(MINUITMAP)

# include all dependency files
INCLUDEFILES += $(MINUITDEP)

##### local rules #####
include/%.h:    $(MINUITDIRI)/%.h
		cp $< $@

#LF
$(MINUITLIB):     $(MINUITO) $(MINUITTMPDO) $(MINUITTMP2DO) $(MINUITDO) $(ORDER_) $(MAINLIBS) $(MINUITLIBDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		"$(SOFLAGS)" libMinuit.$(SOEXT) $@ "$(MINUITO) $(MINUITTMPDO) $(MINUITTMP2DO) $(MINUITDO)" \
		"$(MINUITLIBEXTRA)"

#LF
#$(MINUITDICTLIB): $(MINUITDO) $(ORDER_) $(MAINLIBS) $(MINUITDICTLIBDEP)
#		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
#		"$(SOFLAGS)" libMinuitDict.$(SOEXT) $@ "$(MINUITDO) $(MINUITTMP2DO)"\
#		"$(MINUITDICTLIBEXTRA)"

#LF
$(MINUITTMPDS):   $(MINUITH) $(MINUITL) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(MINUITH) $(MINUITL)

#LF
$(MINUITTMP2DS):  $(MINUITH) $(MINUITL) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(MINUITH) $(MINUITL)

#LF
$(MINUITDS):      $(MINUITH) $(MINUITL) $(ROOTCINTTMPEXE) $(MINUITNM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(MINUITNM) -. 3 -c $(MINUITH) $(MINUITL)
#LF
$(MINUITMAP): $(RLIBMAP) $(MAKEFILEDEP) $(MINUITL)
		$(RLIBMAP) -o $(MINUITMAP) -l $(MINUITLIB) \
		-d $(MINUITLIBDEPM) -c $(MINUITL)
#LF
$(MINUITNM):      $(MINUITO) $(MINUITTMPDO) $(MINUITTMP2DO)
		@echo "Generating symbols file $@..."
		nm -g -p --defined-only $(MINUITTMPDO) | awk '{printf("%s\n", $$3)'} > $(MINUITNM)
		nm -g -p --defined-only $(MINUITTMP2DO) | awk '{printf("%s\n", $$3)'} >> $(MINUITNM)
		nm -g -p --defined-only $(MINUITO) | awk '{printf("%s\n", $$3)'} >> $(MINUITNM)

all-minuit:     $(MINUITLIB) $(MINUITMAP)

clean-minuit:
		@rm -f $(MINUITO) $(MINUITDO)

clean::         clean-minuit clean-pds-minuit

#LF
clean-pds-minuit:	
		rm -f $(MINUITTMPDS) $(MINUITTMPDO) $(MINUITTMPDH) \
		$(MINUITTMPDEP) $(MINUITTMP2DS) $(MINUITTMP2DO) $(MINUITTMP2DH) $(MINUITNM)

distclean-minuit: clean-minuit
		@rm -f $(MINUITDEP) $(MINUITDS) $(MINUITDH) $(MINUITLIB) $(MINUITMAP)

distclean::     distclean-minuit
