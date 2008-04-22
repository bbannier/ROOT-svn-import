# Makefile for the ROOT test programs.
# This Makefile shows nicely how to compile and link applications
# using the ROOT libraries on all supported platforms.
#
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: Fons Rademakers, 29/2/2000

include $(ROOTSYS)/test/Makefile.arch

#------------------------------------------------------------------------------

ifeq ($(PLATFORM),win32)
LIBS        += -include:_G__cpp_setupG__MathCore  "$(ROOTSYS)/lib/libMathMore.lib" \
	"$(ROOTSYS)/lib/libGenetic.lib" "$(ROOTSYS)/lib/libTMVA.lib" \
	"$(ROOTSYS)/lib/libMLP.lib" "$(ROOTSYS)/lib/libTreePlayer.lib" \
	"$(ROOTSYS)/lib/libMinuit.lib" "$(ROOTSYS)/lib/libMinuit2.lib"
else
LIBS        += -lGenetic -lMathMore -lTMVA -lMLP -lTreePlayer -lMinuit -lMinuit2
CXXFLAGS += -g
endif

ifeq ($(DIM),2)
CXXFLAGS += -DDIM_2
endif
ifeq ($(DIM),3)
CXXFLAGS += -DDIM_3
endif
ifneq ($(USE_CLHEP),)
CXXFLAGS += -DUSE_CLHEP
endif
ifneq ($(USE_ROOT),)
CXXFLAGS += -DUSE_ROOT
endif

# if have clhep
ifneq ($(CLHEPBASE),)
CXXFLAGS+=  -I$(CLHEPBASE)/include -DHAVE_CLHEP
ifeq ($(PLATFORM),win32)
EXTRALIBS += "$(CLHEPBASE)/lib/CLHEP.lib"
else
#EXTRALIBS+=  $(CLHEPBASE)/lib/libCLHEP.a
EXTRALIBS+=  -L$(CLHEPBASE)/lib -lCLHEP
endif
endif


TESTGAMINOBJ     = testGAMinimizer.$(ObjSuf)
TESTGAMINSRC     = testGAMinimizer.$(SrcSuf)
TESTGAMIN        = testGAMinimizer$(ExeSuf)

OBJS          = $(TESTGAMINOBJ)

PROGRAMS      = $(TESTGAMIN)

.SUFFIXES: .$(SrcSuf) .$(ObjSuf) $(ExeSuf)

all:            $(PROGRAMS) 

$(TESTGAMIN):      $(TESTGAMINOBJ)
		    $(LD) $(LDFLAGS) $^ $(LIBS)  $(OutPutOpt)$@
		    @echo "$@ done"

clean:
		@rm -f $(OBJS) core TrackDict.$(SrcSuf) TrackDict.h

distclean:      clean
		@rm -f $(PROGRAMS) libTrackDict.*


.SUFFIXES: .$(SrcSuf)


.$(SrcSuf).$(ObjSuf):
	$(CXX) $(CXXFLAGS) -c $<
