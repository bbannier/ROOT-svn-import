# Module.mk for io module
# Copyright (c) 2007 Rene Brun and Fons Rademakers
#
# Author: Rene Brun 06/02/2007

MODDIR       := io
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

IODIR        := $(MODDIR)
IODIRS       := $(IODIR)/src
IODIRI       := $(IODIR)/inc

##### libRIO #####
IOL          := $(MODDIRI)/LinkDef.h
IODS         := $(MODDIRS)/G__IO.cxx
IODO         := $(IODS:.cxx=.o)
IODH         := $(IODS:.cxx=.h)

#LF
IOTMPDS    := $(MODDIRS)/G__IOTmp.cxx
IOTMPDO    := $(IOTMPDS:.cxx=.o)
IOTMPDH    := $(IOTMPDS:.cxx=.h)
IOTMP2DS    := $(MODDIRS)/G__IOTmp2.cxx
IOTMP2DO    := $(IOTMP2DS:.cxx=.o)
IOTMP2DH    := $(IOTMP2DS:.cxx=.h)

IOH          := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
IOS          := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
IOO          := $(IOS:.cxx=.o)

IODEP        := $(IOO:.o=.d) $(IODO:.o=.d)

#LF
IOTMPDEP   := $(IOTMPDO:.o=.d)

IOLIB        := $(LPATH)/libRIO.$(SOEXT)

#LF
IOMAP        := $(IOLIB:.$(SOEXT)=.rootmap)
IODICTLIB  := $(LPATH)/libRIODict.$(SOEXT)
IODICTMAP  := $(IODICTLIB:.$(SOEXT)=.rootmap)

#LF
IONM       := $(IOLIB:.$(SOEXT)=.nm)

# used in the main Makefile
ALLHDRS      += $(patsubst $(MODDIRI)/%.h,include/%.h,$(IOH))
ALLLIBS      += $(IOLIB)
ALLMAPS      += $(IOMAP)

# include all dependency files
INCLUDEFILES += $(IODEP)

##### local rules #####
include/%.h:    $(IODIRI)/%.h
		cp $< $@

#LF
$(IOLIB):     $(IOO) $(IOTMPDO) $(IOTMP2DO) $(IODO) $(ORDER_) $(MAINLIBS)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		"$(SOFLAGS)" libRIO.$(SOEXT) $@ "$(IOO) $(IOTMPDO) $(IOTMP2DO) $(IODO)"\
		"$(IOLIBEXTRA)"

#LF
#$(IODICTLIB): $(IODO) $(ORDER_) $(MAINLIBS)
#		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
#		"$(SOFLAGS)" libRIODict.$(SOEXT) $@ "$(IODO)" "$(IOTMP2DO)"\
#		"$(IODICTLIBEXTRA)"

#LF
$(IOTMPDS):   $(IOH) $(IOL) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(IOH) $(IOL)

#LF
$(IOTMP2DS):  $(IOH) $(IOL) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(IOH) $(IOL)

#LF
$(IODS):      $(IOH) $(IOL) $(ROOTCINTTMPEXE) $(IONM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(IONM) -. 3 -c $(IOH) $(IOL)

#LF
$(IOMAP): $(RLIBMAP) $(MAKEFILEDEP) $(IOL)
		$(RLIBMAP) -o $(IOMAP) -l $(IOLIB) \
		-d $(IOLIBDEPM) -c $(IOL)
#LF
$(IONM):      $(IOO) $(IOTMPDO) $(IOTMP2DO)
		@echo "Generating symbols file $@..."
		nm -g -p --defined-only $(IOTMPDO) | awk '{printf("%s\n", $$3)'} > $(IONM)
		nm -g -p --defined-only $(IOTMP2DO) | awk '{printf("%s\n", $$3)'} >> $(IONM)
		nm -g -p --defined-only $(IOO) | awk '{printf("%s\n", $$3)'} >> $(IONM)

all-io:         $(IOLIB) $(IOMAP)

clean-io:
		@rm -f $(IOO) $(IODO)

clean::         clean-io clean-pds-io

#LF
clean-pds-io:	
		rm -f $(IOTMPDS) $(IOTMPDO) $(IOTMPDH) \
		$(IOTMPDEP) $(IOTMP2DS) $(IOTMP2DO) $(IOTMP2DH) $(IONM)

distclean-io:   clean-io
		@rm -f $(IODEP) $(IODS) $(IODH) $(IOLIB) $(IOMAP)

distclean::     distclean-io
