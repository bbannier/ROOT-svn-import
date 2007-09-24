# Module.mk for krb5 authentication module
# Copyright (c) 2002 Rene Brun and Fons Rademakers
#
# Author: Fons Rademakers, 18/3/2002

MODDIR       := krb5auth
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

KRB5AUTHDIR  := $(MODDIR)
KRB5AUTHDIRS := $(KRB5AUTHDIR)/src
KRB5AUTHDIRI := $(KRB5AUTHDIR)/inc

##### libKrb5Auth #####
KRB5AUTHL    := $(MODDIRI)/LinkDef.h
KRB5AUTHDS   := $(MODDIRS)/G__Krb5Auth.cxx
KRB5AUTHDO   := $(KRB5AUTHDS:.cxx=.o)
KRB5AUTHDH   := $(KRB5AUTHDS:.cxx=.h)

KRB5AUTHH1   := $(patsubst %,$(MODDIRI)/%,TKSocket.h)

KRB5AUTHH    := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
KRB5AUTHS    := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
KRB5AUTHO    := $(KRB5AUTHS:.cxx=.o)

#LF
KRB5AUTHTMPDS    := $(MODDIRS)/G__Krb5AuthTmp.cxx
KRB5AUTHTMPDO    := $(KRB5AUTHTMPDS:.cxx=.o)
KRB5AUTHTMPDH    := $(KRB5AUTHTMPDS:.cxx=.h)
KRB5AUTHTMP2DS   := $(MODDIRS)/G__Krb5AuthTmp2.cxx
KRB5AUTHTMP2DO   := $(KRB5AUTHTMP2DS:.cxx=.o)
KRB5AUTHTMP2DH   := $(KRB5AUTHTMP2DS:.cxx=.h)

KRB5AUTHDEP  := $(KRB5AUTHO:.o=.d)

#LF
KRB5AUTHTMPDEP  := $(KRB5AUTHTMPDO:.o=.d)

KRB5AUTHLIB  := $(LPATH)/libKrb5Auth.$(SOEXT)
KRB5AUTHMAP  := $(KRB5AUTHLIB:.$(SOEXT)=.rootmap)

#LF
KRB5AUTHNM       := $(KRB5AUTHLIB:.$(SOEXT)=.nm)

# used in the main Makefile
ALLHDRS     += $(patsubst $(MODDIRI)/%.h,include/%.h,$(KRB5AUTHH))
ALLLIBS     += $(KRB5AUTHLIB)
ALLMAPS     += $(KRB5AUTHMAP)

# include all dependency files
INCLUDEFILES += $(KRB5AUTHDEP)

##### local rules #####
include/%.h:    $(KRB5AUTHDIRI)/%.h
		cp $< $@

#LF
$(KRB5AUTHLIB):   $(KRB5AUTHO) $(KRB5AUTHTMPDO) $(KRB5AUTHTMP2DO) $(KRB5AUTHDO) $(ORDER_) $(MAINLIBS) $(KRB5AUTHLIBDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		   "$(SOFLAGS)" libKrb5Auth.$(SOEXT) $@ "$(KRB5AUTHO) $(KRB5AUTHTMPDO) $(KRB5AUTHTMP2DO) $(KRB5AUTHDO)" \
		   "$(KRB5AUTHLIBEXTRA) $(KRB5LIBDIR) $(KRB5LIB) \
		    $(COMERRLIBDIR) $(COMERRLIB) $(RESOLVLIB) \
		    $(CRYPTOLIBDIR) $(CRYPTOLIB) $(KRB5AUTHLIBEXTRA)"

#LF
$(KRB5AUTHTMPDS):   $(KRB5AUTHH1) $(KRB5AUTHL) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(KRB5INCDIR:%=-I%) $(KRB5AUTHH1) $(KRB5AUTHL)

#LF
$(KRB5AUTHTMP2DS):  $(KRB5AUTHH1) $(KRB5AUTHL) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(KRB5INCDIR:%=-I%) $(KRB5AUTHH1) $(KRB5AUTHL)

#LF
$(KRB5AUTHDS):    $(KRB5AUTHH1) $(KRB5AUTHL) $(ROOTCINTTMPEXE) $(KRB5AUTHNM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(KRB5AUTHNM) -. 3 -c $(KRB5INCDIR:%=-I%) $(KRB5AUTHH1) $(KRB5AUTHL)

#LF
$(KRB5AUTHNM):      $(KRB5AUTHO) $(KRB5AUTHTMPDO) $(KRB5AUTHTMP2DO) 
		@echo "Generating symbols file $@..."
		nm -g -p --defined-only $(KRB5AUTHTMPDO) | awk '{printf("%s\n", $$3)'} > $(KRB5AUTHNM)
		nm -g -p --defined-only $(KRB5AUTHTMP2DO) | awk '{printf("%s\n", $$3)'} >> $(KRB5AUTHNM)
		nm -g -p --defined-only $(KRB5AUTHO) | awk '{printf("%s\n", $$3)'} >> $(KRB5AUTHNM)

$(KRB5AUTHMAP): $(RLIBMAP) $(MAKEFILEDEP) $(KRB5AUTHL)
		$(RLIBMAP) -o $(KRB5AUTHMAP) -l $(KRB5AUTHLIB) \
		   -d $(KRB5AUTHLIBDEPM) -c $(KRB5AUTHL)

all-krb5auth:   $(KRB5AUTHLIB) $(KRB5AUTHMAP)

clean-krb5auth:
		@rm -f $(KRB5AUTHO) $(KRB5AUTHDO)

clean::         clean-krb5auth clean-pds-krb5auth

#LF
clean-pds-krb5auth:	
		rm -f $(KRB5AUTHTMPDS) $(KRB5AUTHTMPDO) $(KRB5AUTHTMPDH) \
		$(KRB5AUTHTMPDEP) $(KRB5AUTHTMP2DS) $(KRB5AUTHTMP2DO) $(KRB5AUTHTMP2DH) $(KRB5AUTHNM)

distclean-krb5auth: clean-krb5auth
		@rm -f $(KRB5AUTHDEP) $(KRB5AUTHDS) $(KRB5AUTHDH) \
		   $(KRB5AUTHLIB) $(KRB5AUTHMAP)

distclean::     distclean-krb5auth

##### extra rules ######
$(KRB5AUTHDO): CXXFLAGS += $(KRB5INCDIR:%=-I%)

#LF
$(KRB5AUTHTMPDO): CXXFLAGS += $(KRB5INCDIR:%=-I%)
$(KRB5AUTHTMP2DO): CXXFLAGS += $(KRB5INCDIR:%=-I%)

$(KRB5AUTHO): CXXFLAGS += -DR__KRB5INIT="\"$(KRB5INIT)\"" $(KRB5INCDIR:%=-I%)
$(KRB5AUTHO): PCHCXXFLAGS =
