# Makefile for the ROOT test programs.
# This Makefile shows nicely how to compile and link applications
# using the ROOT libraries on all supported platforms.
#
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: Fons Rademakers, 29/2/2000

include $(ROOTSYS)/test/Makefile.arch

#------------------------------------------------------------------------------


STRESSTMVAO   = stressTMVA.$(ObjSuf)
STRESSTMVAS   = stressTMVA.$(SrcSuf)
STRESSTMVA    = stressTMVA$(ExeSuf)

OBJS          = $(STRESSTMVAO)

PROGRAMS      = $(STRESSTMVA)

#------------------------------------------------------------------------------

.SUFFIXES: .$(SrcSuf) .$(ObjSuf) .$(DllSuf)

all:            $(PROGRAMS)


TMVAUTDIR        = tmvaut
TMVAUTSOURCES    = $(wildcard $(TMVAUTDIR)/*.cxx)
TMVAUTOBJECTS    = $(TMVAUTSOURCES:.cxx=.o)
TMVAUTH          = $(TMVAUTSOURCES:.cxx=.h)

$(TMVAUTOBJECTS): %.o : %.cxx $(TMVAUTH)
	$(CXX)  $(CXXFLAGS) -c $< -o $@

$(STRESSTMVA):  $(STRESSTMVAO) $(TMVAUTOBJECTS)
	$(LD) $(LDFLAGS) $^ $(LIBS) -lTMVA -lMinuit -lXMLIO -lMLP -lTreePlayer $(OutPutOpt)$@
	$(MT_EXE)
	@echo "$@ done"


clean:
		@rm -f $(OBJS)
		-@cd tmvaut && $(MAKE) distclean

.$(SrcSuf).$(ObjSuf):
	$(CXX)  $(CXXFLAGS) -c $<
