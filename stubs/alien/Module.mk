# Module.mk for alien module
# Copyright (c) 2002 Rene Brun and Fons Rademakers
#
# Author: Fons Rademakers, 12/5/2002

MODDIR       := alien
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

ALIENDIR     := $(MODDIR)
ALIENDIRS    := $(ALIENDIR)/src
ALIENDIRI    := $(ALIENDIR)/inc

##### libRAliEn #####
ALIENL       := $(MODDIRI)/LinkDef.h
ALIENDS      := $(MODDIRS)/G__Alien.cxx
ALIENDO      := $(ALIENDS:.cxx=.o)
ALIENDH      := $(ALIENDS:.cxx=.h)

#LF
ALIENTMPDS    := $(MODDIRS)/G__AlienTmp.cxx
ALIENTMPDO    := $(ALIENTMPDS:.cxx=.o)
ALIENTMPDH    := $(ALIENTMPDS:.cxx=.h)
ALIENTMP2DS   := $(MODDIRS)/G__AlienTmp2.cxx
ALIENTMP2DO   := $(ALIENTMP2DS:.cxx=.o)
ALIENTMP2DH   := $(ALIENTMP2DS:.cxx=.h)

ALIENH       := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
ALIENS       := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
ALIENO       := $(ALIENS:.cxx=.o)

ALIENDEP     := $(ALIENO:.o=.d) $(ALIENDO:.o=.d)

#LF
ALIENTMPDEP  := $(ALIENTMPDO:.o=.d)

ALIENLIB     := $(LPATH)/libRAliEn.$(SOEXT)
ALIENMAP     := $(ALIENLIB:.$(SOEXT)=.rootmap)

# used in the main Makefile
ALLHDRS     += $(patsubst $(MODDIRI)/%.h,include/%.h,$(ALIENH))
ALLLIBS     += $(ALIENLIB)
ALLMAPS     += $(ALIENMAP)

#LF
ALIENNM       := $(ALIENLIB:.$(SOEXT)=.nm)

# include all dependency files
INCLUDEFILES += $(ALIENDEP)

##### local rules #####
include/%.h:    $(ALIENDIRI)/%.h
		cp $< $@

#LF
$(ALIENLIB):   $(ALIENO) $(ALIENTMPDO) $(ALIENTMP2DO) $(ALIENDO) $(ORDER_) $(MAINLIBS) $(ALIENLIBDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		   "$(SOFLAGS)" libRAliEn.$(SOEXT) $@ "$(ALIENO) $(ALIENTMPDO) $(ALIENTMP2DO) $(ALIENDO)" \
		   "$(ALIENLIBEXTRA) $(ALIENLIBDIR) $(ALIENCLILIB)"

#LF
$(ALIENTMPDS):   $(ALIENH) $(ALIENL) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(ALIENH) $(ALIENL)

#LF
$(ALIENTMP2DS):  $(ALIENH) $(ALIENL) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(ALIENH) $(ALIENL)

#LF
$(ALIENDS):    $(ALIENH) $(ALIENL) $(ROOTCINTTMPEXE) $(ALIENNM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(ALIENNM) -. 3 -c $(ALIENH) $(ALIENL)

#LF
$(ALIENDICTMAP): $(RLIBMAP) $(MAKEFILEDEP) $(ALIENL)
		$(RLIBMAP) -o $(ALIENDICTMAP) -l $(ALIENDICTLIB) \
		-d $(ALIENLIB) $(ALIENLIBDEPM) -c $(ALIENL)
#LF
$(ALIENNM):      $(ALIENO) $(ALIENTMPDO) $(ALIENTMP2DO) 
		@echo "Generating symbols file $@..."
		nm -p --defined-only $(ALIENTMPDO) | awk '{printf("%s\n", $$3)'} > $(ALIENNM)
		nm -p --defined-only $(ALIENTMP2DO) | awk '{printf("%s\n", $$3)'} >> $(ALIENNM)
		nm -p --defined-only $(ALIENO) | awk '{printf("%s\n", $$3)'} >> $(ALIENNM)

#LF
$(ALIENMAP):   $(RLIBMAP) $(MAKEFILEDEP) $(ALIENL)
		$(RLIBMAP) -o $(ALIENMAP) -l $(ALIENLIB) \
		   -d $(ALIENLIBDEPM) -c $(ALIENL)

all-alien:      $(ALIENLIB) $(ALIENMAP)

clean-alien:
		@rm -f $(ALIENO) $(ALIENDO)

clean::         clean-alien clean-pds-alien

#LF
clean-pds-alien:	
		rm -f $(ALIENTMPDS) $(ALIENTMPDO) $(ALIENTMPDH) \
		$(ALIENTMPDEP) $(ALIENTMP2DS) $(ALIENTMP2DO) $(ALIENTMP2DH) $(ALIENNM)

distclean-alien: clean-alien
		@rm -f $(ALIENDEP) $(ALIENDS) $(ALIENDH) $(ALIENLIB) $(ALIENMAP)

distclean::     distclean-alien

##### extra rules ######
$(ALIENO) $(ALIENDO): CXXFLAGS += $(ALIENINCDIR:%=-I%)
