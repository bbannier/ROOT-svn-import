# Module.mk for fftw module
# Copyright (c) 2006 Rene Brun and Fons Rademakers
#
# Author: Fons Rademakers, 23/1/2006

MODDIR       := fftw
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

FFTWDIR      := $(MODDIR)
FFTWDIRS     := $(FFTWDIR)/src
FFTWDIRI     := $(FFTWDIR)/inc

#### libFFTW ####
FFTWL        := $(MODDIRI)/LinkDef.h
FFTWDS       := $(MODDIRS)/G__FFTW.cxx
FFTWDO       := $(FFTWDS:.cxx=.o)
FFTWDH       := $(FFTWDS:.cxx=.h)

#LF
FFTWTMPDS    := $(MODDIRS)/G__FFTWTmp.cxx
FFTWTMPDO    := $(FFTWTMPDS:.cxx=.o)
FFTWTMPDH    := $(FFTWTMPDS:.cxx=.h)
FFTWTMP2DS   := $(MODDIRS)/G__FFTWTmp2.cxx
FFTWTMP2DO   := $(FFTWTMP2DS:.cxx=.o)
FFTWTMP2DH   := $(FFTWTMP2DS:.cxx=.h)

FFTWH        := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
FFTWS        := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
FFTWO        := $(FFTWS:.cxx=.o)

FFTWDEP      := $(FFTWO:.o=.d) $(FFTWDO:.o=.d)

#LF
FFTWTMPDEP  := $(FFTWTMPDO:.o=.d)

FFTWLIB      := $(LPATH)/libFFTW.$(SOEXT)
FFTWMAP      := $(FFTWLIB:.$(SOEXT)=.rootmap)

#LF
FFTWNM       := $(FFTWLIB:.$(SOEXT)=.nm)

# used in the main Makefile
ALLHDRS      += $(patsubst $(MODDIRI)/%.h,include/%.h,$(FFTWH))
ALLLIBS      += $(FFTWLIB)
ALLMAPS      += $(FFTWLIB)

# include all dependency files
INCLUDEFILES += $(FFTWDEP)

##### local rules #####
include/%.h:    $(FFTWDIRI)/%.h
		cp $< $@

#LF
$(FFTWLIB):   $(FFTWO) $(FFTWTMPDO) $(FFTWTMP2DO) $(FFTWDO) $(ORDER_) $(MAINLIBS) $(FFTWLIBDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		   "$(SOFLAGS)" libFFTW.$(SOEXT) $@ "$(FFTWO) $(FFTWTMPDO) $(FFTWTMP2DO) $(FFTWDO)" \
		   "$(FFTWLIBEXTRA) $(FFTW3LIBDIR) $(FFTW3LIB)"

#LF
$(FFTWTMPDS):   $(FFTWH) $(FFTWL) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(FFTWH) $(FFTWL)

#LF
$(FFTWTMP2DS):  $(FFTWH) $(FFTWL) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(FFTWH) $(FFTWL)

#LF
$(FFTWDS):    $(FFTWH) $(FFTWL) $(ROOTCINTTMPEXE) $(FFTWNM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(FFTWNM) -. 3 -c $(FFTWH) $(FFTWL)

#LF
$(FFTWNM):      $(FFTWO) $(FFTWTMPDO) $(FFTWTMP2DO) 
		@echo "Generating symbols file $@..."
		nm -p --defined-only $(FFTWTMPDO) | awk '{printf("%s\n", $$3)'} > $(FFTWNM)
		nm -p --defined-only $(FFTWTMP2DO) | awk '{printf("%s\n", $$3)'} >> $(FFTWNM)
		nm -p --defined-only $(FFTWO) | awk '{printf("%s\n", $$3)'} >> $(FFTWNM)

$(FFTWMAP):       $(RLIBMAP) $(MAKEFILEDEP) $(FFTWL)
		$(RLIBMAP) -o $(FFTWMAP) -l $(FFTWLIB) \
		   -d $(FFTWLIBDEPM) -c $(FFTWL)

all-fft:        $(FFTWLIB) $(FFTWMAP)

clean-fft:
		@rm -f $(FFTWO) $(FFTWDO)

clean::         clean-fft clean-pds-fft

#LF
clean-pds-fft:	
		rm -f $(FFTWTMPDS) $(FFTWTMPDO) $(FFTWTMPDH) \
		$(FFTWTMPDEP) $(FFTWTMP2DS) $(FFTWTMP2DO) $(FFTWTMP2DH) $(FFTWNM)

distclean-fft:  clean-fft
		@rm -f $(FFTWDEP) $(FFTWDS) $(FFTWDH) $(FFTWLIB) $(FFTWMAP)

distclean::     distclean-fft

##### extra rules ######
$(FFTWO) $(FFTWDO): CXXFLAGS += $(FFTW3INCDIR:%=-I%)
