# Module.mk for graf module
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: Fons Rademakers, 29/2/2000

MODDIR       := graf
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

GRAFDIR      := $(MODDIR)
GRAFDIRS     := $(GRAFDIR)/src
GRAFDIRI     := $(GRAFDIR)/inc

##### libGraf #####
GRAFL1       := $(MODDIRI)/LinkDef1.h
GRAFL2       := $(MODDIRI)/LinkDef2.h
GRAFDS1      := $(MODDIRS)/G__Graf1.cxx
GRAFDS2      := $(MODDIRS)/G__Graf2.cxx
GRAFDO1      := $(GRAFDS1:.cxx=.o)
GRAFDO2      := $(GRAFDS2:.cxx=.o)
GRAFDS       := $(GRAFDS1) $(GRAFDS2)
GRAFDO       := $(GRAFDO1) $(GRAFDO2)
GRAFDH       := $(GRAFDS:.cxx=.h)

#LF
GRAFTMPDS1    := $(MODDIRS)/G__Graf1Tmp.cxx
GRAFTMPDO1    := $(GRAFTMPDS1:.cxx=.o)
GRAFTMPDH1    := $(GRAFTMPDS1:.cxx=.h)
GRAFTMP2DS1   := $(MODDIRS)/G__Graf1Tmp2.cxx
GRAFTMP2DO1   := $(GRAFTMP2DS1:.cxx=.o)
GRAFTMP2DH1   := $(GRAFTMP2DS1:.cxx=.h)

#LF
GRAFTMPDS2    := $(MODDIRS)/G__Graf2Tmp.cxx
GRAFTMPDO2    := $(GRAFTMPDS2:.cxx=.o)
GRAFTMPDH2    := $(GRAFTMPDS2:.cxx=.h)
GRAFTMP2DS2   := $(MODDIRS)/G__Graf2Tmp2.cxx
GRAFTMP2DO2   := $(GRAFTMP2DS2:.cxx=.o)
GRAFTMP2DH2   := $(GRAFTMP2DS2:.cxx=.h)

GRAFH        := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
GRAFHD       := $(filter-out $(MODDIRI)/TTF.h,$(GRAFH))
GRAFHD       := $(filter-out $(MODDIRI)/TText.h,$(GRAFHD))
GRAFHD       := $(filter-out $(MODDIRI)/TLatex.h,$(GRAFHD))
GRAFS        := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
GRAFO        := $(GRAFS:.cxx=.o)

#LF
GRAFTMPDS       := $(GRAFTMPDS1) $(GRAFTMPDS2)
GRAFTMPDO       := $(GRAFTMPDO1) $(GRAFTMPDO2)
GRAFTMP2DS      := $(GRAFTMP2DS1) $(GRAFTMP2DS2)
GRAFTMP2DO      := $(GRAFTMP2DO1) $(GRAFTMP2DO2)

GRAFDEP      := $(GRAFO:.o=.d) $(GRAFDO:.o=.d)
#LF
GRAFTMPDEP   := $(GRAFTMPDO:.o=.d)

GRAFLIB      := $(LPATH)/libGraf.$(SOEXT)

#LF
GRAFMAP      := $(GRAFLIB:.$(SOEXT)=.rootmap)
GRAFLIB       := $(LPATH)/libGraf.$(SOEXT)
GRAFDICTLIB   := $(LPATH)/libGrafDict.$(SOEXT)
GRAFDICTMAP   := $(GRAFDICTLIB:.$(SOEXT)=.rootmap)
GRAFNM        := $(GRAFLIB:.$(SOEXT)=.nm)

# used in the main Makefile
ALLHDRS     += $(patsubst $(MODDIRI)/%.h,include/%.h,$(GRAFH))
ALLLIBS     += $(GRAFLIB)
ALLMAPS     += $(GRAFMAP)

# include all dependency files
INCLUDEFILES += $(GRAFDEP)

##### local rules #####
include/%.h:    $(GRAFDIRI)/%.h
		cp $< $@

#LF
$(GRAFLIB):      $(GRAFO) $(GRAFTMPDO) $(GRAFTMP2DO) $(GRAFDO) $(FREETYPEDEP) $(ORDER_) $(MAINLIBS) $(GRAFLIBDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		"$(SOFLAGS)" libGraf.$(SOEXT) $@ "$(GRAFO) $(GRAFTMPDO) $(GRAFTMP2DO) $(GRAFDO)"\
		"$(FREETYPELDFLAGS) $(FREETYPELIB) $(GRAFLIBEXTRA)"
#LF
#$(GRAFDICTLIB):  $(GRAFDO) $(FREETYPEDEP) $(ORDER_) $(MAINLIBS) $(GRAFDICTLIBDEP) $(GRAFTMP2DO)
#		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
#		"$(SOFLAGS)" libGrafDict.$(SOEXT) $@ "$(GRAFDO) $(GRAFTMP2DO)"\
#		"$(FREETYPELDFLAGS) $(FREETYPELIB) $(GRAFDICTLIBEXTRA)"

#LF
$(GRAFTMPDS1):   $(GRAFHD) $(GRAFL1) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(GRAFHD) $(GRAFL1)
#LF
$(GRAFTMP2DS1):  $(GRAFHD) $(GRAFL1) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(GRAFHD) $(GRAFL1)
#LF
$(GRAFDS1):      $(GRAFHD) $(GRAFL1) $(ROOTCINTTMPEXE) $(GRAFNM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(GRAFNM) -. 3 -c $(GRAFHD) $(GRAFL1)

#LF
$(GRAFTMPDS2):   $(GRAFH) $(GRAFL2) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(FREETYPEINC) $(GRAFH) $(GRAFL2)
#LF
$(GRAFTMP2DS2):  $(GRAFH) $(GRAFL2) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(FREETYPEINC) $(GRAFH) $(GRAFL2)
#LF
$(GRAFDS2):     $(GRAFH) $(GRAFL2) $(ROOTCINTTMPEXE) $(GRAFNM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(GRAFNM) -. 3 -c $(FREETYPEINC) $(GRAFH) $(GRAFL2)

#LF
$(GRAFMAP):     $(RLIBMAP) $(MAKEFILEDEP) $(GRAFL1) $(GRAFL2)
		$(RLIBMAP) -o $(GRAFMAP) -l $(GRAFLIB) \
		-d $(GRAFLIBDEPM) -c $(GRAFL1) $(GRAFL2)

#LF
$(GRAFNM):      $(GRAFO) $(GRAFTMPDO) $(GRAFTMP2DO)
		@echo "Generating symbols file $@..."
		nm -g -p --defined-only $(GRAFTMPDO1) | awk '{printf("%s\n", $$3)'} > $(GRAFNM)
		nm -g -p --defined-only $(GRAFTMPDO2) | awk '{printf("%s\n", $$3)'} >> $(GRAFNM)
		nm -g -p --defined-only $(GRAFTMP2DO1) | awk '{printf("%s\n", $$3)'} >> $(GRAFNM)
		nm -g -p --defined-only $(GRAFTMP2DO2) | awk '{printf("%s\n", $$3)'} >> $(GRAFNM)
		nm -g -p --defined-only $(GRAFO) | awk '{printf("%s\n", $$3)'} >> $(GRAFNM)

all-graf:       $(GRAFLIB) $(GRAFMAP)

clean-graf:
		@rm -f $(GRAFO) $(GRAFDO)

clean::         clean-graf

#LF
clean-pds-graf:	
		rm -f $(GRAFTMPDS) $(GRAFTMP2DS) $(GRAFTMPDO) $(GRAFTMP2DO) $(GRAFTMPDH) \
		$(GRAFTMP2DH) $(GRAFTMPDEP) $(GRAFNM)

distclean-graf: clean-graf
		@rm -f $(GRAFDEP) $(GRAFDS) $(GRAFDH) $(GRAFLIB) $(GRAFMAP)

distclean::     distclean-graf

##### extra rules ######
$(GRAFDO2):     $(FREETYPEDEP)
$(GRAFDO2):     OPT = $(NOOPT)
$(GRAFDO2):     CXXFLAGS += $(FREETYPEINC)

#LF
$(GRAFTMPDO2):     $(FREETYPEDEP)
$(GRAFTMPDO2):     OPT = $(NOOPT)
$(GRAFTMPDO2):     CXXFLAGS += $(FREETYPEINC)

$(GRAFTMP2DO2):     $(FREETYPEDEP)
$(GRAFTMP2DO2):     OPT = $(NOOPT)
$(GRAFTMP2DO2):     CXXFLAGS += $(FREETYPEINC)


graf/src/TTF.o graf/src/TText.o graf/src/TLatex.o: \
                $(FREETYPEDEP)
graf/src/TTF.o graf/src/TText.o graf/src/TLatex.o: \
                CXXFLAGS += $(FREETYPEINC)

ifeq ($(PLATFORM),win32)
graf/src/TLatex.o: OPT = $(NOOPT)
endif
