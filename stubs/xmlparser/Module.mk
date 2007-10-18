# Module.mk for xmlparser module
# Copyright (c) 2004 Rene Brun and Fons Rademakers
#
# Authors: Jose Lo

MODDIR        := xmlparser
MODDIRS       := $(MODDIR)/src
MODDIRI       := $(MODDIR)/inc

XMLPARSERDIR  := $(MODDIR)
XMLPARSERDIRS := $(XMLPARSERDIR)/src
XMLPARSERDIRI := $(XMLPARSERDIR)/inc

##### libXMLParser #####
XMLPARSERL    := $(MODDIRI)/LinkDef.h
XMLPARSERDS   := $(MODDIRS)/G__XMLParser.cxx
XMLPARSERDO   := $(XMLPARSERDS:.cxx=.o)
XMLPARSERDH   := $(XMLPARSERDS:.cxx=.h)

#LF
XMLPARSERTMPDS    := $(MODDIRS)/G__XMLParserTmp.cxx
XMLPARSERTMPDO    := $(XMLPARSERTMPDS:.cxx=.o)
XMLPARSERTMPDH    := $(XMLPARSERTMPDS:.cxx=.h)
XMLPARSERTMP2DS   := $(MODDIRS)/G__XMLParserTmp2.cxx
XMLPARSERTMP2DO   := $(XMLPARSERTMP2DS:.cxx=.o)
XMLPARSERTMP2DH   := $(XMLPARSERTMP2DS:.cxx=.h)

XMLPARSERH    := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
XMLPARSERS    := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
XMLPARSERO    := $(XMLPARSERS:.cxx=.o)

XMLPARSERDEP  := $(XMLPARSERO:.o=.d) $(XMLPARSERDO:.o=.d)

#LF
XMLPARSERTMPDEP  := $(XMLPARSERTMPDO:.o=.d)

XMLPARSERLIB  := $(LPATH)/libXMLParser.$(SOEXT)
XMLPARSERMAP  := $(XMLPARSERLIB:.$(SOEXT)=.rootmap)

#LF
XMLPARSERNM       := $(XMLPARSERLIB:.$(SOEXT)=.nm)

# used in the main Makefile
ALLHDRS      += $(patsubst $(MODDIRI)/%.h,include/%.h,$(XMLPARSERH))
ALLLIBS      += $(XMLPARSERLIB)
ALLMAPS      += $(XMLPARSERMAP)

# include all dependency files
INCLUDEFILES += $(XMLPARSERDEP)

##### local rules #####
include/%.h:    $(XMLPARSERDIRI)/%.h
		cp $< $@

#LF
$(XMLPARSERLIB):   $(XMLPARSERO) $(XMLPARSERTMPDO) $(XMLPARSERTMP2DO) $(XMLPARSERDO) $(ORDER_) $(MAINLIBS)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		   "$(SOFLAGS)" libXMLParser.$(SOEXT) $@ "$(XMLPARSERO) $(XMLPARSERTMPDO) $(XMLPARSERTMP2DO) $(XMLPARSERDO)" \
		   "$(XMLLIBDIR) $(XMLCLILIB)"

#LF
$(XMLPARSERTMPDS):   $(XMLPARSERH) $(XMLPARSERL) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(XMLPARSERH) $(XMLPARSERL)

#LF
$(XMLPARSERTMP2DS):  $(XMLPARSERH) $(XMLPARSERL) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(XMLPARSERH) $(XMLPARSERL)

#LF
$(XMLPARSERDS):    $(XMLPARSERH) $(XMLPARSERL) $(ROOTCINTTMPEXE) $(XMLPARSERNM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(XMLPARSERNM) -. 3 -c $(XMLPARSERH) $(XMLPARSERL)

#LF
$(XMLPARSERNM):      $(XMLPARSERO) $(XMLPARSERTMPDO) $(XMLPARSERTMP2DO) 
		@echo "Generating symbols file $@..."
		nm -p --defined-only $(XMLPARSERTMPDO) | awk '{printf("%s\n", $$3)'} > $(XMLPARSERNM)
		nm -p --defined-only $(XMLPARSERTMP2DO) | awk '{printf("%s\n", $$3)'} >> $(XMLPARSERNM)
		nm -p --defined-only $(XMLPARSERO) | awk '{printf("%s\n", $$3)'} >> $(XMLPARSERNM)

$(XMLPARSERMAP): $(RLIBMAP) $(MAKEFILEDEP) $(XMLPARSERL)
		$(RLIBMAP) -o $(XMLPARSERMAP) -l $(XMLPARSERLIB) \
		   -d $(XMLPARSERLIBDEPM) -c $(XMLPARSERL)

all-xmlparser:  $(XMLPARSERLIB) $(XMLPARSERMAP)

clean-xmlparser:
		@rm -f $(XMLPARSERO) $(XMLPARSERDO)

clean::         clean-xmlparser clean-pds-xmlparser

#LF
clean-pds-xmlparser:	
		rm -f $(XMLPARSERTMPDS) $(XMLPARSERTMPDO) $(XMLPARSERTMPDH) \
		$(XMLPARSERTMPDEP) $(XMLPARSERTMP2DS) $(XMLPARSERTMP2DO) $(XMLPARSERTMP2DH) $(XMLPARSERNM)

distclean-xmlparser: clean-xmlparser
		@rm -f $(XMLPARSERDEP) $(XMLPARSERDS) $(XMLPARSERDH) \
		   $(XMLPARSERLIB) $(XMLPARSERMAP)

distclean::     distclean-xmlparser

##### extra rules ######
$(XMLPARSERO): CXXFLAGS += $(XMLINCDIR:%=-I%)
