# Module.mk for x11ttf module
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: Fons Rademakers, 29/2/2000

MODDIR       := x11ttf
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

X11TTFDIR    := $(MODDIR)
X11TTFDIRS   := $(X11TTFDIR)/src
X11TTFDIRI   := $(X11TTFDIR)/inc

##### libGX11TTF #####
X11TTFL      := $(MODDIRI)/LinkDef.h
X11TTFDS     := $(MODDIRS)/G__X11TTF.cxx
X11TTFDO     := $(X11TTFDS:.cxx=.o)
X11TTFDH     := $(X11TTFDS:.cxx=.h)

#LF
X11TTFTMPDS    := $(MODDIRS)/G__X11TTFTmp.cxx
X11TTFTMPDO    := $(X11TTFTMPDS:.cxx=.o)
X11TTFTMPDH    := $(X11TTFTMPDS:.cxx=.h)
X11TTFTMP2DS    := $(MODDIRS)/G__X11TTFTmp2.cxx
X11TTFTMP2DO    := $(X11TTFTMP2DS:.cxx=.o)
X11TTFTMP2DH    := $(X11TTFTMP2DS:.cxx=.h)

X11TTFH      := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
X11TTFS      := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
X11TTFO      := $(X11TTFS:.cxx=.o)

X11TTFDEP    := $(X11TTFO:.o=.d) $(X11TTFDO:.o=.d)

#LF
X11TTFTMPDEP   := $(X11TMPDO:.o=.d)

X11TTFLIB    := $(LPATH)/libGX11TTF.$(SOEXT)
X11TTFMAP    := $(X11TTFLIB:.$(SOEXT)=.rootmap)

#LF
X11TTFNM       := $(X11TTFLIB:.$(SOEXT)=.nm)

# used in the main Makefile
ALLHDRS     += $(patsubst $(MODDIRI)/%.h,include/%.h,$(X11TTFH))
ALLLIBS     += $(X11TTFLIB)
ALLMAPS     += $(X11TTFMAP)

#ifeq ($(XFTLIB),yes)
XLIBS       += $(X11LIBDIR) -lXft
#endif

# include all dependency files
INCLUDEFILES += $(X11TTFDEP)

##### local rules #####
include/%.h:    $(X11TTFDIRI)/%.h
		cp $< $@

#LF
$(X11TTFLIB):     $(X11TTFO) $(X11TTFTMPDO) $(X11TTFTMP2DO) $(X11TTFDO) $(ORDER_) $(MAINLIBS) $(X11TTFLIBDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		"$(SOFLAGS)" libGX11TTF.$(SOEXT) $@ "$(X11TTFO) $(X11TTFTMPDO) $(X11TTFTMP2DO) $(X11TTFDO)"\
		   "$(FREETYPELDFLAGS) $(FREETYPELIB) \
		    $(X11TTFLIBEXTRA) $(XLIBS)"

#LF
#$(X11TTFDICTLIB): $(X11TTFDO) $(ORDER_) $(MAINLIBS) $(X11TTFDICTLIBDEP)
#		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
#		"$(SOFLAGS)" libGX11TTFDict.$(SOEXT) $@ "$(X11TTFDO) $(X11TTFTMP2DO)"\
#		"$(X11TTFDICTLIBEXTRA)"

#LF
$(X11TTFTMPDS):   $(X11TTFH) $(X11TTFL) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(FREETYPEINC) $(X11TTFH) $(X11TTFL)

#LF
$(X11TTFTMP2DS):  $(X11TTFH) $(X11TTFL) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(FREETYPEINC) $(X11TTFH) $(X11TTFL)

#LF
$(X11TTFDS):      $(X11TTFH) $(X11TTFL) $(ROOTCINTTMPEXE) $(X11TTFNM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(X11TTFNM) -. 3 -c $(FREETYPEINC) $(X11TTFH) $(X11TTFL)

#LF
$(X11TTFMAP):      $(RLIBMAP) $(MAKEFILEDEP) $(X11TTFL)
		$(RLIBMAP) -o $(X11TTFMAP) -l $(X11TTFLIB) \
		-d $(X11TTFLIBDEPM) -c $(X11TTFL)
#LF
$(X11TTFNM):      $(X11TTFO) $(X11TTFTMPDO) $(X11TTFTMP2DO)
		@echo "Generating symbols file $@..."
		nm -g -p --defined-only $(X11TTFTMPDO) | awk '{printf("%s\n", $$3)'} > $(X11TTFNM)
		nm -g -p --defined-only $(X11TTFTMP2DO) | awk '{printf("%s\n", $$3)'} >> $(X11TTFNM)
		nm -g -p --defined-only $(X11TTFO) | awk '{printf("%s\n", $$3)'} >> $(X11TTFNM)

all-x11ttf:     $(X11TTFLIB) $(X11TTFMAP)

clean-x11ttf:
		@rm -f $(X11TTFO) $(X11TTFDO)

clean::         clean-x11ttf clean-pds-x11ttf

#LF
clean-pds-x11ttf:	
		rm -f $(X11TTFTMPDS) $(X11TTFTMPDO) $(X11TTFTMPDH) \
		$(X11TTFTMPDEP) $(X11TTFTMP2DS) $(X11TTFTMP2DO) $(X11TTFTMP2DH) $(X11TTFNM)

distclean-x11ttf: clean-x11ttf
		@rm -f $(X11TTFDEP) $(X11TTFDS) $(X11TTFDH) $(X11TTFLIB) $(X11TTFMAP)

distclean::     distclean-x11ttf

##### extra rules ######
$(X11TTFO) $(X11TTFTMPDO) $(X11TTFTMP2DO) $(X11TTFDO): $(FREETYPEDEP)
ifeq ($(PLATFORM),macosx)
$(X11TTFO) $(X11TTFTMPDO) $(X11TTFTMP2DO) $(X11TTFDO): CXXFLAGS += -I/usr/X11R6/include $(FREETYPEINC)
else
$(X11TTFO) $(X11TTFTMPDO) $(X11TTFTMP2DO) $(X11TTFDO): CXXFLAGS += $(FREETYPEINC)
endif
