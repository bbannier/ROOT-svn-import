# Module.mk for xml module
# Copyright (c) 2004 Rene Brun and Fons Rademakers
#
# Authors: Linev Sergey, Rene Brun 10/05/2004

MODDIR       := xml
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

XMLDIR       := $(MODDIR)
XMLDIRS      := $(XMLDIR)/src
XMLDIRI      := $(XMLDIR)/inc

##### libXMLIO #####
XMLL         := $(MODDIRI)/LinkDef.h
XMLDS        := $(MODDIRS)/G__XML.cxx
XMLDO        := $(XMLDS:.cxx=.o)
XMLDH        := $(XMLDS:.cxx=.h)

#LF
XMLTMPDS    := $(MODDIRS)/G__XMLTmp.cxx
XMLTMPDO    := $(XMLTMPDS:.cxx=.o)
XMLTMPDH    := $(XMLTMPDS:.cxx=.h)
XMLTMP2DS   := $(MODDIRS)/G__XMLTmp2.cxx
XMLTMP2DO   := $(XMLTMP2DS:.cxx=.o)
XMLTMP2DH   := $(XMLTMP2DS:.cxx=.h)

XMLH         := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
XMLS         := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
XMLO         := $(XMLS:.cxx=.o)

XMLDEP       := $(XMLO:.o=.d) $(XMLDO:.o=.d)

#LF
XMLTMPDEP  := $(XMLTMPDO:.o=.d)

XMLLIB       := $(LPATH)/libXMLIO.$(SOEXT)
XMLMAP       := $(XMLLIB:.$(SOEXT)=.rootmap)

#LF
XMLNM       := $(XMLLIB:.$(SOEXT)=.nm)

# used in the main Makefile
ALLHDRS     += $(patsubst $(MODDIRI)/%.h,include/%.h,$(XMLH))
ALLLIBS     += $(XMLLIB)
ALLMAPS     += $(XMLMAP)

# include all dependency files
INCLUDEFILES += $(XMLDEP)

##### local rules #####
include/%.h:    $(XMLDIRI)/%.h
		cp $< $@

#LF
$(XMLLIB):   $(XMLO) $(XMLTMPDO) $(XMLTMP2DO) $(XMLDO) $(ORDER_) $(MAINLIBS) $(XMLLIBDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		   "$(SOFLAGS)" libXMLIO.$(SOEXT) $@ "$(XMLO) $(XMLTMPDO) $(XMLTMP2DO) $(XMLDO)" \
		   "$(XMLLIBEXTRA)"

#LF
$(XMLTMPDS):   $(XMLH) $(XMLL) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(XMLH) $(XMLL)

#LF
$(XMLTMP2DS):  $(XMLH) $(XMLL) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(XMLH) $(XMLL)

#LF
$(XMLDS):    $(XMLH) $(XMLL) $(ROOTCINTTMPEXE) $(XMLNM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(XMLNM) -. 3 -c $(XMLH) $(XMLL)

#LF
$(XMLNM):      $(XMLO) $(XMLTMPDO) $(XMLTMP2DO) 
		@echo "Generating symbols file $@..."
		nm -g -p --defined-only $(XMLTMPDO) | awk '{printf("%s\n", $$3)'} > $(XMLNM)
		nm -g -p --defined-only $(XMLTMP2DO) | awk '{printf("%s\n", $$3)'} >> $(XMLNM)
		nm -g -p --defined-only $(XMLO) | awk '{printf("%s\n", $$3)'} >> $(XMLNM)

$(XMLMAP):      $(RLIBMAP) $(MAKEFILEDEP) $(XMLL)
		$(RLIBMAP) -o $(XMLMAP) -l $(XMLLIB) \
		   -d $(XMLLIBDEPM) -c $(XMLL)

all-xml:        $(XMLLIB) $(XMLMAP)

clean-xml:
		@rm -f $(XMLO) $(XMLDO)

clean::         clean-xml clean-pds-xml

#LF
clean-pds-xml:	
		rm -f $(XMLTMPDS) $(XMLTMPDO) $(XMLTMPDH) \
		$(XMLTMPDEP) $(XMLTMP2DS) $(XMLTMP2DO) $(XMLTMP2DH) $(XMLNM)

distclean-xml:  clean-xml
		@rm -f $(XMLDEP) $(XMLDS) $(XMLDH) $(XMLLIB) $(XMLMAP)

distclean::     distclean-xml

