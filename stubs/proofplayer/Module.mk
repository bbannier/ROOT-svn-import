# Module.mk for proofplayer module
# Copyright (c) 2007 Rene Brun and Fons Rademakers
#
# Author: Fons Rademakers, 16/3/2007

MODDIR       := proofplayer
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

PROOFPLAYERDIR  := $(MODDIR)
PROOFPLAYERDIRS := $(PROOFPLAYERDIR)/src
PROOFPLAYERDIRI := $(PROOFPLAYERDIR)/inc

##### libProofPlayer #####
PROOFPLAYERL  := $(MODDIRI)/LinkDef.h
PROOFPLAYERDS := $(MODDIRS)/G__ProofPlayer.cxx
PROOFPLAYERDO := $(PROOFPLAYERDS:.cxx=.o)
PROOFPLAYERDH := $(PROOFPLAYERDS:.cxx=.h)

#LF
PROOFPLAYERTMPDS    := $(MODDIRS)/G__ProofPlayerTmp.cxx
PROOFPLAYERTMPDO    := $(PROOFPLAYERTMPDS:.cxx=.o)
PROOFPLAYERTMPDH    := $(PROOFPLAYERTMPDS:.cxx=.h)
PROOFPLAYERTMP2DS   := $(MODDIRS)/G__ProofPlayerTmp2.cxx
PROOFPLAYERTMP2DO   := $(PROOFPLAYERTMP2DS:.cxx=.o)
PROOFPLAYERTMP2DH   := $(PROOFPLAYERTMP2DS:.cxx=.h)

PROOFPLAYERH  := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
PROOFPLAYERH  := $(filter-out $(MODDIRI)/TProofDraw%,$(PROOFPLAYERH))
PROOFPLAYERS  := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
PROOFPLAYERS  := $(filter-out $(MODDIRS)/TProofDraw%,$(PROOFPLAYERS))
PROOFPLAYERO  := $(PROOFPLAYERS:.cxx=.o)

PROOFPLAYERDEP := $(PROOFPLAYERO:.o=.d) $(PROOFPLAYERDO:.o=.d)

#LF
PROOFPLAYERTMPDEP  := $(PROOFPLAYERTMPDO:.o=.d)

PROOFPLAYERLIB := $(LPATH)/libProofPlayer.$(SOEXT)
PROOFPLAYERMAP := $(PROOFPLAYERLIB:.$(SOEXT)=.rootmap)

#LF
PROOFPLAYERNM       := $(PROOFPLAYERLIB:.$(SOEXT)=.nm)

##### libProofDraw #####
PROOFDRAWL   := $(MODDIRI)/LinkDefDraw.h
PROOFDRAWDS  := $(MODDIRS)/G__ProofDraw.cxx
PROOFDRAWDO  := $(PROOFDRAWDS:.cxx=.o)
PROOFDRAWDH  := $(PROOFDRAWDS:.cxx=.h)

#LF
PROOFDRAWTMPDS    := $(MODDIRS)/G__ProofDrawTmp.cxx
PROOFDRAWTMPDO    := $(PROOFDRAWTMPDS:.cxx=.o)
PROOFDRAWTMPDH    := $(PROOFDRAWTMPDS:.cxx=.h)
PROOFDRAWTMP2DS   := $(MODDIRS)/G__ProofDrawTmp2.cxx
PROOFDRAWTMP2DO   := $(PROOFDRAWTMP2DS:.cxx=.o)
PROOFDRAWTMP2DH   := $(PROOFDRAWTMP2DS:.cxx=.h)

PROOFDRAWH   := $(MODDIRI)/TProofDraw.h
PROOFDRAWS   := $(MODDIRS)/TProofDraw.cxx
PROOFDRAWO   := $(PROOFDRAWS:.cxx=.o)

PROOFDRAWDEP := $(PROOFDRAWO:.o=.d) $(PROOFDRAWDO:.o=.d)

#LF
PROOFDRAWTMPDEP  := $(PROOFDRAWTMPDO:.o=.d)

PROOFDRAWLIB := $(LPATH)/libProofDraw.$(SOEXT)
PROOFDRAWMAP := $(PROOFDRAWLIB:.$(SOEXT)=.rootmap)

#LF
PROOFDRAWNM       := $(PROOFDRAWLIB:.$(SOEXT)=.nm)

# used in the main Makefile
ALLHDRS       += $(patsubst $(MODDIRI)/%.h,include/%.h,$(PROOFPLAYERH))
ALLHDRS       += $(patsubst $(MODDIRI)/%.h,include/%.h,$(PROOFDRAWH))
ALLLIBS       += $(PROOFPLAYERLIB) $(PROOFDRAWLIB)
ALLMAPS       += $(PROOFPLAYERMAP) $(PROOFDRAWMAP)

# include all dependency files
INCLUDEFILES += $(PROOFPLAYERDEP) $(PROOFDRAWDEP)

##### local rules #####
include/%.h:    $(PROOFPLAYERDIRI)/%.h
		cp $< $@

#LF
$(PROOFPLAYERLIB):   $(PROOFPLAYERO) $(PROOFPLAYERTMPDO) $(PROOFPLAYERTMP2DO) \
			$(PROOFPLAYERDO) $(ORDER_) $(MAINLIBS) $(PROOFPLAYERLIBDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		   "$(SOFLAGS)" libProofPlayer.$(SOEXT) $@ "$(PROOFPLAYERO) $(PROOFPLAYERTMPDO) \
			$(PROOFPLAYERTMP2DO) $(PROOFPLAYERDO)" \
		   "$(PROOFPLAYERLIBEXTRA)"

#LF
$(PROOFPLAYERTMPDS):   $(PROOFPLAYERH) $(PROOFPLAYERL) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(PROOFPLAYERH) $(PROOFPLAYERL)

#LF
$(PROOFPLAYERTMP2DS):  $(PROOFPLAYERH) $(PROOFPLAYERL) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(PROOFPLAYERH) $(PROOFPLAYERL)

#LF
$(PROOFPLAYERDS):    $(PROOFPLAYERH) $(PROOFPLAYERL) $(ROOTCINTTMPEXE) $(PROOFPLAYERNM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(PROOFPLAYERNM) -. 3 -c $(PROOFPLAYERH) $(PROOFPLAYERL)

#LF
$(PROOFPLAYERNM):      $(PROOFPLAYERO) $(PROOFPLAYERTMPDO) $(PROOFPLAYERTMP2DO) 
		@echo "Generating symbols file $@..."
		nm -g -p --defined-only $(PROOFPLAYERTMPDO) | awk '{printf("%s\n", $$3)'} > $(PROOFPLAYERNM)
		nm -g -p --defined-only $(PROOFPLAYERTMP2DO) | awk '{printf("%s\n", $$3)'} >> $(PROOFPLAYERNM)
		nm -g -p --defined-only $(PROOFPLAYERO) | awk '{printf("%s\n", $$3)'} >> $(PROOFPLAYERNM)

$(PROOFPLAYERMAP): $(RLIBMAP) $(MAKEFILEDEP) $(PROOFPLAYERL)
		$(RLIBMAP) -o $(PROOFPLAYERMAP) -l $(PROOFPLAYERLIB) \
		   -d $(PROOFPLAYERLIBDEPM) -c $(PROOFPLAYERL)


#LF
$(PROOFDRAWLIB):   $(PROOFDRAWO) $(PROOFDRAWTMPDO) $(PROOFDRAWTMP2DO) $(PROOFDRAWDO) $(ORDER_) $(MAINLIBS) $(PROOFDRAWLIBDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		   "$(SOFLAGS)" libProofDraw.$(SOEXT) $@ "$(PROOFDRAWO) $(PROOFDRAWTMPDO) $(PROOFDRAWTMP2DO) $(PROOFDRAWDO)" \
		   "$(PROOFDRAWLIBEXTRA)"

#LF
$(PROOFDRAWTMPDS):   $(PROOFDRAWH) $(PROOFDRAWL) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(PROOFDRAWH) $(PROOFDRAWL)

#LF
$(PROOFDRAWTMP2DS):  $(PROOFDRAWH) $(PROOFDRAWL) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(PROOFDRAWH) $(PROOFDRAWL)

#LF
$(PROOFDRAWDS):    $(PROOFDRAWH) $(PROOFDRAWL) $(ROOTCINTTMPEXE) $(PROOFDRAWNM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(PROOFDRAWNM) -. 3 -c $(PROOFDRAWH) $(PROOFDRAWL)

#LF
$(PROOFDRAWNM):      $(PROOFDRAWO) $(PROOFDRAWTMPDO) $(PROOFDRAWTMP2DO) 
		@echo "Generating symbols file $@..."
		nm -g -p --defined-only $(PROOFDRAWTMPDO) | awk '{printf("%s\n", $$3)'} > $(PROOFDRAWNM)
		nm -g -p --defined-only $(PROOFDRAWTMP2DO) | awk '{printf("%s\n", $$3)'} >> $(PROOFDRAWNM)
		nm -g -p --defined-only $(PROOFDRAWO) | awk '{printf("%s\n", $$3)'} >> $(PROOFDRAWNM)

$(PROOFDRAWMAP): $(RLIBMAP) $(MAKEFILEDEP) $(PROOFDRAWL)
		$(RLIBMAP) -o $(PROOFDRAWMAP) -l $(PROOFDRAWLIB) \
		   -d $(PROOFDRAWLIBDEPM) -c $(PROOFDRAWL)

all-proofplayer: $(PROOFPLAYERLIB) $(PROOFPLAYERMAP) $(PROOFDRAWLIB) $(PROOFDRAWMAP)

clean-proofplayer:
		@rm -f $(PROOFPLAYERO) $(PROOFPLAYERDO) $(PROOFDRAWO) $(PROOFDRAWDO)

clean::         clean-proofplayer clean-pds-proofplayer

#LF
clean-pds-proofplayer:	
		rm -f $(PROOFPLAYERTMPDS) $(PROOFPLAYERTMPDO) $(PROOFPLAYERTMPDH) \
		$(PROOFPLAYERTMPDEP) $(PROOFPLAYERTMP2DS) $(PROOFPLAYERTMP2DO) $(PROOFPLAYERTMP2DH) $(PROOFPLAYERNM)
#LF
clean-pds-proofdraw:	
		rm -f $(PROOFDRAWTMPDS) $(PROOFDRAWTMPDO) $(PROOFDRAWTMPDH) \
		$(PROOFDRAWTMPDEP) $(PROOFDRAWTMP2DS) $(PROOFDRAWTMP2DO) $(PROOFDRAWTMP2DH) $(PROOFDRAWNM)

distclean-proofplayer: clean-proofplayer
		@rm -f $(PROOFPLAYERDEP) $(PROOFPLAYERDS) $(PROOFPLAYERDH) \
		   $(PROOFPLAYERLIB) $(PROOFPLAYERMAP) \
		   $(PROOFDRAWDEP) $(PROOFDRAWDS) $(PROOFDRAWDH) \
		   $(PROOFDRAWLIB) $(PROOFDRAWMAP) \

distclean::     distclean-proofplayer

##### extra rules ######
