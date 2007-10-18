# Module.mk for html module
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: Fons Rademakers, 29/2/2000

MODDIR       := html
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

HTMLDIR      := $(MODDIR)
HTMLDIRS     := $(HTMLDIR)/src
HTMLDIRI     := $(HTMLDIR)/inc

##### libHtml #####
HTMLL        := $(MODDIRI)/LinkDef.h
HTMLDS       := $(MODDIRS)/G__Html.cxx
HTMLDO       := $(HTMLDS:.cxx=.o)
HTMLDH       := $(HTMLDS:.cxx=.h)

#LF
HTMLTMPDS    := $(MODDIRS)/G__HtmlTmp.cxx
HTMLTMPDO    := $(HTMLTMPDS:.cxx=.o)
HTMLTMPDH    := $(HTMLTMPDS:.cxx=.h)
HTMLTMP2DS   := $(MODDIRS)/G__HtmlTmp2.cxx
HTMLTMP2DO   := $(HTMLTMP2DS:.cxx=.o)
HTMLTMP2DH   := $(HTMLTMP2DS:.cxx=.h)

HTMLH        := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
HTMLS        := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
HTMLO        := $(HTMLS:.cxx=.o)

HTMLDEP      := $(HTMLO:.o=.d) $(HTMLDO:.o=.d)

#LF
HTMLTMPDEP  := $(HTMLTMPDO:.o=.d)

HTMLLIB      := $(LPATH)/libHtml.$(SOEXT)
HTMLMAP      := $(HTMLLIB:.$(SOEXT)=.rootmap)

#LF
HTMLNM       := $(HTMLLIB:.$(SOEXT)=.nm)

# used in the main Makefile
ALLHDRS     += $(patsubst $(MODDIRI)/%.h,include/%.h,$(HTMLH))
ALLLIBS     += $(HTMLLIB)
ALLMAPS     += $(HTMLMAP)

# include all dependency files
INCLUDEFILES += $(HTMLDEP)

##### local rules #####
include/%.h:    $(HTMLDIRI)/%.h
		cp $< $@

#LF
$(HTMLLIB):   $(HTMLO) $(HTMLTMPDO) $(HTMLTMP2DO) $(HTMLDO) $(ORDER_) $(MAINLIBS) $(HTMLLIBDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		   "$(SOFLAGS)" libHtml.$(SOEXT) $@ "$(HTMLO) $(HTMLTMPDO) $(HTMLTMP2DO) $(HTMLDO)" \
		   "$(HTMLLIBEXTRA)"

#LF
$(HTMLTMPDS):   $(HTMLH) $(HTMLL) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(HTMLH) $(HTMLL)

#LF
$(HTMLTMP2DS):  $(HTMLH) $(HTMLL) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(HTMLH) $(HTMLL)

#LF
$(HTMLDS):    $(HTMLH) $(HTMLL) $(ROOTCINTTMPEXE) $(HTMLNM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(HTMLNM) -. 3 -c $(HTMLH) $(HTMLL)

#LF
$(HTMLDICTMAP): $(RLIBMAP) $(MAKEFILEDEP) $(HTMLL)
		$(RLIBMAP) -o $(HTMLDICTMAP) -l $(HTMLDICTLIB) \
		-d $(HTMLLIB) $(HTMLLIBDEPM) -c $(HTMLL)
#LF
$(HTMLNM):      $(HTMLO) $(HTMLTMPDO) $(HTMLTMP2DO) 
		@echo "Generating symbols file $@..."
		nm -p --defined-only $(HTMLTMPDO) | awk '{printf("%s\n", $$3)'} > $(HTMLNM)
		nm -p --defined-only $(HTMLTMP2DO) | awk '{printf("%s\n", $$3)'} >> $(HTMLNM)
		nm -p --defined-only $(HTMLO) | awk '{printf("%s\n", $$3)'} >> $(HTMLNM)

$(HTMLMAP):     $(RLIBMAP) $(MAKEFILEDEP) $(HTMLL)
		$(RLIBMAP) -o $(HTMLMAP) -l $(HTMLLIB) \
		   -d $(HTMLLIBDEPM) -c $(HTMLL)

all-html:       $(HTMLLIB) $(HTMLMAP)

clean-html:
		@rm -f $(HTMLO) $(HTMLDO)

clean::         clean-html clean-pds-html

#LF
clean-pds-html:	
		rm -f $(HTMLTMPDS) $(HTMLTMPDO) $(HTMLTMPDH) \
		$(HTMLTMPDEP) $(HTMLTMP2DS) $(HTMLTMP2DO) $(HTMLTMP2DH) $(HTMLNM)

distclean-html: clean-html
		@rm -f $(HTMLDEP) $(HTMLDS) $(HTMLDH) $(HTMLLIB) $(HTMLMAP)

distclean::     distclean-html
