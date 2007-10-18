# Module.mk for pgsql module
# Copyright (c) 2001 Rene Brun and Fons Rademakers
#
# Author: g.p.ciceri <gp.ciceri@acm.org>, 1/06/2001

MODDIR       := pgsql
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

PGSQLDIR     := $(MODDIR)
PGSQLDIRS    := $(PGSQLDIR)/src
PGSQLDIRI    := $(PGSQLDIR)/inc

##### libPgSQL #####
PGSQLL       := $(MODDIRI)/LinkDef.h
PGSQLDS      := $(MODDIRS)/G__PgSQL.cxx
PGSQLDO      := $(PGSQLDS:.cxx=.o)
PGSQLDH      := $(PGSQLDS:.cxx=.h)

#LF
PGSQLTMPDS    := $(MODDIRS)/G__PgSQLTmp.cxx
PGSQLTMPDO    := $(PGSQLTMPDS:.cxx=.o)
PGSQLTMPDH    := $(PGSQLTMPDS:.cxx=.h)
PGSQLTMP2DS   := $(MODDIRS)/G__PgSQLTmp2.cxx
PGSQLTMP2DO   := $(PGSQLTMP2DS:.cxx=.o)
PGSQLTMP2DH   := $(PGSQLTMP2DS:.cxx=.h)

PGSQLH       := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
PGSQLS       := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
PGSQLO       := $(PGSQLS:.cxx=.o)

PGSQLDEP     := $(PGSQLO:.o=.d) $(PGSQLDO:.o=.d)

#LF
PGSQLTMPDEP  := $(PGSQLTMPDO:.o=.d)

PGSQLLIB     := $(LPATH)/libPgSQL.$(SOEXT)
PGSQLMAP     := $(PGSQLLIB:.$(SOEXT)=.rootmap)

#LF
PGSQLNM       := $(PGSQLLIB:.$(SOEXT)=.nm)

# used in the main Makefile
ALLHDRS     += $(patsubst $(MODDIRI)/%.h,include/%.h,$(PGSQLH))
ALLLIBS     += $(PGSQLLIB)
ALLMAPS     += $(PGSQLMAP)

# include all dependency files
INCLUDEFILES += $(PGSQLDEP)

##### local rules #####
include/%.h:    $(PGSQLDIRI)/%.h
		cp $< $@

#LF
$(PGSQLLIB):   $(PGSQLO) $(PGSQLTMPDO) $(PGSQLTMP2DO) $(PGSQLDO) $(ORDER_) $(MAINLIBS) $(PGSQLLIBDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		   "$(SOFLAGS)" libPgSQL.$(SOEXT) $@ "$(PGSQLO) $(PGSQLTMPDO) $(PGSQLTMP2DO) $(PGSQLDO)" \
		   "$(PGSQLLIBEXTRA) $(PGSQLLIBDIR) $(PGSQLCLILIB)"

#LF
$(PGSQLTMPDS):   $(PGSQLH) $(PGSQLL) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(PGSQLH) $(PGSQLL)

#LF
$(PGSQLTMP2DS):  $(PGSQLH) $(PGSQLL) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(PGSQLH) $(PGSQLL)

#LF
$(PGSQLDS):    $(PGSQLH) $(PGSQLL) $(ROOTCINTTMPEXE) $(PGSQLNM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(PGSQLNM) -. 3 -c $(PGSQLH) $(PGSQLL)

#LF
$(PGSQLNM):      $(PGSQLO) $(PGSQLTMPDO) $(PGSQLTMP2DO) 
		@echo "Generating symbols file $@..."
		nm -p --defined-only $(PGSQLTMPDO) | awk '{printf("%s\n", $$3)'} > $(PGSQLNM)
		nm -p --defined-only $(PGSQLTMP2DO) | awk '{printf("%s\n", $$3)'} >> $(PGSQLNM)
		nm -p --defined-only $(PGSQLO) | awk '{printf("%s\n", $$3)'} >> $(PGSQLNM)

$(PGSQLMAP):    $(RLIBMAP) $(MAKEFILEDEP) $(PGSQLL)
		$(RLIBMAP) -o $(PGSQLMAP) -l $(PGSQLLIB) \
		   -d $(PGSQLLIBDEPM) -c $(PGSQLL)

all-pgsql:      $(PGSQLLIB) $(PGSQLMAP)

clean-pgsql:
		@rm -f $(PGSQLO) $(PGSQLDO)

clean::         clean-pgsql clean-pds-pgsql

clean-pds-pgsql:	
		rm -f $(PGSQLTMPDS) $(PGSQLTMPDO) $(PGSQLTMPDH) \
		$(PGSQLTMPDEP) $(PGSQLTMP2DS) $(PGSQLTMP2DO) $(PGSQLTMP2DH) $(PGSQLNM)

distclean-pgsql: clean-pgsql
		@rm -f $(PGSQLDEP) $(PGSQLDS) $(PGSQLDH) $(PGSQLLIB) $(PGSQLMAP)

distclean::     distclean-pgsql

##### extra rules ######
$(PGSQLO) $(PGSQLDO): CXXFLAGS += $(PGSQLINCDIR:%=-I%)
