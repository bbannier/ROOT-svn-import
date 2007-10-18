# Module.mk for postscript module
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: Fons Rademakers, 29/2/2000

MODDIR       := postscript
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

POSTSCRIPTDIR  := $(MODDIR)
POSTSCRIPTDIRS := $(POSTSCRIPTDIR)/src
POSTSCRIPTDIRI := $(POSTSCRIPTDIR)/inc

##### libTree #####
POSTSCRIPTL  := $(MODDIRI)/LinkDef.h
POSTSCRIPTDS := $(MODDIRS)/G__PostScript.cxx
POSTSCRIPTDO := $(POSTSCRIPTDS:.cxx=.o)
POSTSCRIPTDH := $(POSTSCRIPTDS:.cxx=.h)

#LF
POSTSCRIPTTMPDS    := $(MODDIRS)/G__PostScriptTmp.cxx
POSTSCRIPTTMPDO    := $(POSTSCRIPTTMPDS:.cxx=.o)
POSTSCRIPTTMPDH    := $(POSTSCRIPTTMPDS:.cxx=.h)
POSTSCRIPTTMP2DS   := $(MODDIRS)/G__PostScriptTmp2.cxx
POSTSCRIPTTMP2DO   := $(POSTSCRIPTTMP2DS:.cxx=.o)
POSTSCRIPTTMP2DH   := $(POSTSCRIPTTMP2DS:.cxx=.h)

POSTSCRIPTH  := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
POSTSCRIPTS  := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
POSTSCRIPTO  := $(POSTSCRIPTS:.cxx=.o)

POSTSCRIPTDEP := $(POSTSCRIPTO:.o=.d) $(POSTSCRIPTDO:.o=.d)

#LF
POSTSCRIPTTMPDEP  := $(POSTSCRIPTTMPDO:.o=.d)

POSTSCRIPTLIB := $(LPATH)/libPostscript.$(SOEXT)
POSTSCRIPTMAP := $(POSTSCRIPTLIB:.$(SOEXT)=.rootmap)

#LF
POSTSCRIPTNM       := $(POSTSCRIPTLIB:.$(SOEXT)=.nm)

# used in the main Makefile
ALLHDRS       += $(patsubst $(MODDIRI)/%.h,include/%.h,$(POSTSCRIPTH))
ALLLIBS       += $(POSTSCRIPTLIB)
ALLMAPS       += $(POSTSCRIPTMAP)

# include all dependency files
INCLUDEFILES += $(POSTSCRIPTDEP)

##### local rules #####
include/%.h:    $(POSTSCRIPTDIRI)/%.h
		cp $< $@

#LF
$(POSTSCRIPTLIB):   $(POSTSCRIPTO) $(POSTSCRIPTTMPDO) $(POSTSCRIPTTMP2DO) $(POSTSCRIPTDO) $(ORDER_) $(MAINLIBS) $(POSTSCRIPTLIBDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		   "$(SOFLAGS)" libPostscript.$(SOEXT) $@ "$(POSTSCRIPTO) $(POSTSCRIPTTMPDO) $(POSTSCRIPTTMP2DO) \
			$(POSTSCRIPTDO)" \
			"$(POSTSCRIPTLIBEXTRA)"

#LF
$(POSTSCRIPTTMPDS):   $(POSTSCRIPTH) $(POSTSCRIPTL) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(POSTSCRIPTH) $(POSTSCRIPTL)

#LF
$(POSTSCRIPTTMP2DS):  $(POSTSCRIPTH) $(POSTSCRIPTL) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(POSTSCRIPTH) $(POSTSCRIPTL)

#LF
$(POSTSCRIPTDS):    $(POSTSCRIPTH) $(POSTSCRIPTL) $(ROOTCINTTMPEXE) $(POSTSCRIPTNM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(POSTSCRIPTNM) -. 3 -c $(POSTSCRIPTH) $(POSTSCRIPTL)

#LF
$(POSTSCRIPTNM):      $(POSTSCRIPTO) $(POSTSCRIPTTMPDO) $(POSTSCRIPTTMP2DO) 
		@echo "Generating symbols file $@..."
		nm -p --defined-only $(POSTSCRIPTTMPDO) | awk '{printf("%s\n", $$3)'} > $(POSTSCRIPTNM)
		nm -p --defined-only $(POSTSCRIPTTMP2DO) | awk '{printf("%s\n", $$3)'} >> $(POSTSCRIPTNM)
		nm -p --defined-only $(POSTSCRIPTO) | awk '{printf("%s\n", $$3)'} >> $(POSTSCRIPTNM)

$(POSTSCRIPTMAP): $(RLIBMAP) $(MAKEFILEDEP) $(POSTSCRIPTL)
		$(RLIBMAP) -o $(POSTSCRIPTMAP) -l $(POSTSCRIPTLIB) \
		   -d $(POSTSCRIPTLIBDEPM) -c $(POSTSCRIPTL)

all-postscript: $(POSTSCRIPTLIB) $(POSTSCRIPTMAP)

clean-postscript:
		@rm -f $(POSTSCRIPTO) $(POSTSCRIPTDO)

clean::         clean-postscript clean-pds-postscript

#LF
clean-pds-postscript:	
		rm -f $(POSTSCRIPTTMPDS) $(POSTSCRIPTTMPDO) $(POSTSCRIPTTMPDH) \
		$(POSTSCRIPTTMPDEP) $(POSTSCRIPTTMP2DS) $(POSTSCRIPTTMP2DO) $(POSTSCRIPTTMP2DH) $(POSTSCRIPTNM)

distclean-postscript: clean-postscript
		@rm -f $(POSTSCRIPTDEP) $(POSTSCRIPTDS) $(POSTSCRIPTDH) \
		   $(POSTSCRIPTLIB) $(POSTSCRIPTMAP)

distclean::     distclean-postscript

##### extra rules ######
ifeq ($(ARCH),alphacxx6)
$(POSTSCRIPTDIRS)/TPostScript.o: OPT = $(NOOPT)
endif
