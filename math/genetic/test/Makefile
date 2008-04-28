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
LIBS        += "$(ROOTSYS)/lib/libGenetic.lib" "$(ROOTSYS)/lib/libTMVA.lib" 
	"$(ROOTSYS)/lib/libMLP.lib" "$(ROOTSYS)/lib/libTreePlayer.lib" \
	"$(ROOTSYS)/lib/libMinuit.lib" "$(ROOTSYS)/lib/libMinuit2.lib"
else
LIBS        += -lGenetic -lTMVA -lMLP -lTreePlayer -lMinuit -lMinuit2
CXXFLAGS += -g
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
		@rm -f $(OBJS) core 

distclean:      clean
		@rm -f $(PROGRAMS) *.so


.SUFFIXES: .$(SrcSuf)


.$(SrcSuf).$(ObjSuf):
	$(CXX) $(CXXFLAGS) -c $<
