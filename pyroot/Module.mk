# Module.mk for pyroot module
# Copyright (c) 2004 Rene Brun and Fons Rademakers
#
# Authors: Pere Mato, Wim Lavrijsen, 22/4/2004

MODDIR       := pyroot
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

PYROOTDIR    := $(MODDIR)
PYROOTDIRS   := $(PYROOTDIR)/src
PYROOTDIRI   := $(PYROOTDIR)/inc

##### libPyROOT #####
PYROOTL      := $(MODDIRI)/LinkDef.h
PYROOTDS     := $(MODDIRS)/G__PyROOT.cxx
PYROOTDO     := $(PYROOTDS:.cxx=.o)
PYROOTDH     := $(PYROOTDS:.cxx=.h)

#LF
PYROOTTMPDS    := $(MODDIRS)/G__PyROOTTmp.cxx
PYROOTTMPDO    := $(PYROOTTMPDS:.cxx=.o)
PYROOTTMPDH    := $(PYROOTTMPDS:.cxx=.h)
PYROOTTMP2DS   := $(MODDIRS)/G__PyROOTTmp2.cxx
PYROOTTMP2DO   := $(PYROOTTMP2DS:.cxx=.o)
PYROOTTMP2DH   := $(PYROOTTMP2DS:.cxx=.h)

PYROOTH      := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
PYROOTS      := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
PYROOTO      := $(PYROOTS:.cxx=.o)

PYROOTDEP    := $(PYROOTO:.o=.d) $(PYROOTDO:.o=.d)

#LF
PYROOTTMPDEP  := $(PYROOTTMPDO:.o=.d)

PYROOTLIB    := $(LPATH)/libPyROOT.$(SOEXT)
PYROOTMAP    := $(PYROOTLIB:.$(SOEXT)=.rootmap)

#LF
PYROOTNM       := $(PYROOTLIB:.$(SOEXT)=.nm)

ROOTPYS      := $(wildcard $(MODDIR)/*.py)
ifeq ($(PLATFORM),win32)
ROOTPY       := $(subst $(MODDIR),bin,$(ROOTPYS))
bin/%.py: $(MODDIR)/%.py; cp $< $@
else
ROOTPY       := $(subst $(MODDIR),$(LPATH),$(ROOTPYS))
$(LPATH)/%.py: $(MODDIR)/%.py; cp $< $@
endif
ROOTPYC      := $(ROOTPY:.py=.pyc)
ROOTPYO      := $(ROOTPY:.py=.pyo)

# used in the main Makefile
ALLHDRS     += $(patsubst $(MODDIRI)/%.h,include/%.h,$(PYROOTH))
ALLLIBS     += $(PYROOTLIB)
ALLMAPS     += $(PYROOTMAP)

# include all dependency files
INCLUDEFILES += $(PYROOTDEP)

##### local rules #####
include/%.h:    $(PYROOTDIRI)/%.h
		cp $< $@

%.pyc: %.py;    python -c 'import py_compile; py_compile.compile( "$<" )'
%.pyo: %.py;    python -O -c 'import py_compile; py_compile.compile( "$<" )'

#LF
$(PYROOTLIB):   $(PYROOTO) $(PYROOTTMPDO) $(PYROOTTMP2DO) $(PYROOTDO) \
		$(ROOTPY) $(ROOTPYC) $(ROOTPYO) $(ROOTLIBSDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		   "$(SOFLAGS)" libPyROOT.$(SOEXT) $@ "$(PYROOTO) $(PYROOTTMPDO) $(PYROOTTMP2DO) $(PYROOTDO)" \
		  "$(ROOTULIBS) $(RPATH) $(ROOTLIBS) \
		   $(PYTHONLIBDIR) $(PYTHONLIB) \
		   $(OSTHREADLIBDIR) $(OSTHREADLIB)" "$(PYTHONLIBFLAGS)"

#LF
$(PYROOTTMPDS):   $(PYROOTH) $(PYROOTL) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(PYROOTH) $(PYROOTL)

#LF
$(PYROOTTMP2DS):  $(PYROOTH) $(PYROOTL) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(PYROOTH) $(PYROOTL)

#LF
$(PYROOTDS):    $(PYROOTH) $(PYROOTL) $(ROOTCINTTMPEXE) $(PYROOTNM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(PYROOTNM) -. 3 -c $(PYROOTH) $(PYROOTL)

#LF
$(PYROOTNM):      $(PYROOTO) $(PYROOTTMPDO) $(PYROOTTMP2DO) 
		@echo "Generating symbols file $@..."
		nm -p --defined-only $(PYROOTTMPDO) | awk '{printf("%s\n", $$3)'} > $(PYROOTNM)
		nm -p --defined-only $(PYROOTTMP2DO) | awk '{printf("%s\n", $$3)'} >> $(PYROOTNM)
		nm -p --defined-only $(PYROOTO) | awk '{printf("%s\n", $$3)'} >> $(PYROOTNM)

$(PYROOTMAP):   $(RLIBMAP) $(MAKEFILEDEP) $(PYROOTL)
		$(RLIBMAP) -o $(PYROOTMAP) -l $(PYROOTLIB) \
		   -d $(PYROOTLIBDEPM) -c $(PYROOTL)

all-pyroot:     $(PYROOTLIB) $(PYROOTMAP)

clean-pyroot:
		@rm -f $(PYROOTO) $(PYROOTDO)

clean::         clean-pyroot clean-pds-pyroot

#LF
clean-pds-pyroot:	
		rm -f $(PYROOTTMPDS) $(PYROOTTMPDO) $(PYROOTTMPDH) \
		$(PYROOTTMPDEP) $(PYROOTTMP2DS) $(PYROOTTMP2DO) $(PYROOTTMP2DH) $(PYROOTNM)

distclean-pyroot: clean-pyroot
		@rm -f $(PYROOTDEP) $(PYROOTDS) $(PYROOTDH) $(PYROOTLIB) \
		   $(ROOTPY) $(ROOTPYC) $(ROOTPYO) $(PYROOTMAP)

distclean::     distclean-pyroot

##### extra rules ######
# Require Reflex support?
ifeq ($(BUILDREFLEX),yes)
$(PYROOTO): CXXFLAGS += -DPYROOT_USE_REFLEX $(PYTHONINCDIR:%=-I%)
$(PYROOTLIB): ROOTLIBS += $(RFLX_REFLEXLL)
$(PYROOTLIB): $(LPATH)/libReflex.$(SOEXT)
else
$(PYROOTO): CXXFLAGS += $(PYTHONINCDIR:%=-I%)
endif
