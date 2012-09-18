# Makefile for the ROOT test programs.
# This Makefile shows nicely how to compile and link applications
# using the ROOT libraries on all supported platforms.
#
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: Fons Rademakers, 29/2/2000

ROOTSYS = ../../../../..
include $(ROOTSYS)/etc/Makefile.arch

#------------------------------------------------------------------------------

CXXFLAGS += -std=c++11 -march=native -DNDEBUG -ffast-math -ftree-vectorize -fassociative-math -ftree-vectorizer-verbose=3
LDFLAGS += -lrt


CHI2VECTOROBJ     = chi2Vector.$(ObjSuf)
CHI2VECTORSRC     = chi2Vector.$(SrcSuf)
CHI2VECTOR        = chi2Vector$(ExeSuf)

CHI2PARALLELOMPOBJ     = chi2ParallelOMP.$(ObjSuf)
CHI2PARALLELOMPSRC     = chi2ParallelOMP.$(SrcSuf)
CHI2PARALLELOMP        = chi2ParallelOMP$(ExeSuf)

$(CHI2PARALLELOMPOBJ)     : CXXFLAGS += -fopenmp
$(CHI2PARALLELOMP)     :    LDFLAGS += -fopenmp



OBJS          = $(CHI2VECTOROBJ)    $(CHI2PARALLELOMPOBJ)  


PROGRAMS      =$(CHI2VECTOR) $(CHI2PARALLELOMP)      


.SUFFIXES: .$(SrcSuf) .$(ObjSuf) $(ExeSuf)


all:            $(PROGRAMS) 

$(CHI2VECTOR):      $(CHI2VECTOROBJ)
		    $(LD) $(LDFLAGS) $^ $(LIBS)  $(OutPutOpt)$@
		    @echo "$@ done"

$(CHI2PARALLELOMP):      $(CHI2PARALLELOMPOBJ)
		    $(LD) $(LDFLAGS) $^ $(LIBS)  $(OutPutOpt)$@
		    @echo "$@ done"

clean:
		@rm -f $(OBJS) core TrackDict.$(SrcSuf) TrackDict.h

distclean:      clean
		@rm -f $(PROGRAMS) libTrackDict.*


.SUFFIXES: .$(SrcSuf)


.$(SrcSuf).$(ObjSuf):
	$(CXX) $(CXXFLAGS) -c $<
