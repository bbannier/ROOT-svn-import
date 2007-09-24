# Module.mk for proof module
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: Fons Rademakers, 29/2/2000

MODDIR       := proof
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

PROOFDIR     := $(MODDIR)
PROOFDIRS    := $(PROOFDIR)/src
PROOFDIRI    := $(PROOFDIR)/inc

##### libProof #####
PROOFL       := $(MODDIRI)/LinkDef.h
PROOFDS      := $(MODDIRS)/G__Proof.cxx
PROOFDO      := $(PROOFDS:.cxx=.o)
PROOFDH      := $(PROOFDS:.cxx=.h)

#LF
PROOFTMPDS    := $(MODDIRS)/G__ProofTmp.cxx
PROOFTMPDO    := $(PROOFTMPDS:.cxx=.o)
PROOFTMPDH    := $(PROOFTMPDS:.cxx=.h)
PROOFTMP2DS   := $(MODDIRS)/G__ProofTmp2.cxx
PROOFTMP2DO   := $(PROOFTMP2DS:.cxx=.o)
PROOFTMP2DH   := $(PROOFTMP2DS:.cxx=.h)

PROOFH       := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
PROOFS       := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
PROOFO       := $(PROOFS:.cxx=.o)

PROOFDEP     := $(PROOFO:.o=.d) $(PROOFDO:.o=.d)

#LF
PROOFTMPDEP  := $(PROOFTMPDO:.o=.d)

PROOFLIB     := $(LPATH)/libProof.$(SOEXT)
PROOFMAP     := $(PROOFLIB:.$(SOEXT)=.rootmap)

#LF
PROOFNM       := $(PROOFLIB:.$(SOEXT)=.nm)

# used in the main Makefile
ALLHDRS     += $(patsubst $(MODDIRI)/%.h,include/%.h,$(PROOFH))
ALLLIBS     += $(PROOFLIB)
ALLMAPS     += $(PROOFMAP)

# include all dependency files
INCLUDEFILES += $(PROOFDEP)

##### local rules #####
include/%.h:    $(PROOFDIRI)/%.h
		cp $< $@

#LF
$(PROOFLIB):   $(PROOFO) $(PROOFTMPDO) $(PROOFTMP2DO) $(PROOFDO) $(ORDER_) $(MAINLIBS) $(PROOFLIBDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		   "$(SOFLAGS)" libProof.$(SOEXT) $@ "$(PROOFO) $(PROOFTMPDO) $(PROOFTMP2DO) $(PROOFDO)" \
		   "$(PROOFLIBEXTRA)"

#LF
$(PROOFTMPDS):   $(PROOFH) $(PROOFL) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(PROOFH) $(PROOFL)

#LF
$(PROOFTMP2DS):  $(PROOFH) $(PROOFL) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(PROOFH) $(PROOFL)

#LF
$(PROOFDS):    $(PROOFH) $(PROOFL) $(ROOTCINTTMPEXE) $(PROOFNM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(PROOFNM) -. 3 -c $(PROOFH) $(PROOFL)

#LF
$(PROOFDICTMAP): $(RLIBMAP) $(MAKEFILEDEP) $(PROOFL)
		$(RLIBMAP) -o $(PROOFDICTMAP) -l $(PROOFDICTLIB) \
		-d $(PROOFLIB) $(PROOFLIBDEPM) -c $(PROOFL)
#LF
$(PROOFNM):      $(PROOFO) $(PROOFTMPDO) $(PROOFTMP2DO) 
		@echo "Generating symbols file $@..."
		nm -g -p --defined-only $(PROOFTMPDO) | awk '{printf("%s\n", $$3)'} > $(PROOFNM)
		nm -g -p --defined-only $(PROOFTMP2DO) | awk '{printf("%s\n", $$3)'} >> $(PROOFNM)
		nm -g -p --defined-only $(PROOFO) | awk '{printf("%s\n", $$3)'} >> $(PROOFNM)

$(PROOFMAP):    $(RLIBMAP) $(MAKEFILEDEP) $(PROOFL)
		$(RLIBMAP) -o $(PROOFMAP) -l $(PROOFLIB) \
		   -d $(PROOFLIBDEPM) -c $(PROOFL)

all-proof:      $(PROOFLIB) $(PROOFMAP)

clean-proof:
		@rm -f $(PROOFO) $(PROOFDO)

clean::         clean-proof clean-pds-proof

#LF
clean-pds-proof:	
		rm -f $(PROOFTMPDS) $(PROOFTMPDO) $(PROOFTMPDH) \
		$(PROOFTMPDEP) $(PROOFTMP2DS) $(PROOFTMP2DO) $(PROOFTMP2DH) $(PROOFNM)

distclean-proof: clean-proof
		@rm -f $(PROOFDEP) $(PROOFDS) $(PROOFDH) $(PROOFLIB) $(PROOFMAP)

distclean::     distclean-proof
