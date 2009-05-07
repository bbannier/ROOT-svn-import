# Module.mk for cint7 module
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: Fons Rademakers, 29/2/2000

MODNAME      := cint7
MODDIRBASE   := cint
MODDIR       := $(MODDIRBASE)/$(MODNAME)
MODDIRS      := $(MODDIR)/src
MODDIRSD     := $(MODDIRS)/dict
MODDIRI      := $(MODDIR)/inc

CINTDIR      := $(MODDIR)
CINTDIRS     := $(CINTDIR)/src
CINTDIRSD    := $(CINTDIRS)/dict
CINTDIRI     := $(CINTDIR)/inc
CINTDIRM     := $(CINTDIR)/main
CINTDIRL     := $(CINTDIR)/lib
CINTDIRDLLS  := $(CINTDIR)/include
CINTDIRSTL   := $(CINTDIR)/stl
CINTDIRDLLSTL:= $(CINTDIRL)/dll_stl
CINTDIRIOSEN := $(MODDIRBASE)/iosenum
CINTDIRT     := $(MODDIRBASE)/tool

##### libCint #####
CINTCONF     := $(CINTDIRI)/configcint.h
CINTH        := $(wildcard $(CINTDIRI)/*.h)
CINTHT       := $(sort $(patsubst $(CINTDIRI)/%.h,include/cint/%.h,$(CINTH) $(CINTCONF)))
CINTBWHT     := $(patsubst include/cint/%,include/%,$(CINTHT))
CINTS1       := $(wildcard $(MODDIRS)/*.c)
CINTS2       := $(wildcard $(MODDIRS)/*.cxx) \
                $(MODDIRSD)/longif.cxx $(MODDIRSD)/Apiif.cxx \
                $(MODDIRSD)/stdstrct.cxx

CINTS1       += $(CINTDIRM)/G__setup.c

CINTALLO     := $(CINTS1:.c=.o) $(CINTS2:.cxx=.o)
CINTALLDEP   := $(CINTALLO:.o=.d)

CINTCONFMK   := cint/ROOT/configcint.mk

CINTS1       := $(filter-out $(MODDIRS)/dlfcn.%,$(CINTS1))

CINTS2       := $(filter-out $(MODDIRS)/sunos.%,$(CINTS2))
CINTS2       := $(filter-out $(MODDIRS)/macos.%,$(CINTS2))
CINTS2       := $(filter-out $(MODDIRS)/winnt.%,$(CINTS2))
CINTS2       := $(filter-out $(MODDIRS)/newsos.%,$(CINTS2))
CINTS2       := $(filter-out $(MODDIRS)/loadfile_tmp.%,$(CINTS2))
CINTS2       := $(filter-out $(MODDIRS)/symbols.%,$(CINTS2))

# strip off possible leading path from compiler command name
CXXCMD       := $(shell echo $(CXX) | sed s/".*\/"//)

ifeq ($(CXXCMD),KCC)
CINTS2       += $(MODDIRSD)/kccstrm.cxx
CINTS2       := $(filter-out $(MODDIRSD)/longif.%,$(CINTS2))
CINTS2       += $(MODDIRSD)/longif3.cxx
else
ifeq ($(PLATFORM),linux)
CINTS2       += $(MODDIRSD)/libstrm.cxx
endif
ifeq ($(PLATFORM),hurd)
CINTS2       += $(MODDIRSD)/libstrm.cxx
endif
ifeq ($(PLATFORM),fbsd)
CINTS2       += $(MODDIRSD)/libstrm.cxx
endif
ifeq ($(PLATFORM),obsd)
CINTS2       += $(MODDIRSD)/libstrm.cxx
endif
ifeq ($(PLATFORM),hpux)
ifeq ($(ARCH),hpuxia64acc)
CINTS2       += $(MODDIRSD)/accstrm.cxx
CINTS2       := $(filter-out $(MODDIRSD)/longif.%,$(CINTS2))
CINTS2       += $(MODDIRSD)/longif3.cxx
else
CINTS2       += $(MODDIRSD)/libstrm.cxx
endif
endif
ifeq ($(PLATFORM),solaris)
ifeq ($(SUNCC5),true)
CINTS2       := $(filter-out $(MODDIRSD)/longif.%,$(CINTS2))
CINTS2       += $(MODDIRSD)/longif3.cxx
ifeq ($(findstring $(CXXFLAGS),-library=iostream,no%Cstd),)
CINTS2       += $(MODDIRSD)/sunstrm.cxx
#CINTS2       += $(MODDIRSD)/sun5strm.cxx
else
CINTS2       += $(MODDIRSD)/libstrm.cxx
endif
else
CINTS2       += $(MODDIRSD)/libstrm.cxx
endif
endif
ifeq ($(PLATFORM),aix3)
CINTS1       += $(MODDIRS)/dlfcn.c
CINTS2       += $(MODDIRSD)/libstrm.cxx
endif
ifeq ($(PLATFORM),aix)
CINTS2       += $(MODDIRSD)/libstrm.cxx
endif
ifeq ($(PLATFORM),aix5)
CINTS2       += $(MODDIRSD)/libstrm.cxx
endif
ifeq ($(PLATFORM),sgi)
CINTS2       += $(MODDIRSD)/libstrm.cxx
endif
ifeq ($(PLATFORM),alpha)
CINTS2       += $(MODDIRSD)/alphastrm.cxx
endif
ifeq ($(PLATFORM),alphagcc)
CINTS2       += $(MODDIRSD)/libstrm.cxx
endif
endif
ifeq ($(PLATFORM),sunos)
CINTS1       += $(MODDIRS)/sunos.c
endif
ifeq ($(PLATFORM),macos)
CINTS2       += $(MODDIRS)/macos.cxx
CINTS2       += $(MODDIRSD)/fakestrm.cxx
endif
ifeq ($(PLATFORM),macosx)
CINTS2       += $(MODDIRSD)/libstrm.cxx
endif
ifeq ($(PLATFORM),lynxos)
CINTS2       += $(MODDIRSD)/fakestrm.cxx
endif
ifeq ($(PLATFORM),win32)
CINTS2       += $(MODDIRS)/winnt.cxx
CINTS2       := $(filter-out $(MODDIRSD)/longif.%,$(CINTS2))
CINTS2       += $(MODDIRSD)/longif3.cxx
ifeq ($(VC_MAJOR),13)
 ifeq ($(VC_MINOR),10)
  CINTS2       += $(MODDIRSD)/vc7strm.cxx
 else
  CINTS2       += $(MODDIRSD)/iccstrm.cxx
 endif
else
 ifeq ($(VC_MAJOR),14)
  CINTS2       += $(MODDIRSD)/vc7strm.cxx
 else
  CINTS2       += $(MODDIRSD)/iccstrm.cxx
 endif
endif
endif
ifeq ($(PLATFORM),vms)
CINTS2       += $(MODDIRSD)/fakestrm.cxx
endif
ifeq ($(CXXCMD),icc)
CINTS2       := $(filter-out $(MODDIRSD)/libstrm.%,$(CINTS2))
CINTS2       := $(filter-out $(MODDIRSD)/longif.%,$(CINTS2))
ifneq ($(ICC_GE_9),)
  ifneq ($(ICC_GE_101),)
    CINTS2       += $(MODDIRSD)/gcc4strm.cxx
  else
    CINTS2       += $(MODDIRSD)/gcc3strm.cxx
  endif
else
  CINTS2       += $(MODDIRSD)/iccstrm.cxx
endif
CINTS2       += $(MODDIRSD)/longif3.cxx
endif
ifeq ($(GCC_MAJOR),3)
CINTS2       := $(filter-out $(MODDIRSD)/libstrm.%,$(CINTS2))
CINTS2       := $(filter-out $(MODDIRSD)/longif.%,$(CINTS2))
CINTS2       += $(MODDIRSD)/gcc3strm.cxx
CINTS2       += $(MODDIRSD)/longif3.cxx
endif
ifeq ($(GCC_MAJOR),4)
CINTS2       := $(filter-out $(MODDIRSD)/libstrm.%,$(CINTS2))
CINTS2       := $(filter-out $(MODDIRSD)/longif.%,$(CINTS2))
CINTS2       += $(MODDIRSD)/gcc4strm.cxx
CINTS2       += $(MODDIRSD)/longif3.cxx
endif
ifeq ($(CXXCMD),xlC)
ifeq ($(PLATFORM),macosx)
CINTS2       := $(filter-out $(MODDIRSD)/libstrm.%,$(CINTS2))
CINTS2       := $(filter-out $(MODDIRSD)/longif.%,$(CINTS2))
CINTS2       += $(MODDIRSD)/gcc3strm.cxx
CINTS2       += $(MODDIRSD)/longif3.cxx
endif
endif

ifneq ($(findstring -DG__NOSTUBS,$(CINTCXXFLAGS)),)
CINTS2       += $(MODDIRS)/symbols.cxx
endif

CINTS        := $(CINTS1) $(CINTS2)
CINTO        := $(CINTS1:.c=.o) $(CINTS2:.cxx=.o)
CINTTMPO     := $(subst loadfile.o,loadfile_tmp.o,$(CINTO))
CINTTMPINC   := -I$(MODDIR)/include -I$(MODDIR)/stl -I$(MODDIR)/lib
CINTDEP      := $(CINTO:.o=.d)
CINTDEP      += $(MODDIRS)/loadfile_tmp.d
CINTALLDEP   += $(MODDIRS)/loadfile_tmp.d

CINTLIB      := $(LPATH)/libCint.$(SOEXT)

##### cint #####
CINTEXES     := $(CINTDIRM)/cppmain.cxx
CINTEXEO     := $(CINTEXES:.cxx=.o)
CINTEXEDEP   := $(CINTEXEO:.o=.d)
CINTTMP      := $(CINTDIRM)/cint_tmp$(EXEEXT)
CINT         := bin/cint$(EXEEXT)

##### makecint #####
MAKECINTS    := $(CINTDIRT)/makecint.cxx
MAKECINTO    := $(MAKECINTS:.cxx=.o)
MAKECINT     := bin/makecint$(EXEEXT)

##### iosenum.h #####
IOSENUM      := $(MODDIR)/include/iosenum.h
IOSENUMC     := $(CINTDIRIOSEN)/iosenum.cxx
ifeq ($(GCC_MAJOR),4)
IOSENUMA     := $(CINTDIRIOSEN)/iosenum.$(ARCH)3
else
ifeq ($(GCC_MAJOR),3)
IOSENUMA     := $(CINTDIRIOSEN)/iosenum.$(ARCH)3
else
IOSENUMA     := $(CINTDIRIOSEN)/iosenum.$(ARCH)
endif
endif

# used in the main Makefile
ALLHDRS     += $(CINTHT)

#ifeq ($(BUILDCINT7),)
ALLHDRS     += $(CINTBWHT)
#endif


CINTCXXFLAGS += -DG__HAVE_CONFIG -DG__NOMAKEINFO -DG__CINTBODY -I$(CINTDIRI) -I$(CINTDIRS) -I$(CINTDIRSD)
CINTCFLAGS += -DG__HAVE_CONFIG -DG__NOMAKEINFO -DG__CINTBODY -I$(CINTDIRI) -I$(CINTDIRS) -I$(CINTDIRSD)

dummy := $(shell echo "CINTCXXFLAGS=$(CINTCXXFLAGS)" 1>&2)

# Make sure -Iinclude/ is _after_ CINT7 -I
#CINT7CXXFLAGS = $(subst -Iinclude ,-I$(CINT7DIRI) -I$(CINT7DIRS) -I$(CINT7DIRSD) ,\
#                   $(patsubst -Icint/cint/%,,$(CINTCXXFLAGS))) \
#                -Iinclude
#CINT7CFLAGS   = $(subst -Iinclude ,-I$(CINT7DIRI) -I$(CINT7DIRS) -I$(CINT7DIRSD) ,\
#                   $(patsubst -Icint/cint/%,,$(CINTCFLAGS))) \
#                -Iinclude
#
#CINT7CXXFLAGS += -DG__CINTBODY -DG__HAVE_CONFIG -DG__NOMAKEINFO
#CINT7CFLAGS   += -DG__CINTBODY -DG__HAVE_CONFIG -DG__NOMAKEINFO

##### used by configcint.mk #####
G__CFG_CXXFLAGS := $(CINTCXXFLAGS)
G__CFG_CFLAGS   := $(CINTCFLAGS)
G__CFG_DIR      := $(CINTDIR)
G__CFG_CONF     := $(CINTCONF)
G__CFG_CONFMK   := $(CINTCONFMK)

##### used by cintdlls.mk #####
CINTDLLDIRSTL_BAK := $(CINTDLLDIRSTL) 
CINTDLLDIRDLLS_BAK := $(CINTDLLDIRDLLS) 
CINTDLLDIRDLLSTL_BAK := $(CINTDLLDIRDLLSTL)
CINTDLLDIRL_BAK := $(CINTDLLDIRL) 
CINTDLLIOSENUM_BAK := $(CINTDLLIOSENUM) 
CINTDLLDICTVER_BAK := $(CINTDLLDICTVER) 
CINTDLLCINTTMP_BAK := $(CINTDLLCINTTMP) 
CINTDLLCFLAGS_BAK := $(CINTDLLCFLAGS) 
CINTDLLCXXFLAGS_BAK := $(CINTDLLCXXFLAGS) 

CINTDLLDIRSTL    := $(CINTDIRSTL)
CINTDLLDIRDLLS   := $(CINTDIRDLLS)
CINTDLLDIRDLLSTL := $(CINTDIRDLLSTL)
CINTDLLDIRL      := $(CINTDIRL)
CINTDLLIOSENUM   := $(IOSENUM)
CINTDLLDICTVER   := $(CINTDIRI)/cintdictversion.h
CINTDLLCINTTMP   := $(CINTTMP)
CINTDLLCFLAGS    := $(filter-out -DG__CINTBODY,$(CINTCFLAGS))
CINTDLLCXXFLAGS  := $(filter-out -DG__CINTBODY,$(CINTCXXFLAGS))
dummy := $(shell echo "CINTDLLCXXFLAGS=$(CINTDLLCXXFLAGS)" 1>&2)
CINTDLLROOTCINTTMPDEP = $(ROOTCINTTMPDEP)

# include all dependency files
INCLUDEFILES += $(CINTDEP) $(CINTEXEDEP)

##### local rules #####
.PHONY:         all-$(MODNAME) clean-$(MODNAME) distclean-$(MODNAME)

include/cint/%.h: $(CINTDIRI)/%.h
		@(if [ ! -d "include/cint" ]; then    \
		   mkdir -p include/cint;             \
		fi)
		cp $< $@

#ifeq ($(BUILDCINT7),)
$(CINTBWHT): include/%.h: $(CINTDIRI)/%.h
		cp $< $@
#endif

$(CINTLIB):     $(CINTO)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		   "$(SOFLAGS)" libCint.$(SOEXT) $@ "$^" "$(CINTLIBEXTRA)"

$(CINT):        $(CINTEXEO) $(CINTLIB) $(REFLEXLIB)
		$(LD) $(LDFLAGS) -o $@ $(CINTEXEO) \
		   $(RPATH) $(CINTLIBS) $(CILIBS)

$(CINTTMP):     $(CINTEXEO) $(CINTTMPO) $(REFLEXLIB)
		$(LD) $(LDFLAGS) -o $@ $(CINTEXEO) \
		   $(CINTTMPO) $(REFLEXLIB) $(CILIBS)

$(MAKECINT):    $(MAKECINTO) $(REFLEXLIB)
		$(LD) $(LDFLAGS) -o $@ $(MAKECINTO) $(REFLEXLIB) $(CILIBS)

$(IOSENUM):     $(IOSENUMA)
		cp $< $@

$(IOSENUMA):    $(CINTTMP)
		@(if [ ! -r $@ ]; then \
			echo "Making $@..."; \
			$(CINTTMP) $(CINTTMPINC) $(IOSENUMC) > /dev/null; \
			mv iosenum.h $@; \
		else \
			touch $@; \
		fi)

all-$(MODNAME): $(CINTLIB) $(CINT) $(CINTTMP) $(MAKECINT) $(IOSENUM)

clean-$(MODNAME):
		@rm -f $(CINTTMPO) $(CINTALLO) $(CINTEXEO) $(MAKECINTO) \
		   $(CINTBWHT)

clean::         clean-$(MODNAME)

distclean-$(MODNAME): clean-$(MODNAME)
		@rm -f $(CINTALLDEP) $(CINTLIB) $(IOSENUM) $(CINTEXEDEP) \
		   $(CINT) $(CINTTMP) $(MAKECINT) $(CINTDIRM)/*.exp \
		   $(CINTDIRM)/*.lib $(CINTDIRS)/loadfile_tmp.cxx \
		   $(CINTDIRDLLS)/sys/types.h $(CINTDIRDLLS)/systypes.h \
		   $(CINTHT) $(CINTCONF)
		@rm -rf include/cint

distclean::     distclean-$(MODNAME)

##### extra rules ######
$(CINTDIRSD)/libstrm.o:  CINTCXXFLAGS += -I$(CINTDIRL)/stream
$(CINTDIRSD)/sun5strm.o: CINTCXXFLAGS += -I$(CINTDIRL)/sunstrm
$(CINTDIRSD)/vcstrm.o:   CINTCXXFLAGS += -I$(CINTDIRL)/vcstream
$(CINTDIRSD)/%strm.o:    CINTCXXFLAGS += -I$(CINTDIRL)/$(notdir $(basename $@))
ifeq ($(GCC_MAJOR),4)
$(CINTDIRSD)/gcc4strm.o:  CINTCXXFLAGS += -Wno-strict-aliasing
endif
#ifeq ($(GCC_MAJOR),4)
#$(CINT7DIRSD)/gcc4strm.o:  CINT7CXXFLAGS += -Wno-strict-aliasing
#endif

$(MAKECINTO) $(CINTALLO): $(CINTCONF)

$(MAKECINTO): CXXFLAGS := $(CINTCXXFLAGS)
$(CINTDIRSD)/stdstrct.o:     CINTCXXFLAGS += -I$(CINTDIRL)/stdstrct
$(CINTDIRS)/loadfile_tmp.o: CINTCXXFLAGS += -UR__HAVE_CONFIG -DROOTBUILD

$(CINTDIRS)/loadfile_tmp.cxx: $(CINTDIRS)/loadfile.cxx
	cp -f $< $@


ifneq ($(findstring -DG__NOSTUBS,$(CINTCXXFLAGS)),)
$(CINTDIRS)/newlink.o: OPT = $(NOOPT)
endif

##### configcint.h
ifeq ($(CPPPREP),)
# cannot use "CPPPREP?=", as someone might set "CPPPREP="
  CPPPREP = $(CXX) -E -C
endif

include $(CINTCONFMK)
##### configcint.h - END

##### cintdlls #####
include cint/ROOT/cintdlls.mk

#CINTDLLDIRSTL := $(CINTDLLDIRSTL_BAK) 
#CINTDLLDIRDLLS := $(CINTDLLDIRDLLS_BAK) 
#CINTDLLDIRDLLSTL := $(CINTDLLDIRDLLSTL_BAK)
#CINTDLLDIRL := $(CINTDLLDIRL_BAK) 
#CINTDLLIOSENUM := $(CINTDLLIOSENUM_BAK) 
#CINTDLLDICTVER := $(CINTDLLDICTVER_BAK) 
#CINTDLLCINTTMP := $(CINTDLLCINTTMP_BAK) 
#CINTDLLCFLAGS := $(CINTDLLCFLAGS_BAK) 
#CINTDLLCXXFLAGS := $(CINTDLLCXXFLAGS_BAK) 
