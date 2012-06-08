# Module.mk for Vc module
# Generated on Tue Apr  3 17:31:31 CEST 2012 by Vc/makeRootRelease.sh

MODNAME      := vc
VCVERS       := vc-0.6.70-root

MODDIR       := $(ROOT_SRCDIR)/misc/$(MODNAME)
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc
#VCBUILDDIR   := build/misc/$(MODNAME)


ifeq ($(PLATFORM),win32)
#VCLIBVCA     := $(call stripsrc,$(MODDIRS)/win32/libVc-0.6.70.lib)
VCLIBVC      := $(LPATH)/libVc.lib
else
VCLIBVC      := $(LPATH)/libVc.a
endif

VCH1          := $(wildcard $(MODDIRI)/Vc/*.h $(MODDIRI)/Vc/*/*.h)
VCH2         := $(wildcard $(MODDIRI)/Vc/*.tcc $(MODDIRI)/Vc/*/*.tcc)
VCH          := $(VCH1) $(VCH2)

ALLHDRS      += $(patsubst $(MODDIRI)/%.h,include/%.h,$(VCH))
ALLHDRS      += $(patsubst $(MODDIRI)/%.tcc,include/%.tcc,$(VCH2))
ALLLIBS      += $(VCLIBVC)

##### local rules #####
.PHONY:         all-$(MODNAME) clean-$(MODNAME) distclean-$(MODNAME)


include/%.h: $(MODDIRI)/%.h
	@(if [ ! -d "include/Vc" ]; then    \
	   mkdir -p include/Vc;             \
	fi)
	@(if [ ! -d "include/Vc/avx" ]; then    \
	   mkdir -p include/Vc/avx;             \
	fi)
	@(if [ ! -d "include/Vc/internal" ]; then    \
	   mkdir -p include/Vc/internal;             \
	fi)
	@(if [ ! -d "include/Vc/sse" ]; then    \
	   mkdir -p include/Vc/sse;             \
	fi)
	@(if [ ! -d "include/Vc/common" ]; then    \
	   mkdir -p include/Vc/common;             \
	fi)
	@(if [ ! -d "include/Vc/scalar" ]; then    \
	   mkdir -p include/Vc/scalar;             \
	fi)
	cp $< $@

include/%.tcc: $(MODDIRI)/%.tcc
	@(if [ ! -d "include/Vc" ]; then    \
	   mkdir -p include/Vc;             \
	fi)
	@(if [ ! -d "include/Vc/avx" ]; then    \
	   mkdir -p include/Vc/avx;             \
	fi)
	@(if [ ! -d "include/Vc/internal" ]; then    \
	   mkdir -p include/Vc/internal;             \
	fi)
	@(if [ ! -d "include/Vc/sse" ]; then    \
	   mkdir -p include/Vc/sse;             \
	fi)
	@(if [ ! -d "include/Vc/common" ]; then    \
	   mkdir -p include/Vc/common;             \
	fi)
	@(if [ ! -d "include/Vc/scalar" ]; then    \
	   mkdir -p include/Vc/scalar;             \
	fi)
	cp $< $@



VCFLAGS      := -DVC_COMPILE_LIB $(OPT) $(CXXFLAGS) -Iinclude/Vc
VCLIBVCOBJ   := $(patsubst $(MODDIRS)/%.cpp,$(MODDIRS)/%.cpp.o,$(wildcard $(MODDIRS)/*.cpp))
ifndef AVXFLAGS
VCLIBVCOBJ   := $(filter-out $(MODDIRS)/avx-%.cpp,$(VCLIBVCOBJ))
endif

$(VCLIBVC): $(VCLIBVCOBJ)
	$(MAKEDIR)
	@echo "Create static library $@"
	@ar r $@ $?
	@ranlib $@

$(MODDIRS)/avx-%.cpp.o: $(MODDIRS)/avx-%.cpp
	$(MAKEDIR)
	@echo "Compiling (AVX) $<"
	@$(CXX) $(VCFLAGS) $(AVXFLAGS) -Iinclude/Vc/avx -c $(CXXOUT)$@ $<

$(MODDIRS)/%.cpp.o: $(MODDIRS)/%.cpp
	$(MAKEDIR)
	@echo "Compiling $<"
	@$(CXX) $(VCFLAGS) -c $(CXXOUT)$@ $<



all-$(MODNAME): $(VCLIBVC)

clean-$(MODNAME):
	@rm -f $(VCLIBVC) $(VCLIBVCOBJ)

clean:: clean-$(MODNAME)

distclean-$(MODNAME): clean-$(MODNAME)
	@rm -rf include/Vc

distclean:: distclean-$(MODNAME)

