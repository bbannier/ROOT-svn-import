# Module.mk for geombuilder module
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: Fons Rademakers, 29/2/2000

MODDIR           := geombuilder
MODDIRS          := $(MODDIR)/src
MODDIRI          := $(MODDIR)/inc

GEOMBUILDERDIR   := $(MODDIR)
GEOMBUILDERDIRS  := $(GEOMBUILDERDIR)/src
GEOMBUILDERDIRI  := $(GEOMBUILDERDIR)/inc

##### libGeomBuilder #####
GEOMBUILDERL     := $(MODDIRI)/LinkDef.h
GEOMBUILDERDS    := $(MODDIRS)/G__GeomBuilder.cxx
GEOMBUILDERDO    := $(GEOMBUILDERDS:.cxx=.o)
GEOMBUILDERDH    := $(GEOMBUILDERDS:.cxx=.h)

GEOMBUILDERH     := TGeoVolumeEditor.h TGeoBBoxEditor.h TGeoMediumEditor.h \
                    TGeoNodeEditor.h TGeoMatrixEditor.h TGeoManagerEditor.h \
                    TGeoTubeEditor.h TGeoConeEditor.h TGeoTrd1Editor.h \
                    TGeoTrd2Editor.h TGeoMaterialEditor.h TGeoTabManager.h \
                    TGeoSphereEditor.h TGeoPconEditor.h TGeoParaEditor.h \
                    TGeoTorusEditor.h TGeoEltuEditor.h TGeoHypeEditor.h \
                    TGeoPgonEditor.h TGeoTrapEditor.h TGeoGedFrame.h
GEOMBUILDERH     := $(patsubst %,$(MODDIRI)/%,$(GEOMBUILDERH))
GEOMBUILDERS     := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
GEOMBUILDERO     := $(GEOMBUILDERS:.cxx=.o)

#LF
GEOMBUILDERTMPDS    := $(MODDIRS)/G__GeomBuilderTmp.cxx
GEOMBUILDERTMPDO    := $(GEOMBUILDERTMPDS:.cxx=.o)
GEOMBUILDERTMPDH    := $(GEOMBUILDERTMPDS:.cxx=.h)
GEOMBUILDERTMP2DS   := $(MODDIRS)/G__GeomBuilderTmp2.cxx
GEOMBUILDERTMP2DO   := $(GEOMBUILDERTMP2DS:.cxx=.o)
GEOMBUILDERTMP2DH   := $(GEOMBUILDERTMP2DS:.cxx=.h)

GEOMBUILDERDEP   := $(GEOMBUILDERO:.o=.d) $(GEOMBUILDERDO:.o=.d)

#LF
GEOMBUILDERTMPDEP  := $(GEOMBUILDERTMPDO:.o=.d)

GEOMBUILDERLIB   := $(LPATH)/libGeomBuilder.$(SOEXT)
GEOMBUILDERMAP   := $(GEOMBUILDERLIB:.$(SOEXT)=.rootmap)

#LF
GEOMBUILDERNM       := $(GEOMBUILDERLIB:.$(SOEXT)=.nm)

# used in the main Makefile
ALLHDRS          += $(patsubst $(MODDIRI)/%.h,include/%.h,$(GEOMBUILDERH))
ALLLIBS          += $(GEOMBUILDERLIB)
ALLMAPS          += $(GEOMBUILDERMAP)

# include all dependency files
INCLUDEFILES     += $(GEOMBUILDERDEP)

##### local rules #####
include/%.h:    $(GEOMBUILDERDIRI)/%.h
		cp $< $@

#LF
$(GEOMBUILDERLIB):   $(GEOMBUILDERO) $(GEOMBUILDERTMPDO) $(GEOMBUILDERTMP2DO) $(GEOMBUILDERDO) \
			$(ORDER_) $(MAINLIBS) $(GEOMBUILDERLIBDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		   "$(SOFLAGS)" libGeomBuilder.$(SOEXT) $@ "$(GEOMBUILDERO) \
			$(GEOMBUILDERTMPDO) $(GEOMBUILDERTMP2DO) $(GEOMBUILDERDO)" \
		   "$(GEOMBUILDERLIBEXTRA)"

#LF
$(GEOMBUILDERTMPDS):   $(GEOMBUILDERH) $(GEOMBUILDERL) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(GEOMBUILDERH) $(GEOMBUILDERL)

#LF
$(GEOMBUILDERTMP2DS):  $(GEOMBUILDERH) $(GEOMBUILDERL) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(GEOMBUILDERH) $(GEOMBUILDERL)

#LF
$(GEOMBUILDERDS):    $(GEOMBUILDERH) $(GEOMBUILDERL) $(ROOTCINTTMPEXE) $(GEOMBUILDERNM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(GEOMBUILDERNM) -. 3 -c $(GEOMBUILDERH) $(GEOMBUILDERL)

#LF
$(GEOMBUILDERNM):      $(GEOMBUILDERO) $(GEOMBUILDERTMPDO) $(GEOMBUILDERTMP2DO) 
		@echo "Generating symbols file $@..."
		nm -p --defined-only $(GEOMBUILDERTMPDO) | awk '{printf("%s\n", $$3)'} > $(GEOMBUILDERNM)
		nm -p --defined-only $(GEOMBUILDERTMP2DO) | awk '{printf("%s\n", $$3)'} >> $(GEOMBUILDERNM)
		nm -p --defined-only $(GEOMBUILDERO) | awk '{printf("%s\n", $$3)'} >> $(GEOMBUILDERNM)

$(GEOMBUILDERMAP): $(RLIBMAP) $(MAKEFILEDEP) $(GEOMBUILDERL)
		$(RLIBMAP) -o $(GEOMBUILDERMAP) -l $(GEOMBUILDERLIB) \
		   -d $(GEOMBUILDERLIBDEPM) -c $(GEOMBUILDERL)

all-geombuilder: $(GEOMBUILDERLIB) $(GEOMBUILDERMAP)

clean-geombuilder:
		@rm -f $(GEOMBUILDERO) $(GEOMBUILDERDO)

clean::         clean-geombuilder clean-pds-geombuilder

#LF
clean-pds-geombuilder:	
		rm -f $(GEOMBUILDERTMPDS) $(GEOMBUILDERTMPDO) $(GEOMBUILDERTMPDH) \
		$(GEOMBUILDERTMPDEP) $(GEOMBUILDERTMP2DS) $(GEOMBUILDERTMP2DO) $(GEOMBUILDERTMP2DH) $(GEOMBUILDERNM)

distclean-geombuilder: clean-geombuilder
		@rm -f $(GEOMBUILDERDEP) $(GEOMBUILDERDS) $(GEOMBUILDERDH) \
		   $(GEOMBUILDERLIB) $(GEOMBUILDERMAP)

distclean::     distclean-geombuilder
