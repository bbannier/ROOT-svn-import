# Module.mk for meta module
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: Fons Rademakers, 29/2/2000

MODDIR       := meta
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

METADIR      := $(MODDIR)
METADIRS     := $(METADIR)/src
METADIRI     := $(METADIR)/inc

##### libMeta (part of libCore) #####
METAL        := $(MODDIRI)/LinkDef.h
METADS       := $(MODDIRS)/G__Meta.cxx
METADO       := $(METADS:.cxx=.o)
METADH       := $(METADS:.cxx=.h)

#LF
METATMPDS    := $(MODDIRS)/G__MetaTmp.cxx
METATMPDO    := $(METATMPDS:.cxx=.o)
METATMPDH    := $(METATMPDS:.cxx=.h)
METATMP2DS    := $(MODDIRS)/G__MetaTmp2.cxx
METATMP2DO    := $(METATMP2DS:.cxx=.o)
METATMP2DH    := $(METATMP2DS:.cxx=.h)

METAH        := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
METAS        := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
METAO        := $(METAS:.cxx=.o)

METADEP      := $(METAO:.o=.d) $(METADO:.o=.d)

#LF
METATMPDEP   := $(METATMPDO:.o=.d)

#LF
METALIB      := $(MODDIRS)/libMeta.$(SOEXT)
#METADICTLIB  := $(MODDIRS)/libMetaDict.o
METANM       := $(METALIB:.$(SOEXT)=.nm)

# used in the main Makefile
ALLHDRS     += $(patsubst $(MODDIRI)/%.h,include/%.h,$(METAH))

# include all dependency files
INCLUDEFILES += $(METADEP)

##### local rules #####
include/%.h:    $(METADIRI)/%.h
		cp $< $@

#LF
$(METATMPDS):   $(METAH) $(METAL) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(METAH) $(METAL)
#LF
$(METATMP2DS):  $(METAH) $(METAL) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(METAH) $(METAL)
#LF
$(METADS):     $(METAH) $(METAL) $(ROOTCINTTMPEXE) $(METANM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(METANM) -. 3 -c $(METAH) $(METAL)

#LF
$(METANM):      $(METAO) $(METATMPDO) $(METATMP2DO)
		@echo "Generating symbols file $@..."
		nm -p --defined-only $(METATMPDO) | awk '{printf("%s\n", $$3)'} > $(METANM)
		nm -p --defined-only $(METATMP2DO) | awk '{printf("%s\n", $$3)'} >> $(METANM)
		nm -p --defined-only $(METAO) | awk '{printf("%s\n", $$3)'} >> $(METANM)

all-meta:       $(METAO) $(METADO)

clean-meta:
		@rm -f $(METAO) $(METADO)

clean::         clean-meta

distclean-meta: clean-meta
		@rm -f $(METADEP) $(METADS) $(METADH)

distclean::     distclean-meta

