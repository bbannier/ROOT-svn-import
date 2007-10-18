# Module.mk for vmc module
# Copyright (c) 2002 Rene Brun and Fons Rademakers
#
# Author: Fons Rademakers, 12/4/2002

MODDIR       := vmc
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

VMCDIR       := $(MODDIR)
VMCDIRS      := $(VMCDIR)/src
VMCDIRI      := $(VMCDIR)/inc

##### libVMC #####
VMCL         := $(MODDIRI)/LinkDef.h
VMCDS        := $(MODDIRS)/G__VMC.cxx
VMCDO        := $(VMCDS:.cxx=.o)
VMCDH        := $(VMCDS:.cxx=.h)

#LF
VMCTMPDS    := $(MODDIRS)/G__VMCTmp.cxx
VMCTMPDO    := $(VMCTMPDS:.cxx=.o)
VMCTMPDH    := $(VMCTMPDS:.cxx=.h)
VMCTMP2DS   := $(MODDIRS)/G__VMCTmp2.cxx
VMCTMP2DO   := $(VMCTMP2DS:.cxx=.o)
VMCTMP2DH   := $(VMCTMP2DS:.cxx=.h)

VMCH1        := $(wildcard $(MODDIRI)/T*.h)
VMCH         := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
VMCS         := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
VMCO         := $(VMCS:.cxx=.o)

VMCDEP       := $(VMCO:.o=.d) $(VMCDO:.o=.d)

#LF
VMCTMPDEP  := $(VMCTMPDO:.o=.d)

VMCLIB       := $(LPATH)/libVMC.$(SOEXT)
VMCMAP       := $(VMCLIB:.$(SOEXT)=.rootmap)

#LF
VMCNM       := $(VMCLIB:.$(SOEXT)=.nm)

# used in the main Makefile
ALLHDRS     += $(patsubst $(MODDIRI)/%.h,include/%.h,$(VMCH))
ALLLIBS     += $(VMCLIB)
ALLMAPS     += $(VMCMAP)

# include all dependency files
INCLUDEFILES += $(VMCDEP)

##### local rules #####
include/%.h:    $(VMCDIRI)/%.h
		cp $< $@

#LF
$(VMCLIB):   $(VMCO) $(VMCTMPDO) $(VMCTMP2DO) $(VMCDO) $(ORDER_) $(MAINLIBS) $(VMCLIBDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		   "$(SOFLAGS)" libVMC.$(SOEXT) $@ "$(VMCO) $(VMCTMPDO) $(VMCTMP2DO) $(VMCDO)" \
		   "$(VMCLIBEXTRA) $(VMCLIBDIR) $(VMCCLILIB)"

#LF
$(VMCTMPDS):   $(VMCH1) $(VMCL) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(VMCH1) $(VMCL)

#LF
$(VMCTMP2DS):  $(VMCH1) $(VMCL) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(VMCH1) $(VMCL)

#LF
$(VMCDS):    $(VMCH1) $(VMCL) $(ROOTCINTTMPEXE) $(VMCNM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(VMCNM) -. 3 -c $(VMCH1) $(VMCL)

#LF
$(VMCDICTMAP): $(RLIBMAP) $(MAKEFILEDEP) $(VMCL)
		$(RLIBMAP) -o $(VMCDICTMAP) -l $(VMCDICTLIB) \
		-d $(VMCLIB) $(VMCLIBDEPM) -c $(VMCL)
#LF
$(VMCNM):      $(VMCO) $(VMCTMPDO) $(VMCTMP2DO) 
		@echo "Generating symbols file $@..."
		nm -p --defined-only $(VMCTMPDO) | awk '{printf("%s\n", $$3)'} > $(VMCNM)
		nm -p --defined-only $(VMCTMP2DO) | awk '{printf("%s\n", $$3)'} >> $(VMCNM)
		nm -p --defined-only $(VMCO) | awk '{printf("%s\n", $$3)'} >> $(VMCNM)

$(VMCMAP):      $(RLIBMAP) $(MAKEFILEDEP) $(VMCL)
		$(RLIBMAP) -o $(VMCMAP) -l $(VMCLIB) \
		   -d $(VMCLIBDEPM) -c $(VMCL)

all-vmc:        $(VMCLIB) $(VMCMAP)

clean-vmc:
		@rm -f $(VMCO) $(VMCDO)

clean::         clean-vmc clean-pds-vmc

#LF
clean-pds-vmc:	
		rm -f $(VMCTMPDS) $(VMCTMPDO) $(VMCTMPDH) \
		$(VMCTMPDEP) $(VMCTMP2DS) $(VMCTMP2DO) $(VMCTMP2DH) $(VMCNM)

distclean-vmc:   clean-vmc
		@rm -f $(VMCDEP) $(VMCDS) $(VMCDH) $(VMCLIB) $(VMCMAP)

distclean::     distclean-vmc
