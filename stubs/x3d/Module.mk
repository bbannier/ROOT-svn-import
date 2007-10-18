# Module.mk for x3d module
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: Fons Rademakers, 29/2/2000

MODDIR       := x3d
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

X3DDIR       := $(MODDIR)
X3DDIRS      := $(X3DDIR)/src
X3DDIRI      := $(X3DDIR)/inc

##### libX3d #####
X3DL         := $(MODDIRI)/LinkDef.h
X3DDS        := $(MODDIRS)/G__X3D.cxx
X3DDO        := $(X3DDS:.cxx=.o)
X3DDH        := $(X3DDS:.cxx=.h)

#LF
X3DTMPDS    := $(MODDIRS)/G__X3DTmp.cxx
X3DTMPDO    := $(X3DTMPDS:.cxx=.o)
X3DTMPDH    := $(X3DTMPDS:.cxx=.h)
X3DTMP2DS   := $(MODDIRS)/G__X3dTmp2.cxx
X3DTMP2DO   := $(X3DTMP2DS:.cxx=.o)
X3DTMP2DH   := $(X3DTMP2DS:.cxx=.h)

X3DH1        := $(MODDIRI)/TViewerX3D.h $(MODDIRI)/TX3DFrame.h
X3DH2        := $(MODDIRI)/x3d.h
X3DH         := $(X3DH1) $(X3DH2)
X3DS1        := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
X3DS2        := $(wildcard $(MODDIRS)/*.c)
X3DO         := $(X3DS1:.cxx=.o) $(X3DS2:.c=.o)

X3DDEP       := $(X3DO:.o=.d) $(X3DDO:.o=.d)

#LF
X3DTMPDEP  := $(X3DTMPDO:.o=.d)

X3DLIB       := $(LPATH)/libX3d.$(SOEXT)
X3DMAP       := $(X3DLIB:.$(SOEXT)=.rootmap)

#LF
X3DNM       := $(X3DLIB:.$(SOEXT)=.nm)

# used in the main Makefile
ALLHDRS     += $(patsubst $(MODDIRI)/%.h,include/%.h,$(X3DH))
ALLLIBS     += $(X3DLIB)
ALLMAPS     += $(X3DMAP)

# include all dependency files
INCLUDEFILES += $(X3DDEP)

##### local rules #####
include/%.h:    $(X3DDIRI)/%.h
		cp $< $@

$(X3DLIB):      $(X3DO) $(X3DTMPDO) $(X3DTMP2DO) $(X3DDO) $(ORDER_) $(MAINLIBS) $(X3DLIBDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		   "$(SOFLAGS)" libX3d.$(SOEXT) $@ "$(X3DO) $(X3DTMPDO) $(X3DTMP2DO) $(X3DDO)" \
		   "$(X3DLIBEXTRA) $(XLIBS)"

#LF
$(X3DTMPDS):   $(X3DH1) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(X3DH1) $(X3DL)

#LF
$(X3DTMP2DS):  $(X3DH1) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(X3DH1) $(X3DL)

#LF
$(X3DDS):    $(X3DH1) $(X3DL) $(ROOTCINTTMPEXE) $(X3DNM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(X3DNM) -. 3 -c $(X3DH1) $(X3DL)

#LF
$(X3DNM):      $(X3DO) $(X3DTMPDO) $(X3DTMP2DO) 
		@echo "Generating symbols file $@..."
		nm -p --defined-only $(X3DTMPDO) | awk '{printf("%s\n", $$3)'} > $(X3DNM)
		nm -p --defined-only $(X3DTMP2DO) | awk '{printf("%s\n", $$3)'} >> $(X3DNM)
		nm -p --defined-only $(X3DO) | awk '{printf("%s\n", $$3)'} >> $(X3DNM)

$(X3DMAP):      $(RLIBMAP) $(MAKEFILEDEP) $(X3DL)
		$(RLIBMAP) -o $(X3DMAP) -l $(X3DLIB) \
		   -d $(X3DLIBDEPM) -c $(X3DL)

all-x3d:        $(X3DLIB) $(X3DMAP)

clean-x3d:
		@rm -f $(X3DO) $(X3DDO)

clean::         clean-x3d clean-pds-x3d

#LF
clean-pds-x3d:	
		rm -f $(X3DTMPDS) $(X3DTMPDO) $(X3DTMPDH) \
		$(X3DTMPDEP) $(X3DTMP2DS) $(X3DTMP2DO) $(X3DTMP2DH) $(X3DNM)

distclean-x3d:  clean-x3d
		@rm -f $(X3DDEP) $(X3DDS) $(X3DDH) $(X3DLIB) $(X3DMAP)

distclean::     distclean-x3d
