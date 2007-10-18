# Module.mk for geompainter module
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: Fons Rademakers, 29/2/2000

MODDIR       := geompainter
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

GEOMPAINTERDIR  := $(MODDIR)
GEOMPAINTERDIRS := $(GEOMPAINTERDIR)/src
GEOMPAINTERDIRI := $(GEOMPAINTERDIR)/inc

##### libGeomPainter #####
GEOMPAINTERL  := $(MODDIRI)/LinkDef.h
GEOMPAINTERDS := $(MODDIRS)/G__GeomPainter.cxx
GEOMPAINTERDO := $(GEOMPAINTERDS:.cxx=.o)
GEOMPAINTERDH := $(GEOMPAINTERDS:.cxx=.h)

GEOMPAINTERH1 := $(wildcard $(MODDIRI)/T*.h)
GEOMPAINTERH  := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
GEOMPAINTERS  := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
GEOMPAINTERO  := $(GEOMPAINTERS:.cxx=.o)

#LF
GEOMPAINTERTMPDS    := $(MODDIRS)/G__GeomPainterTmp.cxx
GEOMPAINTERTMPDO    := $(GEOMPAINTERTMPDS:.cxx=.o)
GEOMPAINTERTMPDH    := $(GEOMPAINTERTMPDS:.cxx=.h)
GEOMPAINTERTMP2DS   := $(MODDIRS)/G__GeomPainterTmp2.cxx
GEOMPAINTERTMP2DO   := $(GEOMPAINTERTMP2DS:.cxx=.o)
GEOMPAINTERTMP2DH   := $(GEOMPAINTERTMP2DS:.cxx=.h)

GEOMPAINTERDEP := $(GEOMPAINTERO:.o=.d) $(GEOMPAINTERDO:.o=.d)

#LF
GEOMPAINTERTMPDEP  := $(GEOMPAINTERTMPDO:.o=.d)

GEOMPAINTERLIB := $(LPATH)/libGeomPainter.$(SOEXT)
GEOMPAINTERMAP := $(GEOMPAINTERLIB:.$(SOEXT)=.rootmap)

#LF
GEOMPAINTERNM       := $(GEOMPAINTERLIB:.$(SOEXT)=.nm)

# used in the main Makefile
ALLHDRS       += $(patsubst $(MODDIRI)/%.h,include/%.h,$(GEOMPAINTERH))
ALLLIBS       += $(GEOMPAINTERLIB)
ALLMAPS       += $(GEOMPAINTERMAP)

# include all dependency files
INCLUDEFILES += $(GEOMPAINTERDEP)

##### local rules #####
include/%.h:    $(GEOMPAINTERDIRI)/%.h
		cp $< $@

#LF
$(GEOMPAINTERLIB):   $(GEOMPAINTERO) $(GEOMPAINTERTMPDO) $(GEOMPAINTERTMP2DO) $(GEOMPAINTERDO) \
			$(ORDER_) $(MAINLIBS) $(GEOMPAINTERLIBDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		   "$(SOFLAGS)" libGeomPainter.$(SOEXT) $@ "$(GEOMPAINTERO) $(GEOMPAINTERTMPDO) \
			$(GEOMPAINTERTMP2DO) $(GEOMPAINTERDO)" \
		   "$(GEOMPAINTERLIBEXTRA)"

#LF
$(GEOMPAINTERTMPDS):   $(GEOMPAINTERH1) $(GEOMPAINTERL) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(GEOMPAINTERH1) $(GEOMPAINTERL)

#LF
$(GEOMPAINTERTMP2DS):  $(GEOMPAINTERH1) $(GEOMPAINTERL) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(GEOMPAINTERH1) $(GEOMPAINTERL)

#LF
$(GEOMPAINTERDS):    $(GEOMPAINTERH1) $(GEOMPAINTERL) $(ROOTCINTTMPEXE) $(GEOMPAINTERNM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(GEOMPAINTERNM) -. 3 -c $(GEOMPAINTERH1) $(GEOMPAINTERL)

#LF
$(GEOMPAINTERDICTMAP): $(RLIBMAP) $(MAKEFILEDEP) $(GEOMPAINTERL)
		$(RLIBMAP) -o $(GEOMPAINTERDICTMAP) -l $(GEOMPAINTERDICTLIB) \
		-d $(GEOMPAINTERLIB) $(GEOMPAINTERLIBDEPM) -c $(GEOMPAINTERL)
#LF
$(GEOMPAINTERNM):      $(GEOMPAINTERO) $(GEOMPAINTERTMPDO) $(GEOMPAINTERTMP2DO) 
		@echo "Generating symbols file $@..."
		nm -p --defined-only $(GEOMPAINTERTMPDO) | awk '{printf("%s\n", $$3)'} > $(GEOMPAINTERNM)
		nm -p --defined-only $(GEOMPAINTERTMP2DO) | awk '{printf("%s\n", $$3)'} >> $(GEOMPAINTERNM)
		nm -p --defined-only $(GEOMPAINTERO) | awk '{printf("%s\n", $$3)'} >> $(GEOMPAINTERNM)


$(GEOMPAINTERMAP): $(RLIBMAP) $(MAKEFILEDEP) $(GEOMPAINTERL)
		$(RLIBMAP) -o $(GEOMPAINTERMAP) -l $(GEOMPAINTERLIB) \
		   -d $(GEOMPAINTERLIBDEPM) -c $(GEOMPAINTERL)

all-geompainter: $(GEOMPAINTERLIB) $(GEOMPAINTERMAP)

clean-geompainter:
		@rm -f $(GEOMPAINTERO) $(GEOMPAINTERDO)

clean::         clean-geompainter clean-pds-geompainter

#LF
clean-pds-geompainter:	
		rm -f $(GEOMPAINTERTMPDS) $(GEOMPAINTERTMPDO) $(GEOMPAINTERTMPDH) \
		$(GEOMPAINTERTMPDEP) $(GEOMPAINTERTMP2DS) $(GEOMPAINTERTMP2DO) $(GEOMPAINTERTMP2DH) $(GEOMPAINTERNM)

distclean-geompainter: clean-geompainter
		@rm -f $(GEOMPAINTERDEP) $(GEOMPAINTERDS) $(GEOMPAINTERDH) \
		   $(GEOMPAINTERLIB) $(GEOMPAINTERMAP)

distclean::     distclean-geompainter
