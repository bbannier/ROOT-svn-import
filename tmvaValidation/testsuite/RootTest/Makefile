# Makefile for the ROOT test programs.
# For stand-alone UnitTests
# make -f Makefile.standalone
# using the local TMVA libraries
#

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


$(STRESSTMVA):  $(STRESSTMVAO) 
	$(LD) $(LDFLAGS) $< $(LIBS) -lTMVA -lMinuit -lXMLIO -lMLP -lTreePlayer $(OutPutOpt)$@
	$(MT_EXE)
	@echo "$@ done"


clean:
	@rm -f $(PROGRAMS) $(OBJS) $(STRESSTMVAS)

.$(SrcSuf).$(ObjSuf):
	$(CXX) -g $(CXXFLAGS) -c $<

$(STRESSTMVAS): ../tmvaut/*.cxx ../tmvaut/*.h ../stressTMVA.cxx
	./buildRootTest.sh

