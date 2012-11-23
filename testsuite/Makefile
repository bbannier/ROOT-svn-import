# Makefile for the ROOT test programs.
# This Makefile shows nicely how to compile and link applications
# using the ROOT libraries on all supported platforms.
#
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: Fons Rademakers, 29/2/2000

include $(ROOTSYS)/etc/Makefile.arch

ifeq ($(TMVASYS),)
TMVAINC = -I../../tmva
TMVALD  = -L../../tmva/lib
TMVALIB = -lTMVA.1
else ifeq ($(TMVASYS),ROOT)
TMVAINC = 
TMVALD  = 
TMVALIB = 
else
TMVAINC = -I$(TMVASYS)
TMVALD  = -L$(TMVASYS)/lib
TMVALIB = -lTMVA.1
endif

ifneq ($(FULL),)
PREPROCOPT += -DFULL
endif

ifneq ($(DEBUG),)
PREPROCOPT +=  -ggdb -DCOUTDEBUG
endif


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
	$(CXX) $(TMVAINC) $(CXXFLAGS) $(PREPROCOPT) -c $< -o $@

$(STRESSTMVA):  $(STRESSTMVAO) $(TMVAUTOBJECTS)
	$(LD) $(LDFLAGS) $^ $(TMVALD) $(LIBS) $(TMVALIB) -lTMVA -lMinuit -lXMLIO -lMLP -lTreePlayer $(OutPutOpt)$@
	$(MT_EXE)
	@echo "$@ done"


clean:
		@rm -f testmakeclass_*
		@rm -f $(PROGRAMS) $(OBJS) $(TMVAUTOBJECTS)

.$(SrcSuf).$(ObjSuf):
	$(CXX)  $(CXXFLAGS) $(PREPROCOPT) -c $<
