# Module.mk for math module
# Copyright (c) 2007 Rene Brun and Fons Rademakers
#
# Author: Rene Brun 06/02/2007

MODDIR       := math
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

MATHDIR      := $(MODDIR)
MATHDIRS     := $(MATHDIR)/src
MATHDIRI     := $(MATHDIR)/inc

##### libMath  #####
MATHL        := $(MODDIRI)/LinkDef.h
MATHDS       := $(MODDIRS)/G__Math.cxx
MATHDO       := $(MATHDS:.cxx=.o)
MATHDH       := $(MATHDS:.cxx=.h)

#LF
MATHTMPDS    := $(MODDIRS)/G__MathTmp.cxx
MATHTMPDO    := $(MATHTMPDS:.cxx=.o)
MATHTMPDH    := $(MATHTMPDS:.cxx=.h)
MATHTMP2DS    := $(MODDIRS)/G__MathTmp2.cxx
MATHTMP2DO    := $(MATHTMP2DS:.cxx=.o)
MATHTMP2DH    := $(MATHTMP2DS:.cxx=.h)

MATHH1       := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
MATHH2       := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/Math/*.h))
MATHH        := $(MATHH1) $(MATHH2)
MATHS        := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
MATHO        := $(MATHS:.cxx=.o)

MATHDEP      := $(MATHO:.o=.d) $(MATHDO:.o=.d)

#LF
MATHTMPDEP   := $(MATHTMPDO:.o=.d)

# LF
MATHMAP      := $(MATHLIB:.$(SOEXT)=.rootmap)
MATHLIB      := $(MODDIRS)/libRMath.$(SOEXT)
MATHNM       := $(MATHLIB:.$(SOEXT)=.nm)
#MATHDICTLIB  := $(LPATH)/libRMathDict.$(SOEXT)
#MATHDICTMAP  := $(MATHDICTLIB:.$(SOEXT)=.rootmap)

#LF
MATHNM       := $(MATHLIB:.$(SOEXT)=.nm)

# used in the main Makefile
ALLHDRS      += $(patsubst $(MODDIRI)/%.h,include/%.h,$(MATHH))
#ALLLIBS     += $(MATHLIB)
#ALLMAPS     += $(MATHMAP)

# include all dependency files
INCLUDEFILES += $(MATHDEP)

##### local rules #####
include/Math/%.h: $(MATHDIRI)/Math/%.h
		@(if [ ! -d "include/Math" ]; then     \
		   mkdir -p include/Math;              \
		fi)
		cp $< $@


include/%.h:    $(MATHDIRI)/%.h
		cp $< $@

#LF
#$(MATHLIB):     $(MATHO) $(MATHTMPDO) $(ORDER_) $(MAINLIBS)
#		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
#		"$(SOFLAGS)" libRMath.$(SOEXT) $@ "$(MATHO)" $(MATHTMPDO)\
#		"$(MATHLIBEXTRA)"

#LF
#$(MATHDICTLIB): $(MATHDO) $(ORDER_) $(MAINLIBS)
#		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
#		"$(SOFLAGS)" libRMathDict.$(SOEXT) $@ "$(MATHDO)" "$(MATHTMP2DO)"\
#		"$(MATHDICTLIBEXTRA)"

#LF
$(MATHTMPDS):   $(MATHH) $(MATHL) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(MATHH) $(MATHL)

#LF
$(MATHTMP2DS):  $(MATHH) $(MATHL) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(MATHH) $(MATHL)

#LF
$(MATHDS):      $(MATHH) $(MATHL) $(ROOTCINTTMPEXE) $(MATHNM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(MATHNM) -. 3 -c $(MATHH) $(MATHL)

#LF
#$(MATHDICTMAP): $(RLIBMAP) $(MAKEFILEDEP) $(MATHL)
#		$(RLIBMAP) -o $(MATHDICTMAP) -l $(MATHDICTLIB) \
3		-d $(MATHLIB) $(MATHLIBDEPM) -c $(MATHL)
#LF
$(MATHNM):      $(MATHO) $(MATHTMPDO) $(MATHTMP2DO) 
		@echo "Generating symbols file $@..."
		nm -p --defined-only $(MATHTMPDO) | awk '{printf("%s\n", $$3)'} > $(MATHNM)
		nm -p --defined-only $(MATHTMP2DO) | awk '{printf("%s\n", $$3)'} >> $(MATHNM)
		nm -p --defined-only $(MATHO) | awk '{printf("%s\n", $$3)'} >> $(MATHNM)

#all-math:       $(MATHLIB) $(MATHMAP)
all-math:       $(MATHO) $(MATHTMPDO) $(MATHTMP2DO) $(MATHDO)

clean-math:
		@rm -f $(MATHO) $(MATHTMPDO) $(MATHTMP2DO) $(MATHDO)

clean::         clean-math clean-pds-math

#LF
clean-pds-math:	
		rm -f $(MATHTMPDS) $(MATHTMPDO) $(MATHTMPDH) \
		$(MATHTMPDEP) $(MATHTMP2DS) $(MATHTMP2DO) $(MATHTMP2DH) $(MATHNM)

distclean-math: clean-math
		@rm -f $(MATHDEP) $(MATHDS) $(MATHDH) $(MATHLIB) $(MATHMAP)
		@rm -rf include/Math

distclean::     distclean-math
