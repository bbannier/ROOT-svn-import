# Module.mk for foam module
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: Fons Rademakers, 29/2/2000

MODDIR       := foam
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

FOAMDIR      := $(MODDIR)
FOAMDIRS     := $(FOAMDIR)/src
FOAMDIRI     := $(FOAMDIR)/inc

##### libFoam.so #####
FOAML      := $(MODDIRI)/LinkDef.h
FOAMDS     := $(MODDIRS)/G__Foam.cxx
FOAMDO     := $(FOAMDS:.cxx=.o)
FOAMDH     := $(FOAMDS:.cxx=.h)

#LF
FOAMTMPDS    := $(MODDIRS)/G__FoamTmp.cxx
FOAMTMPDO    := $(FOAMTMPDS:.cxx=.o)
FOAMTMPDH    := $(FOAMTMPDS:.cxx=.h)
FOAMTMP2DS   := $(MODDIRS)/G__FoamTmp2.cxx
FOAMTMP2DO   := $(FOAMTMP2DS:.cxx=.o)
FOAMTMP2DH   := $(FOAMTMP2DS:.cxx=.h)

FOAMH      := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
FOAMS      := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
FOAMO      := $(FOAMS:.cxx=.o)

FOAMDEP    := $(FOAMO:.o=.d) $(FOAMDO:.o=.d)

#LF
FOAMTMPDEP  := $(FOAMTMPDO:.o=.d)

FOAMLIB    := $(LPATH)/libFoam.$(SOEXT)
FOAMMAP    := $(FOAMLIB:.$(SOEXT)=.rootmap)

#LF
FOAMNM       := $(FOAMLIB:.$(SOEXT)=.nm)

# used in the main Makefile
ALLHDRS     += $(patsubst $(MODDIRI)/%.h,include/%.h,$(FOAMH))
ALLLIBS     += $(FOAMLIB)
ALLMAPS     += $(FOAMMAP)

# include all dependency files
INCLUDEFILES += $(FOAMDEP)

##### local rules #####
include/%.h:    $(FOAMDIRI)/%.h
		cp $< $@

$(FOAMLIB):   $(FOAMO) $(FOAMTMPDO) $(FOAMTMP2DO) $(FOAMDO) $(ORDER_) $(MAINLIBS) $(FOAMLIBDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		   "$(SOFLAGS)" libFoam.$(SOEXT) $@ "$(FOAMO) $(FOAMTMPDO) $(FOAMTMP2DO) $(FOAMDO)" \
		   "$(FOAMLIBEXTRA)"

#LF
$(FOAMTMPDS):   $(FOAMH) $(FOAML) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(FOAMH) $(FOAML)

#LF
$(FOAMTMP2DS):  $(FOAMH) $(FOAML) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(FOAMH) $(FOAML)

#LF
$(FOAMDS):    $(FOAMH) $(FOAML) $(ROOTCINTTMPEXE) $(FOAMNM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(FOAMNM) -. 3 -c $(FOAMH) $(FOAML)

#LF
$(FOAMDICTMAP): $(RLIBMAP) $(MAKEFILEDEP) $(FOAML)
		$(RLIBMAP) -o $(FOAMDICTMAP) -l $(FOAMDICTLIB) \
		-d $(FOAMLIB) $(FOAMLIBDEPM) -c $(FOAML)
#LF
$(FOAMNM):      $(FOAMO) $(FOAMTMPDO) $(FOAMTMP2DO) 
		@echo "Generating symbols file $@..."
		nm -p --defined-only $(FOAMTMPDO) | awk '{printf("%s\n", $$3)'} > $(FOAMNM)
		nm -p --defined-only $(FOAMTMP2DO) | awk '{printf("%s\n", $$3)'} >> $(FOAMNM)
		nm -p --defined-only $(FOAMO) | awk '{printf("%s\n", $$3)'} >> $(FOAMNM)

$(FOAMMAP):     $(RLIBMAP) $(MAKEFILEDEP) $(FOAML)
		$(RLIBMAP) -o $(FOAMMAP) -l $(FOAMLIB) \
		   -d $(FOAMLIBDEPM) -c $(FOAML)

all-foam:       $(FOAMLIB) $(FOAMMAP)

clean-foam:
		@rm -f $(FOAMO) $(FOAMDO)

clean::         clean-foam

#LF
clean-pds-foam:	
		rm -f $(FOAMTMPDS) $(FOAMTMPDO) $(FOAMTMPDH) \
		$(FOAMTMPDEP) $(FOAMTMP2DS) $(FOAMTMP2DO) $(FOAMTMP2DH) $(FOAMNM)

distclean-foam: clean-foam
		@rm -f $(FOAMDEP) $(FOAMDS) $(FOAMDH) $(FOAMLIB) $(FOAMMAP)

distclean::     distclean-foam
