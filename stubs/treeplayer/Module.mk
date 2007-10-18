# Module.mk for treeplayer module
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: Fons Rademakers, 29/2/2000

MODDIR       := treeplayer
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

TREEPLAYERDIR  := $(MODDIR)
TREEPLAYERDIRS := $(TREEPLAYERDIR)/src
TREEPLAYERDIRI := $(TREEPLAYERDIR)/inc

##### libTreePlayer #####
TREEPLAYERL  := $(MODDIRI)/LinkDef.h
TREEPLAYERDS := $(MODDIRS)/G__TreePlayer.cxx
TREEPLAYERDO := $(TREEPLAYERDS:.cxx=.o)
TREEPLAYERDH := $(TREEPLAYERDS:.cxx=.h)

#LF
TREEPLAYERTMPDS    := $(MODDIRS)/G__TreePlayerTmp.cxx
TREEPLAYERTMPDO    := $(TREEPLAYERTMPDS:.cxx=.o)
TREEPLAYERTMPDH    := $(TREEPLAYERTMPDS:.cxx=.h)
TREEPLAYERTMP2DS    := $(MODDIRS)/G__TreePlayerTmp2.cxx
TREEPLAYERTMP2DO    := $(TREEPLAYERTMP2DS:.cxx=.o)
TREEPLAYERTMP2DH    := $(TREEPLAYERTMP2DS:.cxx=.h)

TREEPLAYERH  := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
TREEPLAYERS  := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
TREEPLAYERO  := $(TREEPLAYERS:.cxx=.o)

TREEPLAYERDEP := $(TREEPLAYERO:.o=.d) $(TREEPLAYERDO:.o=.d)
#LF
TREEPLAYERTMPDEP   := $(TREEPLAYERTMPDO:.o=.d)

TREEPLAYERLIB := $(LPATH)/libTreePlayer.$(SOEXT)

#LF
TREEPLAYERMAP := $(TREEPLAYERLIB:.$(SOEXT)=.rootmap)
TREEPLAYERDICTLIB  := $(LPATH)/libTreePlayerDict.$(SOEXT)
TREEPLAYERDICTMAP  := $(TREEPLAYERDICTLIB:.$(SOEXT)=.rootmap)

#LF
TREEPLAYERNM       := $(TREEPLAYERLIB:.$(SOEXT)=.nm)

# used in the main Makefile
ALLHDRS       += $(patsubst $(MODDIRI)/%.h,include/%.h,$(TREEPLAYERH))
ALLLIBS       += $(TREEPLAYERLIB)
ALLMAPS       += $(TREEPLAYERMAP)

# include all dependency files
INCLUDEFILES += $(TREEPLAYERDEP)

##### local rules #####
include/%.h:    $(TREEPLAYERDIRI)/%.h
		cp $< $@

#LF
$(TREEPLAYERLIB):     $(TREEPLAYERO) $(TREEPLAYERTMPDO) $(TREEPLAYERTMP2DO) $(TREEPLAYERDO) $(ORDER_) $(MAINLIBS)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		"$(SOFLAGS)" libTreePlayer.$(SOEXT) $@ "$(TREEPLAYERO) $(TREEPLAYERTMPDO) $(TREEPLAYERTMP2DO) $(TREEPLAYERDO)" \
		"$(TREEPLAYERLIBEXTRA)"

#LF
#$(TREEPLAYERDICTLIB): $(TREEPLAYERDO) $(ORDER_) $(MAINLIBS)
#		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
#		"$(SOFLAGS)" libTreePlayerDict.$(SOEXT) $@ "$(TREEPLAYERDO) $(TREEPLAYERTMP2DO)"\
#		"$(TREEPLAYERDICTLIBEXTRA)"

#LF
$(TREEPLAYERTMPDS):   $(TREEPLAYERH) $(TREEPLAYERL) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(TREEPLAYERH) $(TREEPLAYERL)

#LF
$(TREEPLAYERTMP2DS):  $(TREEPLAYERH) $(TREEPLAYERL) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(TREEPLAYERH) $(TREEPLAYERL)

#LF
$(TREEPLAYERDS):      $(TREEPLAYERH) $(TREEPLAYERL) $(ROOTCINTTMPEXE) $(TREEPLAYERNM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(TREEPLAYERNM) -. 3 -c $(TREEPLAYERH) $(TREEPLAYERL)
#LF
$(TREEPLAYERMAP): $(RLIBMAP) $(MAKEFILEDEP) $(TREEPLAYERL)
		$(RLIBMAP) -o $(TREEPLAYERMAP) -l $(TREEPLAYERLIB) \
		-d $(TREEPLAYERLIBDEPM) -c $(TREEPLAYERL)
#LF
$(TREEPLAYERNM):      $(TREEPLAYERO) $(TREEPLAYERTMPDO) $(TREEPLAYERTMP2DO)
		@echo "Generating symbols file $@..."
		nm -p --defined-only $(TREEPLAYERTMPDO) | awk '{printf("%s\n", $$3)'} > $(TREEPLAYERNM)
		nm -p --defined-only $(TREEPLAYERTMP2DO) | awk '{printf("%s\n", $$3)'} >> $(TREEPLAYERNM)
		nm -p --defined-only $(TREEPLAYERO) | awk '{printf("%s\n", $$3)'} >> $(TREEPLAYERNM)

all-treeplayer: $(TREEPLAYERLIB) $(TREEPLAYERMAP)

clean-treeplayer:
		@rm -f $(TREEPLAYERO) $(TREEPLAYERDO)

clean::         clean-treeplayer clean-pds-treeplayer

#LF
clean-pds-treeplayer:	
		rm -f $(TREEPLAYERTMPDS) $(TREEPLAYERTMPDO) $(TREEPLAYERTMPDH) \
		$(TREEPLAYERTMPDEP) $(TREEPLAYERTMP2DS) $(TREEPLAYERTMP2DO) $(TREEPLAYERTMP2DH) $(TREEPLAYERNM)

distclean-treeplayer: clean-treeplayer
		@rm -f $(TREEPLAYERDEP) $(TREEPLAYERDS) $(TREEPLAYERDH) \
		   $(TREEPLAYERLIB) $(TREEPLAYERMAP)

distclean::     distclean-treeplayer

##### extra rules ######
ifeq ($(PLATFORM),macosx)
ifeq ($(GCC_VERS_FULL),gcc-4.0.1)
$(TREEPLAYERDIRS)/TTreeFormula.o: OPT = $(NOOPT)
endif
ifeq ($(ICC_MAJOR),10)
$(TREEPLAYERDIRS)/TTreeFormula.o: OPT = $(NOOPT)
endif
endif
