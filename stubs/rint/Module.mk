# Module.mk for rint module
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: Fons Rademakers, 29/2/2000

MODDIR       := rint
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

RINTDIR      := $(MODDIR)
RINTDIRS     := $(RINTDIR)/src
RINTDIRI     := $(RINTDIR)/inc

##### libRint #####
RINTL        := $(MODDIRI)/LinkDef.h
RINTDS       := $(MODDIRS)/G__Rint.cxx
RINTDO       := $(RINTDS:.cxx=.o)
RINTDH       := $(RINTDS:.cxx=.h)

#LF
RINTTMPDS    := $(MODDIRS)/G__RintTmp.cxx
RINTTMPDO    := $(RINTTMPDS:.cxx=.o)
RINTTMPDH    := $(RINTTMPDS:.cxx=.h)
RINTTMP2DS    := $(MODDIRS)/G__RintTmp2.cxx
RINTTMP2DO    := $(RINTTMP2DS:.cxx=.o)
RINTTMP2DH    := $(RINTTMP2DS:.cxx=.h)

RINTH        := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
RINTS        := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
RINTO        := $(RINTS:.cxx=.o)

RINTDEP      := $(RINTO:.o=.d) $(RINTDO:.o=.d)

#LF
RINTTMPDEP   := $(RINTTMPDO:.o=.d)

RINTLIB      := $(LPATH)/libRint.$(SOEXT)

#LF
RINTMAP      := $(RINTLIB:.$(SOEXT)=.rootmap)
RINTDICTLIB  := $(LPATH)/libRintDict.$(SOEXT)
RINTDICTMAP  := $(RINTDICTLIB:.$(SOEXT)=.rootmap)

#LF
RINTNM       := $(RINTLIB:.$(SOEXT)=.nm)

# used in the main Makefile
ALLHDRS     += $(patsubst $(MODDIRI)/%.h,include/%.h,$(RINTH))
ALLLIBS     += $(RINTLIB)
ALLMAPS     += $(RINTMAP)

# include all dependency files
INCLUDEFILES += $(RINTDEP)

##### local rules #####
include/%.h:    $(RINTDIRI)/%.h
		cp $< $@

#LF
$(RINTLIB):     $(RINTO) $(RINTTMPDO) $(RINTTMP2DO) $(RINTDO) $(ORDER_) $(MAINLIBS)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		"$(SOFLAGS)" libRint.$(SOEXT) $@ "$(RINTO) $(RINTTMPDO) $(RINTTMP2DO) $(RINTDO)"\
		"$(RINTLIBEXTRA)"

#LF
#$(RINTDICTLIB): $(RINTDO) $(ORDER_) $(MAINLIBS)
#		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
#		"$(SOFLAGS)" libRintDict.$(SOEXT) $@ "$(RINTDO) $(RINTTMP2DO)"\
#		"$(RINTDICTLIBEXTRA)"

#LF
$(RINTTMPDS):   $(RINTH) $(RINTL) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(RINTH) $(RINTL)

#LF
$(RINTTMP2DS):  $(RINTH) $(RINTL) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(RINTH) $(RINTL)

#LF
$(RINTDS):      $(RINTH) $(RINTL) $(ROOTCINTTMPEXE) $(RINTNM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(RINTNM) -. 3 -c $(RINTH) $(RINTL)

#LF
$(RINTMAP): $(RLIBMAP) $(MAKEFILEDEP) $(RINTL)
		$(RLIBMAP) -o $(RINTMAP) -l $(RINTLIB) \
		-d $(RINTLIBDEPM) -c $(RINTL)
#LF
$(RINTNM):      $(RINTO) $(RINTTMPDO) $(RINTTMP2DO) 
		@echo "Generating symbols file $@..."
		nm -p --defined-only $(RINTTMPDO) | awk '{printf("%s\n", $$3)'} > $(RINTNM)
		nm -p --defined-only $(RINTTMP2DO) | awk '{printf("%s\n", $$3)'} >> $(RINTNM)
		nm -p --defined-only $(RINTO) | awk '{printf("%s\n", $$3)'} >> $(RINTNM)

all-rint:       $(RINTLIB) $(RINTMAP)

clean-rint:
		@rm -f $(RINTO) $(RINTDO)

clean::         clean-rint clean-pds-rint

#LF
clean-pds-rint:	
		rm -f $(RINTTMPDS) $(RINTTMPDO) $(RINTTMPDH) \
		$(RINTTMPDEP) $(RINTTMP2DS) $(RINTTMP2DO) $(RINTTMP2DH) $(RINTNM)

distclean-rint: clean-rint
		@rm -f $(RINTDEP) $(RINTDS) $(RINTDH) $(RINTLIB) $(RINTMAP)

distclean::     distclean-rint
