# Module.mk for eg module
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: Fons Rademakers, 29/2/2000

MODDIR       := eg
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

EGDIR        := $(MODDIR)
EGDIRS       := $(EGDIR)/src
EGDIRI       := $(EGDIR)/inc

##### libEG #####
EGL          := $(MODDIRI)/LinkDef.h
EGDS         := $(MODDIRS)/G__EG.cxx
EGDO         := $(EGDS:.cxx=.o)
EGDH         := $(EGDS:.cxx=.h)

#LF
EGTMPDS    := $(MODDIRS)/G__EGTmp.cxx
EGTMPDO    := $(EGTMPDS:.cxx=.o)
EGTMPDH    := $(EGTMPDS:.cxx=.h)
EGTMP2DS   := $(MODDIRS)/G__EGTmp2.cxx
EGTMP2DO   := $(EGTMP2DS:.cxx=.o)
EGTMP2DH   := $(EGTMP2DS:.cxx=.h)

EGH1         := $(wildcard $(MODDIRI)/T*.h)
EGH          := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
EGS          := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
EGO          := $(EGS:.cxx=.o)

EGDEP        := $(EGO:.o=.d) $(EGDO:.o=.d)

#LF
EGTMPDEP  := $(EGTMPDO:.o=.d)

EGLIB        := $(LPATH)/libEG.$(SOEXT)
EGMAP        := $(EGLIB:.$(SOEXT)=.rootmap)

#LF
EGNM       := $(EGLIB:.$(SOEXT)=.nm)

# used in the main Makefile
ALLHDRS     += $(patsubst $(MODDIRI)/%.h,include/%.h,$(EGH))
ALLLIBS     += $(EGLIB)
ALLMAPS     += $(EGMAP)

# include all dependency files
INCLUDEFILES += $(EGDEP)

##### local rules #####
include/%.h:    $(EGDIRI)/%.h
		cp $< $@

#LF
$(EGLIB):   $(EGO) $(EGTMPDO) $(EGTMP2DO) $(EGDO) $(ORDER_) $(MAINLIBS) $(EGLIBDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		   "$(SOFLAGS)" libEG.$(SOEXT) $@ "$(EGO) $(EGTMPDO) $(EGTMP2DO) $(EGDO)" \
		   "$(EGLIBEXTRA) $(EGLIBDIR) $(EGCLILIB)"

#LF
$(EGTMPDS):   $(EGH1) $(EGL) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(EGH1) $(EGL)

#LF
$(EGTMP2DS):  $(EGH1) $(EGL) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(EGH1) $(EGL)

#LF
$(EGDS):    $(EGH1) $(EGL) $(ROOTCINTTMPEXE) $(EGNM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(EGNM) -. 3 -c $(EGH1) $(EGL)

#LF
$(EGNM):      $(EGO) $(EGTMPDO) $(EGTMP2DO) 
		@echo "Generating symbols file $@..."
		nm -p --defined-only $(EGTMPDO) | awk '{printf("%s\n", $$3)'} > $(EGNM)
		nm -p --defined-only $(EGTMP2DO) | awk '{printf("%s\n", $$3)'} >> $(EGNM)
		nm -p --defined-only $(EGO) | awk '{printf("%s\n", $$3)'} >> $(EGNM)

$(EGMAP):       $(RLIBMAP) $(MAKEFILEDEP) $(EGL)
		$(RLIBMAP) -o $(EGMAP) -l $(EGLIB) \
		   -d $(EGLIBDEPM) -c $(EGL)

all-eg:         $(EGLIB) $(EGMAP)

clean-eg:
		@rm -f $(EGO) $(EGDO)

clean::         clean-eg clean-pds-eg

#LF
clean-pds-eg:	
		rm -f $(EGTMPDS) $(EGTMPDO) $(EGTMPDH) \
		$(EGTMPDEP) $(EGTMP2DS) $(EGTMP2DO) $(EGTMP2DH) $(EGNM)

distclean-eg:   clean-eg
		@rm -f $(EGDEP) $(EGDS) $(EGDH) $(EGLIB) $(EGMAP)

distclean::     distclean-eg
