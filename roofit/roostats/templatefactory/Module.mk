# Module.mk for RooStats/TemplateFactory module
# Copyright (c) 2008 Rene Brun and Fons Rademakers
#
# Author: Kyle Cranmer

MODNAME      := templatefactory
MODDIR       := roofit/roostats/$(MODNAME)
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

#TF_LIBS                := $(ROOFITCORELIB) $(ROOFITLIB) $(ROOSTATSLIB) $(TEMPLATEFACTORYLIB) $(FOAMLIB) $(XMLPARSERLIB) 
TF_LIBS                := -lRooFitCore -lRooFit -lRooStats -lTemplateFactory -lFoam -lHtml -lMinuit -lThread -lXMLParser

TF_LIBSDEP                := $(ROOFITCOREDEP) $(ROOFITDEP) $(ROOSTATSDEP) $(TEMPLATEFACTORYDEP) $(FOAMDEP) $(XMLPARSERDEP)
#-lRooFitCore -lRooFit -lRooStats -lTemplateFactory #-lFoam -lHtml -lMinuit -lThread -lXMLParser

$(TF_MAKEWORKSPACEEXE): $(TF_MAKEWORKSPACEEXEO) $(ROOTLIBSDEP) $(RINTLIB) $(TF_LIBSDEP)
	echo $(TF_LIBS)
	#echo $(TF_LIBSDEP)
		$(LD) $(LDFLAGS) -o $@ $(TF_MAKEWORKSPACEEXEO)  $(ROOTICON) $(BOOTULIBS)  \
		   $(ROOTULIBS) $(RPATH) $(ROOTLIBS)  $(RINTLIBS) $(TF_LIBS) $(SYSLIBS)  

#make XXXDEP for the .so files#
#make XXX for the -lXXX files
# two versions for this see ROOTSYS/Makefile

#remove .exe for non windows case
#man/man1/  see example (bin/thisroot.sh sets man path)


#this works
#g++ -O2 -m64 -O2 /home/cranmer/roostats/root/roofit/roostats/templatefactory/src/MakeModelAndMeasurements.o -L/home/cranmer/roostats/root/lib/ -l TemplateFactory \
#          -L/home/cranmer/roostats/root/lib -lCore -lCint -lRIO -lNet -lHist -lGraf -lGraf3d -lGpad -lTree -lRint -lPostscript -lMatrix -lPhysics -lMathCore -lThread -lz -pthread -lm -ldl -rdynamic -lRooFitCore -lRooFit -lRooStats -lFoam -lHtml -lMinuit -lThread -lXMLParser -o /home/cranmer/roostats/root/bin/testKyle.exe


# used in the main Makefile
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
