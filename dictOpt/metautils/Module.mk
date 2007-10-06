# Module.mk for utilities for libMeta and rootcint
# Copyright (c) 2002 Rene Brun and Fons Rademakers
#
# Author: Philippe Canal 9/1/2004

MODDIR         := metautils
MODDIRS        := $(MODDIR)/src
MODDIRI        := $(MODDIR)/inc

METAUTILSDIR   := $(MODDIR)
METAUTILSDIRS  := $(METAUTILSDIR)/src
METAUTILSDIRI  := $(METAUTILSDIR)/inc

##### $(METAUTILSO) #####
METAUTILSH     := $(filter-out $(MODDIRI)/TDictionaryManager.h,\
                  $(filter-out $(MODDIRI)/%Info.h,\
                  $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))))
METAUTILSS     := $(filter-out $(MODDIRS)/TDictionaryManager.cxx,\
                  $(filter-out $(MODDIRS)/%Info.cxx,\
                  $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))))
METAUTILSO     := $(METAUTILSS:.cxx=.o)

METAUTILSL     := $(MODDIRI)/LinkDef.h
METAUTILSDS    := $(MODDIRS)/G__MetaUtils.cxx
METAUTILSDO    := $(METAUTILSDS:.cxx=.o)
METAUTILSDH    := $(METAUTILSDS:.cxx=.h)

METAUTILSDEP   := $(METAUTILSO:.o=.d) $(METAUTILSDO:.o=.d)

# used in the main Makefile
ALLHDRS     += $(patsubst $(MODDIRI)/%.h,include/%.h,$(METAUTILSH))

# include all dependency files
INCLUDEFILES += $(METAUTILSDEP)

##### libDictIO #####
DICTIODIR  := $(MODDIR)
DICTIODIRS  := $(MODDIR)/src
DICTIODIRI  := $(MODDIR)/inc

##### $(DICTIOO) #####
DICTIOH     := $(MODDIRI)/TDictionaryManager.h $(wildcard $(MODDIRI)/*Info.h)
DICTIOS     := $(MODDIRS)/TDictionaryManager.cxx $(wildcard $(MODDIRS)/*Info.cxx)
DICTIOO     := $(DICTIOS:.cxx=.o)

DICTIOL     := $(MODDIRI)/LinkDefDictIO.h
DICTIODS     := $(MODDIRS)/G__DictIO.cxx
DICTIODO     := $(DICTIODS:.cxx=.o)
DICTIODH     := $(DICTIODS:.cxx=.h)

DICTIODEP  := $(DICTIOO:.o=.d) $(DICTIODO:.o=.d)
DICTIOLIB  := $(LPATH)/libDictIO.$(SOEXT)
DICTIOMAP  := $(DICTIOLIB:.$(SOEXT)=.rootmap)

# used in the main Makefile
ALLHDRS     += $(patsubst $(MODDIRI)/%.h,include/%.h,$(METAUTILSH) $(DICTIOH))
ALLLIBS     += $(DICTIOLIB)
ALLMAPS     += $(DICTIOMAP)

# include all dependency files
INCLUDEFILES += $(METAUTILSDEP) $(DICTIODEP)

##### local rules #####
include/%.h:    $(METAUTILSDIRI)/%.h
		cp $< $@

#$(ROOTCINTTMP) not yet known at this stage, use explicit path of rootcint_tmp
$(METAUTILSDS): $(METAUTILSH) $(METAUTILSL) utils/src/rootcint_tmp$(EXEEXT)
		@echo "Generating dictionary $@..."
		$(ROOTCINTTMP) -f $@ -c -DG__API $(METAUTILSH) $(METAUTILSL)

$(DICTIOLIB):     $(DICTIOO) $(DICTIODO) $(ORDER_) $(MAINLIBS) $(DICTIOLIBDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		   "$(SOFLAGS)" libDictIO.$(SOEXT) $@ "$(DICTIOO) $(DICTIODO)" \
		   "$(DICTIOLIBEXTRA)"

$(DICTIODS):      $(DICTIOH) $(DICTIOL) $(ROOTCINTTMPEXE)
		@echo "Generating dictionary $@..."
		$(ROOTCINTTMP) -f $@ -c $(DICTIOH) $(DICTIOL)

$(DICTIOMAP):     $(RLIBMAP) $(MAKEFILEDEP) $(DICTIOL)
		$(RLIBMAP) -o $(DICTIOMAP) -l $(DICTIOLIB) \
		   -d $(DICTIOLIBDEPM) -c $(DICTIOL)

all-metautils:  $(METAUTILSO) $(METAUTILSDO) $(DICTIOLIB) $(DICTIOMAP) 

clean-metautils:
		@rm -f $(METAUTILSO) $(METAUTILSDO)

clean::         clean-metautils

distclean-metautils: clean-metautils
		@rm -f $(METAUTILSDEP) $(METAUTILSDS) $(METAUTILSDH) $(DICTIODEP) $(DICTIOO) $(DICTIODS) $(DICTIODH) $(DICTIOLIB) $(DICTIOMAP)

distclean::     distclean-metautils

##### extra rules ######
$(METAUTILSO):  PCHCXXFLAGS =
