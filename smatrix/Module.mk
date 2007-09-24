# Module.mk for mathcore module
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: Fons Rademakers, 20/6/2005

MODDIR       := smatrix
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

SMATRIXDIR  := $(MODDIR)
SMATRIXDIRS := $(SMATRIXDIR)/src
SMATRIXDIRI := $(SMATRIXDIR)/inc/Math

##### libSmatrix #####
SMATRIXL    := $(MODDIRI)/LinkDef.h
SMATRIXL32  := $(MODDIRI)/LinkDefD32.h
#SMATRIXLINC :=
SMATRIXDS   := $(MODDIRS)/G__Smatrix.cxx
SMATRIXDS32 := $(MODDIRS)/G__Smatrix32.cxx
SMATRIXDO   := $(SMATRIXDS:.cxx=.o)
SMATRIXDO32 := $(SMATRIXDS32:.cxx=.o)
SMATRIXDH   := $(SMATRIXDS:.cxx=.h)

SMATRIXDH1  :=  $(MODDIRI)/Math/SMatrix.h \
		$(MODDIRI)/Math/SVector.h
#		$(MODDIRI)/Math/SMatrixDfwd.h \
#		$(MODDIRI)/Math/SMatrixFfwd.h \
#		$(MODDIRI)/Math/SMatrixD32fwd.h



SMATRIXH1   := $(filter-out $(MODDIRI)/Math/LinkDef%, $(wildcard $(MODDIRI)/Math/*.h))
SMATRIXH2   := $(filter-out $(MODDIRI)/Math/LinkDef%, $(wildcard $(MODDIRI)/Math/*.icc))
SMATRIXH    := $(SMATRIXH1) $(SMATRIXH2)
SMATRIXS    := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
SMATRIXO    := $(SMATRIXS:.cxx=.o)

#LF
SMATRIXTMPDS    := $(MODDIRS)/G__SmatrixTmp.cxx
SMATRIXTMPDO    := $(SMATRIXTMPDS:.cxx=.o)
SMATRIXTMPDH    := $(SMATRIXTMPDS:.cxx=.h)
SMATRIXTMP2DS   := $(MODDIRS)/G__SmatrixTmp2.cxx
SMATRIXTMP2DO   := $(SMATRIXTMP2DS:.cxx=.o)
SMATRIXTMP2DH   := $(SMATRIXTMP2DS:.cxx=.h)

#LF
SMATRIXTMPDS32    := $(MODDIRS)/G__Smatrix32Tmp.cxx
SMATRIXTMPDO32    := $(SMATRIXTMPDS32:.cxx=.o)
SMATRIXTMPDH32    := $(SMATRIXTMPDS32:.cxx=.h)
SMATRIXTMP2DS32   := $(MODDIRS)/G__Smatrix32Tmp2.cxx
SMATRIXTMP2DO32   := $(SMATRIXTMP2DS32:.cxx=.o)
SMATRIXTMP2DH32   := $(SMATRIXTMP2DS32:.cxx=.h)

SMATRIXDEP  := $(SMATRIXO:.o=.d)  $(SMATRIXDO:.o=.d)

#LF
SMATRIXTMPDEP  := $(SMATRIXTMPDO:.o=.d)

SMATRIXLIB  := $(LPATH)/libSmatrix.$(SOEXT)
SMATRIXMAP  := $(SMATRIXLIB:.$(SOEXT)=.rootmap)

#LF
SMATRIXNM       := $(SMATRIXLIB:.$(SOEXT)=.nm)

# used in the main Makefile
ALLHDRS      += $(patsubst $(MODDIRI)/Math/%.h,include/Math/%.h,$(SMATRIXH1))
ALLHDRS      += $(patsubst $(MODDIRI)/Math/%.icc,include/Math/%.icc,$(SMATRIXH2))
ALLLIBS      += $(SMATRIXLIB)
ALLMAPS      += $(SMATRIXMAP)

# include all dependency files
INCLUDEFILES += $(SMATRIXDEP)

##### local rules #####
include/Math/%.h: $(SMATRIXDIRI)/%.h
		@(if [ ! -d "include/Math" ]; then     \
		   mkdir -p include/Math;              \
		fi)
		cp $< $@

include/Math/%.icc: $(SMATRIXDIRI)/%.icc
		@(if [ ! -d "include/Math" ]; then     \
		   mkdir -p include/Math;              \
		fi)
		cp $< $@

#LF
$(SMATRIXLIB):   $(SMATRIXO) $(SMATRIXTMPDO) $(SMATRIXTMP2DO) $(SMATRIXDO) \
		$(SMATRIXTMPDO32) $(SMATRIXTMP2DO32) $(SMATRIXDO32) $(ORDER_) $(MAINLIBS) $(SMATRIXLIBDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		   "$(SOFLAGS)" libSmatrix.$(SOEXT) $@ "$(SMATRIXO) \
			$(SMATRIXTMPDO) $(SMATRIXTMP2DO) $(SMATRIXDO) \
			$(SMATRIXTMPDO32) $(SMATRIXTMP2DO32) $(SMATRIXDO32)" \
		   "$(SMATRIXLIBEXTRA)"

#LF
$(SMATRIXTMPDS):   $(SMATRIXDH1) $(SMATRIXL) $(SMATRIXLINC) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(SMATRIXDH1) $(SMATRIXL)

#LF
$(SMATRIXTMP2DS):  $(SMATRIXDH1) $(SMATRIXL) $(SMATRIXLINC) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(SMATRIXDH1) $(SMATRIXL)

#LF
$(SMATRIXDS):    $(SMATRIXDH1) $(SMATRIXL) $(SMATRIXLINC) $(ROOTCINTTMPEXE) $(SMATRIXNM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(SMATRIXNM) -. 3 -c $(SMATRIXDH1) $(SMATRIXL)


#LF
$(SMATRIXTMPDS32):   $(SMATRIXDH1) $(SMATRIXL32) $(SMATRIXLINC) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(SMATRIXDH1) $(SMATRIXL32)

#LF
$(SMATRIXTMP2DS32):  $(SMATRIXDH1) $(SMATRIXL32) $(SMATRIXLINC) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(SMATRIXDH1) $(SMATRIXL32)

#LF
$(SMATRIXDS32):    $(SMATRIXDH1) $(SMATRIXL32) $(SMATRIXLINC) $(ROOTCINTTMPEXE) $(SMATRIXNM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(SMATRIXNM) -. 3 -c $(SMATRIXDH1) $(SMATRIXL32)

#LF
$(SMATRIXDICTMAP): $(RLIBMAP) $(MAKEFILEDEP) $(SMATRIXL)
		$(RLIBMAP) -o $(SMATRIXDICTMAP) -l $(SMATRIXDICTLIB) \
		-d $(SMATRIXLIB) $(SMATRIXLIBDEPM) -c $(SMATRIXL)
#LF
$(SMATRIXNM):      $(SMATRIXO) $(SMATRIXTMPDO) $(SMATRIXTMP2DO) $(SMATRIXTMPDO32) $(SMATRIXTMP2DO32)
		@echo "Generating symbols file $@..."
		nm -g -p --defined-only $(SMATRIXTMPDO) | awk '{printf("%s\n", $$3)'} > $(SMATRIXNM)
		nm -g -p --defined-only $(SMATRIXTMP2DO) | awk '{printf("%s\n", $$3)'} >> $(SMATRIXNM)
		nm -g -p --defined-only $(SMATRIXTMPDO32) | awk '{printf("%s\n", $$3)'} >> $(SMATRIXNM)
		nm -g -p --defined-only $(SMATRIXTMP2DO32) | awk '{printf("%s\n", $$3)'} >> $(SMATRIXNM)
		nm -g -p --defined-only $(SMATRIXO) | awk '{printf("%s\n", $$3)'} >> $(SMATRIXNM)

$(SMATRIXMAP):  $(RLIBMAP) $(MAKEFILEDEP) $(SMATRIXL) $(SMATRIXLINC)
		$(RLIBMAP) -o $(SMATRIXMAP) -l $(SMATRIXLIB) \
		   -d $(SMATRIXLIBDEPM) -c $(SMATRIXL) $(SMATRIXLINC)

ifneq ($(ICC_MAJOR),)
# silence warning messages about subscripts being out of range
$(SMATRIXDO): CXXFLAGS += -wd175 -I$(SMATRIXDIRI)
else
$(SMATRIXDO): CXXFLAGS += -I$(SMATRIXDIRI)
endif

all-smatrix:   $(SMATRIXLIB) $(SMATRIXMAP)

clean-smatrix:
		@rm -f $(SMATRIXO) $(SMATRIXDO)

clean::         clean-smatrix clean-pds-smatrix

#LF
clean-pds-smatrix:	
		rm -f $(SMATRIXTMPDS) $(SMATRIXTMPDO) $(SMATRIXTMPDH) \
		$(SMATRIXTMPDEP) $(SMATRIXTMP2DS) $(SMATRIXTMP2DO) $(SMATRIXTMP2DH) $(SMATRIXNM)

distclean-smatrix: clean-smatrix
		@rm -f $(SMATRIXDEP) $(SMATRIXDS) $(SMATRIXDH) $(SMATRIXLIB) $(SMATRIXMAP)
		@rm -rf include/Math
		-@cd $(SMATRIXDIR)/test && $(MAKE) distclean

distclean::     distclean-smatrix

test-smatrix: 	all-smatrix
		@cd $(SMATRIXDIR)/test && $(MAKE)

check-smatrix: 	test-smatrix
		@cd $(SMATRIXDIR)/test && $(MAKE)
