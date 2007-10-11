# Module.mk for base module
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: Fons Rademakers, 29/2/2000

MODDIR       := base
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

BASEDIR      := $(MODDIR)
BASEDIRS     := $(BASEDIR)/src
BASEDIRI     := $(BASEDIR)/inc

##### libBase (part of libCore) #####
BASEL1       := $(MODDIRI)/LinkDef1.h
BASEL2       := $(MODDIRI)/LinkDef2.h
BASEL3       := $(MODDIRI)/LinkDef3.h
BASEDS1      := $(MODDIRS)/G__Base1.cxx
BASEDS2      := $(MODDIRS)/G__Base2.cxx
BASEDS3      := $(MODDIRS)/G__Base3.cxx
BASEDO1      := $(BASEDS1:.cxx=.o)
BASEDO2      := $(BASEDS2:.cxx=.o)
BASEDO3      := $(BASEDS3:.cxx=.o)

#LF 20-07-07
#things get messy here... now we have 3 dict files
#for every old file... but in base we had three files
#so now we have 9 files !!!
BASETMPDS1    := $(MODDIRS)/G__Base1Tmp.cxx
BASETMPDO1    := $(BASETMPDS1:.cxx=.o)
BASETMPDH1    := $(BASETMPDS1:.cxx=.h)
BASETMP2DS1   := $(MODDIRS)/G__Base1Tmp2.cxx
BASETMP2DO1   := $(BASETMP2DS1:.cxx=.o)
BASETMP2DH1   := $(BASETMP2DS1:.cxx=.h)

BASETMPDS2    := $(MODDIRS)/G__Base2Tmp.cxx
BASETMPDO2    := $(BASETMPDS2:.cxx=.o)
BASETMPDH2    := $(BASETMPDS2:.cxx=.h)
BASETMP2DS2   := $(MODDIRS)/G__Base2Tmp2.cxx
BASETMP2DO2   := $(BASETMP2DS2:.cxx=.o)
BASETMP2DH2   := $(BASETMP2DS2:.cxx=.h)

BASETMPDS3    := $(MODDIRS)/G__Base3Tmp.cxx
BASETMPDO3    := $(BASETMPDS3:.cxx=.o)
BASETMPDH3    := $(BASETMPDS3:.cxx=.h)
BASETMP2DS3   := $(MODDIRS)/G__Base3Tmp2.cxx
BASETMP2DO3   := $(BASETMP2DS3:.cxx=.o)
BASETMP2DH3   := $(BASETMP2DS3:.cxx=.h)

# ManualBase4 only needs to be regenerated (and then changed manually) when
# the dictionary interface changes
BASEL4       := $(MODDIRI)/LinkDef4.h
BASEDS4      := $(MODDIRS)/ManualBase4.cxx
BASEDO4      := $(BASEDS4:.cxx=.o)
BASEH4       := TDirectory.h

#LF
BASETMPDS4    := $(MODDIRS)/ManualBase4Tmp.cxx
BASETMPDO4    := $(BASETMPDS4:.cxx=.o)
BASETMPDH4    := $(BASETMPDS4:.cxx=.h)
BASETMP2DS4   := $(MODDIRS)/ManualBase4Tmp2.cxx
BASETMP2DO4   := $(BASETMP2DS4:.cxx=.o)
BASETMP2DH4   := $(BASETMP2DS4:.cxx=.h)

BASEDS       := $(BASEDS1) $(BASEDS2) $(BASEDS3) $(BASEDS4)
ifeq ($(PLATFORM),win32)
BASEDO       := $(BASEDO1) $(BASEDO2) $(BASEDO3) $(BASEDO4)
else
BASEDO       := $(BASEDO1) $(BASEDO2) $(BASEDO3)
endif
BASEDH       := $(BASEDS:.cxx=.h)

#LF
BASETMPDS       := $(BASETMPDS1) $(BASETMPDS2) $(BASETMPDS3) $(BASETMPDS4)
ifeq ($(PLATFORM),win32)
BASETMPDO       := $(BASETMPDO1) $(BASETMPDO2) $(BASETMPDO3) $(BASETMPDO4)
else
BASETMPDO       := $(BASETMPDO1) $(BASETMPDO2) $(BASETMPDO3)
endif
BASETMPDH       := $(BASETMPDS:.cxx=.h)

#LF
BASETMP2DS       := $(BASETMP2DS1) $(BASETMP2DS2) $(BASETMP2DS3) $(BASETMP2DS4)
ifeq ($(PLATFORM),win32)
BASETMP2DO       := $(BASETMP2DO1) $(BASETMP2DO2) $(BASETMP2DO3) $(BASETMP2DO4)
else
BASETMP2DO       := $(BASETMP2DO1) $(BASETMP2DO2) $(BASETMP2DO3)
endif
BASETMP2DH       := $(BASETMP2DS:.cxx=.h)


BASEH1       := $(wildcard $(MODDIRI)/T*.h)
BASEH3       := GuiTypes.h KeySymbols.h Buttons.h TTimeStamp.h TVirtualMutex.h \
                TVirtualPerfStats.h TVirtualX.h TParameter.h \
                TVirtualAuth.h TFileInfo.h TFileCollection.h \
                TRedirectOutputGuard.h TVirtualMonitoring.h TObjectSpy.h \
                TUrl.h TInetAddress.h
BASEH3       := $(patsubst %,$(MODDIRI)/%,$(BASEH3))
BASEH1       := $(filter-out $(BASEH3),$(BASEH1))
BASEH        := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
ifeq ($(PLATFORM),win32)
BASES        := $(filter-out $(BASEDS4),$(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx)))
else
BASES        := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
endif

BASEO        := $(BASES:.cxx=.o)
#LF
#BASEO        :=$(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.o))

BASEDEP      := $(BASEO:.o=.d) $(BASEDO:.o=.d)

#LF
BASETMPDEP   := $(BASETMPDO:.o=.d)

BASEO        := $(filter-out $(MODDIRS)/precompile.o,$(BASEO))

#LF
BASELIB      := $(MODDIRS)/libBase.$(SOEXT)
#BASEDICTLIB  := $(MODDIRS)/libBaseDict.o
BASENM       := $(BASELIB:.$(SOEXT)=.nm)

# used in the main Makefile
ALLHDRS     += $(patsubst $(MODDIRI)/%.h,include/%.h,$(BASEH))

# include all dependency files
INCLUDEFILES += $(BASEDEP)

##### local rules #####
include/%.h:    $(BASEDIRI)/%.h
		cp $< $@

# Explicitely state this dependency.
# rmkdepend does not pick it up if $(COMPILEDATA) doesn't exist yet.
base/src/TSystem.d base/src/TSystem.o: $(COMPILEDATA)

#LF
#$(BASELIB):     $(BASEO) $(BASETMPDO) $(BASETMP2DO) $(BASEDO) $(ORDER_) $(MAINLIBS) $(BASELIBDEP)
#		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
#		"$(SOFLAGS)" libBase.$(SOEXT) $@ "$(BASEO) $(BASETMPDO) $(BASETMP2DO) $(BASEDO)"\
#		"$(BASELIBEXTRA)"

#LF
$(BASETMPDS1):   $(BASEH1) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(BASEH1) $(BASEL1)
#LF
$(BASETMP2DS1):  $(BASEH1) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(BASEH1) $(BASEL1)
#LF
$(BASEDS1):     $(BASEH1) $(BASEL1) $(ROOTCINTTMPEXE) $(BASENM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(BASENM) -. 3 -c $(BASEH1) $(BASEL1)

#LF
$(BASETMPDS2):   $(BASEH1) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(BASEH1) $(BASEL2)
#LF
$(BASETMP2DS2):  $(BASEH1) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(BASEH1) $(BASEL2)
#LF
$(BASEDS2):     $(BASEH1) $(BASEL2) $(ROOTCINTTMPEXE) $(BASENM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(BASENM) -. 3 -c $(BASEH1) $(BASEL2)

#LF
$(BASETMPDS3):   $(BASEH3) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(BASEH3) $(BASEL3)
#LF
$(BASETMP2DS3):  $(BASEH3) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(BASEH3) $(BASEL3)
#LF
$(BASEDS3):     $(BASEH3) $(BASEL3) $(ROOTCINTTMPEXE) $(BASENM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(BASENM) -. 3 -c $(BASEH3) $(BASEL3)
#LF
#$(BASEDICTMAP): $(RLIBMAP) $(MAKEFILEDEP) $(BASEL1) $(BASEL2) $(BASEL3)
#		$(RLIBMAP) -o $(BASEDICTMAP) -l $(COREDICTLIB) \
#		-d $(CORELIB) $(CORELIBDEPM) -c $(COREL)

#LF
$(BASENM):      $(BASEO) $(BASETMPDO1) $(BASETMPDO2) $(BASETMPDO3) $(BASETMP2DO1) $(BASETMP2DO2) $(BASETMP2DO3) 
		@echo "Generating symbols file $@..."
		nm -g -p --defined-only $(BASETMPDO1) | awk '{printf("%s\n", $$3)'} > $(BASENM)
		nm -g -p --defined-only $(BASETMPDO2) | awk '{printf("%s\n", $$3)'} >> $(BASENM)
		nm -g -p --defined-only $(BASETMPDO3) | awk '{printf("%s\n", $$3)'} >> $(BASENM)
		nm -g -p --defined-only $(BASETMP2DO1) | awk '{printf("%s\n", $$3)'} >> $(BASENM)
		nm -g -p --defined-only $(BASETMP2DO2) | awk '{printf("%s\n", $$3)'} >> $(BASENM)
		nm -g -p --defined-only $(BASETMP2DO3) | awk '{printf("%s\n", $$3)'} >> $(BASENM)
		nm -g -p --defined-only $(BASEO) | awk '{printf("%s\n", $$3)'} >> $(BASENM)

# pre-requisites intentionally not specified... should be called only
# on demand after deleting the file
$(BASEDS4):
		@echo "Generating dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(BASELIB) -c $(BASEH4) $(BASEL4)
		@echo "You need to manually fix the generated file as follow:"
		@echo "1. In ManualBase4Body.h, modify the name of the 2 functions to match the name of the CINT wrapper functions in ManualBase4.cxx"
		@echo "2. Replace the implementation of both functions by #include \"ManualBase4Body.h\" "

all-base:       $(BASEO) $(BASEDO) 

clean-base:
		@rm -f $(BASEO) $(BASEDO) $(BASEDIRS)/precompile.o

clean::         clean-base clean-pds-base

#LF
clean-pds-base:	
		rm -f $(BASETMPDS) $(BASETMP2DS) $(BASETMPDO) $(BASETMP3DO) $(BASETMPDH) \
		$(BASETMP2DH) $(BASETMPDEP) $(BASENM)

distclean-base: clean-base
		@rm -f $(BASEDEP) \
		   $(filter-out $(BASEDIRS)/ManualBase4.cxx, $(BASEDS)) \
		   $(filter-out $(BASEDIRS)/ManualBase4.h, $(BASEDH))

distclean::     distclean-base

##### extra rules ######
base/src/TPRegexp.o: $(PCREDEP)
base/src/TPRegexp.o: CXXFLAGS += $(PCREINC)

ifeq ($(ARCH),alphacxx6)
$(BASEDIRS)/TRandom.o: OPT = $(NOOPT)
endif

$(BASEDO1) $(BASEDO2): $(PCREDEP)
$(BASEDO1) $(BASEDO2): CXXFLAGS += $(PCREINC)
ifeq ($(ARCH),linuxicc)
$(BASEDO3):     CXXFLAGS += -wd191
endif
$(BASEDO4): OPT = $(NOOPT)
$(BASEDO4): CXXFLAGS += -I.
