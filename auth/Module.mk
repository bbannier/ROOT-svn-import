# Module.mk for auth module
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: G. Ganis, 7/2005

MODDIR       := auth
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

AUTHDIR      := $(MODDIR)
AUTHDIRS     := $(AUTHDIR)/src
AUTHDIRI     := $(AUTHDIR)/inc

##### libRootAuth #####
RAUTHL       := $(MODDIRI)/LinkDefRoot.h
RAUTHDS      := $(MODDIRS)/G__RootAuth.cxx
RAUTHDO      := $(RAUTHDS:.cxx=.o)
RAUTHDH      := $(RAUTHDS:.cxx=.h)

RAUTHH       := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
RAUTHS       := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))

#LF
RAUTHTMPDS    := $(MODDIRS)/G__RootAuthTmp.cxx
RAUTHTMPDO    := $(RAUTHTMPDS:.cxx=.o)
RAUTHTMPDH    := $(RAUTHTMPDS:.cxx=.h)
RAUTHTMP2DS   := $(MODDIRS)/G__RootAuthTmp2.cxx
RAUTHTMP2DO   := $(RAUTHTMP2DS:.cxx=.o)
RAUTHTMP2DH   := $(RAUTHTMP2DS:.cxx=.h)

RAUTHH       := $(filter-out $(MODDIRI)/DaemonUtils.h,$(RAUTHH))
RAUTHS       := $(filter-out $(MODDIRS)/DaemonUtils.cxx,$(RAUTHS))
RAUTHH       := $(filter-out $(MODDIRI)/AFSAuth.h,$(RAUTHH))
RAUTHH       := $(filter-out $(MODDIRI)/AFSAuthTypes.h,$(RAUTHH))
RAUTHS       := $(filter-out $(MODDIRS)/AFSAuth.cxx,$(RAUTHS))
RAUTHH       := $(filter-out $(MODDIRI)/TAFS.h,$(RAUTHH))
RAUTHS       := $(filter-out $(MODDIRS)/TAFS.cxx,$(RAUTHS))

RAUTHO       := $(RAUTHS:.cxx=.o)

#### for libSrvAuth (built in rpdutils/Module.mk) ####
DAEMONUTILSO := $(MODDIRS)/DaemonUtils.o

RAUTHDEP     := $(RAUTHO:.o=.d) $(RAUTHDO:.o=.d) $(DAEMONUTILSO:.o=.d)

#LF
RAUTHTMPDEP  := $(RAUTHTMPDO:.o=.d)

RAUTHLIB     := $(LPATH)/libRootAuth.$(SOEXT)
RAUTHMAP     := $(RAUTHLIB:.$(SOEXT)=.rootmap)

#LF
RAUTHNM       := $(RAUTHLIB:.$(SOEXT)=.nm)

##### libAFSAuth #####
ifneq ($(AFSLIB),)
AFSAUTHL       := $(MODDIRI)/LinkDefAFS.h
AFSAUTHDS      := $(MODDIRS)/G__AFSAuth.cxx
AFSAUTHDO      := $(AFSAUTHDS:.cxx=.o)
AFSAUTHDH      := $(AFSAUTHDS:.cxx=.h)

AFSAUTHH     := $(MODDIRI)/AFSAuth.h $(MODDIRI)/AFSAuthTypes.h $(MODDIRI)/TAFS.h
AFSAUTHS     := $(MODDIRS)/AFSAuth.cxx $(MODDIRS)/TAFS.cxx

AFSAUTHO     := $(AFSAUTHS:.cxx=.o)

#LF
AFSAUTHTMPDS    := $(MODDIRS)/G__AFSAuthTmp.cxx
AFSAUTHTMPDO    := $(AFSAUTHTMPDS:.cxx=.o)
AFSAUTHTMPDH    := $(AFSAUTHTMPDS:.cxx=.h)
AFSAUTHTMP2DS   := $(MODDIRS)/G__AFSAuthTmp2.cxx
AFSAUTHTMP2DO   := $(AFSAUTHTMP2DS:.cxx=.o)
AFSAUTHTMP2DH   := $(AFSAUTHTMP2DS:.cxx=.h)

AFSAUTHDEP   := $(AFSAUTHO:.o=.d) $(AFSAUTHDO:.o=.d)

AFSAUTHLIB   := $(LPATH)/libAFSAuth.$(SOEXT)
AFSAUTHMAP   := $(AFSAUTHLIB:.$(SOEXT)=.rootmap)

#LF
AFSAUTHNM       := $(AFSAUTHLIB:.$(SOEXT)=.nm)
endif

#### for libSrvAuth (built in rpdutils/Module.mk) ####
DAEMONUTILSO := $(MODDIRS)/DaemonUtils.o

#### for rootd and proofd ####
RSAO         := $(AUTHDIRS)/rsaaux.o $(AUTHDIRS)/rsalib.o $(AUTHDIRS)/rsafun.o
ifneq ($(AFSLIB),)
RSAO         += $(MODDIRS)/AFSAuth.o
endif

# Add SSL flags, if required
EXTRA_RAUTHFLAGS = $(EXTRA_AUTHFLAGS)
EXTRA_RAUTHLIBS  = $(CRYPTLIBS)
ifneq ($(SSLLIB),)
EXTRA_RAUTHFLAGS += $(SSLINCDIR:%=-I%)
EXTRA_RAUTHLIBS  += $(SSLLIBDIR) $(SSLLIB)
endif

# used in the main Makefile
ALLHDRS      += $(patsubst $(MODDIRI)/%.h,include/%.h,$(RAUTHH)) \
                include/DaemonUtils.h
ALLLIBS      += $(RAUTHLIB)
ALLMAPS      += $(RAUTHMAP)
ifneq ($(AFSLIB),)
ALLHDRS      += $(patsubst $(MODDIRI)/%.h,include/%.h,$(AFSAUTHH))
ALLLIBS      += $(AFSAUTHLIB)
ALLMAPS      += $(AFSAUTHMAP)
endif

# include all dependency files
INCLUDEFILES += $(RAUTHDEP)
ifneq ($(AFSLIB),)
INCLUDEFILES += $(AFSAUTHDEP)
endif

##### local rules #####
include/%.h:    $(AUTHDIRI)/%.h
		cp $< $@

#LF
$(RAUTHLIB):   $(RAUTHO) $(RAUTHTMPDO) $(RAUTHTMP2DO) $(RAUTHDO) $(ORDER_) $(MAINLIBS) $(RAUTHLIBDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		   "$(SOFLAGS)" libRootAuth.$(SOEXT) $@ "$(RAUTHO) $(RAUTHTMPDO) $(RAUTHTMP2DO) $(RAUTHDO)" \
		   "$(RAUTHLIBEXTRA) $(EXTRA_RAUTHLIBS)"

#LF
$(RAUTHTMPDS):   $(RAUTHH) $(RAUTHL) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(RAUTHH) $(RAUTHL)

#LF
$(RAUTHTMP2DS):  $(RAUTHH) $(RAUTHL) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(RAUTHH) $(RAUTHL)

#LF
$(RAUTHDS):    $(RAUTHH) $(RAUTHL) $(ROOTCINTTMPEXE) $(RAUTHNM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(RAUTHNM) -. 3 -c $(RAUTHH) $(RAUTHL)

#LF
$(RAUTHNM):      $(RAUTHO) $(RAUTHTMPDO) $(RAUTHTMP2DO) 
		@echo "Generating symbols file $@..."
		nm -p --defined-only $(RAUTHTMPDO) | awk '{printf("%s\n", $$3)'} > $(RAUTHNM)
		nm -p --defined-only $(RAUTHTMP2DO) | awk '{printf("%s\n", $$3)'} >> $(RAUTHNM)
		nm -p --defined-only $(RAUTHO) | awk '{printf("%s\n", $$3)'} >> $(RAUTHNM)

$(RAUTHMAP):    $(RLIBMAP) $(MAKEFILEDEP) $(RAUTHL)
		$(RLIBMAP) -o $(RAUTHMAP) -l $(RAUTHLIB) \
		   -d $(RAUTHLIBDEPM) -c $(RAUTHL)


#LF
$(AFSAUTHLIB):   $(AFSAUTHO) $(AFSAUTHTMPDO) $(AFSAUTHTMP2DO) $(AFSAUTHDO) $(ORDER_) $(MAINLIBS) $(AFSAUTHLIBDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		   "$(SOFLAGS)" libAFSAuth.$(SOEXT) $@ "$(AFSAUTHO) $(AFSAUTHTMPDO) $(AFSAUTHTMP2DO) $(AFSAUTHDO)" \
		   "$(AFSLIBDIR) $(AFSLIB) $(RESOLVLIB)"

#LF
$(AFSAUTHTMPDS):   $(AFSAUTHH) $(AFSAUTHL) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(AFSAUTHH) $(AFSAUTHL)

#LF
$(AFSAUTHTMP2DS):  $(AFSAUTHH) $(AFSAUTHL) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(AFSAUTHH) $(AFSAUTHL)

#LF
$(AFSAUTHDS):    $(AFSAUTHH) $(AFSAUTHL) $(ROOTCINTTMPEXE) $(AFSAUTHNM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(AFSAUTHNM) -. 3 -c $(AFSAUTHH) $(AFSAUTHL)

#LF
$(AFSAUTHDICTMAP): $(RLIBMAP) $(MAKEFILEDEP) $(AFSAUTHL)
		$(RLIBMAP) -o $(AFSAUTHDICTMAP) -l $(AFSAUTHDICTLIB) \
		-d $(AFSAUTHLIB) $(AFSAUTHLIBDEPM) -c $(AFSAUTHL)
#LF
$(AFSAUTHNM):      $(AFSAUTHO) $(AFSAUTHTMPDO) $(AFSAUTHTMP2DO) 
		@echo "Generating symbols file $@..."
		nm -p --defined-only $(AFSAUTHTMPDO) | awk '{printf("%s\n", $$3)'} > $(AFSAUTHNM)
		nm -p --defined-only $(AFSAUTHTMP2DO) | awk '{printf("%s\n", $$3)'} >> $(AFSAUTHNM)
		nm -p --defined-only $(AFSAUTHO) | awk '{printf("%s\n", $$3)'} >> $(AFSAUTHNM)

$(AFSAUTHMAP):  $(RLIBMAP) $(MAKEFILEDEP) $(AFSAUTHL)
		$(RLIBMAP) -o $(AFSAUTHMAP) -l $(AFSAUTHLIB) \
		   -d $(AFSAUTHLIBDEPM) -c $(AFSAUTHL)

all-auth:       $(RAUTHLIB) $(AFSAUTHLIB) $(RAUTHMAP) $(AFSAUTHMAP)

clean-auth:
		@rm -f $(RAUTHO) $(RAUTHDO) $(DAEMONUTILSO) $(AFSAUTHO) \
		       $(AFSAUTHDO)

clean::         clean-auth

#LF
clean-pds-rauth:	
		rm -f $(RAUTHTMPDS) $(RAUTHTMPDO) $(RAUTHTMPDH) \
		$(RAUTHTMPDEP) $(RAUTHTMP2DS) $(RAUTHTMP2DO) $(RAUTHTMP2DH) $(RAUTHNM)

#LF
clean-pds-afsauth:	
		rm -f $(AFSAUTHTMPDS) $(AFSAUTHTMPDO) $(AFSAUTHTMPDH) \
		$(AFSAUTHTMPDEP) $(AFSAUTHTMP2DS) $(AFSAUTHTMP2DO) $(AFSAUTHTMP2DH) $(AFSAUTHNM)

distclean-auth: clean-auth
		@rm -f $(RAUTHDEP) $(RAUTHDS) $(RAUTHDH) $(RAUTHLIB) \
		       $(AFSAUTHDEP) $(AFSAUTHDS) $(AFSAUTHLIB) \
		       $(RAUTHMAP) $(AFSAUTHMAP)

distclean::     distclean-auth

##### extra rules ######
$(RAUTHO):      CXXFLAGS += $(EXTRA_RAUTHFLAGS)
$(RAUTHO):      PCHCXXFLAGS =
$(AFSAUTHO):    CXXFLAGS += $(AFSINCDIR)
$(AFSAUTHO):    PCHCXXFLAGS =
