# Module.mk for cont module
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: Fons Rademakers, 29/2/2000

MODDIR       := cont
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

CONTDIR      := $(MODDIR)
CONTDIRS     := $(CONTDIR)/src
CONTDIRI     := $(CONTDIR)/inc

##### libCont (part of libCore) #####
CONTL        := $(MODDIRI)/LinkDef.h
CONTDS       := $(MODDIRS)/G__Cont.cxx
CONTDO       := $(CONTDS:.cxx=.o)
CONTDH       := $(CONTDS:.cxx=.h)

#LF
CONTTMPDS    := $(MODDIRS)/G__ContTmp.cxx
CONTTMPDO    := $(CONTTMPDS:.cxx=.o)
CONTTMPDH    := $(CONTTMPDS:.cxx=.h)
CONTTMP2DS    := $(MODDIRS)/G__ContTmp2.cxx
CONTTMP2DO    := $(CONTTMP2DS:.cxx=.o)
CONTTMP2DH    := $(CONTTMP2DS:.cxx=.h)

CONTH        := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
CONTS        := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
CONTO        := $(CONTS:.cxx=.o)

CONTDEP      := $(CONTO:.o=.d) $(CONTDO:.o=.d)

#LF
CONTTMPDEP   := $(CONTTMPDO:.o=.d)

#LF
CONTLIB      := $(MODDIRS)/libCont.$(SOEXT)
#CONTDICTLIB  := $(MODDIRS)/libContDict.o
CONTNM       := $(CONTLIB:.$(SOEXT)=.nm)

# used in the main Makefile
ALLHDRS     += $(patsubst $(MODDIRI)/%.h,include/%.h,$(CONTH))

# include all dependency files
INCLUDEFILES += $(CONTDEP)

##### local rules #####
include/%.h:    $(CONTDIRI)/%.h
		cp $< $@

#LF
$(CONTTMPDS):   $(CONTH) $(CONTL) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(CONTH) $(CONTL)

#LF
$(CONTTMP2DS):  $(CONTH) $(CONTL) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(CONTH) $(CONTL)

$(CONTDS):      $(CONTH) $(CONTL) $(ROOTCINTTMPEXE) $(CONTNM)
		@echo "Generating dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(CONTNM) -. 3 -c $(CONTH) $(CONTL)

#LF
$(CONTNM):      $(CONTO) $(CONTTMPDO) $(CONTTMP2DO)
		@echo "Generating symbols file $@..."
		nm -g -p --defined-only $(CONTTMPDO) | awk '{printf("%s\n", $$3)'} > $(CONTNM)
		nm -g -p --defined-only $(CONTTMP2DO) | awk '{printf("%s\n", $$3)'} >> $(CONTNM)
		nm -g -p --defined-only $(CONTO) | awk '{printf("%s\n", $$3)'} >> $(CONTNM)

all-cont:       $(CONTO) $(CONTDO)

clean-cont:
		@rm -f $(CONTO) $(CONTDO)

clean::         clean-cont

distclean-cont: clean-cont
		@rm -f $(CONTDEP) $(CONTDS) $(CONTDH)

distclean::     distclean-cont
