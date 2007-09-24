# Module.mk for matrix module
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: Fons Rademakers, 29/2/2000

MODDIR       := matrix
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

MATRIXDIR    := $(MODDIR)
MATRIXDIRS   := $(MATRIXDIR)/src
MATRIXDIRI   := $(MATRIXDIR)/inc

##### libMatrix #####
MATRIXL      := $(MODDIRI)/LinkDef.h
MATRIXDS     := $(MODDIRS)/G__Matrix.cxx
MATRIXDO     := $(MATRIXDS:.cxx=.o)
MATRIXDH     := $(MATRIXDS:.cxx=.h)

#LF
MATRIXTMPDS    := $(MODDIRS)/G__MatrixTmp.cxx
MATRIXTMPDO    := $(MATRIXTMPDS:.cxx=.o)
MATRIXTMPDH    := $(MATRIXTMPDS:.cxx=.h)
MATRIXTMP2DS   := $(MODDIRS)/G__MatrixTmp2.cxx
MATRIXTMP2DO   := $(MATRIXTMP2DS:.cxx=.o)
MATRIXTMP2DH   := $(MATRIXTMP2DS:.cxx=.h)

MATRIXH      := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
MATRIXS      := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
MATRIXO      := $(MATRIXS:.cxx=.o)

MATRIXDEP    := $(MATRIXO:.o=.d) $(MATRIXDO:.o=.d)

#LF
MATRIXTMPDEP  := $(MATRIXTMPDO:.o=.d)

MATRIXLIB    := $(LPATH)/libMatrix.$(SOEXT)

#LF
MATRIXMAP    := $(MATRIXLIB:.$(SOEXT)=.rootmap)
MATRIXDICTLIB  := $(LPATH)/libMatrixDict.$(SOEXT)
MATRIXDICTMAP  := $(MATRIXDICTLIB:.$(SOEXT)=.rootmap)

# used in the main Makefile
ALLHDRS     += $(patsubst $(MODDIRI)/%.h,include/%.h,$(MATRIXH))
ALLLIBS     += $(MATRIXLIB)
ALLMAPS     += $(MATRIXMAP)

#LF
MATRIXNM       := $(MATRIXLIB:.$(SOEXT)=.nm)

# include all dependency files
INCLUDEFILES += $(MATRIXDEP)

##### local rules #####
include/%.h:    $(MATRIXDIRI)/%.h
		cp $< $@

#LF
$(MATRIXLIB):   $(MATRIXO) $(MATRIXTMPDO) $(MATRIXTMP2DO) $(MATRIXDO) $(ORDER_) $(MAINLIBS) $(MATRIXLIBDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		   "$(SOFLAGS)" libMatrix.$(SOEXT) $@ "$(MATRIXO) $(MATRIXTMPDO) $(MATRIXTMP2DO) $(MATRIXDO)" \
		   "$(MATRIXLIBEXTRA)"
#LF
$(MATRIXDICTLIB):   $(MATRIXDO) $(ORDER_) $(MAINLIBS) $(MATRIXDICTLIBDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		   "$(SOFLAGS)" libMatrixDict.$(SOEXT) $@ "$(MATRIXO) $(MATRIXTMP2DO)" \
		   "$(MATRIXLIBEXTRA)"
#LF
$(MATRIXTMPDS):   $(MATRIXH) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(MATRIXH) $(MATRIXL)

#LF
$(MATRIXTMP2DS):  $(MATRIXH) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(MATRIXH) $(MATRIXL)

#LF
$(MATRIXDS):    $(MATRIXH) $(MATRIXL) $(ROOTCINTTMPEXE) $(MATRIXNM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(MATRIXNM) -. 3 -c $(MATRIXH) $(MATRIXL)

#LF
$(MATRIXDICTMAP): $(RLIBMAP) $(MAKEFILEDEP) $(MATRIXL)
		$(RLIBMAP) -o $(MATRIXDICTMAP) -l $(MATRIXDICTLIB) \
		-d $(MATRIXLIB) $(MATRIXLIBDEPM) -c $(MATRIXL)
#LF
$(MATRIXNM):      $(MATRIXO) $(MATRIXTMPDO) $(MATRIXTMP2DO) 
		@echo "Generating symbols file $@..."
		nm -g -p --defined-only $(MATRIXTMPDO) | awk '{printf("%s\n", $$3)'} > $(MATRIXNM)
		nm -g -p --defined-only $(MATRIXTMP2DO) | awk '{printf("%s\n", $$3)'} >> $(MATRIXNM)
		nm -g -p --defined-only $(MATRIXO) | awk '{printf("%s\n", $$3)'} >> $(MATRIXNM)

#LF
$(MATRIXMAP):   $(RLIBMAP) $(MAKEFILEDEP) $(MATRIXL)
		$(RLIBMAP) -o $(MATRIXMAP) -l $(MATRIXLIB) \
		   -d $(MATRIXLIBDEPM) -c $(MATRIXL)

#LF
all-matrix:     $(MATRIXLIB) $(MATRIXMAP)

clean-matrix:
		@rm -f $(MATRIXO) $(MATRIXDO) $(MATRIXNM)

clean::         clean-matrix clean-pds-matrix

#LF
clean-pds-matrix:	
		rm -f $(MATRIXTMPDS) $(MATRIXTMPDO) $(MATRIXTMPDH) \
		$(MATRIXTMPDEP) $(MATRIXTMP2DS) $(MATRIXTMP2DO) $(MATRIXTMP2DH) $(MATRIXNM)

#LF
distclean-matrix: clean-matrix
		@rm -f $(MATRIXDEP) $(MATRIXDS) $(MATRIXDH) $(MATRIXLIB) $(MATRIXDICTMAP)

distclean::     distclean-matrix
