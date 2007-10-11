# Module.mk for guibuilder module
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: Valeriy Onuchin, 19/8/2004

MODDIR       := guibuilder
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

GUIBLDDIR    := $(MODDIR)
GUIBLDDIRS   := $(GUIBLDDIR)/src
GUIBLDDIRI   := $(GUIBLDDIR)/inc

##### libGuiBld #####
GUIBLDL      := $(MODDIRI)/LinkDef.h
GUIBLDDS     := $(MODDIRS)/G__GuiBld.cxx
GUIBLDDO     := $(GUIBLDDS:.cxx=.o)
GUIBLDDH     := $(GUIBLDDS:.cxx=.h)

#LF
GUIBLDTMPDS    := $(MODDIRS)/G__GuiBldTmp.cxx
GUIBLDTMPDO    := $(GUIBLDTMPDS:.cxx=.o)
GUIBLDTMPDH    := $(GUIBLDTMPDS:.cxx=.h)
GUIBLDTMP2DS   := $(MODDIRS)/G__GuiBldTmp2.cxx
GUIBLDTMP2DO   := $(GUIBLDTMP2DS:.cxx=.o)
GUIBLDTMP2DH   := $(GUIBLDTMP2DS:.cxx=.h)

GUIBLDH      := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
GUIBLDS      := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
GUIBLDO      := $(GUIBLDS:.cxx=.o)

GUIBLDDEP    := $(GUIBLDO:.o=.d) $(GUIBLDDO:.o=.d)

#LF
GUIBLDTMPDEP  := $(GUIBLDTMPDO:.o=.d)

GUIBLDLIB    := $(LPATH)/libGuiBld.$(SOEXT)
GUIBLDMAP    := $(GUIBLDLIB:.$(SOEXT)=.rootmap)

#LF
GUIBLDNM       := $(GUIBLDLIB:.$(SOEXT)=.nm)

# used in the main Makefile
ALLHDRS     += $(patsubst $(MODDIRI)/%.h,include/%.h,$(GUIBLDH))
ALLLIBS     += $(GUIBLDLIB)
ALLMAPS     += $(GUIBLDMAP)

# include all dependency files
INCLUDEFILES += $(GUIBLDDEP)

##### local rules #####
include/%.h:    $(GUIBLDDIRI)/%.h
		cp $< $@

#LF
$(GUIBLDLIB):   $(GUIBLDO) $(GUIBLDTMPDO) $(GUIBLDTMP2DO) $(GUIBLDDO) $(ORDER_) $(MAINLIBS) $(GUIBLDLIBDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		   "$(SOFLAGS)" libGuiBld.$(SOEXT) $@ "$(GUIBLDO) $(GUIBLDTMPDO) $(GUIBLDTMP2DO) $(GUIBLDDO)" \
		   "$(GUIBLDLIBEXTRA)"

#LF
$(GUIBLDTMPDS):   $(GUIBLDH) $(GUIBLDL) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(GUIBLDH) $(GUIBLDL)

#LF
$(GUIBLDTMP2DS):  $(GUIBLDH) $(GUIBLDL) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(GUIBLDH) $(GUIBLDL)

#LF
$(GUIBLDDS):    $(GUIBLDH) $(GUIBLDL) $(ROOTCINTTMPEXE) $(GUIBLDNM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(GUIBLDNM) -. 3 -c $(GUIBLDH) $(GUIBLDL)

#LF
$(GUIBLDDICTMAP): $(RLIBMAP) $(MAKEFILEDEP) $(GUIBLDL)
		$(RLIBMAP) -o $(GUIBLDDICTMAP) -l $(GUIBLDDICTLIB) \
		-d $(GUIBLDLIB) $(GUIBLDLIBDEPM) -c $(GUIBLDL)
#LF
$(GUIBLDNM):      $(GUIBLDO) $(GUIBLDTMPDO) $(GUIBLDTMP2DO) 
		@echo "Generating symbols file $@..."
		nm -g -p --defined-only $(GUIBLDTMPDO) | awk '{printf("%s\n", $$3)'} > $(GUIBLDNM)
		nm -g -p --defined-only $(GUIBLDTMP2DO) | awk '{printf("%s\n", $$3)'} >> $(GUIBLDNM)
		nm -g -p --defined-only $(GUIBLDO) | awk '{printf("%s\n", $$3)'} >> $(GUIBLDNM)

$(GUIBLDMAP):   $(RLIBMAP) $(MAKEFILEDEP) $(GUIBLDL)
		$(RLIBMAP) -o $(GUIBLDMAP) -l $(GUIBLDLIB) \
		   -d $(GUIBLDLIBDEPM) -c $(GUIBLDL)

all-guibuilder: $(GUIBLDLIB) $(GUIBLDMAP)

clean-guibuilder:
		@rm -f $(GUIBLDO) $(GUIBLDDO)

clean::         clean-guibuilder clean-pds-guibuilder

#LF
clean-pds-guibuilder:	
		rm -f $(GUIBLDTMPDS) $(GUIBLDTMPDO) $(GUIBLDTMPDH) \
		$(GUIBLDTMPDEP) $(GUIBLDTMP2DS) $(GUIBLDTMP2DO) $(GUIBLDTMP2DH) $(GUIBLDNM)

distclean-guibuilder: clean-guibuilder
		@rm -f $(GUIBLDDEP) $(GUIBLDDS) $(GUIBLDDH) $(GUIBLDLIB) $(GUIBLDMAP)

distclean::     distclean-guibuilder
