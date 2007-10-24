# Module.mk for tree module
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: Fons Rademakers, 29/2/2000

MODDIR       := tree
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

TREEDIR      := $(MODDIR)
TREEDIRS     := $(TREEDIR)/src
TREEDIRI     := $(TREEDIR)/inc

##### libTree #####
TREEL        := $(MODDIRI)/LinkDef.h
TREEDS       := $(MODDIRS)/G__Tree.cxx
TREEDO       := $(TREEDS:.cxx=.o)
TREEDH       := $(TREEDS:.cxx=.h)

#LF
TREETMPDS    := $(MODDIRS)/G__TreeTmp.cxx
TREETMPDO    := $(TREETMPDS:.cxx=.o)
TREETMPDH    := $(TREETMPDS:.cxx=.h)
TREETMP2DS    := $(MODDIRS)/G__TreeTmp2.cxx
TREETMP2DO    := $(TREETMP2DS:.cxx=.o)
TREETMP2DH    := $(TREETMP2DS:.cxx=.h)

# ManualBase4 only needs to be regenerated (and then changed manually) when
# the dictionary interface changes
TREEL2       := $(MODDIRI)/LinkDef2.h
TREEDS2      := $(MODDIRS)/ManualTree2.cxx
TREEDO2      := $(TREEDS2:.cxx=.o)
TREEDH2      := TTree.h

#LF
#TREETMPDS2    := $(MODDIRS)/ManualTree2Tmp.cxx
#TREETMPDO2    := $(TREETMPDS2:.cxx=.o)
#TREETMPDH2    := TTree.h
#TREETMP2DS2    := $(MODDIRS)/ManualTree2Tmp2.cxx
#TREETMP2DO2    := $(TREETMP2DS2:.cxx=.o)
#TREETMP2DH2    := TTree.h

TREEH        := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
TREES        := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
TREEO        := $(TREES:.cxx=.o)

#TREEO        := $(filter-out $(MODDIRS)/ManualTree%,$(TREEO))

TREEDEP      := $(TREEO:.o=.d) $(TREEDO:.o=.d)

#LF
TREETMPDEP   := $(TREETMPDO:.o=.d)

TREELIB      := $(LPATH)/libTree.$(SOEXT)

#LF
TREEMAP      := $(TREELIB:.$(SOEXT)=.rootmap)
TREEDICTLIB   := $(LPATH)/libTreeDict.$(SOEXT)
TREEDICTMAP   := $(TREEDICTLIB:.$(SOEXT)=.rootmap)
TREENM        := $(TREELIB:.$(SOEXT)=.nm)

# used in the main Makefile
ALLHDRS     += $(patsubst $(MODDIRI)/%.h,include/%.h,$(TREEH))
ALLLIBS     += $(TREELIB)
ALLMAPS     += $(TREEMAP)

# include all dependency files
INCLUDEFILES += $(TREEDEP)

##### local rules #####
include/%.h:    $(TREEDIRI)/%.h
		cp $< $@

#LF
$(TREELIB):      $(TREEO) $(TREETMPDO) $(TREETMP2DO) $(TREEDO) $(ORDER_) $(MAINLIBS) $(TREELIBDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		"$(SOFLAGS)" libTree.$(SOEXT) $@ "$(TREEO) $(TREETMPDO) $(TREETMP2DO) $(TREEDO)"\
		"$(TREELIBEXTRA)"
#LF
#$(TREEDICTLIB):  $(TREEDO) $(ORDER_) $(MAINLIBS) $(TREEDICTLIBDEP) $(TREETMP2DO)
#		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
#		"$(SOFLAGS)" libTreeDict.$(SOEXT) $@ "$(TREEDO) $(TREETMP2DO)"\
#		"$(TREEDICTLIBEXTRA)"

#LF
$(TREETMPDS):   $(TREEH) $(TREEL) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(TREEH) $(TREEL)
#LF
$(TREETMP2DS):  $(TREEH) $(TREEL) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(TREEH) $(TREEL)
#LF
$(TREEDS):      $(TREEH) $(TREEL) $(ROOTCINTTMPEXE) $(TREENM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(TREENM) -. 3 -c $(TREEH) $(TREEL)

# pre-requisites intentionally not specified... should be called only
# on demand after deleting the file
$(TREEDS2):
		@echo "Generating dictionary $@..."
		$(ROOTCINTTMP) -f $@ -c $(TREEDH2) $(TREEL2)

#LF
#$(TREETMPDS2):  #$(TREEDH2) $(TREEL2) $(ROOTCINTTMPEXE)
#		@echo "Generating first dictionary $@..."
#		$(ROOTCINTTMP) -f $@ -. 1 -c $(TREEDH2) $(TREEL2)
#LF
#$(TREETMP2DS2): #$(TREEDH2) $(TREEL2) $(ROOTCINTTMPEXE)
#		@echo "Generating second dictionary $@..."
#		$(ROOTCINTTMP) -f $@ -. 2 -c $(TREEDH2) $(TREEL2)
#LF
#$(TREEDS2):     #$(TREEDH2) $(TREEL2) $(ROOTCINTTMPEXE) $(TREENM)
#		@echo "Generating third dictionary $@..."
#		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(TREENM) -. 3 -c $(TREEDH2) $(TREEL2)

#LF
$(TREEMAP):  $(RLIBMAP) $(MAKEFILEDEP) $(TREEL)
		$(RLIBMAP) -o $(TREEMAP) -l $(TREELIB) \
		-d $(TREELIBDEPM) -c $(TREEL)

#LF
$(TREENM):      $(TREEO) $(TREETMPDO) $(TREETMP2DO) #$(TREETMP2DO2) 
		@echo "Generating symbols file $@..."
		nm -p --defined-only $(TREETMPDO) | awk '{printf("%s\n", $$3)'} > $(TREENM)
		nm -p --defined-only $(TREETMP2DO) | awk '{printf("%s\n", $$3)'} >> $(TREENM)
		#nm -p --defined-only $(TREETMPDO2) | awk '{printf("%s\n", $$3)'} >> $(TREENM)
		nm -p --defined-only $(TREEO) | awk '{printf("%s\n", $$3)'} >> $(TREENM)

all-tree:       $(TREELIB) $(TREEMAP)

clean-tree:
		@rm -f $(TREEO) $(TREEDO)

clean::         clean-tree clean-pds-tree

#LF
clean-pds-tree:	
		rm -f $(TREETMPDS) $(TREETMPDO) $(TREETMPDH) \
		$(TREETMPDEP) $(TREETMP2DS) $(TREETMP2DO) $(TREETMP2DH) $(TREENM)

distclean-tree: clean-tree
		@rm -f $(TREEDEP) $(TREEDS) $(TREEDH) $(TREELIB) $(TREEMAP)

distclean::     distclean-tree
