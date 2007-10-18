# Module.mk for ldap module
# Copyright (c) 2002 Rene Brun and Fons Rademakers
#
# Author: Fons Rademakers, 20/11/2002

MODDIR       := ldap
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

LDAPDIR      := $(MODDIR)
LDAPDIRS     := $(LDAPDIR)/src
LDAPDIRI     := $(LDAPDIR)/inc

##### libRLDAP #####
LDAPL        := $(MODDIRI)/LinkDef.h
LDAPDS       := $(MODDIRS)/G__LDAP.cxx
LDAPDO       := $(LDAPDS:.cxx=.o)
LDAPDH       := $(LDAPDS:.cxx=.h)

#LF
LDAPTMPDS    := $(MODDIRS)/G__LDAPTmp.cxx
LDAPTMPDO    := $(LDAPTMPDS:.cxx=.o)
LDAPTMPDH    := $(LDAPTMPDS:.cxx=.h)
LDAPTMP2DS   := $(MODDIRS)/G__LDAPTmp2.cxx
LDAPTMP2DO   := $(LDAPTMP2DS:.cxx=.o)
LDAPTMP2DH   := $(LDAPTMP2DS:.cxx=.h)

LDAPH        := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
LDAPS        := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
LDAPO        := $(LDAPS:.cxx=.o)

LDAPDEP      := $(LDAPO:.o=.d) $(LDAPDO:.o=.d)

#LF
LDAPTMPDEP  := $(LDAPTMPDO:.o=.d)

LDAPLIB      := $(LPATH)/libRLDAP.$(SOEXT)
LDAPMAP      := $(LDAPLIB:.$(SOEXT)=.rootmap)

#LF
LDAPNM       := $(LDAPLIB:.$(SOEXT)=.nm)

# used in the main Makefile
ALLHDRS     += $(patsubst $(MODDIRI)/%.h,include/%.h,$(LDAPH))
ALLLIBS     += $(LDAPLIB)
ALLMAPS     += $(LDAPMAP)

# include all dependency files
INCLUDEFILES += $(LDAPDEP)

##### local rules #####
include/%.h:    $(LDAPDIRI)/%.h
		cp $< $@

#LF
$(LDAPLIB):   $(LDAPO) $(LDAPTMPDO) $(LDAPTMP2DO) $(LDAPDO) $(ORDER_) $(MAINLIBS)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		   "$(SOFLAGS)" libLDAP.$(SOEXT) $@ "$(LDAPO) $(LDAPTMPDO) $(LDAPTMP2DO) $(LDAPDO)" \
		   "$(LDAPLIBEXTRA) $(LDAPLIBDIR) $(LDAPCLILIB)"

#LF
$(LDAPTMPDS):   $(LDAPH) $(LDAPL) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(LDAPH) $(LDAPL)

#LF
$(LDAPTMP2DS):  $(LDAPH) $(LDAPL) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(LDAPH) $(LDAPL)

#LF
$(LDAPDS):    $(LDAPH) $(LDAPL) $(ROOTCINTTMPEXE) $(LDAPNM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(LDAPNM) -. 3 -c $(LDAPH) $(LDAPL)

#LF
$(LDAPNM):      $(LDAPO) $(LDAPTMPDO) $(LDAPTMP2DO) 
		@echo "Generating symbols file $@..."
		nm -p --defined-only $(LDAPTMPDO) | awk '{printf("%s\n", $$3)'} > $(LDAPNM)
		nm -p --defined-only $(LDAPTMP2DO) | awk '{printf("%s\n", $$3)'} >> $(LDAPNM)
		nm -p --defined-only $(LDAPO) | awk '{printf("%s\n", $$3)'} >> $(LDAPNM)

$(LDAPMAP):     $(RLIBMAP) $(MAKEFILEDEP) $(LDAPL)
		$(RLIBMAP) -o $(LDAPMAP) -l $(LDAPLIB) \
		   -d $(LDAPLIBDEPM) -c $(LDAPL)

all-ldap:       $(LDAPLIB) $(LDAPMAP)

clean-ldap:
		@rm -f $(LDAPO) $(LDAPDO)

clean::         clean-ldap clean-pds-ldap

#LF
clean-pds-ldap:	
		rm -f $(LDAPTMPDS) $(LDAPTMPDO) $(LDAPTMPDH) \
		$(LDAPTMPDEP) $(LDAPTMP2DS) $(LDAPTMP2DO) $(LDAPTMP2DH) $(LDAPNM)

distclean-ldap: clean-ldap
		@rm -f $(LDAPDEP) $(LDAPDS) $(LDAPDH) $(LDAPLIB) $(LDAPMAP)

distclean::     distclean-ldap

##### extra rules ######
$(LDAPO): CXXFLAGS += -DLDAP_DEPRECATED $(LDAPINCDIR:%=-I%)
