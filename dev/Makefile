RC := root-config
ROOTSYS = ../trunk
include $(ROOTSYS)/etc/Makefile.arch
MATHLIBS = -lRooFit -lRooFitCore -lRooStats -Llib -lRIO -lMatrix -lMinuit -lFoam -lHistFactory


#------------------------------------------------------------------------------

ifeq ($(shell $(RC) --has-roofit),yes)
TESTOBJ = test.$(ObjSuf)
TESTSRC = test.$(SrcSuf)
TEST    = test$(ExeSuf)

TEST2OBJ = test2.$(ObjSuf)
TEST2SRC = test2.$(SrcSuf)
TEST2    = test2$(ExeSuf)

STRESSROOSTATSOBJ = stressRooStats.$(ObjSuf)
STRESSROOSTATSSRC = stressRooStats.$(SrcSuf)
STRESSROOSTATS    = stressRooStats$(ExeSuf)

MODELOBJ = model.$(ObjSuf)
MODELSRC = model.$(SrcSuf)
MODEL    = model$(ExeSuf)

PVALUEOBJ = pvalue.$(ObjSuf)
PVALUESRC = pvalue.$(SrcSuf)
PVALUE    = pvalue$(ExeSuf)

TFNOBJ = tfn.$(ObjSuf)
TFNSRC = tfn.$(SrcSuf)
TFN = tfn$(ExeSuf)

COMBINEDOBJ = combined.$(ObjSuf)
COMBINEDSRC = combined.$(SrcSuf)
COMBINED = combined$(ExeSuf)

endif

#----------------------------------------------------------------------------

PROGRAMS = $(TEST) $(TEST2) $(STRESSROOSTATS) $(MODEL) $(PVALUE) $(COMBINED) $(TFN)
OBJS = $(TESTOBJ) $(TEST2OBJ) $(STRESSROOSTATSOBJ) $(MODELOBJ) $(PVALUEOBJ) $(COMBINEDOBJ) $(TFNOBJ)
TESTS = $(TEST) $(TEST2)

$(TEST):	$(TESTOBJ)
	$(LD) $(LDFLAGS) $^ $(LIBS) $(MATHLIBS) $(OutPutOpt)$@
	@echo "$@ done"

$(TEST2): $(TEST2OBJ)
	$(LD) $(LDFLAGS) $^ $(LIBS) $(MATHLIBS) $(OutPutOpt)$@
	@echo "$@ done"

$(MODEL):    $(MODELOBJ)
	$(LD) $(LDFLAGS) $^ $(LIBS) $(MATHLIBS) $(OutPutOpt)$@
	@echo "$@ done"

$(STRESSROOSTATS):	$(STRESSROOSTATSOBJ)
	$(LD) $(LDFLAGS) $^ $(LIBS) $(MATHLIBS) $(OutPutOpt)$@
	@echo "$@ done"

$(PVALUE): $(PVALUEOBJ)
	$(LD) $(LDFLAGS) $^ $(LIBS) $(MATHLIBS) $(OutPutOpt)$@
	@echo "$@ done"

$(TFN) : $(TFNOBJ)
	$(LD) $(LDFLAGS) $^ $(LIBS) $(MATHLIBS) $(OutPutOpt)$@
	@echo "$@ done"

$(COMBINED) : $(COMBINEDOBJ)
	$(LD) $(LDFLAGS) $^ $(LIBS) $(MATHLIBS) $(OutPutOpt)$@
	@echo "$@ done"

#----------------------------------------------------------------------------

all:	$(PROGRAMS)

print:
	@echo "$(OBJS)"   
  
tests:	$(TESTS)

clean:
	@rm -f $(OBJS) core TrackDict.$(SrcSuf) TrackDict.h

distclean:      clean
	@rm -f $(PROGRAMS) libTrackDict.*

.SUFFIXES: .$(SrcSuf)

.$(SrcSuf).$(ObjSuf):
	$(CXX) $(CXXFLAGS) -c $<


