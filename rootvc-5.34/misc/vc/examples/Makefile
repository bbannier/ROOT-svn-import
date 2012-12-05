# Makefile for the ROOT test programs.
# This Makefile shows nicely how to compile and link applications
# using the ROOT libraries on all supported platforms.
#
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: Fons Rademakers, 29/2/2000

ROOTSYS = ../../..
include $(ROOTSYS)/etc/Makefile.arch
include $(ROOTSYS)/config/Makefile.config

#------------------------------------------------------------------------------

# ifeq ($(PLATFORM),macosx) 
# #unroll loop better on gcc > 4
# CXXFLAGS+= -O3 -g  -funroll-loops 
# endif


EXTRALIBS += $(ROOTSYS)/lib/libVc.a



POLARCOORDOBJ     = polarcoord.$(ObjSuf)
POLARCOORDSRC     = polarcoord.$(SrcSuf)
POLARCOORD        = polarcoord$(ExeSuf)


OBJS          = $(POLARCOORDOBJ)

PROGRAMS      = $(POLARCOORD) 


.SUFFIXES: .$(SrcSuf) .$(ObjSuf) $(ExeSuf)


all:            $(PROGRAMS) 


$(POLARCOORD):   $(POLARCOORDOBJ)
		    $(LD) $(LDFLAGS) $^  $(LIBM) $(EXTRALIBS) $(OutPutOpt)$@
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
