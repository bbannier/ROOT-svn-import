# Makefile for the ROOT test programs.
# This Makefile shows nicely how to compile and link applications
# using the ROOT libraries on all supported platforms.
#
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: Fons Rademakers, 29/2/2000


include $(ROOTSYS)/config/Makefile.config
include $(ROOTSYS)/test/Makefile.arch

#------------------------------------------------------------------------------

CXXFLAGS += -g


# ifeq ($(PLATFORM),win32)
# EXTRALIBS        = "$(ROOTSYS)/lib/libMathMore.lib" "$(ROOTSYS)/lib/libMathCore.lib" $(GSLLIBS)
# else
# EXTRALIBS        = -lunuran
# endif

#UNURANBASE=/Users/moneta/mathlibs/Unuran
#CXXFLAGS+=  -I$(UNURANBASE)/include 
#EXTRALIBS+=  $(UNURANBASE)/lib/libunuran.a 
EXTRALIBS +=   -lMathCore  -lMathMore -lMathnag 
ifeq ($(PLATFORM),macosx)
EXTRALIBS += -L/usr/lib -lnagc_nag
endif

#endif

# TESTFITOBJ      = testFit.$(ObjSuf)
# TESTFITSRC      = testFit.$(SrcSuf)
# TESTFIT         = testFit$(ExeSuf)

# TESTPERFOBJ      = testFitPerf.$(ObjSuf)
# TESTPERFSRC      = testFitPerf.$(SrcSuf)
# TESTPERF         = testFitPerf$(ExeSuf)

TESTMINIMOBJ      = testMinim.$(ObjSuf)
TESTMINIMSRC      = testMinim.$(SrcSuf)
TESTMINIM         = testMinim$(ExeSuf)

# TESTROOFITOBJ      = testRooFit.$(ObjSuf)
# TESTROOFITSRC      = testRooFit.$(SrcSuf)
# TESTROOFIT         = testRooFit$(ExeSuf)





#OBJS          = $(TESTFITOBJ) $(TESTPERFOBJ) $(TESTMINIMOBJ) $(TESTROOFITOBJ)
OBJS          =  $(TESTMINIMOBJ) 


#PROGRAMS      =  $(TESTFIT) $(TESTPERF) $(TESTMINIM) $(TESTROOFIT)
PROGRAMS      =  $(TESTMINIM) 

		  
.SUFFIXES: .$(SrcSuf) .$(ObjSuf) $(ExeSuf)


all:            $(PROGRAMS)




# $(TESTFIT): $(TESTFITOBJ)
# 		$(LD) $(LDFLAGS) $^ $(LIBS) $(EXTRALIBS) $(OutPutOpt)$@
# 		@echo "$@ done"

# $(TESTPERF): $(TESTPERFOBJ)
# 		$(LD) $(LDFLAGS) $^ $(LIBS) $(EXTRALIBS) -lMinuit  -lThread -lRooFit -lRooFitCore -lHtml $(OutPutOpt)$@
# 		@echo "$@ done"

$(TESTMINIM): $(TESTMINIMOBJ)
		$(LD) $(LDFLAGS) $^ $(LIBS) $(EXTRALIBS) $(OutPutOpt)$@
		@echo "$@ done"

# $(TESTROOFIT): $(TESTROOFITOBJ)
# 		$(LD) $(LDFLAGS) $^ $(LIBS) $(EXTRALIBS) -lMinuit  -lThread -lRooFit -lRooFitCore -lHtml $(OutPutOpt)$@
# 		@echo "$@ done"



clean:
		@rm -f $(OBJS) core

distclean:      clean
		@rm -f $(PROGRAMS) 


.SUFFIXES: .$(SrcSuf)


.$(SrcSuf).$(ObjSuf):
	$(CXX) $(CXXFLAGS) -c $<
