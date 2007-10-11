# Module.mk for physics module
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: Fons Rademakers, 29/2/2000

MODDIR       := physics
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

PHYSICSDIR   := $(MODDIR)
PHYSICSDIRS  := $(PHYSICSDIR)/src
PHYSICSDIRI  := $(PHYSICSDIR)/inc

##### libPhysics #####
PHYSICSL     := $(MODDIRI)/LinkDef.h
PHYSICSDS    := $(MODDIRS)/G__Physics.cxx
PHYSICSDO    := $(PHYSICSDS:.cxx=.o)
PHYSICSDH    := $(PHYSICSDS:.cxx=.h)

#LF
PHYSICSTMPDS    := $(MODDIRS)/G__PhysicsTmp.cxx
PHYSICSTMPDO    := $(PHYSICSTMPDS:.cxx=.o)
PHYSICSTMPDH    := $(PHYSICSTMPDS:.cxx=.h)
PHYSICSTMP2DS   := $(MODDIRS)/G__PhysicsTmp2.cxx
PHYSICSTMP2DO   := $(PHYSICSTMP2DS:.cxx=.o)
PHYSICSTMP2DH   := $(PHYSICSTMP2DS:.cxx=.h)

PHYSICSH     := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
PHYSICSS     := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
PHYSICSO     := $(PHYSICSS:.cxx=.o)

PHYSICSDEP   := $(PHYSICSO:.o=.d) $(PHYSICSDO:.o=.d)

#LF
PHYSICSTMPDEP  := $(PHYSICSTMPDO:.o=.d)

PHYSICSLIB   := $(LPATH)/libPhysics.$(SOEXT)
PHYSICSMAP   := $(PHYSICSLIB:.$(SOEXT)=.rootmap)

#LF
PHYSICSNM       := $(PHYSICSLIB:.$(SOEXT)=.nm)

# used in the main Makefile
ALLHDRS     += $(patsubst $(MODDIRI)/%.h,include/%.h,$(PHYSICSH))
ALLLIBS     += $(PHYSICSLIB)
ALLMAPS     += $(PHYSICSMAP)

# include all dependency files
INCLUDEFILES += $(PHYSICSDEP)

##### local rules #####
include/%.h:    $(PHYSICSDIRI)/%.h
		cp $< $@

#LF
$(PHYSICSLIB):   $(PHYSICSO) $(PHYSICSTMPDO) $(PHYSICSTMP2DO) $(PHYSICSDO) $(ORDER_) $(MAINLIBS) $(PHYSICSLIBDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		   "$(SOFLAGS)" libPhysics.$(SOEXT) $@ "$(PHYSICSO) $(PHYSICSTMPDO) $(PHYSICSTMP2DO) $(PHYSICSDO)" \
		   "$(PHYSICSLIBEXTRA)"

#LF
$(PHYSICSTMPDS):   $(PHYSICSH) $(PHYSICSL) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(PHYSICSH) $(PHYSICSL)

#LF
$(PHYSICSTMP2DS):  $(PHYSICSH) $(PHYSICSL) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(PHYSICSH) $(PHYSICSL)

#LF
$(PHYSICSDS):    $(PHYSICSH) $(PHYSICSL) $(ROOTCINTTMPEXE) $(PHYSICSNM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(PHYSICSNM) -. 3 -c $(PHYSICSH) $(PHYSICSL)

#LF
$(PHYSICSNM):      $(PHYSICSO) $(PHYSICSTMPDO) $(PHYSICSTMP2DO) 
		@echo "Generating symbols file $@..."
		nm -g -p --defined-only $(PHYSICSTMPDO) | awk '{printf("%s\n", $$3)'} > $(PHYSICSNM)
		nm -g -p --defined-only $(PHYSICSTMP2DO) | awk '{printf("%s\n", $$3)'} >> $(PHYSICSNM)
		nm -g -p --defined-only $(PHYSICSO) | awk '{printf("%s\n", $$3)'} >> $(PHYSICSNM)

$(PHYSICSMAP):  $(RLIBMAP) $(MAKEFILEDEP) $(PHYSICSL)
		$(RLIBMAP) -o $(PHYSICSMAP) -l $(PHYSICSLIB) \
		   -d $(PHYSICSLIBDEPM) -c $(PHYSICSL)

all-physics:    $(PHYSICSLIB) $(PHYSICSMAP)

clean-physics:
		@rm -f $(PHYSICSO) $(PHYSICSDO)

clean::         clean-physics clean-pds-physics

#LF
clean-pds-physics:	
		rm -f $(PHYSICSTMPDS) $(PHYSICSTMPDO) $(PHYSICSTMPDH) \
		$(PHYSICSTMPDEP) $(PHYSICSTMP2DS) $(PHYSICSTMP2DO) $(PHYSICSTMP2DH) $(PHYSICSNM)

distclean-physics: clean-physics
		@rm -f $(PHYSICSDEP) $(PHYSICSDS) $(PHYSICSDH) $(PHYSICSLIB) $(PHYSICSMAP)

distclean::     distclean-physics
