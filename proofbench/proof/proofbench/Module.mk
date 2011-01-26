# Module.mk for the bench module
# Copyright (c) 2005 Rene Brun and Fons Rademakers
#
# Author: 

MODNAME      := proofbench
MODDIR       := proof/$(MODNAME)
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

BENCHDIR    := $(MODDIR)
BENCHDIRS   := $(BENCHDIR)/src
BENCHDIRI   := $(BENCHDIR)/inc

##### libProofBench #####
BENCHL      := $(MODDIRI)/LinkDef.h
BENCHDS     := $(MODDIRS)/G__Bench.cxx
BENCHDO     := $(BENCHDS:.cxx=.o)
BENCHDH     := $(BENCHDS:.cxx=.h)

BENCHH      := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
BENCHH      := $(filter-out $(MODDIRI)/TSel%,$(BENCHH))

BENCHS      := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
BENCHS      := $(filter-out $(MODDIRS)/TSel%,$(BENCHS))

##### ProofBenchSel PAR file #####
BENCHDIRP   := $(BENCHDIRS)/ProofBenchSel
BENCHDIRINF := $(BENCHDIRP)/PROOF-INF
BENCHPH     := test/Event.h $(MODDIRI)/TProofBenchTypes.h
BENCHPS     := test/Event.cxx
BENCHPH     += $(wildcard $(MODDIRI)/TSel*.h)
BENCHPS     += $(wildcard $(MODDIRS)/TSel*.cxx)

BENCHO      := $(BENCHS:.cxx=.o)

BENCHDEP    := $(BENCHO:.o=.d) $(BENCHDO:.o=.d)

BENCHLIB    := $(LPATH)/libProofBench.$(SOEXT)
BENCHMAP    := $(BENCHLIB:.$(SOEXT)=.rootmap)
BENCHPAR    := $(MODDIRS)/ProofBenchSel.par

# used in the main Makefile
ALLHDRS      += $(patsubst $(MODDIRI)/%.h,include/%.h,$(BENCHH))
ALLLIBS      += $(BENCHLIB) $(BENCHPAR)
ALLMAPS      += $(BENCHMAP)

# include all dependency files
INCLUDEFILES += $(BENCHDEP)

##### local rules #####
.PHONY:         all-$(MODNAME) clean-$(MODNAME) distclean-$(MODNAME)

include/%.h:    $(BENCHDIRI)/%.h
		cp $< $@

$(BENCHLIB):   $(BENCHO) $(BENCHDO) $(ORDER_) $(MAINLIBS) \
                $(BENCHLIBDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		   "$(SOFLAGS)" libProofBench.$(SOEXT) $@ \
		   "$(BENCHO) $(BENCHDO)"

$(BENCHDS):    $(BENCHH) $(BENCHL) $(ROOTCINTTMPDEP)
		@echo "Generating dictionary $@..."
		$(ROOTCINTTMP) -f $@ -c $(BENCHH) $(BENCHL)

$(BENCHMAP):   $(RLIBMAP) $(MAKEFILEDEP) $(BENCHL)
		$(RLIBMAP) -o $(BENCHMAP) -l $(BENCHLIB) \
		   -d $(PROOFBENCHLIBDEPM) -c $(BENCHL)

$(BENCHPAR):   $(BENCHPH) $(BENCHPS)
		@echo "Generating PAR file $@..."
		@(if test -d $(BENCHDIRP); then\
		   rm -fr $(BENCHDIRP); \
		fi;\
		mkdir -p $(BENCHDIRINF); \
		for f in $(BENCHPH) $(BENCHPS); do \
		   cp -rp $$f $(BENCHDIRP); \
		done; \
		echo "#include \"TROOT.h\"" > $(BENCHDIRINF)/SETUP.C ; \
		echo "Int_t SETUP() {" >> $(BENCHDIRINF)/SETUP.C ; \
		for f in $(BENCHPS); do \
		   b=`basename $$f`; \
		   echo "   gROOT->ProcessLine(\".L $$b+\");" >> $(BENCHDIRINF)/SETUP.C ; \
		done; \
		echo "   return 0;" >> $(BENCHDIRINF)/SETUP.C ; \
		echo "}" >> $(BENCHDIRINF)/SETUP.C ; \
		builddir=$(PWD); \
		cd $(BENCHDIRS); \
		par=`basename $(BENCHPAR)`;\
		pardir=`basename $(BENCHDIRP)`;\
		tar czvf $$par $$pardir; \
		cd $$builddir; \
		rm -fr $(BENCHDIRP))

all-$(MODNAME): $(BENCHLIB) $(BENCHMAP) $(BENCHPAR)

clean-$(MODNAME):
		@rm -f $(BENCHO) $(BENCHDO)

clean::         clean-$(MODNAME)

distclean-$(MODNAME): clean-$(MODNAME)
		@rm -f $(BENCHDEP) $(BENCHDS) $(BENCHDH) $(BENCHLIB) $(BENCHMAP) $(BENCHPAR); \
		if test -d $(BENCHDIRP); then\
		   rm -fr $(BENCHDIRP); \
		fi

distclean::     distclean-$(MODNAME)

##### extra rules ######
$(BENCHO) $(BENCHDO): CXXFLAGS += -I.
