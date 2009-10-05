# Module.mk for mathnag module
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: Fons Rademakers, 29/2/2000

MODNAME      := mathnag
MODDIR       := math/$(MODNAME)
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

MATHNAGDIR  := $(MODDIR)
MATHNAGDIRS := $(MATHNAGDIR)/src
MATHNAGDIRI := $(MATHNAGDIR)/inc


###pre-compiled GSL DLL require Mathnag to be compiled with -DGSL_DLL
#ifeq ($(PLATFORM),win32)
#GSLFLAGS += "-DGSL_DLL"
#endif

##### libMathnag #####
MATHNAGL    := $(MODDIRI)/Math/LinkDef.h
MATHNAGDS   := $(MODDIRS)/G__Mathnag.cxx
MATHNAGDO   := $(MATHNAGDS:.cxx=.o)
MATHNAGDH   := $(MATHNAGDS:.cxx=.h)
MATHNAGDH1  := 
#$(MODDIRI)/Math/DistFuncMathnag.h \

MATHNAGH    := $(filter-out $(MODDIRI)/Math/LinkDef%,$(wildcard $(MODDIRI)/Math/*.h))
MATHNAGS    := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
MATHNAGO    := $(MATHNAGS:.cxx=.o)

MATHNAGDEP  := $(MATHNAGO:.o=.d) $(MATHNAGDO:.o=.d)

MATHNAGLIB  := $(LPATH)/libMathnag.$(SOEXT)
MATHNAGMAP  := $(MATHNAGLIB:.$(SOEXT)=.rootmap)

# used in the main Makefile
ALLHDRS      += $(patsubst $(MODDIRI)/Math/%.h,include/Math/%.h,$(MATHNAGH))
ALLLIBS      += $(MATHNAGLIB)
ALLMAPS      += $(MATHNAGMAP)

ifeq ($(PLATFORM),macosx)
NAGLIBS := -lnagc_nag
endif
# for linux 64 bits
ifeq ($(PLATFORM),linux)
NAGLIBS := /afs/cern.ch/pttools/nag/Mark8/cll6a08dgl/lib/libnagc_nag.a
NAGFLAGS := -I/afs/cern.ch/pttools/nag/Mark8/cll6a08dgl/include
endif

# include all dependency files
INCLUDEFILES += $(MATHNAGDEP)

##### local rules #####
include/Math/%.h: $(MATHNAGDIRI)/Math/%.h
		@(if [ ! -d "include/Math" ]; then     \
		   mkdir -p include/Math;              \
		fi)
		cp $< $@

$(MATHNAGLIB): $(MATHNAGO)  $(ORDER_) $(MAINLIBS)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)"  \
		   "$(SOFLAGS)" libMathnag.$(SOEXT) $@     \
		   "$(MATHNAGO) "             \
		   "$(MATHNAGLIBEXTRA)  $(NAGLIBS)"

$(MATHNAGDS):  $(MATHNAGDH1) $(MATHNAGL) $(MATHNAGLINC) $(ROOTCINTTMPEXE)
		@echo "Generating dictionary $@..."
		$(ROOTCINTTMP)  -f  $@ -c $(MATHNAGDH1)  $(MATHNAGL)

$(MATHNAGMAP): $(RLIBMAP) $(MAKEFILEDEP) $(MATHNAGL) $(MATHNAGLINC)
		$(RLIBMAP) -o $(MATHNAGMAP) -l $(MATHNAGLIB) \
		   -d $(MATHNAGLIBDEPM) -c $(MATHNAGL) $(MATHNAGLINC)

all-mathnag:   $(MATHNAGLIB) $(MATHNAGMAP)

clean-mathnag:
		@rm -f $(MATHNAGO) $(MATHNAGDO)

clean::         clean-mathnag

distclean-mathnag: clean-mathnag
		@rm -f $(MATHNAGDEP) $(MATHNAGDS) $(MATHNAGDH) \
		   $(MATHNAGLIB) $(MATHNAGMAP)
		@rm -rf include/Math

distclean::     distclean-mathnag

##### extra rules ######
$(MATHNAGO): CXXFLAGS += $(NAGFLAGS)  -DUSE_ROOT_ERROR
$(MATHNAGDO): CXXFLAGS += $(NAGFLAGS)  -DUSE_ROOT_ERROR
