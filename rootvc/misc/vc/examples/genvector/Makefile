# Makefile for the ROOT test programs.
# This Makefile shows nicely how to compile and link applications
# using the ROOT libraries on all supported platforms.
#
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: Fons Rademakers, 29/2/2000

#ROOTSYS = ../../../..
include $(ROOTSYS)/etc/Makefile.arch
include $(ROOTSYS)/config/Makefile.config

#------------------------------------------------------------------------------

# ifeq ($(PLATFORM),macosx) 
# #unroll loop better on gcc > 4
#CXXFLAGS+= -O3 -g  
# endif
CXXFLAGS+= -DUSE_VC

LIBS =  -L$(ROOTSYS)/lib -lCore -lCint -lMathCore -lMatrix
EXTRALIBS += $(ROOTSYS)/lib/libVc.a

VECTOROPOBJ     = vectorOperation.$(ObjSuf)
VECTOROPSRC     = vectorOperation.$(SrcSuf)
VECTOROP        = vectorOperation$(ExeSuf)



OBJS          = $(VECTOROPOBJ)

PROGRAMS      = $(VECTOROP) 


.SUFFIXES: .$(SrcSuf) .$(ObjSuf) $(ExeSuf)


all:            $(PROGRAMS) 


$(VECTOROP):   	$(VECTOROPOBJ)
		    $(LD) $(LDFLAGS) $^ $(LIBS) $(EXTRALIBS)  $(OutPutOpt)$@
		    @echo "$@ done"



check: 	all
	for prog in $(PROGRAMS); do \
	   ./$$prog > $$prog.out; \
	done; 

clean:
		@rm -f $(OBJS) $(PROGRAMS) 

distclean:      clean
		@rm -f $(PROGRAMS) 


.SUFFIXES: .$(SrcSuf)


.$(SrcSuf).$(ObjSuf):  
	$(CXX)  $(CXXFLAGS) -c $<
