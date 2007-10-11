# Module.mk for guibuilder module
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: Valeriy Onuchin, 24/4/2007

MODDIR       := guihtml
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

GUIHTMLDIR   := $(MODDIR)
GUIHTMLDIRS  := $(GUIHTMLDIR)/src
GUIHTMLDIRI  := $(GUIHTMLDIR)/inc

##### libGuiHtml #####
GUIHTMLL     := $(MODDIRI)/LinkDef.h
GUIHTMLDS    := $(MODDIRS)/G__GuiHtml.cxx
GUIHTMLDO    := $(GUIHTMLDS:.cxx=.o)
GUIHTMLDH    := $(GUIHTMLDS:.cxx=.h)

#LF
GUIHTMLTMPDS    := $(MODDIRS)/G__GuiHtmlTmp.cxx
GUIHTMLTMPDO    := $(GUIHTMLTMPDS:.cxx=.o)
GUIHTMLTMPDH    := $(GUIHTMLTMPDS:.cxx=.h)
GUIHTMLTMP2DS   := $(MODDIRS)/G__GuiHtmlTmp2.cxx
GUIHTMLTMP2DO   := $(GUIHTMLTMP2DS:.cxx=.o)
GUIHTMLTMP2DH   := $(GUIHTMLTMP2DS:.cxx=.h)

GUIHTMLH     := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
GUIHTMLS     := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
GUIHTMLO     := $(GUIHTMLS:.cxx=.o)

GUIHTMLDEP   := $(GUIHTMLO:.o=.d) $(GUIHTMLDO:.o=.d)

#LF
GUIHTMLTMPDEP  := $(GUIHTMLTMPDO:.o=.d)

GUIHTMLLIB   := $(LPATH)/libGuiHtml.$(SOEXT)
GUIHTMLMAP   := $(GUIHTMLLIB:.$(SOEXT)=.rootmap)

#LF
GUIHTMLNM       := $(GUIHTMLLIB:.$(SOEXT)=.nm)

# used in the main Makefile
ALLHDRS     += $(patsubst $(MODDIRI)/%.h,include/%.h,$(GUIHTMLH))
ALLLIBS     += $(GUIHTMLLIB)
ALLMAPS     += $(GUIHTMLMAP)

# include all dependency files
INCLUDEFILES += $(GUIHTMLDEP)

##### local rules #####
include/%.h:    $(GUIHTMLDIRI)/%.h
		cp $< $@

#LF
$(GUIHTMLLIB):   $(GUIHTMLO) $(GUIHTMLTMPDO) $(GUIHTMLTMP2DO) $(GUIHTMLDO) $(ORDER_) $(MAINLIBS) $(GUIHTMLLIBDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		   "$(SOFLAGS)" libGuiHtml.$(SOEXT) $@ "$(GUIHTMLO) $(GUIHTMLTMPDO) $(GUIHTMLTMP2DO) $(GUIHTMLDO)" \
		   "$(GUIHTMLLIBEXTRA)"

#LF
$(GUIHTMLTMPDS):   $(GUIHTMLH) $(GUIHTMLL) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(GUIHTMLH) $(GUIHTMLL)

#LF
$(GUIHTMLTMP2DS):  $(GUIHTMLH) $(GUIHTMLL) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(GUIHTMLH) $(GUIHTMLL)

#LF
$(GUIHTMLDS):    $(GUIHTMLH) $(GUIHTMLL) $(ROOTCINTTMPEXE) $(GUIHTMLNM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(GUIHTMLNM) -. 3 -c $(GUIHTMLH) $(GUIHTMLL)

#LF
$(GUIHTMLNM):      $(GUIHTMLO) $(GUIHTMLTMPDO) $(GUIHTMLTMP2DO) 
		@echo "Generating symbols file $@..."
		nm -g -p --defined-only $(GUIHTMLTMPDO) | awk '{printf("%s\n", $$3)'} > $(GUIHTMLNM)
		nm -g -p --defined-only $(GUIHTMLTMP2DO) | awk '{printf("%s\n", $$3)'} >> $(GUIHTMLNM)
		nm -g -p --defined-only $(GUIHTMLO) | awk '{printf("%s\n", $$3)'} >> $(GUIHTMLNM)

$(GUIHTMLMAP):  $(RLIBMAP) $(MAKEFILEDEP) $(GUIHTMLL)
		$(RLIBMAP) -o $(GUIHTMLMAP) -l $(GUIHTMLLIB) \
		   -d $(GUIHTMLLIBDEPM) -c $(GUIHTMLL)

all-guihtml:    $(GUIHTMLLIB) $(GUIHTMLMAP)

clean-guihtml:
		@rm -f $(GUIHTMLO) $(GUIHTMLDO)

clean::         clean-guihtml clean-pds-guihtml

#LF
clean-pds-guihtml:	
		rm -f $(GUIHTMLTMPDS) $(GUIHTMLTMPDO) $(GUIHTMLTMPDH) \
		$(GUIHTMLTMPDEP) $(GUIHTMLTMP2DS) $(GUIHTMLTMP2DO) $(GUIHTMLTMP2DH) $(GUIHTMLNM)

distclean-guihtml: clean-guihtml
		@rm -f $(GUIHTMLDEP) $(GUIHTMLDS) $(GUIHTMLDH) $(GUIHTMLLIB) $(GUIHTMLMAP)

distclean::     distclean-guihtml
