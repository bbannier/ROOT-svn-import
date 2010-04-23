# Module.mk for mathmore module
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: Fons Rademakers, 29/2/2000

MODNAME      := unuran
MODDIR       := math/$(MODNAME)
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

UNURANDIR    := $(MODDIR)
UNURANDIRS   := $(UNURANDIR)/src
UNURANDIRI   := $(UNURANDIR)/inc

UNRVERS      := unuran-1.7.0-root

UNRSRCS      := $(MODDIRS)/$(UNRVERS).tar.gz
UNRDIRS      := $(MODDIRS)/$(UNRVERS)
UNURANETAG   := $(UNURANDIRS)/headers.d
UNRCFG       := $(UNURANDIRS)/$(UNRVERS)/config.h

UNRTARCONTENT:=$(subst $(UNRVERS),$(UNRDIRS),$(shell cd $(UNURANDIRS); gunzip -c $(UNRVERS).tar.gz | tar tf -))
UNRS         := $(filter %.c,\
                $(filter $(UNRDIRS)/src/utils/%,$(UNRTARCONTENT))\
                $(filter $(UNRDIRS)/src/methods/%,$(UNRTARCONTENT)) \
                $(filter $(UNRDIRS)/src/specfunct/%,$(UNRTARCONTENT)) \
                $(filter $(UNRDIRS)/src/distr/%,$(UNRTARCONTENT)) \
                $(filter $(UNRDIRS)/src/distributions/%,$(UNRTARCONTENT)) \
                $(filter $(UNRDIRS)/src/parser/%,$(UNRTARCONTENT)) \
                $(filter $(UNRDIRS)/src/tests/%,$(UNRTARCONTENT)) \
                $(filter $(UNRDIRS)/src/uniform/%,$(UNRTARCONTENT)) \
                $(filter $(UNRDIRS)/src/urng/%,$(UNRTARCONTENT)))
UNRO         := $(UNRS:.c=.o)

ifeq ($(PLATFORM),win32)
UNRLIBS      := $(MODDIRS)/$(UNRVERS)/src/.libs/libunuran.lib
else
UNRLIBS      := $(MODDIRS)/$(UNRVERS)/src/.libs/libunuran.a
endif

UNRFLAGS     :=  -I$(MODDIRS)/$(UNRVERS)/src

##### libUnuran #####
UNURANL      := $(MODDIRI)/LinkDef.h
UNURANDS     := $(MODDIRS)/G__Unuran.cxx
UNURANDO     := $(UNURANDS:.cxx=.o)
UNURANDH     := $(UNURANDS:.cxx=.h)
UNURANDH1    := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))

UNURANH      := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
UNURANS      := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))

UNURANO      := $(UNURANS:.cxx=.o)

UNURANDEP    := $(UNURANO:.o=.d) $(UNURANDO:.o=.d)

UNURANLIB    := $(LPATH)/libUnuran.$(SOEXT)
UNURANMAP    := $(UNURANLIB:.$(SOEXT)=.rootmap)

# used in the main Makefile
ALLHDRS      += $(patsubst $(MODDIRI)/%.h,include/%.h,$(UNURANH))
ALLLIBS      += $(UNURANLIB)
ALLMAPS      += $(UNURANMAP)

# include all dependency files
INCLUDEFILES += $(UNURANDEP)

##### local rules #####
.PHONY:         all-$(MODNAME) clean-$(MODNAME) distclean-$(MODNAME)

include/%.h: 	$(UNURANDIRI)/%.h $(UNURANETAG)
		cp $< $@

$(UNURANDEP):   $(UNRCFG)
$(UNRS):        $(UNURANETAG)

$(UNURANETAG):	$(UNRSRCS)
		@echo "**** untarring UNURAN !!!!"
		@(if  [ -d $(UNRDIRS) ]; then \
		   rm -rf $(UNRDIRS); \
		fi; \
		cd $(UNURANDIRS); \
		rm -rf unuran*root; \
		if [ ! -d $(UNRVERS) ]; then \
		   gunzip -c $(UNRVERS).tar.gz | tar xf -; \
		   etag=`basename $(UNURANETAG)` ; \
		   touch $$etag ; \
		fi); 

#configure unuran (required for creating the config.h used by unuran source files)
$(UNRCFG):	$(UNURANETAG)
		@(cd $(UNURANDIRS)/$(UNRVERS) ; \
		ACC=$(CC); \
		if [ "$(CC)" = "icc" ]; then \
			ACC="icc"; \
		fi; \
		if [ "$(ARCH)" = "sgicc64" ]; then \
			ACC="gcc -mabi=64"; \
		fi; \
		if [ "$(ARCH)" = "hpuxia64acc" ]; then \
			ACC="cc +DD64 -Ae"; \
		fi; \
		if [ "$(ARCH)" = "linuxppc64gcc" ]; then \
			ACC="gcc -m64 -fPIC"; \
		fi; \
		if [ "$(ARCH)" = "linuxx8664gcc" ]; then \
			ACC="gcc"; \
			ACFLAGS="-m64 -fPIC"; \
		fi; \
		if [ "$(ARCH)" = "linuxicc" ]; then \
			ACC="icc"; \
			ACFLAGS="-m32"; \
		fi; \
		if [ "$(ARCH)" = "linuxx8664icc" ]; then \
			ACC="icc"; \
			ACFLAGS="-m64"; \
		fi; \
		if [ "$(ARCH)" = "win32" ]; then \
			export LD="cl"; \
			ACC="cl.exe"; \
			ACFLAGS="-MD -G5 -GX"; \
		fi; \
		GNUMAKE=$(MAKE) ./configure  CC="$$ACC"  \
		CFLAGS="$$ACFLAGS");

$(UNURANLIB):   $(UNRCFG) $(UNRO) $(UNURANO) $(UNURANDO) $(ORDER_) \
                $(MAINLIBS) $(UNURANLIBDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)"  \
		   "$(SOFLAGS)" libUnuran.$(SOEXT) $@     \
		   "$(UNURANO) $(UNURANDO)"             \
		   "$(UNURANLIBEXTRA) $(UNRO)"

$(UNURANDS):    $(UNRINIT) $(UNURANDH1) $(UNURANL) $(ROOTCINTTMPDEP)
		@echo "Generating dictionary $@..."
		$(ROOTCINTTMP) -f $@ -c $(UNRFLAGS) $(UNURANDH1) $(UNURANL)

$(UNURANMAP):   $(RLIBMAP) $(MAKEFILEDEP) $(UNURANL) $(UNURANLINC)
		$(RLIBMAP) -o $(UNURANMAP) -l $(UNURANLIB) \
		   -d $(UNURANLIBDEPM) -c $(UNURANL) $(UNURANLINC)

all-$(MODNAME): $(UNURANLIB) $(UNURANMAP)

clean-$(MODNAME):
		@rm -f $(UNURANO) $(UNURANDO)
		-@(if [ -d $(UNRDIRS) ]; then \
			cd $(UNRDIRS); \
			$(MAKE) clean; \
		fi)

clean::         clean-$(MODNAME)

distclean-$(MODNAME): clean-$(MODNAME)
		@rm -f $(UNURANO) $(UNURANDO) $(UNURANETAG) $(UNURANDEP) \
		   $(UNURANDS) $(UNURANDH) $(UNURANLIB) $(UNURANMAP)
		@mv $(UNRSRCS) $(UNURANDIRS)/-$(UNRVERS).tar.gz
ifeq ($(UNURKEEP),yes)
		@mv $(UNURANDIRS)/$(UNRVERS) $(UNURANDIRS)/$(UNRVERS).keep
endif
		@rm -rf $(UNURANDIRS)/$(UNRVERS)
ifeq ($(UNURKEEP),yes)
		@mv $(UNURANDIRS)/$(UNRVERS).keep $(UNURANDIRS)/$(UNRVERS)
endif
		@mv $(UNURANDIRS)/-$(UNRVERS).tar.gz $(UNRSRCS)

distclean::     distclean-$(MODNAME)

##### extra rules ######

$(UNURANO): CXXFLAGS += $(UNRFLAGS)

ifeq ($(PLATFORM),win32)
$(UNRO): CFLAGS := $(filter-out -FIsehmap.h,$(filter-out -Iinclude,$(CFLAGS) -I$(UNRDIRS) -I$(UNRDIRS)/src/ -I$(UNRDIRS)/src/utils -DHAVE_CONFIG_H))
else
$(UNRO): CFLAGS := $(filter-out -Iinclude,$(CFLAGS) -I$(UNRDIRS) -I$(UNRDIRS)/src/ -I$(UNRDIRS)/src/utils -DHAVE_CONFIG_H)
endif
ifeq ($(CC),icc)
$(UNRO): CFLAGS += -mp
endif
