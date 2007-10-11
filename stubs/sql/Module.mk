# Module.mk for sql module
# Copyright (c) 2005 Rene Brun and Fons Rademakers
#
# Author: Fons Rademakers, 7/12/2005

MODDIR       := sql
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

SQLDIR       := $(MODDIR)
SQLDIRS      := $(SQLDIR)/src
SQLDIRI      := $(SQLDIR)/inc

##### libSQL #####
SQLL         := $(MODDIRI)/LinkDef.h
SQLDS        := $(MODDIRS)/G__SQL.cxx
SQLDO        := $(SQLDS:.cxx=.o)
SQLDH        := $(SQLDS:.cxx=.h)

#LF
SQLTMPDS    := $(MODDIRS)/G__SQLTmp.cxx
SQLTMPDO    := $(SQLTMPDS:.cxx=.o)
SQLTMPDH    := $(SQLTMPDS:.cxx=.h)
SQLTMP2DS   := $(MODDIRS)/G__SQLTmp2.cxx
SQLTMP2DO   := $(SQLTMP2DS:.cxx=.o)
SQLTMP2DH   := $(SQLTMP2DS:.cxx=.h)

SQLH         := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
SQLS         := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
SQLO         := $(SQLS:.cxx=.o)

SQLDEP       := $(SQLO:.o=.d) $(SQLDO:.o=.d)

#LF
SQLTMPDEP  := $(SQLTMPDO:.o=.d)

SQLLIB       := $(LPATH)/libSQL.$(SOEXT)
SQLMAP       := $(SQLLIB:.$(SOEXT)=.rootmap)

#LF
SQLNM       := $(SQLLIB:.$(SOEXT)=.nm)

# used in the main Makefile
ALLHDRS      += $(patsubst $(MODDIRI)/%.h,include/%.h,$(SQLH))
ALLLIBS      += $(SQLLIB)
ALLMAPS      += $(SQLMAP)

# include all dependency files
INCLUDEFILES += $(SQLDEP)

##### local rules #####
include/%.h:    $(SQLDIRI)/%.h
		cp $< $@

#LF
$(SQLLIB):   $(SQLO) $(SQLTMPDO) $(SQLTMP2DO) $(SQLDO) $(ORDER_) $(MAINLIBS) $(SQLLIBDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		   "$(SOFLAGS)" libSQL.$(SOEXT) $@ "$(SQLO) $(SQLTMPDO) $(SQLTMP2DO) $(SQLDO)" \
		   "$(SQLLIBEXTRA)"

#LF
$(SQLTMPDS):   $(SQLH) $(SQLL) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(SQLH) $(SQLL)

#LF
$(SQLTMP2DS):  $(SQLH) $(SQLL) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(SQLH) $(SQLL)

#LF
$(SQLDS):    $(SQLH) $(SQLL) $(ROOTCINTTMPEXE) $(SQLNM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(SQLNM) -. 3 -c $(SQLH) $(SQLL)

#LF
$(SQLNM):      $(SQLO) $(SQLTMPDO) $(SQLTMP2DO) 
		@echo "Generating symbols file $@..."
		nm -g -p --defined-only $(SQLTMPDO) | awk '{printf("%s\n", $$3)'} > $(SQLNM)
		nm -g -p --defined-only $(SQLTMP2DO) | awk '{printf("%s\n", $$3)'} >> $(SQLNM)
		nm -g -p --defined-only $(SQLO) | awk '{printf("%s\n", $$3)'} >> $(SQLNM)

$(SQLMAP):      $(RLIBMAP) $(MAKEFILEDEP) $(SQLL)
		$(RLIBMAP) -o $(SQLMAP) -l $(SQLLIB) \
		   -d $(SQLLIBDEPM) -c $(SQLL)

all-sql:        $(SQLLIB) $(SQLMAP)

clean-sql:
		@rm -f $(SQLO) $(SQLDO)

clean::         clean-sql clean-pds-sql

#LF
clean-pds-sql:	
		rm -f $(SQLTMPDS) $(SQLTMPDO) $(SQLTMPDH) \
		$(SQLTMPDEP) $(SQLTMP2DS) $(SQLTMP2DO) $(SQLTMP2DH) $(SQLNM)

distclean-sql: clean-sql
		@rm -f $(SQLDEP) $(SQLDS) $(SQLDH) $(SQLLIB) $(SQLMAP)

distclean::     distclean-sql
