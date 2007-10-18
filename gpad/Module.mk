# Module.mk for gpad module
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: Fons Rademakers, 29/2/2000

MODDIR       := gpad
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

GPADDIR      := $(MODDIR)
GPADDIRS     := $(GPADDIR)/src
GPADDIRI     := $(GPADDIR)/inc

##### libGpad #####
GPADL        := $(MODDIRI)/LinkDef.h
GPADDS       := $(MODDIRS)/G__GPad.cxx
GPADDO       := $(GPADDS:.cxx=.o)
GPADDH       := $(GPADDS:.cxx=.h)

#LF
GPADTMPDS    := $(MODDIRS)/G__GPadTmp.cxx
GPADTMPDO    := $(GPADTMPDS:.cxx=.o)
GPADTMPDH    := $(GPADTMPDS:.cxx=.h)
GPADTMP2DS    := $(MODDIRS)/G__GPadTmp2.cxx
GPADTMP2DO    := $(GPADTMP2DS:.cxx=.o)
GPADTMP2DH    := $(GPADTMP2DS:.cxx=.h)

GPADH        := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
GPADS        := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
GPADO        := $(GPADS:.cxx=.o)

GPADDEP      := $(GPADO:.o=.d) $(GPADDO:.o=.d)

#LF
GPADTMPDEP   := $(GPADTMPDO:.o=.d)

GPADLIB      := $(LPATH)/libGpad.$(SOEXT)

#LF
GPADMAP      := $(GPADLIB:.$(SOEXT)=.rootmap)
GPADDICTLIB  := $(LPATH)/libGpadDict.$(SOEXT)
GPADDICTMAP  := $(GPADDICTLIB:.$(SOEXT)=.rootmap)

#LF
GPADNM       := $(GPADLIB:.$(SOEXT)=.nm)

# used in the main Makefile
ALLHDRS     += $(patsubst $(MODDIRI)/%.h,include/%.h,$(GPADH))
ALLLIBS     += $(GPADLIB)
ALLMAPS     += $(GPADMAP)

# include all dependency files
INCLUDEFILES += $(GPADDEP)

##### local rules #####
include/%.h:    $(GPADDIRI)/%.h
		cp $< $@

#LF
$(GPADLIB):     $(GPADO) $(GPADTMPDO) $(GPADTMP2DO) $(GPADDO) $(ORDER_) $(MAINLIBS) $(GPADLIBDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		"$(SOFLAGS)" libGpad.$(SOEXT) $@ "$(GPADO) $(GPADTMPDO) $(GPADTMP2DO) $(GPADDO)" \
		"$(GPADLIBEXTRA)"

#LF
#$(GPADDICTLIB): $(GPADDO) $(ORDER_) $(MAINLIBS) $(GPADDICTLIBDEP)
#		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
#		"$(SOFLAGS)" libGpadDict.$(SOEXT) $@ "$(GPADDO)" "$(GPADTMP2DO)"\
#		"$(GPADDICTLIBEXTRA)"

#LF
$(GPADTMPDS):   $(GPADH) $(GPADL) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(GPADH) $(GPADL)

#LF
$(GPADTMP2DS):  $(GPADH) $(GPADL) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(GPADH) $(GPADL)

#LF
$(GPADDS):      $(GPADH) $(GPADL) $(ROOTCINTTMPEXE) $(GPADNM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(GPADNM) -. 3 -c $(GPADH) $(GPADL)
#LF
#$(GPADDICTMAP): $(RLIBMAP) $(MAKEFILEDEP) $(GPADL)
#		$(RLIBMAP) -o $(GPADDICTMAP) -l $(GPADDICTLIB) \
#		-d $(GPADLIB) $(GPADLIBDEPM) -c $(GPADL)

#LF
$(GPADNM):      $(GPADO) $(GPADTMPDO) $(GPADTMP2DO)
		@echo "Generating symbols file $@..."
		nm -p --defined-only $(GPADTMPDO) | awk '{printf("%s\n", $$3)'} > $(GPADNM)
		nm -p --defined-only $(GPADTMP2DO) | awk '{printf("%s\n", $$3)'} >> $(GPADNM)
		nm -p --defined-only $(GPADO) | awk '{printf("%s\n", $$3)'} >> $(GPADNM)

$(GPADMAP):     $(RLIBMAP) $(MAKEFILEDEP) $(GPADL)
		$(RLIBMAP) -o $(GPADMAP) -l $(GPADLIB) \
		   -d $(GPADLIBDEPM) -c $(GPADL)

all-gpad:       $(GPADLIB) $(GPADMAP)

clean-gpad:
		@rm -f $(GPADO) $(GPADDO)

clean::         clean-gpad clean-pds-gpad

#LF
clean-pds-gpad:	
		rm -f $(GPADTMPDS) $(GPADTMPDO) $(GPADTMPDH) \
		$(GPADTMPDEP) $(GPADTMP2DS) $(GPADTMP2DO) $(GPADTMP2DH) $(GPADNM)

distclean-gpad: clean-gpad
		@rm -f $(GPADDEP) $(GPADDS) $(GPADDH) $(GPADLIB) $(GPADDICTMAP) 

distclean::     distclean-gpad
