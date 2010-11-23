# Module.mk for RooStats/TemplateFactory module
# Copyright (c) 2008 Rene Brun and Fons Rademakers
#
# Author: Kyle Cranmer

MODNAME      := templatefactory
MODDIR       := roofit/$(MODNAME)
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

TEMPLATEFACTORYDIR  := $(MODDIR)
TEMPLATEFACTORYDIRS := $(TEMPLATEFACTORYDIR)/src
TEMPLATEFACTORYDIRI := $(TEMPLATEFACTORYDIR)/inc

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

#TF_LIBS := $(TEMPLATEFACTORYLIBEXTRA)

ifeq ($(PLATFORM),win32)
TF_LIBS= "$(ROOTSYS)/lib/libMinuit.lib" "$(ROOTSYS)/lib/libThread.lib" "$(ROOTSYS)/lib/libRooFit.lib" "$(ROOTSYS)/lib/libRooFitCore.lib" "$(ROOTSYS)/lib/libRoostats.lib"  "$(ROOTSYS)/lib/libXMLParser.lib"  "$(ROOTSYS)/lib/libTemplateFactory.lib"
else
TF_LIBS= -lMinuit   -lRooFit -lRooFitCore -lFoam -lRooStats  -lTemplateFactory -lXMLParser 
# #TF_LIBS= -lMinuit  -lThread -lRooFit -lRooFitCore -lHtml -lFoam -lRooStats -lTemplateFactory
endif


TF_LIBSDEP := $(TEMPLATEFACTORYLIBDEP)


$(TF_MAKEWORKSPACEEXE): $(TF_MAKEWORKSPACEEXEO) $(ROOTLIBSDEP) $(RINTLIB) $(TEMPLATEFACTORYLIBDEP)
		$(LD) $(LDFLAGS) -o $@ $(TF_MAKEWORKSPACEEXEO)  $(ROOTICON) $(BOOTULIBS)  \
		   $(ROOTULIBS) $(RPATH) $(ROOTLIBS)  $(RINTLIBS) $(TF_LIBS) $(SYSLIBS)  

ALLEXECS     += $(TF_MAKEWORKSPACEEXE) 


##### libRooStats #####
TEMPLATEFACTORYL    := $(MODDIRI)/LinkDef.h
TEMPLATEFACTORYDS   := $(MODDIRS)/G__TemplateFactory.cxx
TEMPLATEFACTORYDO   := $(TEMPLATEFACTORYDS:.cxx=.o)
TEMPLATEFACTORYDH   := $(TEMPLATEFACTORYDS:.cxx=.h)

TEMPLATEFACTORYH    := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
TEMPLATEFACTORYS    := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
TEMPLATEFACTORYO    := $(TEMPLATEFACTORYS:.cxx=.o)

TEMPLATEFACTORYDEP  := $(TEMPLATEFACTORYO:.o=.d) $(TEMPLATEFACTORYDO:.o=.d)

TEMPLATEFACTORYLIB  := $(LPATH)/libTemplateFactory.$(SOEXT)
TEMPLATEFACTORYMAP  := $(TEMPLATEFACTORYLIB:.$(SOEXT)=.rootmap)

# used in the main Makefile
ALLHDRS      += $(patsubst $(MODDIRI)/%.h,include/RooStats/TemplateFactory/%.h,$(TEMPLATEFACTORYH))
ALLLIBS      += $(TEMPLATEFACTORYLIB)
ALLMAPS      += $(TEMPLATEFACTORYMAP)

# include all dependency files
INCLUDEFILES += $(TEMPLATEFACTORYDEP)

#needed since include are in inc and not inc/RooStats
TEMPLATEFACTORYH_DIC   := $(subst $(MODDIRI),include/RooStats/TemplateFactory,$(TEMPLATEFACTORYH))
##### local rules #####
.PHONY:         all-$(MODNAME) clean-$(MODNAME) distclean-$(MODNAME)

include/RooStats/TemplateFactory/%.h:    $(TEMPLATEFACTORYDIRI)/%.h
		@(if [ ! -d "include/RooStats/TemplateFactory" ]; then    \
		   mkdir -p include/RooStats/TemplateFactory;             \
		fi)
		cp $< $@

$(TEMPLATEFACTORYLIB): $(TEMPLATEFACTORYO) $(TEMPLATEFACTORYDO) $(ORDER_) $(MAINLIBS) \
                $(TEMPLATEFACTORYLIBDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		   "$(SOFLAGS)" libTemplateFactory.$(SOEXT) $@ \
		   "$(TEMPLATEFACTORYO) $(TEMPLATEFACTORYDO)" \
		   "$(TEMPLATEFACTORYLIBEXTRA)"

$(TEMPLATEFACTORYDS):  $(TEMPLATEFACTORYH_DIC) $(TEMPLATEFACTORYL) $(ROOTCINTTMPDEP)
		@echo "Generating dictionary $@..."
		$(ROOTCINTTMP) -f $@ -c $(TEMPLATEFACTORYH_DIC) $(TEMPLATEFACTORYL)

$(TEMPLATEFACTORYMAP): $(RLIBMAP) $(MAKEFILEDEP) $(TEMPLATEFACTORYL)
		$(RLIBMAP) -o $(TEMPLATEFACTORYMAP) -l $(TEMPLATEFACTORYLIB) \
		   -d $(TEMPLATEFACTORYLIBDEPM) -c $(TEMPLATEFACTORYL)

all-$(MODNAME): $(TEMPLATEFACTORYLIB) $(TEMPLATEFACTORYMAP)

clean-$(MODNAME):
		@rm -f $(TEMPLATEFACTORYO) $(TEMPLATEFACTORYDO)

clean::         clean-$(MODNAME)

distclean-$(MODNAME): clean-$(MODNAME)
		@rm -rf $(TEMPLATEFACTORYDEP) $(TEMPLATEFACTORYLIB) $(TEMPLATEFACTORYMAP) \
		   $(TEMPLATEFACTORYDS) $(TEMPLATEFACTORYDH)

distclean::     distclean-$(MODNAME)
