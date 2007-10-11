# Module.mk for gl module
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: Fons Rademakers, 29/2/2000

MODDIR       := gl
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

GLDIR        := $(MODDIR)
GLDIRS       := $(GLDIR)/src
GLDIRI       := $(GLDIR)/inc

##### libRGL #####
GLL          := $(MODDIRI)/LinkDef.h
GLDS         := $(MODDIRS)/G__GL.cxx
GLDO         := $(GLDS:.cxx=.o)
GLDH         := $(GLDS:.cxx=.h)

#LF
GLTMPDS    := $(MODDIRS)/G__GLTmp.cxx
GLTMPDO    := $(GLTMPDS:.cxx=.o)
GLTMPDH    := $(GLTMPDS:.cxx=.h)
GLTMP2DS    := $(MODDIRS)/G__GLTmp2.cxx
GLTMP2DO    := $(GLTMP2DS:.cxx=.o)
GLTMP2DH    := $(GLTMP2DS:.cxx=.h)

GLH          := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
GLS          := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
GLS1         := $(wildcard $(MODDIRS)/*.c)

# Excluded from win32 builds
ifeq ($(ARCH),win32)
GLS          := $(filter-out $(MODDIRS)/TX11GL.cxx, $(GLS))
GLH          := $(filter-out $(MODDIRI)/TX11GL.h, $(GLH))
endif

# Excluded from rootcint
GLH1         := $(MODDIRI)/gl2ps.h $(MODDIRI)/CsgOps.h $(MODDIRI)/TGLKernel.h \
                $(MODDIRI)/TGLIncludes.h $(MODDIRI)/TRootGLU.h \
                $(MODDIRI)/TRootGLX.h $(MODDIRI)/TGLContextPrivate.h

# Used by rootcint
GLH2         := $(filter-out $(GLH1), $(GLH))

ifneq ($(OPENGLLIB),)
GLLIBS       := $(OPENGLLIBDIR) $(OPENGLULIB) $(OPENGLLIB) \
                $(X11LIBDIR) -lX11 -lm
endif
ifeq ($(ARCH),win32)
GLLIBS       := opengl32.lib glu32.lib
endif

GLO          := $(GLS:.cxx=.o)
GLO1         := $(GLS1:.c=.o)

GLDEP        := $(GLO:.o=.d) $(GLDO:.o=.d) $(GLO1:.o=.d)

#LF
GLTMPDEP   := $(GLTMPDO:.o=.d)

GLLIB        := $(LPATH)/libRGL.$(SOEXT)

#LF
GLMAP        := $(GLLIB:.$(SOEXT)=.rootmap)
GLDICTLIB  := $(LPATH)/libRGLDict.$(SOEXT)
GLDICTMAP  := $(GLDICTLIB:.$(SOEXT)=.rootmap)

#LF
GLNM       := $(GLLIB:.$(SOEXT)=.nm)

# used in the main Makefile
ALLHDRS      += $(patsubst $(MODDIRI)/%.h,include/%.h,$(GLH))
ALLLIBS      += $(GLLIB)
ALLMAPS      += $(GLMAP)

# include all dependency files
INCLUDEFILES += $(GLDEP)

##### local rules #####
include/%.h:    $(GLDIRI)/%.h
		cp $< $@

#LF
$(GLLIB):     $(GLO) $(GLO1) $(GLTMPDO) $(GLTMP2DO) $(GLDO) $(ORDER_) $(MAINLIBS) $(GLLIBDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		"$(SOFLAGS)" libRGL.$(SOEXT) $@ "$(GLO) $(GLO1) $(GLTMPDO) $(GLTMP2DO) $(GLDO)" \
		"$(GLLIBEXTRA) $(GLLIBS)"

#LF
#$(GLDICTLIB): $(GLDO) $(ORDER_) $(MAINLIBS) $(GLDICTLIBDEP)
#		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
#		"$(SOFLAGS)" libRGLDict.$(SOEXT) $@ "$(GLDO) $(GLTMP2DO)"\
#		"$(GLDICTLIBEXTRA) $(GLLIBS)"

#LF
$(GLTMPDS):   $(GLH2) $(GLL) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(GLH2) $(GLL)

#LF
$(GLTMP2DS):  $(GLH2) $(GLL) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(GLH2) $(GLL)

#LF
$(GLDS):      $(GLH2) $(GLL) $(ROOTCINTTMPEXE) $(GLNM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(GLNM) -. 3 -c $(GLH2) $(GLL)
#LF
$(GLMAP): $(RLIBMAP) $(MAKEFILEDEP) $(GLL)
		$(RLIBMAP) -o $(GLMAP) -l $(GLLIB) \
		-d $(GLLIBDEPM) -c $(GLL)
#LF
$(GLNM):      $(GLO) $(GLTMPDO) $(GLTMP2DO)
		@echo "Generating symbols file $@..."
		nm -g -p --defined-only $(GLTMPDO) | awk '{printf("%s\n", $$3)'} > $(GLNM)
		nm -g -p --defined-only $(GLTMP2DO) | awk '{printf("%s\n", $$3)'} >> $(GLNM)
		nm -g -p --defined-only $(GLO) | awk '{printf("%s\n", $$3)'} >> $(GLNM)

all-gl:         $(GLLIB) $(GLMAP)

clean-gl:
		@rm -f $(GLO) $(GLO1) $(GLDO)

clean::         clean-gl clean-pds-gl

#LF
clean-pds-gl:	
		rm -f $(GLTMPDS) $(GLTMPDO) $(GLTMPDH) \
		$(GLTMPDEP) $(GLTMP2DS) $(GLTMP2DO) $(GLTMP2DH) $(GLNM)

distclean-gl:   clean-gl
		@rm -f $(GLDEP) $(GLLIB) $(GLDS) $(GLDH) $(GLMAP)

distclean::     distclean-gl

##### extra rules ######
ifeq ($(ARCH),win32)
$(GLO) $(GLDO): CXXFLAGS += $(OPENGLINCDIR:%=-I%) -I$(WIN32GDKDIR)/gdk/src \
                            $(GDKDIRI:%=-I%) $(GLIBDIRI:%=-I%)
else
$(GLO) $(GLDO): CXXFLAGS += $(OPENGLINCDIR:%=-I%)
endif

$(GLDIRS)/gl2ps.o: CFLAGS += $(OPENGLINCDIR:%=-I%)

gl/src/TGLText.o: \
                $(FREETYPEDEP)
gl/src/TGLText.o: \
                CXXFLAGS += $(FREETYPEINC)
