# Module.mk for quadp module
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: Fons Rademakers, Eddy Offermann, 21/05/2003

MODDIR       := quadp
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

QUADPDIR     := $(MODDIR)
QUADPDIRS    := $(QUADPDIR)/src
QUADPDIRI    := $(QUADPDIR)/inc

##### libQuadp #####
QUADPL       := $(MODDIRI)/LinkDef.h
QUADPDS      := $(MODDIRS)/G__Quadp.cxx
QUADPDO      := $(QUADPDS:.cxx=.o)
QUADPDH      := $(QUADPDS:.cxx=.h)

#LF
QUADPTMPDS    := $(MODDIRS)/G__QuadpTmp.cxx
QUADPTMPDO    := $(QUADPTMPDS:.cxx=.o)
QUADPTMPDH    := $(QUADPTMPDS:.cxx=.h)
QUADPTMP2DS   := $(MODDIRS)/G__QuadpTmp2.cxx
QUADPTMP2DO   := $(QUADPTMP2DS:.cxx=.o)
QUADPTMP2DH   := $(QUADPTMP2DS:.cxx=.h)

QUADPH       := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
QUADPS       := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
QUADPO       := $(QUADPS:.cxx=.o)

QUADPDEP     := $(QUADPO:.o=.d) $(QUADPDO:.o=.d)

#LF
QUADPTMPDEP  := $(QUADPTMPDO:.o=.d)

QUADPLIB     := $(LPATH)/libQuadp.$(SOEXT)
QUADPMAP     := $(QUADPLIB:.$(SOEXT)=.rootmap)

#LF
QUADPNM       := $(QUADPLIB:.$(SOEXT)=.nm)

# used in the main Makefile
ALLHDRS     += $(patsubst $(MODDIRI)/%.h,include/%.h,$(QUADPH))
ALLLIBS     += $(QUADPLIB)
ALLMAPS     += $(QUADPMAP)

# include all dependency files
INCLUDEFILES += $(QUADPDEP)

##### local rules #####
include/%.h:    $(QUADPDIRI)/%.h
		cp $< $@

#LF
$(QUADPLIB):   $(QUADPO) $(QUADPTMPDO) $(QUADPTMP2DO) $(QUADPDO) $(ORDER_) $(MAINLIBS) $(QUADPLIBDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		   "$(SOFLAGS)" libQuadp.$(SOEXT) $@ "$(QUADPO) $(QUADPTMPDO) $(QUADPTMP2DO) $(QUADPDO)" \
		   "$(QUADPLIBEXTRA)"

#LF
$(QUADPTMPDS):   $(QUADPH) $(QUADPL) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(QUADPH) $(QUADPL)

#LF
$(QUADPTMP2DS):  $(QUADPH) $(QUADPL) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(QUADPH) $(QUADPL)

#LF
$(QUADPDS):    $(QUADPH) $(QUADPL) $(ROOTCINTTMPEXE) $(QUADPNM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(QUADPNM) -. 3 -c $(QUADPH) $(QUADPL)

#LF
$(QUADPDICTMAP): $(RLIBMAP) $(MAKEFILEDEP) $(QUADPL)
		$(RLIBMAP) -o $(QUADPDICTMAP) -l $(QUADPDICTLIB) \
		-d $(QUADPLIB) $(QUADPLIBDEPM) -c $(QUADPL)
#LF
$(QUADPNM):      $(QUADPO) $(QUADPTMPDO) $(QUADPTMP2DO) 
		@echo "Generating symbols file $@..."
		nm -p --defined-only $(QUADPTMPDO) | awk '{printf("%s\n", $$3)'} > $(QUADPNM)
		nm -p --defined-only $(QUADPTMP2DO) | awk '{printf("%s\n", $$3)'} >> $(QUADPNM)
		nm -p --defined-only $(QUADPO) | awk '{printf("%s\n", $$3)'} >> $(QUADPNM)

$(QUADPMAP):    $(RLIBMAP) $(MAKEFILEDEP) $(QUADPL)
		$(RLIBMAP) -o $(QUADPMAP) -l $(QUADPLIB) \
		   -d $(QUADPLIBDEPM) -c $(QUADPL)

all-quadp:      $(QUADPLIB) $(QUADPMAP)

clean-quadp:
		@rm -f $(QUADPO) $(QUADPDO)

clean::         clean-quadp clean-pds-quadp

#LF
clean-pds-quadp:	
		rm -f $(QUADPTMPDS) $(QUADPTMPDO) $(QUADPTMPDH) \
		$(QUADPTMPDEP) $(QUADPTMP2DS) $(QUADPTMP2DO) $(QUADPTMP2DH) $(QUADPNM)

distclean-quadp: clean-quadp
		@rm -f $(QUADPDEP) $(QUADPDS) $(QUADPDH) $(QUADPLIB) $(QUADPMAP)

distclean::     distclean-quadp
