# Module.mk for histpainter module
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: Fons Rademakers, 29/2/2000

MODDIR       := histpainter
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

HISTPAINTERDIR  := $(MODDIR)
HISTPAINTERDIRS := $(HISTPAINTERDIR)/src
HISTPAINTERDIRI := $(HISTPAINTERDIR)/inc

##### libHistPainter #####
HISTPAINTERL  := $(MODDIRI)/LinkDef.h
HISTPAINTERDS := $(MODDIRS)/G__HistPainter.cxx
HISTPAINTERDO := $(HISTPAINTERDS:.cxx=.o)
HISTPAINTERDH := $(HISTPAINTERDS:.cxx=.h)

#LF
HISTPAINTERTMPDS    := $(MODDIRS)/G__HistPainterTmp.cxx
HISTPAINTERTMPDO    := $(HISTPAINTERTMPDS:.cxx=.o)
HISTPAINTERTMPDH    := $(HISTPAINTERTMPDS:.cxx=.h)
HISTPAINTERTMP2DS   := $(MODDIRS)/G__HistPainterTmp2.cxx
HISTPAINTERTMP2DO   := $(HISTPAINTERTMP2DS:.cxx=.o)
HISTPAINTERTMP2DH   := $(HISTPAINTERTMP2DS:.cxx=.h)

HISTPAINTERH1 := $(wildcard $(MODDIRI)/T*.h)
HISTPAINTERH  := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
HISTPAINTERS  := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
HISTPAINTERO  := $(HISTPAINTERS:.cxx=.o)

HISTPAINTERDEP := $(HISTPAINTERO:.o=.d) $(HISTPAINTERDO:.o=.d)

#LF
HISTPAINTERTMPDEP  := $(HISTPAINTERTMPDO:.o=.d)

HISTPAINTERLIB := $(LPATH)/libHistPainter.$(SOEXT)
HISTPAINTERMAP := $(HISTPAINTERLIB:.$(SOEXT)=.rootmap)

# used in the main Makefile
ALLHDRS       += $(patsubst $(MODDIRI)/%.h,include/%.h,$(HISTPAINTERH))
ALLLIBS       += $(HISTPAINTERLIB)
ALLMAPS       += $(HISTPAINTERMAP)

#LF
HISTPAINTERNM       := $(HISTPAINTERLIB:.$(SOEXT)=.nm)

# include all dependency files
INCLUDEFILES += $(HISTPAINTERDEP)

##### local rules #####
include/%.h:    $(HISTPAINTERDIRI)/%.h
		cp $< $@

#LF
$(HISTPAINTERLIB):   $(HISTPAINTERO) $(HISTPAINTERTMPDO) $(HISTPAINTERTMP2DO) $(HISTPAINTERDO) $(ORDER_) \
			$(MAINLIBS) $(HISTPAINTERLIBDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		   "$(SOFLAGS)" libHistPainter.$(SOEXT) $@ "$(HISTPAINTERO) $(HISTPAINTERTMPDO) \
			$(HISTPAINTERTMP2DO) $(HISTPAINTERDO)" \
		   "$(HISTPAINTERLIBEXTRA)"

#LF
$(HISTPAINTERTMPDS):   $(HISTPAINTERH1) $(HISTPAINTERL) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(HISTPAINTERH1) $(HISTPAINTERL)

#LF
$(HISTPAINTERTMP2DS):  $(HISTPAINTERH1) $(HISTPAINTERL) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(HISTPAINTERH1) $(HISTPAINTERL)

#LF
$(HISTPAINTERDS):    $(HISTPAINTERH1) $(HISTPAINTERL) $(ROOTCINTTMPEXE) $(HISTPAINTERNM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(HISTPAINTERNM) -. 3 -c $(HISTPAINTERH1) $(HISTPAINTERL)

#LF
$(HISTPAINTERNM):      $(HISTPAINTERO) $(HISTPAINTERTMPDO) $(HISTPAINTERTMP2DO) 
		@echo "Generating symbols file $@..."
		nm -p --defined-only $(HISTPAINTERTMPDO) | awk '{printf("%s\n", $$3)'} > $(HISTPAINTERNM)
		nm -p --defined-only $(HISTPAINTERTMP2DO) | awk '{printf("%s\n", $$3)'} >> $(HISTPAINTERNM)
		nm -p --defined-only $(HISTPAINTERO) | awk '{printf("%s\n", $$3)'} >> $(HISTPAINTERNM)

$(HISTPAINTERMAP): $(RLIBMAP) $(MAKEFILEDEP) $(HISTPAINTERL)
		$(RLIBMAP) -o $(HISTPAINTERMAP) -l $(HISTPAINTERLIB) \
		   -d $(HISTPAINTERLIBDEPM) -c $(HISTPAINTERL)

all-histpainter: $(HISTPAINTERLIB) $(HISTPAINTERMAP)

clean-histpainter:
		@rm -f $(HISTPAINTERO) $(HISTPAINTERDO)

clean::         clean-histpainter clean-pds-histpainter

#LF
clean-pds-histpainter:	
		rm -f $(HISTPAINTERTMPDS) $(HISTPAINTERTMPDO) $(HISTPAINTERTMPDH) \
		$(HISTPAINTERTMPDEP) $(HISTPAINTERTMP2DS) $(HISTPAINTERTMP2DO) $(HISTPAINTERTMP2DH) $(HISTPAINTERNM)

distclean-histpainter: clean-histpainter
		@rm -f $(HISTPAINTERDEP) $(HISTPAINTERDS) $(HISTPAINTERDH) \
		   $(HISTPAINTERLIB) $(HISTPAINTERMAP)

distclean::     distclean-histpainter
