# Module.mk for mathcore module
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: David Gonzalez Maline, 23/2/2009

MODNAME      := galib
MODDIR       := math/$(MODNAME)
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

GALIBDIR  := $(MODDIR)
GALIBDIRS := $(GALIBDIR)/src
GALIBDIRI := $(GALIBDIR)/inc/Math

##### libGenetic #####
GALIBL    := $(MODDIRI)/LinkDef.h
#GALIBLINC :=
GALIBDS   := $(MODDIRS)/G__GAlib.cxx
GALIBDO   := $(GALIBDS:.cxx=.o)
GALIBGASRC:= $(GALIBDIRS)/ga/garandom.C $(GALIBDIRS)/ga/gaerror.C $(GALIBDIRS)/ga/GAParameter.C $(GALIBDIRS)/ga/GAStatistics.C \
	     $(GALIBDIRS)/ga/GABaseGA.C $(GALIBDIRS)/ga/GASStateGA.C $(GALIBDIRS)/ga/GASimpleGA.C $(GALIBDIRS)/ga/GAIncGA.C \
             $(GALIBDIRS)/ga/GADemeGA.C $(GALIBDIRS)/ga/GADCrowdingGA.C $(GALIBDIRS)/ga/GASelector.C \
             $(GALIBDIRS)/ga/GAScaling.C $(GALIBDIRS)/ga/GAPopulation.C $(GALIBDIRS)/ga/GAGenome.C \
             $(GALIBDIRS)/ga/GABinStr.C $(GALIBDIRS)/ga/gabincvt.C $(GALIBDIRS)/ga/GAAllele.C \
             $(GALIBDIRS)/ga/GAStringGenome.C $(GALIBDIRS)/ga/GA1DBinStrGenome.C \
             $(GALIBDIRS)/ga/GA2DBinStrGenome.C $(GALIBDIRS)/ga/GA3DBinStrGenome.C $(GALIBDIRS)/ga/GABin2DecGenome.C \
             $(GALIBDIRS)/ga/GA1DArrayGenome.C $(GALIBDIRS)/ga/GA2DArrayGenome.C $(GALIBDIRS)/ga/GA3DArrayGenome.C \
             $(GALIBDIRS)/ga/GATreeBASE.C $(GALIBDIRS)/ga/GATree.C $(GALIBDIRS)/ga/GATreeGenome.C \
             $(GALIBDIRS)/ga/GAListBASE.C $(GALIBDIRS)/ga/GAList.C $(GALIBDIRS)/ga/GAListGenome.C

GALIBGAO  := $(GALIBGASRC:.C=.o)

GALIBDH   := $(GALIBDS:.cxx=.h)

GALIBDH1  :=  $(MODDIRI)/Math/GAlibMinimizer.h \

GALIBH   := $(filter-out $(MODDIRI)/Math/LinkDef%, $(wildcard $(MODDIRI)/Math/*.h))
GALIBS    := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
GALIBO    := $(GALIBS:.cxx=.o)

GALIBDEP  := $(GALIBO:.o=.d)  $(GALIBDO:.o=.d)

GALIBLIB  := $(LPATH)/libGAlib.$(SOEXT)
GALIBMAP  := $(GALIBLIB:.$(SOEXT)=.rootmap)

# used in the main Makefile
ALLHDRS      += $(patsubst $(MODDIRI)/Math/%.h,include/Math/%.h,$(GALIBH))
ALLLIBS      += $(GALIBLIB)
ALLMAPS      += $(GALIBMAP)

# include all dependency files
INCLUDEFILES += $(GALIBDEP)

##### local rules #####
.PHONY:         all-$(MODNAME) clean-$(MODNAME) distclean-$(MODNAME) \
                test-$(MODNAME) check-$(MODNAME)

include/Math/%.h: $(GALIBDIRI)/%.h
		@(if [ ! -d "include/Math" ]; then     \
		   mkdir -p include/Math;              \
		fi)
		cp $< $@

$(GALIBLIB): $(GALIBGAO) $(GALIBO) $(GALIBDO) $(ORDER_) $(MAINLIBS)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)"  \
		   "$(SOFLAGS)" libGAlib.$(SOEXT) $@     \
		   "$(GALIBO) $(GALIBDO) $(GALIBGAO)" \
		   "$(GALIBLIBEXTRA)"

$(GALIBDS):  $(GALIBDH1) $(GALIBL) $(GALIBLINC) $(ROOTCINTTMPDEP)
		@echo "Generating dictionary $@..."
		@echo "for files $(GALIBDH1)"
		$(ROOTCINTTMP) -f $@ -c $(GALIBDH1) $(GALIBL)

$(GALIBMAP):  $(RLIBMAP) $(MAKEFILEDEP) $(GALIBL) $(GALIBLINC)
		$(RLIBMAP) -o $(GALIBMAP) -l $(GALIBLIB) \
		   -d $(GALIBLIBDEPM) -c $(GALIBL) $(GALIBLINC)

ifneq ($(ICC_MAJOR),)
# silence warning messages about subscripts being out of range
$(GALIBDO):   CXXFLAGS += -wd175 -I$(GALIBDIRI)
else
$(GALIBDO):   CXXFLAGS += -I$(GALIBDIRI) -I.
endif

all-$(MODNAME): $(GALIBLIB) $(GALIBMAP)

clean-$(MODNAME):
		@rm -f $(GALIBO) $(GALIBDO)

clean::         clean-$(MODNAME)

distclean-$(MODNAME): clean-$(MODNAME)
		@rm -f $(GALIBDEP) $(GALIBDS) $(GALIBDH) \
		   $(GALIBLIB) $(GALIBMAP)
		@rm -rf include/Math
		-@cd $(GALIBDIR)/test && $(MAKE) distclean

distclean::     distclean-$(MODNAME)

test-$(MODNAME): all-$(MODNAME)
		@cd $(GALIBDIR)/test && $(MAKE)

check-$(MODNAME): test-$(MODNAME)
		@cd $(GALIBDIR)/test && $(MAKE)

##### extra rules ######
$(GALIBO): CXXFLAGS += -I$(GALIBDIRS)
$(GALIBGAO): CXXFLAGS += -I$(GALIBDIRS) -I$(GALIBDIRS)/ga