# Module.mk for RooStats/HistFactory module
# Copyright (c) 2008 Rene Brun and Fons Rademakers
#
# Author: Kyle Cranmer

MODNAME      := histfactory
MODDIR       := roofit/$(MODNAME)
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

HISTFACTORYDIR  := $(MODDIR)
HISTFACTORYDIRS := $(HISTFACTORYDIR)/src
HISTFACTORYDIRI := $(HISTFACTORYDIR)/inc

##### tf_makeworkspace.exe #####
TF_MAKEWORKSPACEEXES   := $(MODDIRS)/MakeModelAndMeasurements.cxx
TF_MAKEWORKSPACEEXEO   := $(TF_MAKEWORKSPACEEXES:.cxx=.o)
TF_MAKEWORKSPACEEXEDEP := $(TF_MAKEWORKSPACEEXEO:.o=.d)
ifeq ($(ARCH),win32gcc)
TF_MAKEWORKSPACEEXE    := bin/hist2workspace.exe
else
TF_MAKEWORKSPACEEXE    := bin/hist2workspace
#TF_MAKEWORKSPACESH     := bin/tf_makeworkspace
endif
ifeq ($(PLATFORM),win32)
TF_MAKEWORKSPACEEXE    :=
TF_MAKEWORKSPACESH     :=
endif

#TF_LIBS := $(HISTFACTORYLIBEXTRA)

ifeq ($(PLATFORM),win32)
TF_LIBS= "$(ROOTSYS)/lib/libMinuit.lib"  "$(ROOTSYS)/lib/libRooFit.lib" "$(ROOTSYS)/lib/libRooFitCore.lib" "$(ROOTSYS)/lib/libRoostats.lib"  "$(ROOTSYS)/lib/libXMLParser.lib"  "$(ROOTSYS)/lib/libHistFactory.lib" "$(ROOTSYS)/lib/libFoam.lib"
else
TF_LIBS= -lMinuit   -lRooFit -lRooFitCore -lFoam -lRooStats  -lHistFactory -lXMLParser 
endif


TF_LIBSDEP := $(HISTFACTORYLIBDEP)


$(TF_MAKEWORKSPACEEXE): $(TF_MAKEWORKSPACEEXEO) $(ROOTLIBSDEP) $(RINTLIB) $(HISTFACTORYLIBDEPM)
		$(LD) $(LDFLAGS) -o $@ $(TF_MAKEWORKSPACEEXEO)  $(ROOTICON) $(BOOTULIBS)  \
		   $(ROOTULIBS) $(RPATH) $(ROOTLIBS)  $(RINTLIBS) $(TF_LIBS) $(SYSLIBS)  
		cp $(MODDIR)/config/Config.dtd $(ROOTSYS)/etc/	


ALLEXECS     += $(TF_MAKEWORKSPACEEXE) 


##### libRooStats #####
HISTFACTORYL    := $(MODDIRI)/LinkDef.h
HISTFACTORYDS   := $(MODDIRS)/G__HistFactory.cxx
HISTFACTORYDO   := $(HISTFACTORYDS:.cxx=.o)
HISTFACTORYDH   := $(HISTFACTORYDS:.cxx=.h)

HISTFACTORYH    := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
HISTFACTORYS    := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
HISTFACTORYO    := $(HISTFACTORYS:.cxx=.o)

HISTFACTORYDEP  := $(HISTFACTORYO:.o=.d) $(HISTFACTORYDO:.o=.d)

HISTFACTORYLIB  := $(LPATH)/libHistFactory.$(SOEXT)
HISTFACTORYMAP  := $(HISTFACTORYLIB:.$(SOEXT)=.rootmap)

# used in the main Makefile
ALLHDRS      += $(patsubst $(MODDIRI)/%.h,include/RooStats/HistFactory/%.h,$(HISTFACTORYH))
ALLLIBS      += $(HISTFACTORYLIB)
ALLMAPS      += $(HISTFACTORYMAP)

# include all dependency files
INCLUDEFILES += $(HISTFACTORYDEP)

#needed since include are in inc and not inc/RooStats
HISTFACTORYH_DIC   := $(subst $(MODDIRI),include/RooStats/HistFactory,$(HISTFACTORYH))
##### local rules #####
.PHONY:         all-$(MODNAME) clean-$(MODNAME) distclean-$(MODNAME)

include/RooStats/HistFactory/%.h:    $(HISTFACTORYDIRI)/%.h
		@(if [ ! -d "include/RooStats/HistFactory" ]; then    \
		   mkdir -p include/RooStats/HistFactory;             \
		fi)
		cp $< $@


$(HISTFACTORYLIB): $(HISTFACTORYO) $(HISTFACTORYDO) $(ORDER_) $(MAINLIBS) \
                $(HISTFACTORYLIBDEP) 
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		   "$(SOFLAGS)" libHistFactory.$(SOEXT) $@ \
		   "$(HISTFACTORYO) $(HISTFACTORYDO)" \
		   "$(HISTFACTORYLIBEXTRA)"

$(HISTFACTORYDS):  $(HISTFACTORYH_DIC) $(HISTFACTORYL) $(ROOTCINTTMPDEP)
		@echo "Generating dictionary $@..."
		$(ROOTCINTTMP) -f $@ -c $(HISTFACTORYH_DIC) $(HISTFACTORYL)

$(HISTFACTORYMAP): $(RLIBMAP) $(MAKEFILEDEP) $(HISTFACTORYL)
		$(RLIBMAP) -o $(HISTFACTORYMAP) -l $(HISTFACTORYLIB) \
		   -d $(HISTFACTORYLIBDEPM) -c $(HISTFACTORYL)

all-$(MODNAME): $(HISTFACTORYLIB) $(HISTFACTORYMAP)

clean-$(MODNAME):
		@rm -f $(HISTFACTORYO) $(HISTFACTORYDO)

clean::         clean-$(MODNAME)

distclean-$(MODNAME): clean-$(MODNAME)
		@rm -rf $(HISTFACTORYDEP) $(HISTFACTORYLIB) $(HISTFACTORYMAP) \
		   $(HISTFACTORYDS) $(HISTFACTORYDH)

distclean::     distclean-$(MODNAME)
