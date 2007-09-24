# Module.mk for x11 module
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: Fons Rademakers, 29/2/2000

MODDIR       := x11
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

X11DIR       := $(MODDIR)
X11DIRS      := $(X11DIR)/src
X11DIRI      := $(X11DIR)/inc

##### libGX11 #####
X11L         := $(MODDIRI)/LinkDef.h
X11DS        := $(MODDIRS)/G__X11.cxx
X11DO        := $(X11DS:.cxx=.o)
X11DH        := $(X11DS:.cxx=.h)

#LF
X11TMPDS    := $(MODDIRS)/G__X11Tmp.cxx
X11TMPDO    := $(X11TMPDS:.cxx=.o)
X11TMPDH    := $(X11TMPDS:.cxx=.h)
X11TMP2DS    := $(MODDIRS)/G__X11Tmp2.cxx
X11TMP2DO    := $(X11TMP2DS:.cxx=.o)
X11TMP2DH    := $(X11TMP2DS:.cxx=.h)

X11H1        := $(wildcard $(MODDIRI)/T*.h)
X11H         := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
X11S1        := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
X11S2        := $(wildcard $(MODDIRS)/*.c)
X11O         := $(X11S1:.cxx=.o) $(X11S2:.c=.o)

X11DEP       := $(X11O:.o=.d) $(X11DO:.o=.d)

#LF
X11TMPDEP   := $(X11TMPDO:.o=.d)

X11LIB       := $(LPATH)/libGX11.$(SOEXT)

#LF
X11MAP       := $(X11LIB:.$(SOEXT)=.rootmap)
#X11DICTLIB  := $(LPATH)/libGX11Dict.$(SOEXT)
#X11DICTMAP  := $(X11DICTLIB:.$(SOEXT)=.rootmap)

#LF
X11NM       := $(X11LIB:.$(SOEXT)=.nm)

# used in the main Makefile
ALLHDRS     += $(patsubst $(MODDIRI)/%.h,include/%.h,$(X11H))
ALLLIBS     += $(X11LIB)
ALLMAPS     += $(X11MAP)

# include all dependency files
INCLUDEFILES += $(X11DEP)

##### local rules #####
include/%.h:    $(X11DIRI)/%.h
		cp $< $@

#LF
$(X11LIB):     $(X11O) $(X11TMPDO) $(X11TMP2DO) $(X11DO) $(ORDER_) $(MAINLIBS) 
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		"$(SOFLAGS)" libGX11.$(SOEXT) $@ "$(X11O) $(X11TMPDO) $(X11TMP2DO) $(X11DO)"\
		"$(X11LIBEXTRA) $(XLIBS)"

#LF
#$(X11DICTLIB): $(X11DO) $(ORDER_) $(MAINLIBS) $(X11DICTLIBDEP)
#		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
#		"$(SOFLAGS)" libGX11Dict.$(SOEXT) $@ "$(X11DO) $(X11TMP2DO)"\
#		"$(X11DICTLIBEXTRA)"

#LF
$(X11TMPDS):   $(X11H1) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(X11H1) $(X11L1)

#LF
$(X11TMP2DS):  $(X11H1) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(X11H1) $(X11L1)

#LF
$(X11DS):      $(X11H1) $(ROOTCINTTMPEXE) $(X11NM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(X11NM) -. 3 -c $(X11H1) $(X11L1)

#LF
$(X11MAP):      $(RLIBMAP) $(MAKEFILEDEP) $(X11L)
		$(RLIBMAP) -o $(X11MAP) -l $(X11LIB) \
		-d $(X11LIBDEPM) -c $(X11L)
#LF
$(X11NM):      $(X11O) $(X11TMPDO) $(X11TMP2DO)
		@echo "Generating symbols file $@..."
		nm -g -p --defined-only $(X11TMPDO) | awk '{printf("%s\n", $$3)'} > $(X11NM)
		nm -g -p --defined-only $(X11TMP2DO) | awk '{printf("%s\n", $$3)'} >> $(X11NM)
		nm -g -p --defined-only $(X11O) | awk '{printf("%s\n", $$3)'} >> $(X11NM)

all-x11:        $(X11LIB) $(X11MAP)

clean-x11:
		@rm -f $(X11O) $(X11DO)

clean::         clean-x11 clean-pds-x11

#LF
clean-pds-x11:	
		rm -f $(X11TMPDS) $(X11TMPDO) $(X11TMPDH) \
		$(X11TMPDEP) $(X11TMP2DS) $(X11TMP2DO) $(X11TMP2DH) $(X11NM)

distclean-x11:  clean-x11
		@rm -f $(X11DEP) $(X11DS) $(X11DH) $(X11LIB) $(X11MAP)

distclean::     distclean-x11
