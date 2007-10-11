# Module.mk for asimage module
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: Fons Rademakers, 8/8/2002

MODDIR       := asimage
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

ASIMAGEDIR   := $(MODDIR)
ASIMAGEDIRS  := $(ASIMAGEDIR)/src
ASIMAGEDIRI  := $(ASIMAGEDIR)/inc

ASTEPVERS    := libAfterImage
ASTEPSRCS    := $(MODDIRS)/$(ASTEPVERS).tar.gz
ifeq ($(BUILTINASIMAGE),yes)
ASTEPDIRS    := $(MODDIRS)/$(ASTEPVERS)
ASTEPDIRI    := -I$(MODDIRS)/$(ASTEPVERS)
ASTEPETAG    := $(MODDIRS)/headers.d
else
ASTEPDIRS    :=
ASTEPDIRI    := $(ASINCDIR:%=-I%)
endif

##### libAfterImage #####
ifeq ($(BUILTINASIMAGE),yes)
ifeq ($(PLATFORM),win32)
ASTEPLIBA    := $(ASTEPDIRS)/libAfterImage.lib
ASTEPLIB     := $(LPATH)/libAfterImage.lib
ifeq (yes,$(WINRTDEBUG))
ASTEPBLD      = "libAfterImage - Win32 Debug"
else
ASTEPBLD      = "libAfterImage - Win32 Release"
endif
else
ASTEPLIBA    := $(ASTEPDIRS)/libAfterImage.a
ASTEPLIB     := $(LPATH)/libAfterImage.a
endif
ifeq ($(MACOSX_MINOR),3)
ASEXTRALIB   += -lz
endif
ASTEPDEP     := $(ASTEPLIB)
ifeq (debug,$(findstring debug,$(ROOTBUILD)))
ASTEPDBG      = "--enable-gdb"
else
ASTEPDBG      =
endif
else
ASTEPLIBA    := $(ASLIBDIR) $(ASLIB)
ASTEPLIB     := $(ASLIBDIR) $(ASLIB)
ASTEPDEP     :=
endif

##### libASImage #####
ASIMAGEL     := $(MODDIRI)/LinkDef.h
ASIMAGEDS    := $(MODDIRS)/G__ASImage.cxx
ASIMAGEDO    := $(ASIMAGEDS:.cxx=.o)
ASIMAGEDH    := $(ASIMAGEDS:.cxx=.h)

ASIMAGEH     := $(MODDIRI)/TASImage.h $(MODDIRI)/TASImagePlugin.h
ASIMAGES     := $(MODDIRS)/TASImage.cxx
ASIMAGEO     := $(ASIMAGES:.cxx=.o)

#LF
ASIMAGETMPDS    := $(MODDIRS)/G__ASImageTmp.cxx
ASIMAGETMPDO    := $(ASIMAGETMPDS:.cxx=.o)
ASIMAGETMPDH    := $(ASIMAGETMPDS:.cxx=.h)
ASIMAGETMP2DS    := $(MODDIRS)/G__ASImageTmp2.cxx
ASIMAGETMP2DO    := $(ASIMAGETMP2DS:.cxx=.o)
ASIMAGETMP2DH    := $(ASIMAGETMP2DS:.cxx=.h)

ASIMAGEDEP   := $(ASIMAGEO:.o=.d) $(ASIMAGEDO:.o=.d)

#LF
ASIMAGETMPDEP   := $(ASIMAGETMPDO:.o=.d)

ASIMAGELIB   := $(LPATH)/libASImage.$(SOEXT)

#LF
ASIMAGEMAP   := $(ASIMAGELIB:.$(SOEXT)=.rootmap)
ASIMAGEDICTLIB  := $(LPATH)/libASImageDict.$(SOEXT)
ASIMAGEDICTMAP  := $(ASIMAGEDICTLIB:.$(SOEXT)=.rootmap)

#LF
ASIMAGENM       := $(ASIMAGELIB:.$(SOEXT)=.nm)

##### libASImageGui #####
ASIMAGEGUIL  := $(MODDIRI)/LinkDefGui.h
ASIMAGEGUIDS := $(MODDIRS)/G__ASImageGui.cxx
ASIMAGEGUIDO := $(ASIMAGEGUIDS:.cxx=.o)
ASIMAGEGUIDH := $(ASIMAGEGUIDS:.cxx=.h)

ASIMAGEGUIH  := $(MODDIRI)/TASPaletteEditor.h
ASIMAGEGUIS  := $(MODDIRS)/TASPaletteEditor.cxx
ASIMAGEGUIO  := $(ASIMAGEGUIS:.cxx=.o)

#LF
ASIMAGEGUITMPDS    := $(MODDIRS)/G__ASImageGuiTmp.cxx
ASIMAGEGUITMPDO    := $(ASIMAGEGUITMPDS:.cxx=.o)
ASIMAGEGUITMPDH    := $(ASIMAGEGUITMPDS:.cxx=.h)
ASIMAGEGUITMP2DS    := $(MODDIRS)/G__ASImageGuiTmp2.cxx
ASIMAGEGUITMP2DO    := $(ASIMAGEGUITMP2DS:.cxx=.o)
ASIMAGEGUITMP2DH    := $(ASIMAGEGUITMP2DS:.cxx=.h)

ASIMAGEGUIDEP := $(ASIMAGEGUIO:.o=.d) $(ASIMAGEGUIDO:.o=.d)

#LF
ASIMAGEGUITMPDEP   := $(ASIMAGEGUITMPDO:.o=.d)

ASIMAGEGUILIB := $(LPATH)/libASImageGui.$(SOEXT)

#LF
ASIMAGEGUIMAP := $(ASIMAGEGUILIB:.$(SOEXT)=.rootmap)
ASIMAGEGUIDICTLIB  := $(LPATH)/libASImageGuiDict.$(SOEXT)
ASIMAGEGUIDICTMAP  := $(ASIMAGEGUIDICTLIB:.$(SOEXT)=.rootmap)

#LF
ASIMAGEGUINM       := $(ASIMAGEGUILIB:.$(SOEXT)=.nm)

##### libASPluginGS #####
ASIMAGEGSL  := $(MODDIRI)/LinkDefGS.h
ASIMAGEGSDS := $(MODDIRS)/G__ASImageGS.cxx
ASIMAGEGSDO := $(ASIMAGEGSDS:.cxx=.o)
ASIMAGEGSDH := $(ASIMAGEGSDS:.cxx=.h)

ASIMAGEGSH  := $(MODDIRI)/TASPluginGS.h
ASIMAGEGSS  := $(MODDIRS)/TASPluginGS.cxx
ASIMAGEGSO  := $(ASIMAGEGSS:.cxx=.o)

#LF
ASIMAGEGSTMPDS    := $(MODDIRS)/G__ASImageGSTmp.cxx
ASIMAGEGSTMPDO    := $(ASIMAGEGSTMPDS:.cxx=.o)
ASIMAGEGSTMPDH    := $(ASIMAGEGSTMPDS:.cxx=.h)
ASIMAGEGSTMP2DS    := $(MODDIRS)/G__ASImageGSTmp2.cxx
ASIMAGEGSTMP2DO    := $(ASIMAGEGSTMP2DS:.cxx=.o)
ASIMAGEGSTMP2DH    := $(ASIMAGEGSTMP2DS:.cxx=.h)

ASIMAGEGSDEP := $(ASIMAGEGSO:.o=.d) $(ASIMAGEGSDO:.o=.d)

#LF
ASIMAGEGSTMPDEP   := $(ASIMAGEGSTMPDO:.o=.d)

ASIMAGEGSLIB := $(LPATH)/libASPluginGS.$(SOEXT)

#LF
ASIMAGEGSMAP := $(ASIMAGEGSLIB:.$(SOEXT)=.rootmap)
ASIMAGEGSDICTLIB  := $(LPATH)/libASImageGSDict.$(SOEXT)
ASIMAGEGSDICTMAP  := $(ASIMAGEGSDICTLIB:.$(SOEXT)=.rootmap)

#LF
ASIMAGEGSNM       := $(ASIMAGEGSLIB:.$(SOEXT)=.nm)

# used in the main Makefile
ALLHDRS     += $(patsubst $(MODDIRI)/%.h,include/%.h,$(ASIMAGEH))
ALLHDRS     += $(patsubst $(MODDIRI)/%.h,include/%.h,$(ASIMAGEGUIH))
ALLHDRS     += $(patsubst $(MODDIRI)/%.h,include/%.h,$(ASIMAGEGSH))
ALLLIBS     += $(ASIMAGELIB) $(ASIMAGEGUILIB) $(ASIMAGEGSLIB)
ALLMAPS     += $(ASIMAGEMAP) $(ASIMAGEGUIMAP) $(ASIMAGEGSMAP)

# include all dependency files
INCLUDEFILES += $(ASIMAGEDEP) $(ASIMAGEGUIDEP) $(ASIMAGEGSDEP)

##### local rules #####
include/%.h:    $(ASIMAGEDIRI)/%.h $(ASTEPETAG)
		cp $< $@

ifeq ($(BUILTINASIMAGE),yes)
$(ASTEPLIB):    $(ASTEPLIBA)
		cp $< $@
		@(if [ $(PLATFORM) = "macosx" ]; then \
			ranlib $@; \
		fi)

$(ASTEPETAG):   $(ASTEPSRCS)
		@(if [ -d $(ASTEPDIRS) ]; then \
		   rm -rf $(ASTEPDIRS); \
		fi; \
		echo "*** Extracting libAfterimage source ..."; \
		cd $(ASIMAGEDIRS); \
		if [ ! -d $(ASTEPVERS) ]; then \
		   gunzip -c $(ASTEPVERS).tar.gz | tar xf -; \
		   etag=`basename $(ASTEPETAG)` ; \
		   touch $$etag ; \
		fi)

$(ASTEPLIBA):   $(ASTEPETAG)
ifeq ($(PLATFORM),win32)
		@(cd $(ASTEPDIRS); \
		unset MAKEFLAGS; \
		nmake FREETYPEDIRI=-I../../../$(FREETYPEDIRI) \
                -nologo -f libAfterImage.mak \
		CFG=$(ASTEPBLD) NMAKECXXFLAGS="$(BLDCXXFLAGS) -I../../../build/win -FIw32pragma.h")
else
		@(cd $(ASTEPDIRS); \
		ACC=$(CC); \
		ACFLAGS="-O"; \
		if [ "$(CC)" = "icc" ]; then \
			ACC="icc"; \
		fi; \
		if [ "$(ARCH)" = "solarisCC5" ]; then \
			ACFLAGS += " -erroff=E_WHITE_SPACE_IN_DIRECTIVE"; \
		fi; \
		if [ "$(ARCH)" = "sgicc64" ]; then \
			ACC="gcc -mabi=64"; \
		fi; \
		if [ "$(ARCH)" = "hpuxia64acc" ]; then \
			ACC="cc +DD64 -Ae +W863"; \
			ACCALT="gcc -mlp64"; \
		fi; \
		if [ "$(ARCH)" = "macosx64" ]; then \
			ACC="gcc -m64"; \
		fi; \
		if [ "$(ARCH)" = "linuxppc64gcc" ]; then \
			ACC="gcc -m64"; \
		fi; \
		if [ "$(ARCH)" = "linuxx8664gcc" ]; then \
			ACC="gcc -m64"; \
			MMX="--enable-mmx-optimization=no"; \
		fi; \
		if [ "$(ASJPEGINCDIR)" != "" ]; then \
			JPEGINCDIR="--with-jpeg-includes=$(ASJPEGINCDIR)"; \
		fi; \
		if [ "$(ASPNGINCDIR)" != "" ]; then \
			PNGINCDIR="--with-png-includes=$(ASPNGINCDIR)"; \
		fi; \
		if [ "$(ASTIFFINCDIR)" = "--with-tiff=no" ]; then \
			TIFFINCDIR="$(ASTIFFINCDIR)"; \
		elif [ "$(ASTIFFINCDIR)" != "" ]; then \
			TIFFINCDIR="--with-tiff-includes=$(ASTIFFINCDIR)"; \
		fi; \
		if [ "$(ASGIFINCDIR)" != "" ]; then \
			GIFINCDIR="--with-gif-includes=$(ASGIFINCDIR)"; \
		fi; \
		if [ "$(FREETYPEDIRI)" != "" ]; then \
			TTFINCDIR="--with-ttf-includes=-I../../../$(FREETYPEDIRI)"; \
		fi; \
		GNUMAKE=$(MAKE) CC=$$ACC CFLAGS=$$ACFLAGS \
		./configure \
		--with-ttf $$TTFINCDIR \
		--with-afterbase=no \
		--disable-glx \
		$$MMX \
		$(ASTEPDBG) \
		--with-ungif \
		$$GIFINCDIR \
		--with-jpeg \
		$$JPEGINCDIR \
		--with-png \
		$$PNGINCDIR \
		$$TIFFINCDIR; \
		$(MAKE))
endif
endif

##### libASImage #####

#LF
$(ASIMAGELIB):  $(ASIMAGEO) $(ASIMAGETMPDO) $(ASIMAGETMP2DO) $(ASIMAGEDO) $(ASTEPDEP) $(FREETYPEDEP) $(ORDER_) $(MAINLIBS) $(ASIMAGELIBDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		"$(SOFLAGS)" libASImage.$(SOEXT) $@ "$(ASIMAGEO) $(ASIMAGETMPDO) $(ASIMAGETMP2DO) $(ASIMAGEDO)"\
		"$(ASIMAGELIBEXTRA) $(ASTEPLIB) \
                    $(FREETYPELDFLAGS) $(FREETYPELIB) \
		    $(ASEXTRALIBDIR) $(ASEXTRALIB) $(XLIBS)"

#LF
#$(ASIMAGEDICTLIB): $(ASIMAGEDO) $(ASTEPDEP) $(FREETYPEDEP) $(ORDER_) $(MAINLIBS) $(ASIMAGEDICTLIBDEP)
#		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
#		"$(SOFLAGS)" libASImageDict.$(SOEXT) $@ "$(ASIMAGEDO) $(ASIMAGETMP2DO)"\
#		"$(ASIMAGELIBEXTRA) $(ASTEPLIB) \
#                    $(FREETYPELDFLAGS) $(FREETYPELIB) \
#		    $(ASEXTRALIBDIR) $(ASEXTRALIB) $(XLIBS)"

#LF
$(ASIMAGETMPDS):   $(ASIMAGEH) $(ASIMAGEL) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(ASIMAGEH) $(ASIMAGEL)

#LF
$(ASIMAGETMP2DS):  $(ASIMAGEH) $(ASIMAGEL) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(ASIMAGEH) $(ASIMAGEL)

#LF
$(ASIMAGEDS):      $(ASIMAGEH) $(ASIMAGEL) $(ROOTCINTTMPEXE) $(ASIMAGENM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(ASIMAGENM) -. 3 -c $(ASIMAGEH) $(ASIMAGEL)
#LF
$(ASIMAGEMAP): $(RLIBMAP) $(MAKEFILEDEP) $(ASIMAGEL)
		$(RLIBMAP) -o $(ASIMAGEMAP) -l $(ASIMAGELIB) \
		-d $(ASIMAGELIBDEPM) -c $(ASIMAGEL)
#LF
$(ASIMAGENM):      $(ASIMAGEO) $(ASIMAGETMPDO) $(ASIMAGETMP2DO)
		@echo "Generating symbols file $@..."
		nm -g -p --defined-only $(ASIMAGETMPDO) | awk '{printf("%s\n", $$3)'} > $(ASIMAGENM)
		nm -g -p --defined-only $(ASIMAGETMP2DO) | awk '{printf("%s\n", $$3)'} >> $(ASIMAGENM)
		nm -g -p --defined-only $(ASIMAGEO) | awk '{printf("%s\n", $$3)'} > $(ASIMAGENM)

##### libASImageGui #####

#LF
$(ASIMAGEGUILIB):  $(ASIMAGEGUIO) $(ASIMAGEGUITMPDO) $(ASIMAGEGUITMP2DO) $(ASIMAGEGUIDO) $(ASTEPDEP) $(FREETYPEDEP) \
                   $(ORDER_) $(MAINLIBS) $(ASIMAGEGUILIBDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		"$(SOFLAGS)" libASImageGui.$(SOEXT) $@ "$(ASIMAGEGUIO) $(ASIMAGEGUITMPDO) $(ASIMAGEGUITMP2DO) $(ASIMAGEGUIDO)"\
		"$(ASIMAGEGUILIBEXTRA) $(ASTEPLIB) \
                    $(FREETYPELDFLAGS) $(FREETYPELIB) \
		    $(ASEXTRALIBDIR) $(ASEXTRALIB) $(XLIBS)"

#LF
#$(ASIMAGEGUIDICTLIB): $(ASIMAGEGUIDO) $(ASTEPDEP) $(FREETYPEDEP) $(ORDER_) $(MAINLIBS) $(ASIMAGEGUIDICTLIBDEP)
#		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
#		"$(SOFLAGS)" libASImageGuiDict.$(SOEXT) $@ "$(ASIMAGEGUIDO) $(ASIMAGEGUITMP2DO)"\
#		"$(ASIMAGEGUILIBEXTRA) $(ASTEPLIB) \
#                    $(FREETYPELDFLAGS) $(FREETYPELIB) \
#		    $(ASEXTRALIBDIR) $(ASEXTRALIB) $(XLIBS)"

#LF
$(ASIMAGEGUITMPDS):   $(ASIMAGEGUIH) $(ASIMAGEGUIL) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(ASIMAGEGUIH) $(ASIMAGEGUIL)

#LF
$(ASIMAGEGUITMP2DS):  $(ASIMAGEGUIH) $(ASIMAGEGUIL) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(ASIMAGEGUIH) $(ASIMAGEGUIL)

#LF
$(ASIMAGEGUIDS):      $(ASIMAGEGUIH) $(ASIMAGEGUIL) $(ROOTCINTTMPEXE) $(ASIMAGEGUINM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(ASIMAGEGUINM) -. 3 -c $(ASIMAGEGUIH) $(ASIMAGEGUIL)
#LF
$(ASIMAGEGUIMAP): $(RLIBMAP) $(MAKEFILEDEP) $(ASIMAGEGUIL)
		$(RLIBMAP) -o $(ASIMAGEGUIMAP) -l $(ASIMAGEGUILIB) \
		-d $(ASIMAGEGUILIBDEPM) -c $(ASIMAGEGUIL)
#LF
$(ASIMAGEGUINM):      $(ASIMAGEGUIO) $(ASIMAGEGUITMPDO) $(ASIMAGEGUITMP2DO) 
		@echo "Generating symbols file $@..."
		nm -g -p --defined-only $(ASIMAGEGUITMPDO) | awk '{printf("%s\n", $$3)'} > $(ASIMAGEGUINM)
		nm -g -p --defined-only $(ASIMAGEGUITMP2DO) | awk '{printf("%s\n", $$3)'} >> $(ASIMAGEGUINM)
		nm -g -p --defined-only $(ASIMAGEGUIO) | awk '{printf("%s\n", $$3)'} >> $(ASIMAGEGUINM)

##### libASPluginGS #####

#LF
$(ASIMAGEGSLIB):  $(ASIMAGEGSO) $(ASIMAGEGSTMPDO) $(ASIMAGEGSTMP2DO) $(ASIMAGEGSDO) $(ASTEPDEP) $(FREETYPEDEP) \
                   $(ORDER_) $(MAINLIBS) $(ASIMAGEGSLIBDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		"$(SOFLAGS)" libASImageGS.$(SOEXT) $@ "$(ASIMAGEGSO) $(ASIMAGEGSTMPDO) $(ASIMAGEGSTMP2DO) $(ASIMAGEGSDO)"\
		"$(ASIMAGEGSLIBEXTRA) $(ASTEPLIB) \
                    $(FREETYPELDFLAGS) $(FREETYPELIB) \
		    $(ASEXTRALIBDIR) $(ASEXTRALIB) $(XLIBS)"

#LF
#$(ASIMAGEGSDICTLIB): $(ASIMAGEGSDO) $(ASTEPDEP) $(FREETYPEDEP) $(ORDER_) $(MAINLIBS) $(ASIMAGEGSDICTLIBDEP)
#		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
#		"$(SOFLAGS)" libASImageGSDict.$(SOEXT) $@ "$(ASIMAGEGSDO) $(ASIMAGEGSTMP2DO)"\
#		"$(ASIMAGEGSLIBEXTRA) $(ASTEPLIB) \
#                    $(FREETYPELDFLAGS) $(FREETYPELIB) \
#		    $(ASEXTRALIBDIR) $(ASEXTRALIB) $(XLIBS)"

#LF
$(ASIMAGEGSTMPDS):   $(ASIMAGEGSH) $(ASIMAGEGSL) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(ASIMAGEGSH) $(ASIMAGEGSL)

#LF
$(ASIMAGEGSTMP2DS):  $(ASIMAGEGSH) $(ASIMAGEGSL) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(ASIMAGEGSH) $(ASIMAGEGSL)

#LF
$(ASIMAGEGSDS):      $(ASIMAGEGSH) $(ASIMAGEGSL) $(ROOTCINTTMPEXE) $(ASIMAGEGSNM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(ASIMAGEGSNM) -. 3 -c $(ASIMAGEGSH) $(ASIMAGEGSL)
#LF
$(ASIMAGEGSMAP): $(RLIBMAP) $(MAKEFILEDEP) $(ASIMAGEGSL)
		$(RLIBMAP) -o $(ASIMAGEGSMAP) -l $(ASIMAGEGSLIB) \
		-d $(ASIMAGEGSLIBDEPM) -c $(ASIMAGEGSL)
#LF
$(ASIMAGEGSNM):      $(ASIMAGEGSO) $(ASIMAGEGSTMPDO) $(ASIMAGEGSTMP2DO)
		@echo "Generating symbols file $@..."
		nm -g -p --defined-only $(ASIMAGEGSTMPDO) | awk '{printf("%s\n", $$3)'} > $(ASIMAGEGSNM)
		nm -g -p --defined-only $(ASIMAGEGSTMP2DO) | awk '{printf("%s\n", $$3)'} >> $(ASIMAGEGSNM)
		nm -g -p --defined-only $(ASIMAGEGSO) | awk '{printf("%s\n", $$3)'} >> $(ASIMAGEGSNM)

all-asimage:    $(ASIMAGELIB) $(ASIMAGEGUILIB) $(ASIMAGEGSLIB) \
		$(ASIMAGEMAP) $(ASIMAGEGUIMAP) $(ASIMAGEGSMAP)

clean-asimage:
		@rm -f $(ASIMAGEO) $(ASIMAGEDO) $(ASIMAGEGUIO) $(ASIMAGEGUIDO) \
		   $(ASIMAGEGSO) $(ASIMAGEGSDO)
ifeq ($(BUILTINASIMAGE),yes)
ifeq ($(PLATFORM),win32)
		-@(if [ -d $(ASTEPDIRS) ]; then \
			cd $(ASTEPDIRS); \
			unset MAKEFLAGS; \
			nmake -nologo -f libAfterImage.mak clean \
			CFG=$(ASTEPBLD); \
		fi)
else
		-@(if [ -d $(ASTEPDIRS) ]; then \
			cd $(ASTEPDIRS); \
			$(MAKE) clean; \
		fi)
endif
endif

clean::         clean-asimage clean-pds-asimage clean-pds-asimagegui clean-pds-asimagegs

clean-pds-asimage:	
		rm -f $(ASIMAGETMPDS) $(ASIMAGETMPDO) $(ASIMAGETMPDH) \
		$(ASIMAGETMPDEP) $(ASIMAGETMP2DS) $(ASIMAGETMP2DO) $(ASIMAGETMP2DH) $(ASIMAGENM)

clean-pds-asimagegui:	
		rm -f $(ASIMAGEGUITMPDS) $(ASIMAGEGUITMPDO) $(ASIMAGEGUITMPDH) \
		$(ASIMAGEGUITMPDEP) $(ASIMAGEGUITMP2DS) $(ASIMAGEGUITMP2DO) $(ASIMAGEGUITMP2DH) $(ASIMAGEGUINM)

clean-pds-asimagegs:	
		rm -f $(ASIMAGEGSTMPDS) $(ASIMAGEGSTMPDO) $(ASIMAGEGSTMPDH) \
		$(ASIMAGEGSTMPDEP) $(ASIMAGEGSTMP2DS) $(ASIMAGEGSTMP2DO) $(ASIMAGEGSTMP2DH) $(ASIMAGEGSNM)

distclean-asimage: clean-asimage
		@rm -f $(ASIMAGEDEP) $(ASIMAGEDS) $(ASIMAGEDH) \
		   $(ASIMAGELIB) $(ASIMAGEMAP) \
		   $(ASIMAGEGUIDEP) $(ASIMAGEGUIDS) $(ASIMAGEGUIDH) \
		   $(ASIMAGEGUILIB) $(ASIMAGEGUIMAP) \
		   $(ASIMAGEGSDEP) $(ASIMAGEGSDS) $(ASIMAGEGSDH) \
		   $(ASIMAGEGSLIB) $(ASIMAGEGSMAP)
ifeq ($(BUILTINASIMAGE),yes)
		@rm -f $(ASTEPLIB) $(ASTEPETAG)
		@rm -rf $(ASIMAGEDIRS)/$(ASTEPVERS)
endif

distclean::     distclean-asimage

##### extra rules ######
$(ASIMAGEO): $(ASTEPLIB) $(FREETYPEDEP)
$(ASIMAGEO): CXXFLAGS += $(FREETYPEINC) $(ASTEPDIRI)

$(ASIMAGEGUIO) $(ASIMAGEGSO) $(ASIMAGEGSDO) $(ASIMAGEGUIDO) $(ASIMAGEDO): \
  $(ASTEPLIB)
$(ASIMAGEGUIO) $(ASIMAGEGSO) $(ASIMAGEGSDO) $(ASIMAGEGUIDO) $(ASIMAGEDO): \
  CXXFLAGS += $(ASTEPDIRI)
