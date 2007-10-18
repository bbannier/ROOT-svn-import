# Module.mk for g3d module
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: Fons Rademakers, 29/2/2000

MODDIR       := g3d
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

G3DDIR       := $(MODDIR)
G3DDIRS      := $(G3DDIR)/src
G3DDIRI      := $(G3DDIR)/inc

##### libGraf3d #####
G3DL         := $(MODDIRI)/LinkDef.h
G3DDS        := $(MODDIRS)/G__G3D.cxx
G3DDO        := $(G3DDS:.cxx=.o)
G3DDH        := $(G3DDS:.cxx=.h)

#LF
G3DTMPDS    := $(MODDIRS)/G__G3DTmp.cxx
G3DTMPDO    := $(G3DTMPDS:.cxx=.o)
G3DTMPDH    := $(G3DTMPDS:.cxx=.h)
G3DTMP2DS    := $(MODDIRS)/G__G3DTmp2.cxx
G3DTMP2DO    := $(G3DTMP2DS:.cxx=.o)
G3DTMP2DH    := $(G3DTMP2DS:.cxx=.h)

G3DH1        := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
G3DH1        := $(filter-out $(MODDIRI)/X3DBuffer%,$(G3DH1))
G3DH1        := $(filter-out $(MODDIRI)/X3DDefs%,$(G3DH1))
G3DH2        := $(MODDIRI)/X3DBuffer.h $(MODDIRI)/X3DDefs.h
G3DH         := $(G3DH1) $(G3DH2)
G3DS1        := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
G3DS2        := $(wildcard $(MODDIRS)/*.c)
G3DO         := $(G3DS1:.cxx=.o) $(G3DS2:.c=.o)

G3DDEP       := $(G3DO:.o=.d) $(G3DDO:.o=.d)

#LF
G3DTMPDEP   := $(G3DTMPDO:.o=.d)

G3DLIB       := $(LPATH)/libGraf3d.$(SOEXT)

#LF
G3DMAP       := $(G3DLIB:.$(SOEXT)=.rootmap)
G3DDICTLIB  := $(LPATH)/libGraf3dDict.$(SOEXT)
G3DDICTMAP  := $(G3DDICTLIB:.$(SOEXT)=.rootmap)

#LF
G3DNM       := $(G3DLIB:.$(SOEXT)=.nm)


# used in the main Makefile
ALLHDRS     += $(patsubst $(MODDIRI)/%.h,include/%.h,$(G3DH))
ALLLIBS     += $(G3DLIB)
ALLMAPS     += $(G3DMAP)

# include all dependency files
INCLUDEFILES += $(G3DDEP)

##### local rules #####
include/%.h:    $(G3DDIRI)/%.h
		cp $< $@

#LF
$(G3DLIB):     $(G3DO) $(G3DTMPDO) $(G3DTMP2DO) $(G3DDO) $(ORDER_) $(MAINLIBS) $(G3DLIBDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		"$(SOFLAGS)" libGraf3d.$(SOEXT) $@ "$(G3DO) $(G3DTMPDO) $(G3DTMP2DO) $(G3DDO)" \
		"$(G3DLIBEXTRA)"

#LF
#$(G3DDICTLIB): $(G3DDO) $(ORDER_) $(MAINLIBS) $(G3DDICTLIBDEP)
#		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
#		"$(SOFLAGS)" libGraf3dDict.$(SOEXT) $@ "$(G3DDO)" "$(G3DTMP2DO)"\
#		"$(G3DDICTLIBEXTRA)"

#LF
$(G3DTMPDS):   $(G3DH1) $(G3DL) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(G3DH1) $(G3DL)

#LF
$(G3DTMP2DS):  $(G3DH1) $(G3DL) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(G3DH1) $(G3DL)

#LF
$(G3DDS):      $(G3DH1) $(G3DL) $(ROOTCINTTMPEXE) $(G3DNM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(G3DNM) -. 3 -c $(G3DH1) $(G3DL)
#LF
$(G3DMAP): $(RLIBMAP) $(MAKEFILEDEP) $(G3DL)
		$(RLIBMAP) -o $(G3DMAP) -l $(G3DLIB) \
		-d $(G3DLIBDEPM) -c $(G3DL)
#LF
$(G3DNM):      $(G3DO) $(G3DTMPDO) $(G3DTMP2DO)
		@echo "Generating symbols file $@..."
		nm -p --defined-only $(G3DTMPDO) | awk '{printf("%s\n", $$3)'} > $(G3DNM)
		nm -p --defined-only $(G3DTMP2DO) | awk '{printf("%s\n", $$3)'} >> $(G3DNM)
		nm -p --defined-only $(G3DO) | awk '{printf("%s\n", $$3)'} >> $(G3DNM)

all-g3d:        $(G3DLIB) $(G3DMAP)

clean-g3d:
		@rm -f $(G3DO) $(G3DDO)

clean::         clean-g3d clean-pds-g3d

#LF
clean-pds-g3d:	
		rm -f $(G3DTMPDS) $(G3DTMPDO) $(G3DTMPDH) \
		$(G3DTMPDEP) $(G3DTMP2DS) $(G3DTMP2DO) $(G3DTMP2DH) $(G3DNM)

distclean-g3d:  clean-g3d
		@rm -f $(G3DDEP) $(G3DDS) $(G3DDH) $(G3DLIB) $(G3DMAP)

distclean::     distclean-g3d
