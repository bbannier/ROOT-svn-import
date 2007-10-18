# Module.mk for geom module
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: Fons Rademakers, 29/2/2000

MODDIR       := geom
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

GEOMDIR      := $(MODDIR)
GEOMDIRS     := $(GEOMDIR)/src
GEOMDIRI     := $(GEOMDIR)/inc

##### libGeom #####
GEOML1       := $(MODDIRI)/LinkDef1.h
GEOML2       := $(MODDIRI)/LinkDef2.h
GEOMDS1      := $(MODDIRS)/G__Geom1.cxx
GEOMDS2      := $(MODDIRS)/G__Geom2.cxx
GEOMDO1      := $(GEOMDS1:.cxx=.o)
GEOMDO2      := $(GEOMDS2:.cxx=.o)
GEOMDS       := $(GEOMDS1) $(GEOMDS2)
GEOMDO       := $(GEOMDO1) $(GEOMDO2)
GEOMDH       := $(GEOMDS:.cxx=.h)

#LF
GEOMTMPDS1    := $(MODDIRS)/G__Geom1Tmp.cxx
GEOMTMPDO1    := $(GEOMTMPDS1:.cxx=.o)
GEOMTMPDH1    := $(GEOMTMPDS1:.cxx=.h)
GEOMTMP2DS1   := $(MODDIRS)/G__Geom1Tmp2.cxx
GEOMTMP2DO1   := $(GEOMTMP2DS1:.cxx=.o)
GEOMTMP2DH1   := $(GEOMTMP2DS1:.cxx=.h)

GEOMH1       := TGeoAtt.h TGeoBoolNode.h \
                TGeoMedium.h TGeoMaterial.h \
                TGeoMatrix.h TGeoVolume.h TGeoNode.h \
                TGeoVoxelFinder.h TGeoShape.h TGeoBBox.h \
                TGeoPara.h TGeoTube.h TGeoTorus.h TGeoSphere.h \
                TGeoEltu.h TGeoHype.h TGeoCone.h TGeoPcon.h \
                TGeoPgon.h TGeoArb8.h TGeoTrd1.h TGeoTrd2.h \
                TGeoManager.h TGeoCompositeShape.h TGeoShapeAssembly.h \
                TGeoScaledShape.h TVirtualGeoPainter.h TVirtualGeoTrack.h \
		TGeoPolygon.h TGeoXtru.h TGeoPhysicalNode.h \
                TGeoHelix.h TGeoParaboloid.h TGeoElement.h TGeoHalfSpace.h \
                TGeoBuilder.h TGeoNavigator.h
GEOMH2       := TGeoPatternFinder.h TGeoCache.h
GEOMH1       := $(patsubst %,$(MODDIRI)/%,$(GEOMH1))
GEOMH2       := $(patsubst %,$(MODDIRI)/%,$(GEOMH2))
GEOMH        := $(GEOMH1) $(GEOMH2)
GEOMS        := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
GEOMO        := $(GEOMS:.cxx=.o)

#LF
GEOMTMPDS2    := $(MODDIRS)/G__Geom2Tmp.cxx
GEOMTMPDO2    := $(GEOMTMPDS2:.cxx=.o)
GEOMTMPDH2    := $(GEOMTMPDS2:.cxx=.h)
GEOMTMP2DS2   := $(MODDIRS)/G__Geom2Tmp2.cxx
GEOMTMP2DO2   := $(GEOMTMP2DS2:.cxx=.o)
GEOMTMP2DH2   := $(GEOMTMP2DS2:.cxx=.h)

#LF
GEOMTMPDS       := $(GEOMTMPDS1) $(GEOMTMPDS2)
GEOMTMPDO       := $(GEOMTMPDO1) $(GEOMTMPDO2)
GEOMTMPDH       := $(GEOMTMPDS:.cxx=.h)

#LF
GEOMTMP2DS       := $(GEOMTMP2DS1) $(GEOMTMP2DS2)
GEOMTMP2DO       := $(GEOMTMP2DO1) $(GEOMTMP2DO2)
GEOMTMP2DH       := $(GEOMTMP2DS:.cxx=.h)

GEOMDEP      := $(GEOMO:.o=.d) $(GEOMDO:.o=.d)

#LF
GEOMTMPDEP  := $(GEOMTMPDO:.o=.d)

GEOMLIB      := $(LPATH)/libGeom.$(SOEXT)
GEOMMAP      := $(GEOMLIB:.$(SOEXT)=.rootmap)

#LF
GEOMNM       := $(GEOMLIB:.$(SOEXT)=.nm)

# used in the main Makefile
ALLHDRS     += $(patsubst $(MODDIRI)/%.h,include/%.h,$(GEOMH))
ALLLIBS     += $(GEOMLIB)
ALLMAPS     += $(GEOMMAP)

# include all dependency files
INCLUDEFILES += $(GEOMDEP)

##### local rules #####
include/%.h:    $(GEOMDIRI)/%.h
		cp $< $@

#LF
$(GEOMLIB):   $(GEOMO) $(GEOMTMPDO) $(GEOMTMP2DO) $(GEOMDO) $(ORDER_) $(MAINLIBS) $(GEOMLIBDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		   "$(SOFLAGS)" libGeom.$(SOEXT) $@ "$(GEOMO) $(GEOMTMPDO) $(GEOMTMP2DO) $(GEOMDO)" \
		   "$(GEOMLIBEXTRA)"

#LF
$(GEOMTMPDS1):   $(GEOMH1) $(GEOML1) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(GEOMH1) $(GEOML1)

#LF
$(GEOMTMP2DS1):  $(GEOMH1) $(GEOML1) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(GEOMH1) $(GEOML1)

#LF
$(GEOMDS1):    $(GEOMH1) $(GEOML1) $(ROOTCINTTMPEXE) $(GEOMNM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(GEOMNM) -. 3 -c $(GEOMH1) $(GEOML1)


#LF
$(GEOMTMPDS2):   $(GEOMH2) $(GEOML2) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(GEOMH2) $(GEOML2)

#LF
$(GEOMTMP2DS2):  $(GEOMH2) $(GEOML2) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(GEOMH2) $(GEOML2)

#LF
$(GEOMDS2):    $(GEOMH2) $(GEOML2) $(ROOTCINTTMPEXE) $(GEOMNM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(GEOMNM) -. 3 -c $(GEOMH2) $(GEOML2)

#LF
$(GEOMNM):      $(GEOMO) $(GEOMTMPDO) $(GEOMTMP2DO) 
		@echo "Generating symbols file $@..."
		nm -p --defined-only $(GEOMTMPDO) | awk '{printf("%s\n", $$3)'} > $(GEOMNM)
		nm -p --defined-only $(GEOMTMP2DO) | awk '{printf("%s\n", $$3)'} >> $(GEOMNM)
		nm -p --defined-only $(GEOMO) | awk '{printf("%s\n", $$3)'} >> $(GEOMNM)

$(GEOMMAP):     $(RLIBMAP) $(MAKEFILEDEP) $(GEOML1) $(GEOML2)
		$(RLIBMAP) -o $(GEOMMAP) -l $(GEOMLIB) \
		   -d $(GEOMLIBDEPM) -c $(GEOML1) $(GEOML2)

all-geom:       $(GEOMLIB) $(GEOMMAP)

clean-geom:
		@rm -f $(GEOMO) $(GEOMDO)

clean::         clean-geom clean-pds-geom

#LF
clean-pds-geom:	
		rm -f $(GEOMTMPDS) $(GEOMTMPDO) $(GEOMTMPDH) \
		$(GEOMTMPDEP) $(GEOMTMP2DS) $(GEOMTMP2DO) $(GEOMTMP2DH) $(GEOMNM)

distclean-geom: clean-geom
		@rm -f $(GEOMDEP) $(GEOMDS) $(GEOMDH) $(GEOMLIB) $(GEOMMAP)

distclean::     distclean-geom
