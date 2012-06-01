# Module.mk for RooStats/MultiNestPlugin module
# Copyright (c) 2008 Rene Brun and Fons Rademakers
#
# Author: Sven Kreiss
# Author: Kyle Cranmer

MODNAME      := multinest
MODDIR       := $(ROOT_SRCDIR)/roofit/MultiNestPlugin
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc
MODDIRC      := $(MODDIR)/config

MULTINESTPLUGINDIR  := $(MODDIR)
MULTINESTPLUGINDIRS := $(MULTINESTPLUGINDIR)/src
MULTINESTPLUGINDIRI := $(MULTINESTPLUGINDIR)/inc


ifeq ($(PLATFORM),win32)
MNP_LIBS = $(MULTINESTPLUGINLIBEXTRA) "$(ROOTSYS)/lib/libMultiNestPlugin.lib" 
ifeq ($(BUILDMATHMORE),yes)
MNP_LIBS += "$(ROOTSYS)/lib/libMathMore.lib" 
endif
else
#for other platforms MULTINESTPLUGINLIBEXTRA is not defined 
#need to copy from config/Makefile.depend
MNP_LIBS = -Llib -lRooFit -lRooFitCore -lTree -lRIO -lMatrix \
          -lHist -lMathCore -lGraf -lGpad -lMinuit -lFoam \
          -lRooStats 
MNP_LIBS += -lMultiNestPlugin 
ifeq ($(BUILDMATHMORE),yes)
MNP_LIBS += -lMathMore
endif
endif


MNP_LIBSDEP := $(MULTINESTPLUGINLIBDEP)

MULTINESTPLUGINLIBEXTRA += -lRooStats -lRooFitCore -lgfortran -llapack -L$(MULTINESTLIBDIR) -lnest3 
 

##### libMultiNestPlugin #####
MULTINESTPLUGINL    := $(MODDIRI)/LinkDef.h
MULTINESTPLUGINDS   := $(call stripsrc,$(MODDIRS)/G__MultiNestPlugin.cxx)
MULTINESTPLUGINDO   := $(MULTINESTPLUGINDS:.cxx=.o)
MULTINESTPLUGINDH   := $(MULTINESTPLUGINDS:.cxx=.h)

MULTINESTPLUGINH    := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/RooStats/MultiNestPlugin/*.h))
MULTINESTPLUGINS    := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
MULTINESTPLUGINO    := $(call stripsrc,$(MULTINESTPLUGINS:.cxx=.o))


MULTINESTPLUGINDEP  := $(MULTINESTPLUGINO:.o=.d) $(MULTINESTPLUGINDO:.o=.d)

MULTINESTPLUGINLIB  := $(LPATH)/libMultiNestPlugin.$(SOEXT) 
MULTINESTPLUGINMAP  := $(MULTINESTPLUGINLIB:.$(SOEXT)=.rootmap)

# used in the main Makefile
ALLHDRS      += $(patsubst $(MODDIRI)/RooStats/MultiNestPlugin/%.h,include/RooStats/MultiNestPlugin/%.h,$(MULTINESTPLUGINH))
ALLLIBS      += $(MULTINESTPLUGINLIB)
ALLMAPS      += $(MULTINESTPLUGINMAP)

# include all dependency files
INCLUDEFILES += $(MULTINESTPLUGINDEP)

#needed since include are in inc and not inc/RooStats
#MULTINESTPLUGINH_DIC   := $(subst $(MODDIRI),include/RooStats/MultiNestPlugin,$(MULTINESTPLUGINH))
##### local rules #####
.PHONY:         all-$(MODNAME) clean-$(MODNAME) distclean-$(MODNAME)

include/RooStats/MultiNestPlugin/%.h:    $(MULTINESTPLUGINDIRI)/RooStats/MultiNestPlugin/%.h
		@(if [ ! -d "include/RooStats/MultiNestPlugin" ]; then    \
		   mkdir -p include/RooStats/MultiNestPlugin;             \
		fi)
		cp $< $@


$(MULTINESTPLUGINLIB): $(MULTINESTPLUGINO) $(MULTINESTPLUGINDO) $(ORDER_) $(MAINLIBS) \
	               $(MULTINESTPLUGINLIBDEP) 
		@echo "Building MultiNest plugin lib."
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		   "$(SOFLAGS)" libMultiNestPlugin.$(SOEXT) $@ \
		   "$(MULTINESTPLUGINO) $(MULTINESTPLUGINDO)" \
		   "$(MULTINESTPLUGINLIBEXTRA)"

$(MULTINESTPLUGINDS):  $(MULTINESTPLUGINH) $(MULTINESTPLUGINL) $(ROOTCINTTMPDEP)
		$(MAKEDIR)
		@echo "Generating dictionary $@..."
		$(ROOTCINTTMP) -f $@ -c $(MULTINESTPLUGINH) $(MULTINESTPLUGINL)

$(MULTINESTPLUGINMAP): $(RLIBMAP) $(MAKEFILEDEP) $(MULTINESTPLUGINL)
	               $(RLIBMAP) -o  $@ -l $(MULTINESTPLUGINLIB) \
	               -d $(MULTINESTPLUGINLIBDEPM) -c $(MULTINESTPLUGINL)




all-$(MODNAME): $(MULTINESTPLUGINLIB) $(MULTINESTPLUGINMAP) $(MNP_MAKEWORKSPACEEXE) #$(MNP_MAKEWORKSPACEEXE2)

clean-$(MODNAME):
		@rm -f $(MULTINESTPLUGINO) $(MULTINESTPLUGINDO)

clean::         clean-$(MODNAME)

distclean-$(MODNAME): clean-$(MODNAME)
		@rm -rf $(MULTINESTPLUGINDEP) $(MULTINESTPLUGINLIB) $(MULTINESTPLUGINMAP) \
		   $(MULTINESTPLUGINDS) $(MULTINESTPLUGINDH)

distclean::     distclean-$(MODNAME)
