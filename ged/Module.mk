# Module.mk for ged module
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: Ilka Antcheva, 18/2/2004

MODDIR    := ged
MODDIRS   := $(MODDIR)/src
MODDIRI   := $(MODDIR)/inc

GEDDIR    := $(MODDIR)
GEDDIRS   := $(GEDDIR)/src
GEDDIRI   := $(GEDDIR)/inc

##### libGed #####
GEDL      := $(MODDIRI)/LinkDef.h
GEDDS     := $(MODDIRS)/G__Ged.cxx
GEDDO     := $(GEDDS:.cxx=.o)
GEDDH     := $(GEDDS:.cxx=.h)

#LF
GEDTMPDS    := $(MODDIRS)/G__GedTmp.cxx
GEDTMPDO    := $(GEDTMPDS:.cxx=.o)
GEDTMPDH    := $(GEDTMPDS:.cxx=.h)
GEDTMP2DS    := $(MODDIRS)/G__GedTmp2.cxx
GEDTMP2DO    := $(GEDTMP2DS:.cxx=.o)
GEDTMP2DH    := $(GEDTMP2DS:.cxx=.h)

GEDH      := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
GEDS      := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
GEDO      := $(GEDS:.cxx=.o)

GEDDEP    := $(GEDO:.o=.d) $(GEDDO:.o=.d)

#LF
GEDTMPDEP   := $(GEDTMPDO:.o=.d)

GEDLIB    := $(LPATH)/libGed.$(SOEXT)

# LF
GEDMAP    := $(GEDLIB:.$(SOEXT)=.rootmap)
GEDDICTLIB  := $(LPATH)/libGedDict.$(SOEXT)
GEDDICTMAP  := $(GEDDICTLIB:.$(SOEXT)=.rootmap)

#LF
GEDNM       := $(GEDLIB:.$(SOEXT)=.nm)

# used in the main Makefile
ALLHDRS     += $(patsubst $(MODDIRI)/%.h,include/%.h,$(GEDH))
ALLLIBS     += $(GEDLIB)
ALLMAPS     += $(GEDMAP)

# include all dependency files
INCLUDEFILES += $(GEDDEP)

##### local rules #####
include/%.h:    $(GEDDIRI)/%.h
		cp $< $@

#LF
$(GEDLIB):     $(GEDO) $(GEDTMPDO) $(GEDTMP2DO) $(GEDDO) $(ORDER_) $(MAINLIBS) $(GEDLIBDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		"$(SOFLAGS)" libGed.$(SOEXT) $@ "$(GEDO) $(GEDTMPDO) $(GEDTMP2DO) $(GEDDO)" \
		"$(GEDLIBEXTRA)"

#LF
#$(GEDDICTLIB): $(GEDDO) $(ORDER_) $(MAINLIBS) $(GEDDICTLIBDEP)
#		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
#		"$(SOFLAGS)" libGedDict.$(SOEXT) $@ "$(GEDDO) $(GEDTMP2DO)"\
#		"$(GEDDICTLIBEXTRA)"

#LF
$(GEDTMPDS):   $(GEDH) $(GEDL) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(GEDH) $(GEDL)

#LF
$(GEDTMP2DS):  $(GEDH) $(GEDL) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(GEDH) $(GEDL)

#LF
$(GEDDS):      $(GEDH) $(GEDL) $(ROOTCINTTMPEXE) $(GEDNM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(GEDNM) -. 3 -c $(GEDH) $(GEDL)
#LF
$(GEDMAP): $(RLIBMAP) $(MAKEFILEDEP) $(GEDL)
		$(RLIBMAP) -o $(GEDMAP) -l $(GEDLIB) \
		-d $(GEDLIBDEPM) -c $(GEDL)
#LF
$(GEDNM):      $(GEDO) $(GEDTMPDO) $(GEDTMP2DO)
		@echo "Generating symbols file $@..."
		nm -p --defined-only $(GEDTMPDO) | awk '{printf("%s\n", $$3)'} > $(GEDNM)
		nm -p --defined-only $(GEDTMP2DO) | awk '{printf("%s\n", $$3)'} >> $(GEDNM)
		nm -p --defined-only $(GEDO) | awk '{printf("%s\n", $$3)'} >> $(GEDNM)

all-ged:        $(GEDLIB) $(GEDMAP)

clean-ged:
		@rm -f $(GEDO) $(GEDDO)

clean::         clean-ged clean-pds-ged

#LF
clean-pds-ged:	
		rm -f $(GEDTMPDS) $(GEDTMPDO) $(GEDTMPDH) \
		$(GEDTMPDEP) $(GEDTMP2DS) $(GEDTMP2DO) $(GEDTMP2DH) $(GEDNM)

distclean-ged: clean-ged
		@rm -f $(GEDDEP) $(GEDDS) $(GEDDH) $(GEDLIB) $(GEDMAP)

distclean::     distclean-ged
